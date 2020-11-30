/*
    This file is part of Oberon.

    Copyright (c) 2019-2020 Marco Melorio
    Copyright (c) 2010-2020 Vladimír Vondruš

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/* Large portions of this code is taken from:
   https://github.com/mosra/magnum-extras */

#include "SceneImporter.h"

#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Utility/FormatStl.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/LightData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/ObjectData3D.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>

#include "Oberon/LightDrawable.h"
#include "Oberon/PhongDrawable.h"
#include "Oberon/SceneData.h"

namespace Oberon { namespace SceneImporter {

namespace {

using namespace Math::Literals;

Resource<GL::AbstractShaderProgram, Shaders::Phong> phongShader(SceneData& data, Shaders::Phong::Flags flags) {
    std::string shaderKey = "phong";
    if(flags & Shaders::Phong::Flag::VertexColor)
        shaderKey += "-vertexColor";

    Resource<GL::AbstractShaderProgram, Shaders::Phong> shader =
        data.resourceManager.get<GL::AbstractShaderProgram, Shaders::Phong>(shaderKey);
    if(!shader) {
        data.resourceManager.set<GL::AbstractShaderProgram>(shader.key(),
            new Shaders::Phong{flags, data.lightCount});

        (*shader)
            .setSpecularColor(0x11111100_rgbaf)
            .setShininess(80.0f);

        arrayAppend(data.phongShadersKeys, shaderKey);
    }

    return shader;
}

void addObject(const std::string& path, SceneData& data, Containers::ArrayView<const Containers::Pointer<Trade::ObjectData3D>> objects, Containers::ArrayView<const Containers::Optional<Trade::PhongMaterialData>> materials, Containers::ArrayView<const Containers::Optional<Trade::LightData>> lights, Containers::ArrayView<const bool> hasVertexColors, Object3D& parent, UnsignedInt i) {
    /* Object failed to import, skip */
    if(!objects[i]) return;

    const Trade::ObjectData3D& objectData = *objects[i];

    /* Add the object to the scene and set its transformation. If it has a
       separate TRS, use that to avoid precision issues. */
    Object3D* object = new Object3D{&parent};
    if(objectData.flags() & Trade::ObjectFlag3D::HasTranslationRotationScaling)
        (*object).setTranslation(objectData.translation())
                 .setRotation(objectData.rotation())
                 .setScaling(objectData.scaling());
    else object->setTransformation(objectData.transformation());

    /* Save it to the ID -> pointer mapping array */
    data.objects[i].object = object;

    /* Add a drawable if the object has a mesh */
    if(objectData.instanceType() == Trade::ObjectInstanceType3D::Mesh && objectData.instance() != -1) {
        const Int materialId = static_cast<const Trade::MeshObjectData3D&>(objectData).material();

        std::string meshKey = Utility::formatString("{}#{}", path, objectData.instance());
        Resource<GL::Mesh> mesh = data.resourceManager.get<GL::Mesh>(meshKey);

        Shaders::Phong::Flags flags;
        if(hasVertexColors[objectData.instance()])
            flags |= Shaders::Phong::Flag::VertexColor;

       /* Material not available / not loaded */
        if(materialId == -1 || !materials[materialId]) {
        /* Material available */
        } else {
            const Trade::PhongMaterialData& material = *materials[materialId];

            new PhongDrawable{*object, phongShader(data, flags),
                mesh, material.diffuseColor(), data.opaqueDrawables};
        }

    /* Light */
    } else if(objectData.instanceType() == Trade::ObjectInstanceType3D::Light && objectData.instance() != -1) {
        /* Add a light drawable, which puts correct camera-relative position
           to data.lightPositions. */
        const Trade::LightData& light = *lights[objectData.instance()];
        new LightDrawable{*object, light.type() == Trade::LightData::Type::Directional ? true : false, light.color()*light.intensity(), light.range(), data.lightPositions, data.lightColors, data.lightRanges, data.lightDrawables};

    /* This is a node that holds the default camera -> assign the object to the
       global camera pointer */
    } else if(objectData.instanceType() == Trade::ObjectInstanceType3D::Camera && objectData.instance() == 0) {
        data.cameraObject = object;
    }

    /* Recursively add children */
    for(std::size_t id: objectData.children())
        addObject(path, data, objects, materials, lights, hasVertexColors, *object, id);
}

}

void load(const std::string& path, SceneData& data) {
    Containers::Pointer<Trade::AbstractImporter> importer =
        data.manager.loadAndInstantiate("TinyGltfImporter");

    if(!importer->openFile(path)) {
        Error{} << "Cannot open the file" << path;
        return;
    }

    /* Load all lights */
    Containers::Array<Containers::Optional<Trade::LightData>> lights{importer->lightCount()};
    for(UnsignedInt i = 0; i != importer->lightCount(); ++i) {
        Containers::Optional<Trade::LightData> light = importer->light(i);
        if(!light) {
            Warning{} << "Cannot load light" << i << importer->lightName(i);
            continue;
        }

        lights[i] = std::move(light);
    }

    /* Load all materials */
    Containers::Array<Containers::Optional<Trade::PhongMaterialData>> materials{importer->materialCount()};
    for(UnsignedInt i = 0; i != importer->materialCount(); ++i) {
        Containers::Optional<Trade::MaterialData> materialData = importer->material(i);
        if(!materialData || !(materialData->types() & Trade::MaterialType::Phong)) {
            Warning{} << "Cannot load material" << i << importer->materialName(i);
            continue;
        }

        materials[i] = std::move(*materialData).as<Trade::PhongMaterialData>();
    }

    /* Load all meshes. Remember which have vertex colors. */
    Containers::Array<bool> hasVertexColors{Containers::DirectInit, importer->meshCount(), false};
    for(UnsignedInt i = 0; i != importer->meshCount(); ++i) {
        Containers::Optional<Trade::MeshData> meshData = importer->mesh(i);
        if(!meshData) {
            Warning{} << "Cannot load mesh" << i << importer->meshName(i);
            continue;
        }

        hasVertexColors[i] = meshData->hasAttribute(Trade::MeshAttribute::Color);

        /* Compile and save the mesh */
        std::string meshKey = Utility::formatString("{}#{}", path, i);
        data.resourceManager.set<GL::Mesh>(meshKey, MeshTools::compile(*meshData));
    }

    /* Load the scene */
    if(importer->defaultScene() != -1) {
        Containers::Optional<Trade::SceneData> sceneData = importer->scene(importer->defaultScene());
        if(!sceneData) {
            Error{} << "Cannot load the scene, aborting";
            return;
        }

        /* Import all objects and first count how many lights is there first so
           we know which shaders to instantiate */
        data.objects = Containers::Array<ObjectInfo>{Containers::ValueInit, importer->object3DCount()};
        Containers::Array<Containers::Pointer<Trade::ObjectData3D>> objects{importer->object3DCount()};
        for(UnsignedInt i = 0; i != importer->object3DCount(); ++i) {
            objects[i] = importer->object3D(i);
            if(!objects[i]) {
                Error{} << "Cannot import object" << i << importer->object3DName(i);
                continue;
            }

            data.objects[i].name = importer->object3DName(i);
            if(data.objects[i].name.empty())
                data.objects[i].name = Utility::formatString("object #{}", i);

            if(objects[i]->instanceType() == Trade::ObjectInstanceType3D::Light)
                ++data.lightCount;

            data.objects[i].childCount = objects[i]->children().size();
        }

        /* Recursively add all children */
        for(UnsignedInt objectId: sceneData->children3D())
            addObject(path, data, objects, materials, lights, hasVertexColors, data.scene, objectId);

    /* The format has no scene support, display just the first loaded mesh with
       a default material and be done with it */
    } else if(Resource<GL::Mesh> mesh = data.resourceManager.get<GL::Mesh>(Utility::formatString("{}#0", path))) {
        data.objects = Containers::Array<ObjectInfo>{Containers::ValueInit, 1};
        data.objects[0].object = &data.scene;
        data.objects[0].name = "object #0";
        new PhongDrawable{data.scene, phongShader(data, hasVertexColors[0] ? Shaders::Phong::Flag::VertexColor : Shaders::Phong::Flags{}), mesh, 0xffffff_rgbf, data.opaqueDrawables};
    }

    /* Create a camera object in case it wasn't present in the scene already */
    if(!data.cameraObject) {
        data.cameraObject = new Object3D{&data.scene};
        data.cameraObject->translate(Vector3::zAxis(5.0f));
    }

    /* Basic camera setup */
    (*(data.camera = new SceneGraph::Camera3D{*data.cameraObject}))
        .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(75.0_degf, 1.0f, 0.01f, 1000.0f));
}

}}

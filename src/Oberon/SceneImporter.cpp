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
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/LightData.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/MeshObjectData3D.h>
#include <Magnum/Trade/ObjectData3D.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/Trade/SceneData.h>
#include <Magnum/Trade/TextureData.h>

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

void loadImage(GL::Texture2D& texture, Trade::ImageData2D& image) {
    if(!image.isCompressed()) {
        /* Whitelist only things we *can* display */
        GL::TextureFormat format;
        switch(image.format()) {
            case PixelFormat::R8Unorm:
            case PixelFormat::RG8Unorm:
            case PixelFormat::RGB8Unorm:
            case PixelFormat::RGB8Srgb:
            case PixelFormat::RGBA8Unorm:
            case PixelFormat::RGBA8Srgb:
                format = GL::textureFormat(image.format());
                break;
            default:
                Warning{} << "Cannot load an image of format" << image.format();
                return;
        }

        texture
            .setStorage(Math::log2(image.size().max()) + 1, format, image.size())
            .setSubImage(0, {}, image)
            .generateMipmap();

    } else {
        /* Blacklist things we *cannot* display */
        GL::TextureFormat format;
        switch(image.compressedFormat()) {
            case CompressedPixelFormat::Bc4RSnorm:
            case CompressedPixelFormat::Bc5RGSnorm:
            case CompressedPixelFormat::EacR11Snorm:
            case CompressedPixelFormat::EacRG11Snorm:
            case CompressedPixelFormat::Bc6hRGBUfloat:
            case CompressedPixelFormat::Bc6hRGBSfloat:
            case CompressedPixelFormat::Astc4x4RGBAF:
            case CompressedPixelFormat::Astc5x4RGBAF:
            case CompressedPixelFormat::Astc5x5RGBAF:
            case CompressedPixelFormat::Astc6x5RGBAF:
            case CompressedPixelFormat::Astc6x6RGBAF:
            case CompressedPixelFormat::Astc8x5RGBAF:
            case CompressedPixelFormat::Astc8x6RGBAF:
            case CompressedPixelFormat::Astc8x8RGBAF:
            case CompressedPixelFormat::Astc10x5RGBAF:
            case CompressedPixelFormat::Astc10x6RGBAF:
            case CompressedPixelFormat::Astc10x8RGBAF:
            case CompressedPixelFormat::Astc10x10RGBAF:
            case CompressedPixelFormat::Astc12x10RGBAF:
            case CompressedPixelFormat::Astc12x12RGBAF:
                Warning{} << "Cannot load an image of format" << image.compressedFormat();
                return;

            default: format = GL::textureFormat(image.compressedFormat());
        }

        texture
            .setStorage(1, format, image.size())
            .setCompressedSubImage(0, {}, image);
    }
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

            /* Textured material */
            Resource<GL::Texture2D> diffuseTexture;
            Resource<GL::Texture2D> normalTexture;
            Float normalTextureScale = 1.0f;
            if(material.hasAttribute(Trade::MaterialAttribute::DiffuseTexture)) {
                std::string textureKey = Utility::formatString("{}#{}", path, material.diffuseTexture());
                diffuseTexture = data.resourceManager.get<GL::Texture2D>(textureKey);
                if(diffuseTexture) {
                    flags |= Shaders::Phong::Flag::AmbientTexture|
                        Shaders::Phong::Flag::DiffuseTexture;
                }
            }

            /* Normal textured material */
            if(material.hasAttribute(Trade::MaterialAttribute::NormalTexture)) {
                std::string textureKey = Utility::formatString("{}#{}", path, material.normalTexture());
                normalTexture = data.resourceManager.get<GL::Texture2D>(textureKey);
                if(normalTexture) {
                    normalTextureScale = material.normalTextureScale();
                    flags |= Shaders::Phong::Flag::NormalTexture;
                }
            }

            new PhongDrawable{*object, phongShader(data, flags), mesh,
                material.diffuseColor(), diffuseTexture, normalTexture,
                normalTextureScale, data.opaqueDrawables};
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

    /* Load all textures */
    for(UnsignedInt i = 0; i != importer->textureCount(); ++i) {
        Containers::Optional<Trade::TextureData> textureData = importer->texture(i);
        if(!textureData || textureData->type() != Trade::TextureData::Type::Texture2D) {
            Warning{} << "Cannot load texture" << i << importer->textureName(i);
            continue;
        }

        Containers::Optional<Trade::ImageData2D> imageData = importer->image2D(textureData->image());
        if(!imageData) {
            Warning{} << "Cannot load texture" << i << importer->image2DName(textureData->image());
            continue;
        }

        /* Configure the texture */
        GL::Texture2D texture;
        texture
            .setMagnificationFilter(textureData->magnificationFilter())
            .setMinificationFilter(textureData->minificationFilter(), textureData->mipmapFilter())
            .setWrapping(textureData->wrapping().xy());

        loadImage(texture, *imageData);

        /* Save the texture */
        std::string textureKey = Utility::formatString("{}#{}", path, i);
        data.resourceManager.set<GL::Texture2D>(textureKey, std::move(texture));
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
           we know which shaders to instantiate. Also initialize the ObjectInfo array
           with the object count + 1 for the scene. */
        data.objects = Containers::Array<ObjectInfo>{Containers::ValueInit, importer->object3DCount() + 1};
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

            data.objects[i].children = objects[i]->children();
        }

        /* Set scene info */
        data.sceneObjectId = data.objects.size() - 1;
        data.objects[data.sceneObjectId].children = sceneData->children3D();

        /* Recursively add all children */
        for(UnsignedInt objectId: sceneData->children3D())
            addObject(path, data, objects, materials, lights, hasVertexColors, data.scene, objectId);

    /* The format has no scene support, display just the first loaded mesh with
       a default material and be done with it */
    } else if(Resource<GL::Mesh> mesh = data.resourceManager.get<GL::Mesh>(Utility::formatString("{}#0", path))) {
        /* Create a scene child and add the mesh */
        Object3D* object = new Object3D{&data.scene};
        data.objects = Containers::Array<ObjectInfo>{Containers::ValueInit, 2};
        data.objects[0].object = object;
        data.objects[0].name = "object #0";
        new PhongDrawable{*object, phongShader(data, hasVertexColors[0] ? Shaders::Phong::Flag::VertexColor : Shaders::Phong::Flags{}), mesh, 0xffffff_rgbf, data.opaqueDrawables};

        /* Set scene info */
        data.sceneObjectId = 1;
        data.objects[data.sceneObjectId].children = {0};
    }

    /* Complete scene info */
    data.objects[data.sceneObjectId].object = &data.scene;
    data.objects[data.sceneObjectId].name = "scene";

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

/*
    This file is part of Oberon.

    Copyright (c) 2019-2020 Marco Melorio

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

#include "Serializer.h"

#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData2D.h>
#include <Magnum/Trade/MeshData3D.h>

namespace Serializer {

Object3D* createObjectFromConfig(Utility::ConfigurationGroup* objectConfig, Object3D* parent, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, UnsignedByte objectId) {
    Object3D* object = new Object3D{parent};

    /* Transformation */
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);

    for(auto featureConfig: objectConfig->groups("feature"))
        addFeatureFromConfig(featureConfig, object, resourceManager, drawables, scripts, objectId);

    return object;
}

void addFeatureFromConfig(Utility::ConfigurationGroup* featureConfig, Object3D* object, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, UnsignedByte objectId) {
    std::string type = featureConfig->value("type");

    if(type == "mesh") {
        /* Shader */
        Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Shaders::Flat3D>("flat3d");
        CORRADE_INTERNAL_ASSERT(shaderResource);

        /* Mesh */
        Mesh& mesh = object->addFeature<Mesh>(drawables, shaderResource);

        /* Primitive */
        if(featureConfig->hasGroup("primitive")) {
            Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
            setMeshFromConfig(mesh, primitiveConfig, resourceManager);
        }

        if(objectId > 0)
            mesh.setObjectId(objectId);
    } else if(type == "script") {
        /* Script path */
        std::string scriptPath = featureConfig->value("script_path");

        /* Script */
        object->addFeature<Script>(scripts, scriptPath);
    }
}

void resetObjectFromConfig(Object3D* object, Utility::ConfigurationGroup* objectConfig) {
    /* Transformation */
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);

    for(auto featureConfig: objectConfig->groups("feature")) {
        std::string type = featureConfig->value("type");

        if(type == "mesh") {
            /* Mesh */
            auto& features = object->features();
            Mesh* mesh = nullptr;

            for(auto& feature: features) {
                if((mesh = dynamic_cast<Mesh*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(mesh != nullptr);

            /* Size */
            if(!featureConfig->hasValue("size")) featureConfig->setValue<Vector3>("size", {100, 100, 100});
            Vector3 size = featureConfig->value<Vector3>("size");
            mesh->setSize(size);
        }
    }
}

void setMeshFromConfig(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig, OberonResourceManager& resourceManager) {
    std::string primitiveType = primitiveConfig->value("type");
    std::string meshKey = primitiveType;

    if(primitiveConfig->hasValue("rings"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("rings"));

    if(primitiveConfig->hasValue("segments"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("segments"));

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);

    if(!primitiveConfig->hasValue("size")) primitiveConfig->setValue<Vector3>("size", {100, 100, 100});
    Vector3 size = primitiveConfig->value<Vector3>("size");

    if(!meshResource) {
        if(primitiveType == "sphere" && !primitiveConfig->hasValue("rings"))
            primitiveConfig->setValue<UnsignedInt>("rings", 10);

        if((primitiveType == "circle" || primitiveType == "sphere") &&
            !primitiveConfig->hasValue("segments")) {
            primitiveConfig->setValue<UnsignedInt>("segments", 10);
        }

        UnsignedInt segments = primitiveConfig->value<UnsignedInt>("segments");
        UnsignedInt rings = primitiveConfig->value<UnsignedInt>("rings");
        GL::Mesh glMesh{NoCreate};

        if(primitiveType == "circle") glMesh = MeshTools::compile(Primitives::circle3DSolid(segments));
        if(primitiveType == "cube") glMesh = MeshTools::compile(Primitives::cubeSolid());
        if(primitiveType == "plane") glMesh = MeshTools::compile(Primitives::planeSolid());
        if(primitiveType == "sphere") glMesh = MeshTools::compile(Primitives::uvSphereSolid(rings, segments));
        if(primitiveType == "square") glMesh = MeshTools::compile(Primitives::squareSolid());

        resourceManager.set(meshResource.key(), std::move(glMesh), ResourceDataState::Final, ResourcePolicy::ReferenceCounted);
    }

    CORRADE_INTERNAL_ASSERT(meshResource);

    mesh.setMesh(meshResource);
    mesh.setSize(size);
}

}

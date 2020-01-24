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

Object3D* createObjectFromConfig(Utility::ConfigurationGroup* objectConfig, Object3D* parent, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights, UnsignedByte objectId) {
    Object3D* object = new Object3D{parent};

    /* Transformation */
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);

    for(auto featureConfig: objectConfig->groups("feature"))
        addFeatureFromConfig(featureConfig, object, resourceManager, drawables, scripts, lights, objectId);

    return object;
}

void addFeatureFromConfig(Utility::ConfigurationGroup* featureConfig, Object3D* object, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights, UnsignedByte objectId) {
    std::string type = featureConfig->value("type");

    if(type == "mesh") {
        /* Shader */
        Resource<GL::AbstractShaderProgram, Oberon::Shader> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Oberon::Shader>("shader");
        if(!shaderResource)
            resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Oberon::Shader{0}, ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);

        /* Mesh */
        Mesh& mesh = object->addFeature<Mesh>(drawables, shaderResource);

        /* Primitive */
        if(featureConfig->hasGroup("primitive")) {
            Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
            setMeshFromConfig(mesh, primitiveConfig, resourceManager);
        }

        /* Material */
        if(featureConfig->hasGroup("material")) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            Color3 ambient = materialConfig->value<Color3>("ambient");
            mesh.setAmbientColor(ambient);

            Color3 diffuse = materialConfig->value<Color3>("diffuse");
            mesh.setDiffuseColor(diffuse);

            Color3 specular = materialConfig->value<Color3>("specular");
            mesh.setSpecularColor(specular);

            Float shininess = materialConfig->value<Float>("shininess");
            mesh.setShininess(shininess);
        }

        if(objectId > 0) mesh.setObjectId(objectId);
    } else if(type == "light") {
        /* Shader */
        Resource<GL::AbstractShaderProgram, Oberon::Shader> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Oberon::Shader>("shader");
        resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Oberon::Shader{UnsignedInt(lights->size() + 1)}, ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);

        /* Color */
        Color3 color = featureConfig->value<Color3>("color");

        /* Light */
        object->addFeature<Light>(lights, shaderResource, color);
    } else if(type == "script") {
        /* Path */
        std::string path = featureConfig->value("path");

        /* Script */
        object->addFeature<Script>(scripts, path);
    }
}

void resetObjectFromConfig(Object3D* object, Utility::ConfigurationGroup* objectConfig) {
    /* Transformation */
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);
}

void setMeshFromConfig(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig, OberonResourceManager& resourceManager) {
    std::string primitiveType = primitiveConfig->value("type");
    std::string meshKey = primitiveType;

    if(primitiveConfig->hasValue("rings"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("rings"));

    if(primitiveConfig->hasValue("segments"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("segments"));

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);

    if(!primitiveConfig->hasValue("size")) primitiveConfig->setValue<Vector3>("size", {2, 2, 2});
    Vector3 size = primitiveConfig->value<Vector3>("size");

    if(!meshResource) {
        if(primitiveType == "sphere" && !primitiveConfig->hasValue("rings"))
            primitiveConfig->setValue<UnsignedInt>("rings", 20);

        if((primitiveType == "circle" || primitiveType == "sphere") &&
            !primitiveConfig->hasValue("segments")) {
            primitiveConfig->setValue<UnsignedInt>("segments", 20);
        }

        UnsignedInt segments = primitiveConfig->value<UnsignedInt>("segments");
        UnsignedInt rings = primitiveConfig->value<UnsignedInt>("rings");
        GL::Mesh glMesh{NoCreate};

        if(primitiveType == "circle")       glMesh = MeshTools::compile(Primitives::circle3DSolid(segments));
        else if(primitiveType == "cube")    glMesh = MeshTools::compile(Primitives::cubeSolid());
        else if(primitiveType == "plane")   glMesh = MeshTools::compile(Primitives::planeSolid());
        else if(primitiveType == "sphere")  glMesh = MeshTools::compile(Primitives::uvSphereSolid(rings, segments));
        else if(primitiveType == "square")  glMesh = MeshTools::compile(Primitives::squareSolid());

        resourceManager.set(meshResource.key(), std::move(glMesh), ResourceDataState::Final, ResourcePolicy::ReferenceCounted);
    }

    CORRADE_INTERNAL_ASSERT(meshResource);

    mesh.setMesh(meshResource);
    mesh.setSize(size);
}

}

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

#include "Importer.h"

#include <Corrade/Containers/Optional.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/Directory.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/ImageView.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>

#include "Light.h"
#include "Mesh.h"

Object3D* Importer::loadObject(Utility::ConfigurationGroup* objectConfig, Object3D* parent, SceneGraph::DrawableGroup3D* drawables, LightGroup* lights) {
    Object3D* object = new Object3D{parent};

    /* Transformation */
    if(objectConfig->hasValue("transformation")) {
        Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
        object->setTransformation(transformation);
    }

    for(auto& featureConfig: objectConfig->groups("feature"))
        loadFeature(featureConfig, object, drawables, lights);

    return object;
}

Object3D* Importer::loadChildrenObject(Utility::ConfigurationGroup* parentConfig, Object3D* parent, SceneGraph::DrawableGroup3D* drawables, LightGroup* lights) {
    for(auto& childConfig: parentConfig->groups("child")) {
        Object3D* child = loadObject(childConfig, parent, drawables, lights);

        if(childConfig->hasGroup("child"))
            loadChildrenObject(childConfig, child, drawables, lights);
    }

    return parent;
}

SceneGraph::AbstractFeature3D* Importer::loadFeature(Utility::ConfigurationGroup* featureConfig, Object3D* object, SceneGraph::DrawableGroup3D* drawables, LightGroup* lights) {
    std::string type = featureConfig->value("type");
    SceneGraph::AbstractFeature3D* newFeature;

    if(type == "mesh") {
        /* Mesh */
        Mesh& mesh = object->addFeature<Mesh>(drawables);
        newFeature = &mesh;

        /* Material */
        if(featureConfig->hasGroup("material")) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            if(materialConfig->hasValue("ambient_color")) {
                Color4 ambientColor = materialConfig->value<Color4>("ambient_color");
                mesh.setAmbientColor(ambientColor);
            }
            if(materialConfig->hasValue("ambient_texture")) {
                std::string ambientTexturePath = materialConfig->value("ambient_texture");
                Resource<GL::Texture2D> ambientTexture = _resourceManager.get<GL::Texture2D>(ambientTexturePath);
                mesh.setAmbientTexture(ambientTexture);
            }
            if(materialConfig->hasValue("diffuse_color")) {
                Color4 diffuseColor = materialConfig->value<Color4>("diffuse_color");
                mesh.setDiffuseColor(diffuseColor);
            }
            if(materialConfig->hasValue("diffuse_texture")) {
                std::string diffuseTexturePath = materialConfig->value("diffuse_texture");
                Resource<GL::Texture2D> diffuseTexture = _resourceManager.get<GL::Texture2D>(diffuseTexturePath);
                mesh.setDiffuseTexture(diffuseTexture);
            }
            if(materialConfig->hasValue("normal_texture")) {
                std::string normalTexturePath = materialConfig->value("normal_texture");
                Resource<GL::Texture2D> normalTexture = _resourceManager.get<GL::Texture2D>(normalTexturePath);
                mesh.setNormalTexture(normalTexture);
            }
            if(materialConfig->hasValue("specular_color")) {
                Color4 specularColor = materialConfig->value<Color4>("specular_color");
                mesh.setSpecularColor(specularColor);
            }
            if(materialConfig->hasValue("specular_texture")) {
                std::string specularTexturePath = materialConfig->value("specular_texture");
                Resource<GL::Texture2D> specularTexture = _resourceManager.get<GL::Texture2D>(specularTexturePath);
                mesh.setSpecularTexture(specularTexture);
            }
            if(materialConfig->hasValue("shininess")) {
                Float shininess = materialConfig->value<Float>("shininess");
                mesh.setShininess(shininess);
            }
        }

        /* Primitive */
        if(featureConfig->hasGroup("primitive")) {
            Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
            updateMeshPrimitive(mesh, primitiveConfig);
        }
    } else if(type == "light") {
        /* Light */
        Light& light = object->addFeature<Light>(lights, _resourceManager);
        newFeature = &light;

        if(featureConfig->hasValue("color")) {
            Color4 color = featureConfig->value<Color4>("color");
            light.setColor(color);
        }
        if(featureConfig->hasValue("constant")) {
            Float constant = featureConfig->value<Float>("constant");
            light.setConstant(constant);
        }
        if(featureConfig->hasValue("linear")) {
            Float linear = featureConfig->value<Float>("linear");
            light.setLinear(linear);
        }
        if(featureConfig->hasValue("quadratic")) {
            Float quadratic = featureConfig->value<Float>("quadratic");
            light.setQuadratic(quadratic);
        }
    }

    return newFeature;
}

void Importer::resetObject(Object3D* object, Utility::ConfigurationGroup* objectConfig) {
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);
}

void Importer::updateMeshPrimitive(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig) {
    std::string primitiveType = primitiveConfig->value("type");
    std::string meshKey = primitiveType;

    if(primitiveConfig->hasValue("rings"))
        meshKey.append("-rings=" + primitiveConfig->value("rings"));
    if(primitiveConfig->hasValue("segments"))
        meshKey.append("-segments=" + primitiveConfig->value("segments"));

    bool needsTextureCoords = mesh.hasAmbientTexture() || mesh.hasDiffuseTexture() ||
        mesh.hasNormalTexture() || mesh.hasSpecularTexture();
    bool needsTangents = mesh.hasNormalTexture();
    if(needsTextureCoords) meshKey.append("-textureCoordinates");
    if(needsTangents) meshKey.append("-tangents");

    Resource<GL::Mesh> meshResource = _resourceManager.get<GL::Mesh>(meshKey);
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

        if(primitiveType == "circle") {
            Primitives::Circle3DFlags flags;
            if(needsTextureCoords) flags |= Primitives::Circle3DFlag::TextureCoordinates;
            if(needsTangents) flags |= Primitives::Circle3DFlag::Tangents;
            glMesh = MeshTools::compile(Primitives::circle3DSolid(segments, flags));
        } else if(primitiveType == "cube") {
            glMesh = MeshTools::compile(Primitives::cubeSolid());
        } else if(primitiveType == "plane") {
            Primitives::PlaneFlags flags;
            if(needsTextureCoords) flags |= Primitives::PlaneFlag::TextureCoordinates;
            if(needsTangents) flags |= Primitives::PlaneFlag::Tangents;
            glMesh = MeshTools::compile(Primitives::planeSolid(flags));
        } else if(primitiveType == "sphere") {
            Primitives::UVSphereFlags flags;
            if(needsTextureCoords) flags |= Primitives::UVSphereFlag::TextureCoordinates;
            if(needsTangents) flags |= Primitives::UVSphereFlag::Tangents;
            glMesh = MeshTools::compile(Primitives::uvSphereSolid(rings, segments, flags));
        }

        _resourceManager.set(meshResource.key(), std::move(glMesh));
    }

    mesh.setMesh(meshResource);

    if(primitiveConfig->hasValue("size")) {
        Vector3 size = primitiveConfig->value<Vector3>("size");
        mesh.setSize(size);
    }
}

Resource<GL::AbstractShaderProgram, SceneShader> Importer::createShader(Mesh& mesh, UnsignedInt lightCount, std::vector<std::pair<std::string, SceneShader::Flags>>& shaderKeys, bool useObjectId) {
    std::pair<std::string, SceneShader::Flags> shaderKey = calculateShaderKey(mesh, useObjectId);
    Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, SceneShader>(shaderKey.first);

    if(!shaderResource) {
        _resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new SceneShader{shaderKey.second, lightCount},
            ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);
        shaderKeys.push_back(shaderKey);
    }

    return shaderResource;
}

void Importer::createShaders(SceneGraph::DrawableGroup3D* drawables, UnsignedInt lightCount, std::vector<std::pair<std::string, SceneShader::Flags>>& shaderKeys, bool useObjectId) {
    for(std::size_t i = 0; i != drawables->size(); ++i) {
        Mesh* mesh = dynamic_cast<Mesh*>(&(*drawables)[i]);
        if(mesh) {
            Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = createShader(*mesh, lightCount, shaderKeys, useObjectId);
            mesh->setShader(shaderResource);
        }
    }
}

Resource<GL::Texture2D> Importer::loadTexture(const std::string& resourcePath, Utility::Resource& resources) {
    return loadTexture(resourcePath, resources.getRaw(resourcePath));
}

Resource<GL::Texture2D> Importer::loadTexture(const std::string& resourcePath, const std::string& rootPath) {
    return loadTexture(resourcePath, Utility::Directory::mapRead(
        Utility::Directory::join(rootPath, resourcePath)));
}

Resource<GL::Texture2D> Importer::loadTexture(const std::string& resourcePath, Containers::ArrayView<const char> data) {
    Resource<GL::Texture2D> textureResource = _resourceManager.get<GL::Texture2D>(resourcePath);
    if(textureResource)
        return textureResource;

    Containers::Pointer<Trade::AbstractImporter> importer =
        _importerManager.loadAndInstantiate("AnyImageImporter");

    importer->openData(data);
    Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
    CORRADE_INTERNAL_ASSERT(image);

    GL::Texture2D texture;
    texture.setWrapping(GL::SamplerWrapping::ClampToEdge)
        .setMagnificationFilter(GL::SamplerFilter::Linear)
        .setMinificationFilter(GL::SamplerFilter::Linear)
        .setStorage(1, GL::textureFormat(image->format()), image->size())
        .setSubImage(0, {}, *image);

    _resourceManager.set(textureResource.key(), std::move(texture));
    return textureResource;
}

std::pair<std::string, SceneShader::Flags> Importer::calculateShaderKey(Mesh& mesh, bool useObjectId) {
    std::string shaderKey = "scene";
    SceneShader::Flags flags;

    if(mesh.hasAmbientTexture()) {
        shaderKey.append("-ambientTexture");
        flags |= SceneShader::Flag::AmbientTexture;
    }
    if(mesh.hasDiffuseTexture()) {
        shaderKey.append("-diffuseTexture");
        flags |= SceneShader::Flag::DiffuseTexture;
    }
    if(mesh.hasSpecularTexture()) {
        shaderKey.append("-specularTexture");
        flags |= SceneShader::Flag::SpecularTexture;
    }
    if(mesh.hasNormalTexture()) {
        shaderKey.append("-normalTexture");
        flags |= SceneShader::Flag::NormalTexture;
    }
    if(useObjectId) {
        shaderKey.append("-objectId");
        flags |= SceneShader::Flag::ObjectId;
    }

    return std::make_pair(shaderKey, flags);
}

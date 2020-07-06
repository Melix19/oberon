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
#include <Magnum/ImageView.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/Trade/ImageData.h>

#include "GameData.h"
#include "MeshPrimitives.h"
#include "MeshRenderer.h"

namespace Oberon {

Object3D* Importer::loadObject(Utility::ConfigurationGroup* objectConfig, Object3D* parent, GameData& gameData) {
    Object3D* object = new Object3D{parent};

    /* Transformation */
    if(objectConfig->hasValue("transformation")) {
        Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
        object->setTransformation(transformation);
    }

    for(auto& featureConfig: objectConfig->groups("feature"))
        loadFeature(featureConfig, object, gameData);

    return object;
}

Object3D* Importer::loadChildrenObject(Utility::ConfigurationGroup* parentConfig, Object3D* parent, GameData& gameData) {
    for(auto& childConfig: parentConfig->groups("child")) {
        Object3D* child = loadObject(childConfig, parent, gameData);

        if(childConfig->hasGroup("child"))
            loadChildrenObject(childConfig, child, gameData);
    }

    return parent;
}

SceneGraph::AbstractFeature3D* Importer::loadFeature(Utility::ConfigurationGroup* featureConfig, Object3D* object, GameData& gameData) {
    std::string type = featureConfig->value("type");
    SceneGraph::AbstractFeature3D* newFeature;

    if(type == "mesh_renderer") {
        /* Mesh renderer */
        MeshRenderer& meshRenderer = object->addFeature<MeshRenderer>(gameData.drawables());
        newFeature = &meshRenderer;

        /* Mesh */
        if(featureConfig->hasGroup("mesh")) {
            Utility::ConfigurationGroup* meshConfiguration = featureConfig->group("mesh");
            generateMeshPrimitive(meshRenderer, meshConfiguration);
        }

        /* Material */
        if(featureConfig->hasGroup("material")) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            if(materialConfig->hasValue("ambient_color")) {
                Color4 ambientColor = materialConfig->value<Color4>("ambient_color");
                meshRenderer.setAmbientColor(ambientColor);
            }
            if(materialConfig->hasValue("ambient_texture")) {
                std::string ambientTexturePath = materialConfig->value("ambient_texture");
                Resource<GL::Texture2D> ambientTexture = _resourceManager.get<GL::Texture2D>(ambientTexturePath);
                meshRenderer.setAmbientTexture(ambientTexture);
            }
            if(materialConfig->hasValue("diffuse_color")) {
                Color4 diffuseColor = materialConfig->value<Color4>("diffuse_color");
                meshRenderer.setDiffuseColor(diffuseColor);
            }
            if(materialConfig->hasValue("diffuse_texture")) {
                std::string diffuseTexturePath = materialConfig->value("diffuse_texture");
                Resource<GL::Texture2D> diffuseTexture = _resourceManager.get<GL::Texture2D>(diffuseTexturePath);
                meshRenderer.setDiffuseTexture(diffuseTexture);
            }
            if(materialConfig->hasValue("normal_texture")) {
                std::string normalTexturePath = materialConfig->value("normal_texture");
                Resource<GL::Texture2D> normalTexture = _resourceManager.get<GL::Texture2D>(normalTexturePath);
                meshRenderer.setNormalTexture(normalTexture);
            }
            if(materialConfig->hasValue("specular_color")) {
                Color4 specularColor = materialConfig->value<Color4>("specular_color");
                meshRenderer.setSpecularColor(specularColor);
            }
            if(materialConfig->hasValue("specular_texture")) {
                std::string specularTexturePath = materialConfig->value("specular_texture");
                Resource<GL::Texture2D> specularTexture = _resourceManager.get<GL::Texture2D>(specularTexturePath);
                meshRenderer.setSpecularTexture(specularTexture);
            }
            if(materialConfig->hasValue("shininess")) {
                Float shininess = materialConfig->value<Float>("shininess");
                meshRenderer.setShininess(shininess);
            }
        }
    } else if(type == "light") {
        /* Light */
        Light& light = object->addFeature<Light>(gameData.lights(), _resourceManager);
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

void Importer::generateMeshPrimitive(MeshRenderer& meshRenderer, Utility::ConfigurationGroup* meshConfiguration) {
    const std::string primitiveType = meshConfiguration->value("type");
    Resource<GL::Mesh> meshResource;

    if(primitiveType == "capsule") {
        const Float radius = meshConfiguration->value<Float>("radius");
        const Float length = meshConfiguration->value<Float>("length");
        const UnsignedInt hemisphereRings = meshConfiguration->value<UnsignedInt>("hemisphereRings");
        const UnsignedInt cylinderRings = meshConfiguration->value<UnsignedInt>("cylinderRings");
        const UnsignedInt segments = meshConfiguration->value<UnsignedInt>("segments");
        meshResource = MeshPrimitives::capsule(radius, length, hemisphereRings, cylinderRings, segments, _resourceManager);
    } else if(primitiveType == "circle") {
        const Float radius = meshConfiguration->value<Float>("radius");
        const UnsignedInt segments = meshConfiguration->value<UnsignedInt>("segments");
        meshResource = MeshPrimitives::circle(radius, segments, _resourceManager);
    } else if(primitiveType == "cone") {
        const Float radius = meshConfiguration->value<Float>("radius");
        const Float length = meshConfiguration->value<Float>("length");
        const UnsignedInt rings = meshConfiguration->value<UnsignedInt>("rings");
        const UnsignedInt segments = meshConfiguration->value<UnsignedInt>("segments");
        const bool capEnd = meshConfiguration->value<bool>("capEnd");
        meshResource = MeshPrimitives::cone(radius, length, rings, segments, capEnd, _resourceManager);
    } else if(primitiveType == "cylinder") {
        const Float radius = meshConfiguration->value<Float>("radius");
        const Float length = meshConfiguration->value<Float>("length");
        const UnsignedInt rings = meshConfiguration->value<UnsignedInt>("rings");
        const UnsignedInt segments = meshConfiguration->value<UnsignedInt>("segments");
        const bool capEnds = meshConfiguration->value<bool>("capEnds");
        meshResource = MeshPrimitives::cylinder(radius, length, rings, segments, capEnds, _resourceManager);
    } else if(primitiveType == "plane") {
        const Vector2 size = meshConfiguration->value<Vector2>("size");
        meshResource = MeshPrimitives::plane(size, _resourceManager);
    } else if(primitiveType == "sphere") {
        const Float radius = meshConfiguration->value<Float>("radius");
        const UnsignedInt rings = meshConfiguration->value<UnsignedInt>("rings");
        const UnsignedInt segments = meshConfiguration->value<UnsignedInt>("segments");
        meshResource = MeshPrimitives::sphere(radius, rings, segments, _resourceManager);
    }

    meshRenderer.setMesh(meshResource);
}

Resource<GL::AbstractShaderProgram, SceneShader> Importer::createShader(MeshRenderer& meshRenderer, GameData& gameData, bool useObjectId) {
    std::pair<std::string, SceneShader::Flags> shaderKey = calculateShaderKey(meshRenderer, useObjectId);
    Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, SceneShader>(shaderKey.first);

    if(!shaderResource) {
        _resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new SceneShader{shaderKey.second, UnsignedInt(gameData.lights().size())},
            ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);
        gameData.shaderKeys().push_back(shaderKey);
    }

    return shaderResource;
}

void Importer::createShaders(GameData& gameData, bool useObjectId) {
    for(std::size_t i = 0; i != gameData.drawables().size(); ++i) {
        MeshRenderer* meshRenderer = dynamic_cast<MeshRenderer*>(&gameData.drawables()[i]);
        if(meshRenderer) {
            Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = createShader(*meshRenderer, gameData, useObjectId);
            meshRenderer->setShader(shaderResource);
        }
    }
}

Resource<GL::Texture2D> Importer::loadTexture(const std::string& resourcePath, Containers::ArrayView<const char> data) {
    Resource<GL::Texture2D> textureResource = _resourceManager.get<GL::Texture2D>(resourcePath);
    if(textureResource)
        return textureResource;

    Containers::Pointer<Trade::AbstractImporter> importer =
        _importerManager.loadAndInstantiate("StbImageImporter");

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

std::pair<std::string, SceneShader::Flags> Importer::calculateShaderKey(MeshRenderer& meshRenderer, bool useObjectId) {
    std::string shaderKey = "scene";
    SceneShader::Flags flags;

    if(meshRenderer.hasAmbientTexture()) {
        shaderKey.append("-ambientTexture");
        flags |= SceneShader::Flag::AmbientTexture;
    }
    if(meshRenderer.hasDiffuseTexture()) {
        shaderKey.append("-diffuseTexture");
        flags |= SceneShader::Flag::DiffuseTexture;
    }
    if(meshRenderer.hasSpecularTexture()) {
        shaderKey.append("-specularTexture");
        flags |= SceneShader::Flag::SpecularTexture;
    }
    if(meshRenderer.hasNormalTexture()) {
        shaderKey.append("-normalTexture");
        flags |= SceneShader::Flag::NormalTexture;
    }
    if(useObjectId) {
        shaderKey.append("-objectId");
        flags |= SceneShader::Flag::ObjectId;
    }

    return std::make_pair(shaderKey, flags);
}

}

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
#include <Magnum/ImageView.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>

#include "Light.h"
#include "Mesh.h"
#include "Script.h"
#include "Sprite.h"

Object3D* Importer::loadObject(Utility::ConfigurationGroup* objectConfig, Object3D* parent, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights) {
    Object3D* object = new Object3D{parent};

    /* Transformation */
    if(objectConfig->hasValue("transformation")) {
        Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
        object->setTransformation(transformation);
    }

    for(auto& featureConfig: objectConfig->groups("feature"))
        loadFeature(featureConfig, object, resourceManager, drawables, scripts, lights);

    return object;
}

Object3D* Importer::loadChildrenObject(Utility::ConfigurationGroup* parentConfig, Object3D* parent, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights) {
    for(auto& childConfig: parentConfig->groups("child")) {
        Object3D* child = loadObject(childConfig, parent, resourceManager, drawables, scripts, lights);

        if(childConfig->hasGroup("child"))
            loadChildrenObject(childConfig, child, resourceManager, drawables, scripts, lights);
    }

    return parent;
}

void Importer::loadFeature(Utility::ConfigurationGroup* featureConfig, Object3D* object, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights) {
    std::string type = featureConfig->value("type");

    if(type == "mesh") {
        /* Shader */
        Resource<GL::AbstractShaderProgram, Oberon::Shader> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Oberon::Shader>("scene");
        if(!shaderResource)
            resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Oberon::Shader{0}, ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);

        /* Mesh */
        Mesh& mesh = object->addFeature<Mesh>(drawables, shaderResource);

        /* Primitive */
        if(featureConfig->hasGroup("primitive")) {
            Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
            loadMeshFeature(mesh, primitiveConfig, resourceManager);
        }

        /* Material */
        if(featureConfig->hasGroup("material")) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            if(materialConfig->hasValue("ambient")) {
                Color3 ambient = materialConfig->value<Color3>("ambient");
                mesh.setAmbientColor(ambient);
            }

            if(materialConfig->hasValue("diffuse")) {
                Color3 diffuse = materialConfig->value<Color3>("diffuse");
                mesh.setDiffuseColor(diffuse);
            }

            if(materialConfig->hasValue("specular")) {
                Color3 specular = materialConfig->value<Color3>("specular");
                mesh.setSpecularColor(specular);
            }

            if(materialConfig->hasValue("shininess")) {
                Float shininess = materialConfig->value<Float>("shininess");
                mesh.setShininess(shininess);
            }
        }
    } else if(type == "light") {
        /* Shader */
        Resource<GL::AbstractShaderProgram, Oberon::Shader> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Oberon::Shader>("scene");
        resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Oberon::Shader{UnsignedInt(lights->size() + 1)}, ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);

        /* Light */
        Light& light = object->addFeature<Light>(lights, shaderResource);

        if(featureConfig->hasValue("color")) {
            Color3 color = featureConfig->value<Color3>("color");
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
    } else if(type == "script") {
        /* Class name */
        std::string name = featureConfig->value("name");

        /* Script */
        object->addFeature<Script>(scripts, name);
    } else if(type == "sprite") {
        /* Mesh */
        Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>("square_texture_coords");
        if(!meshResource) {
            GL::Mesh glMesh = MeshTools::compile(Primitives::squareSolid(Primitives::SquareFlag::TextureCoordinates));
            resourceManager.set<GL::Mesh>(meshResource.key(), std::move(glMesh), ResourceDataState::Final, ResourcePolicy::ReferenceCounted);
        }

        /* Shader */
        Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shaderResource = resourceManager.get<GL::AbstractShaderProgram, Shaders::Flat3D>("flat");
        if(!shaderResource)
            resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Shaders::Flat3D{Shaders::Flat3D::Flag::ObjectId | Shaders::Flat3D::Flag::Textured}, ResourceDataState::Final, ResourcePolicy::ReferenceCounted);

        /* Sprite */
        Sprite& sprite = object->addFeature<Sprite>(drawables, meshResource, shaderResource);

        /* Pixel size */
        if(featureConfig->hasValue("pixel_size")) {
            Float pixelSize = featureConfig->value<Float>("pixel_size");
            sprite.setPixelSize(pixelSize);
        }

        /* Texture */
        if(featureConfig->hasValue("path")) {
            std::string path = featureConfig->value("path");

            Resource<GL::Texture2D> textureResource = resourceManager.get<GL::Texture2D>(path);
            if(!textureResource) {
                _pngImporter.openData(Utility::Directory::read(path));
                Containers::Optional<Trade::ImageData2D> image = _pngImporter.image2D(0);
                CORRADE_INTERNAL_ASSERT(image);

                GL::Texture2D texture;
                texture.setWrapping(GL::SamplerWrapping::ClampToEdge)
                    .setMagnificationFilter(GL::SamplerFilter::Linear)
                    .setMinificationFilter(GL::SamplerFilter::Linear)
                    .setStorage(1, GL::textureFormat(image->format()), image->size())
                    .setSubImage(0, {}, *image);

                resourceManager.set(textureResource.key(), std::move(texture), ResourceDataState::Final, ResourcePolicy::ReferenceCounted);
            }

            sprite.setTexture(textureResource);
        }
    }
}

void Importer::resetObject(Object3D* object, Utility::ConfigurationGroup* objectConfig) {
    /* Transformation */
    if(!objectConfig->hasValue("transformation")) objectConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = objectConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);
}

void Importer::loadMeshFeature(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig, OberonResourceManager& resourceManager) {
    std::string primitiveType = primitiveConfig->value("type");
    std::string meshKey = primitiveType;

    if(primitiveConfig->hasValue("rings"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("rings"));

    if(primitiveConfig->hasValue("segments"))
        meshKey += std::to_string(primitiveConfig->value<UnsignedInt>("segments"));

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);

    if(primitiveConfig->hasValue("size")) {
        Vector3 size = primitiveConfig->value<Vector3>("size");
        mesh.setSize(size);
    }

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

        if(primitiveType == "circle") glMesh = MeshTools::compile(Primitives::circle3DSolid(segments));
        else if(primitiveType == "cube") glMesh = MeshTools::compile(Primitives::cubeSolid());
        else if(primitiveType == "plane") glMesh = MeshTools::compile(Primitives::planeSolid());
        else if(primitiveType == "sphere") glMesh = MeshTools::compile(Primitives::uvSphereSolid(rings, segments));
        else if(primitiveType == "square") glMesh = MeshTools::compile(Primitives::squareSolid());

        resourceManager.set(meshResource.key(), std::move(glMesh), ResourceDataState::Final, ResourcePolicy::ReferenceCounted);
    }

    CORRADE_INTERNAL_ASSERT(meshResource);

    mesh.setMesh(meshResource);
}

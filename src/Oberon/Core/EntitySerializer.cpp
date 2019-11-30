/*
    MIT License

    Copyright (c) 2019 Marco Melorio

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

#include "EntitySerializer.h"

#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/Trade/MeshData2D.h>

namespace EntitySerializer {

Object3D* createEntityFromConfig(Utility::ConfigurationGroup* entityConfig, Object3D* parent, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts) {
    Object3D* object = new Object3D{parent};

    /* Name */
    std::string name = entityConfig->value("name");
    object->addFeature<Entity>(name);

    /* Transformation */
    if(!entityConfig->hasValue("transformation")) entityConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = entityConfig->value<Matrix4>("transformation");
    object->setTransformation(transformation);

    for(auto featureConfig: entityConfig->groups("feature"))
        addFeatureFromConfig(featureConfig, object, resourceManager, drawables, scripts);

    return object;
}

void addFeatureFromConfig(Utility::ConfigurationGroup* featureConfig, Object3D* object, OberonResourceManager& resourceManager, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts) {
    std::string type = featureConfig->value("type");

    if(type == "rectangle_shape") {
        /* Size */
        if(!featureConfig->hasValue("size")) featureConfig->setValue<Vector2>("size", {200, 100});
        Vector2 size = featureConfig->value<Vector2>("size");

        /* Color */
        if(!featureConfig->hasValue("color")) featureConfig->setValue<Color4>("color", {1, 1, 1, 1});
        Color4 color = featureConfig->value<Color4>("color");

        /* Mesh */
        Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>("square");
        if(!meshResource) {
            GL::Mesh mesh = MeshTools::compile(Primitives::squareSolid());
            resourceManager.set(meshResource.key(), std::move(mesh));
        }

        /* Shader */
        Resource<Shaders::Flat3D> shaderResource = resourceManager.get<Shaders::Flat3D>("flat3d");
        if(!shaderResource) {
            Shaders::Flat3D shader;
            resourceManager.set(shaderResource.key(), std::move(shader));
        }

        /* Rectangle shape */
        object->addFeature<RectangleShape>(drawables, *meshResource, *shaderResource, size, color);
    } else if(type == "script") {
        /* Script path */
        std::string scriptPath = featureConfig->value("script_path");

        /* Script */
        object->addFeature<Script>(scripts, scriptPath);
    }
}

void resetEntityFromConfig(Object3D* entity, Utility::ConfigurationGroup* entityConfig) {
    /* Transformation */
    if(!entityConfig->hasValue("transformation")) entityConfig->setValue<Matrix4>("transformation", Matrix4::scaling({1, 1, 1}));
    Matrix4 transformation = entityConfig->value<Matrix4>("transformation");
    entity->setTransformation(transformation);

    for(auto featureConfig: entityConfig->groups("feature")) {
        std::string type = featureConfig->value("type");

        if(type == "rectangle_shape") {
            /* Rectangle shape */
            auto& features = entity->features();
            RectangleShape* rectangleShape = nullptr;

            for(auto& feature: features) {
                if((rectangleShape = dynamic_cast<RectangleShape*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(rectangleShape != nullptr);

            /* Size */
            if(!featureConfig->hasValue("size")) featureConfig->setValue<Vector2>("size", {200, 100});
            Vector2 size = featureConfig->value<Vector2>("size");
            rectangleShape->setSize(size);

            /* Color */
            if(!featureConfig->hasValue("color")) featureConfig->setValue<Color4>("color", {1, 1, 1, 1});
            Color4 color = featureConfig->value<Color4>("color");
            rectangleShape->setColor(color);
        }
    }
}

}

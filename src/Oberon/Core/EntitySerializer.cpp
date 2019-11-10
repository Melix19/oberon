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

Object3D* EntitySerializer::createEntityFromConfig(Utility::ConfigurationGroup* entityGroup, Object3D* parent, SceneGraph::DrawableGroup3D* drawables, OberonResourceManager& resourceManager) {
    Object3D* object = new Object3D{parent};

    /* Name and Entity */
    std::string name = entityGroup->value("name");
    object->addFeature<Entity>(name);

    /* Position */
    Vector3 position = entityGroup->value<Vector3>("position");
    object->setTranslation(position);

    /* Rotation */
    Vector3 rotation = entityGroup->value<Vector3>("rotation");
    object->setRotation(Quaternion::rotation(Deg(rotation.x()), Vector3::xAxis())*
        Quaternion::rotation(Deg(rotation.y()), Vector3::yAxis())*
        Quaternion::rotation(Deg(rotation.z()), Vector3::zAxis()));

    /* Scale */
    if(!entityGroup->hasValue("scale")) entityGroup->setValue<Vector3>("scale", {1, 1, 1});
    Vector3 scale = entityGroup->value<Vector3>("scale");
    object->setScaling(scale);

    for(auto componentGroup: entityGroup->groups("component"))
        addComponentFromConfig(componentGroup, object, drawables, resourceManager);

    return object;
}

void EntitySerializer::addComponentFromConfig(Utility::ConfigurationGroup* componentGroup, Object3D* object, SceneGraph::DrawableGroup3D* drawables, OberonResourceManager& resourceManager) {
    std::string type = componentGroup->value("type");

    if(type == "rectangle_shape") {
        /* Size */
        if(!componentGroup->hasValue("size")) componentGroup->setValue<Vector2>("size", {200, 100});
        Vector2 size = componentGroup->value<Vector2>("size");

        /* Color */
        if(!componentGroup->hasValue("color")) componentGroup->setValue<Color4>("color", {1, 1, 1, 1});
        Color4 color = componentGroup->value<Color4>("color");

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

        /* RectangleShape */
        object->addFeature<RectangleShape>(drawables, *meshResource, *shaderResource, size, color);
    }
}

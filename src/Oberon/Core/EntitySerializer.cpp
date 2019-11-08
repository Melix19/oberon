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

Object2D* EntitySerializer::createEntityFromConfig(Utility::ConfigurationGroup* entityGroup, Object2D* parent, SceneGraph::DrawableGroup2D* drawables, Shaders::Flat2D& shader) {
    Object2D* entity = new Object2D{parent};

    /* Name and Entity */
    std::string name = entityGroup->value("name");
    entity->addFeature<Entity>(name);

    /* Position */
    Vector2 position = entityGroup->value<Vector2>("position");
    entity->setTranslation(position);

    /* Rotation */
    Float rotation = entityGroup->value<Float>("rotation");
    entity->setRotation(Complex::rotation(Deg(rotation)));

    /* Scale */
    Vector2 scale = entityGroup->value<Vector2>("scale");
    entity->setScaling(scale);

    for(auto& componentGroup: entityGroup->groups(name + "-component")) {
        std::string type = componentGroup->value("type");

        if(type == "rectangle_shape") {
            /* Size */
            Vector2 size = componentGroup->value<Vector2>("size");

            /* Color */
            Color4 color = componentGroup->value<Color4>("color");

            /* RectangleShape */
            entity->addFeature<RectangleShape>(drawables, shader, size, color);
        }
    }

    return entity;
}

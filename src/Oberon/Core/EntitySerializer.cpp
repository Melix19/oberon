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

Object2D* EntitySerializer::createEntityFromJson(Value& jsonEntity, Object2D* parent, SceneGraph::DrawableGroup2D* drawables, Shaders::Flat2D& shader) {
    Object2D* entity = new Object2D{parent};

    /* Name and Entity */
    std::string name = jsonEntity["name"].GetString();
    entity->addFeature<Entity>(name);

    /* Position */
    Vector2 position{jsonEntity["position"][0].GetFloat(), jsonEntity["position"][1].GetFloat()};
    entity->setTranslation(position);

    /* Rotation */
    Float rotation = jsonEntity["rotation"].GetFloat();
    entity->setRotation(Complex::rotation(Deg(rotation)));

    /* Scale */
    Vector2 scale{jsonEntity["scale"][0].GetFloat(), jsonEntity["scale"][1].GetFloat()};
    entity->setScaling(scale);

    auto jsonComponents = jsonEntity["components"].GetArray();
    for(auto& jsonComponent: jsonComponents) {
        std::string type = jsonComponent["type"].GetString();

        if(type == "rectangle_shape") {
            /* Size */
            Vector2 size{jsonComponent["size"][0].GetFloat(), jsonComponent["size"][1].GetFloat()};

            /* Color */
            Color4 color{jsonComponent["color"][0].GetFloat(), jsonComponent["color"][1].GetFloat(), jsonComponent["color"][2].GetFloat(), jsonComponent["color"][3].GetFloat()};

            /* RectangleShape */
            entity->addFeature<RectangleShape>(drawables, shader, size, color);
        }
    }

    return entity;
}

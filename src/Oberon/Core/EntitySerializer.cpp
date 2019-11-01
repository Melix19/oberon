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

#include "EntitySerializer.hpp"

Entity* EntitySerializer::createEntityFromJson(Value* j_entity, Object2D* parent, SceneGraph::DrawableGroup2D* drawables, Shaders::Flat2D& shader)
{
    std::string entity_name = (*j_entity)["name"].GetString();
    Entity* entity_ptr = new Entity{ entity_name, parent };

    // Position
    float position_x = (*j_entity)["position"][0].GetFloat();
    float position_y = (*j_entity)["position"][1].GetFloat();
    entity_ptr->setTranslation({ position_x, position_y });

    // Rotation
    float rotation = (*j_entity)["rotation"].GetFloat();
    entity_ptr->setRotation(Complex::rotation(Deg(rotation)));

    // Scale
    float scale_x = (*j_entity)["scale"][0].GetFloat();
    float scale_y = (*j_entity)["scale"][1].GetFloat();
    entity_ptr->setScaling({ scale_x, scale_y });

    auto j_components = (*j_entity)["components"].GetArray();
    for (auto& j_component : j_components) {
        std::string type = j_component["type"].GetString();

        if (type == "rectangle_shape") {
            // Size
            float size_x = j_component["size"][0].GetFloat();
            float size_y = j_component["size"][1].GetFloat();

            // Color
            float color_r = j_component["color"][0].GetFloat();
            float color_g = j_component["color"][1].GetFloat();
            float color_b = j_component["color"][2].GetFloat();
            float color_a = j_component["color"][3].GetFloat();

            entity_ptr->addFeature<RectangleShape>(drawables, shader, Vector2{ size_x, size_y }, Color4{ color_r, color_g, color_b, color_a });
        }
    }

    return entity_ptr;
}

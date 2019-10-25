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

#include "Inspector.hpp"

Inspector::Inspector()
    : entity_node_ptr(nullptr)
    , COLUMN_WIDTH(80)
{
}

void Inspector::newFrame()
{
    ImGui::Begin("Inspector");

    if (entity_node_ptr) {
        // Position
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Position");
        ImGui::SameLine(COLUMN_WIDTH);
        ImGui::SetNextItemWidth(-1);
        float position[2] = { entity_node_ptr->j_value["position"][0].GetFloat(), entity_node_ptr->j_value["position"][1].GetFloat() };
        ImGui::DragFloat2("##Position", position, 0.5f);
        entity_node_ptr->j_value["position"][0] = position[0];
        entity_node_ptr->j_value["position"][1] = position[1];

        // Rotation
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Rotation");
        ImGui::SameLine(COLUMN_WIDTH);
        ImGui::SetNextItemWidth(-1);
        float rotation = entity_node_ptr->j_value["rotation"].GetFloat();
        ImGui::DragFloat("##Rotation", &rotation, 0.5f);
        entity_node_ptr->j_value["rotation"] = rotation;

        // Scale
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scale");
        ImGui::SameLine(COLUMN_WIDTH);
        ImGui::SetNextItemWidth(-1);
        float scale[2] = { entity_node_ptr->j_value["scale"][0].GetFloat(), entity_node_ptr->j_value["scale"][1].GetFloat() };
        ImGui::DragFloat2("##Scale", scale, 0.005f);
        entity_node_ptr->j_value["scale"][0] = scale[0];
        entity_node_ptr->j_value["scale"][1] = scale[1];

        // Components
        auto j_components = entity_node_ptr->j_value["components"].GetArray();
        for (auto& j_component : j_components) {
            std::string type = j_component["type"].GetString();

            if (type == "rectangle_shape") {
                // Type
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Rectangle shape");

                // Size
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Size");
                ImGui::SameLine(COLUMN_WIDTH);
                ImGui::SetNextItemWidth(-1);
                float size[2] = { j_component["size"][0].GetFloat(), j_component["size"][1].GetFloat() };
                ImGui::DragFloat2("##Size", size, 0.5f);
                j_component["size"][0] = size[0];
                j_component["size"][1] = size[1];

                // Color
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color");
                ImGui::SameLine(COLUMN_WIDTH);
                ImGui::SetNextItemWidth(-1);
                float color[4] = { j_component["color"][0].GetFloat(), j_component["color"][1].GetFloat(), j_component["color"][2].GetFloat(), j_component["color"][3].GetFloat() };
                ImGui::ColorEdit4("##Color", color);
                j_component["color"][0] = color[0];
                j_component["color"][1] = color[1];
                j_component["color"][2] = color[2];
                j_component["color"][3] = color[3];
            }
        }
    }

    ImGui::End();
}

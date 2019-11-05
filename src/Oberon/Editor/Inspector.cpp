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

#include "Inspector.h"

void Inspector::newFrame() {
    const Int columnWidth = 100;

    bool isVisible = ImGui::Begin("Inspector");

    /* If the window is not visible, just end the method here. */
    if(!isVisible) {
        ImGui::End();
        return;
    }

    if(_entityNode) {
        /* Position */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Position");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Float position[2] = {_entityNode->jsonEntity()["position"][0].GetFloat(),
            _entityNode->jsonEntity()["position"][1].GetFloat()};
        ImGui::DragFloat2("##Position", position, 0.5f);
        _entityNode->jsonEntity()["position"][0] = position[0];
        _entityNode->jsonEntity()["position"][1] = position[1];
        _entityNode->entity()->setTranslation({position[0], position[1]});

        /* Rotation */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Rotation");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Float rotation = _entityNode->jsonEntity()["rotation"].GetFloat();
        ImGui::DragFloat("##Rotation", &rotation, 0.5f);
        _entityNode->jsonEntity()["rotation"] = rotation;
        _entityNode->entity()->setRotation(Complex::rotation(Deg(rotation)));

        /* Scale */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scale");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Float scale[2] = {_entityNode->jsonEntity()["scale"][0].GetFloat(),
            _entityNode->jsonEntity()["scale"][1].GetFloat()};
        ImGui::DragFloat2("##Scale", scale, 0.005f);
        _entityNode->jsonEntity()["scale"][0] = scale[0];
        _entityNode->jsonEntity()["scale"][1] = scale[1];
        _entityNode->entity()->setScaling({scale[0], scale[1]});

        /* Components */
        auto jsonComponents = _entityNode->jsonEntity()["components"].GetArray();
        for(auto& jsonComponent: jsonComponents) {
            std::string type = jsonComponent["type"].GetString();

            /* RectangleShape */
            if(type == "rectangle_shape") {
                auto& components = _entityNode->entity()->features();
                RectangleShape* rectangleShape = nullptr;

                for(auto& component: components) {
                    rectangleShape = dynamic_cast<RectangleShape*>(&component);
                    if(rectangleShape)
                        break;
                }

                CORRADE_INTERNAL_ASSERT(rectangleShape != nullptr);

                /* Type */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Rectangle shape");

                /* Size */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Size");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                Float size[2] = {jsonComponent["size"][0].GetFloat(), jsonComponent["size"][1].GetFloat()};
                ImGui::DragFloat2("##Size", size, 0.5f);
                jsonComponent["size"][0] = size[0];
                jsonComponent["size"][1] = size[1];
                rectangleShape->setSize({size[0], size[1]});

                /* Color */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                Float color[4] = {jsonComponent["color"][0].GetFloat(), jsonComponent["color"][1].GetFloat(),
                    jsonComponent["color"][2].GetFloat(), jsonComponent["color"][3].GetFloat()};
                ImGui::ColorEdit4("##Color", color);
                jsonComponent["color"][0] = color[0];
                jsonComponent["color"][1] = color[1];
                jsonComponent["color"][2] = color[2];
                jsonComponent["color"][3] = color[3];
                rectangleShape->setColor({color[0], color[1], color[2], color[3]});
           }
       }
   }

    ImGui::End();
}

void Inspector::clearContent() {
    _entityNode = nullptr;
}

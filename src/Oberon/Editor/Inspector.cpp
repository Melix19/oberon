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

    if(_panel && !_panel->selectedNodes().empty()) {
        auto& selectedNodes = _panel->selectedNodes();
        EntityNode* entityNode = selectedNodes.front();

        /* Position */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Position");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Vector2 position = entityNode->entityGroup()->value<Vector2>("position");
        ImGui::DragFloat2("##Position", position.data(), 0.5f);
        entityNode->entityGroup()->setValue("position", position);
        entityNode->entity()->setTranslation(position);

        /* Rotation */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Rotation");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Float rotation = entityNode->entityGroup()->value<Float>("rotation");
        ImGui::DragFloat("##Rotation", &rotation, 0.5f);
        entityNode->entityGroup()->setValue("rotation", rotation);
        entityNode->entity()->setRotation(Complex::rotation(Deg(rotation)));

        /* Scale */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scale");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Vector2 scale = entityNode->entityGroup()->value<Vector2>("scale");
        ImGui::DragFloat2("##Scale", scale.data(), 0.005f);
        entityNode->entityGroup()->setValue("scale", scale);
        entityNode->entity()->setScaling(scale);

        /* Components */
        for(auto componentGroup: entityNode->entityGroup()->groups("component")) {
            std::string type = componentGroup->value("type");

            /* RectangleShape */
            if(type == "rectangle_shape") {
                auto& components = entityNode->entity()->features();
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
                Vector2 size = componentGroup->value<Vector2>("size");
                ImGui::DragFloat2("##Size", size.data(), 0.5f);
                componentGroup->setValue("size", size);
                rectangleShape->setSize(size);

                /* Color */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                Color4 color = componentGroup->value<Color4>("color");
                ImGui::ColorEdit4("##Color", color.data());
                componentGroup->setValue("color", color);
                rectangleShape->setColor(color);
           }
       }

        if(ImGui::Button("Add component"))
            ImGui::OpenPopup("ComponentPopup");

        if(ImGui::BeginPopup("ComponentPopup")) {
            if(ImGui::Selectable("Rectangle shape")) {
                Utility::ConfigurationGroup* componentGroup = entityNode->entityGroup()->addGroup("component");
                componentGroup->setValue("type", "rectangle_shape");

                _panel->addComponentToEntity(componentGroup, entityNode->entity());
            }

            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

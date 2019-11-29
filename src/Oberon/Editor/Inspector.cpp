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

#include <Corrade/Utility/Assert.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void Inspector::newFrame() {
    bool isVisible = ImGui::Begin("Inspector");

    /* If the window is not visible, just end the method here. */
    if(!isVisible) {
        ImGui::End();
        return;
    }

    const Int columnWidth = 100;

    if(_panel && !_panel->selectedNodes().empty()) {
        auto& selectedNodes = _panel->selectedNodes();
        EntityNode* entityNode = selectedNodes.front();

        if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            /* Translation */
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Translation");
            ImGui::SameLine(columnWidth);
            ImGui::SetNextItemWidth(-1);
            Vector3 translation = entityNode->entityConfig()->value<Matrix4>("transformation").translation();
            if(ImGui::DragFloat3("##Translation", translation.data(), 0.5f)) {
                entityNode->entity()->setTranslation(translation);
                entityNode->entityConfig()->setValue("transformation", entityNode->entity()->transformation());
            }

            /* Rotation */
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Rotation");
            ImGui::SameLine(columnWidth);
            ImGui::SetNextItemWidth(-1);
            Vector3 rotationDegree = entityNode->rotationDegree();
            if(ImGui::DragFloat3("##Rotation", rotationDegree.data(), 0.5f)) {
                entityNode->entity()->setRotation(
                    Quaternion::rotation(Rad(Deg(rotationDegree.z())), Vector3::zAxis())*
                    Quaternion::rotation(Rad(Deg(rotationDegree.y())), Vector3::yAxis())*
                    Quaternion::rotation(Rad(Deg(rotationDegree.x())), Vector3::xAxis()));
                entityNode->entityConfig()->setValue("transformation", entityNode->entity()->transformation());
                entityNode->setRotationDegree(rotationDegree);
            }

            /* Scaling */
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Scaling");
            ImGui::SameLine(columnWidth);
            ImGui::SetNextItemWidth(-1);
            Vector3 scaling = entityNode->entityConfig()->value<Matrix4>("transformation").scaling();
            if(ImGui::DragFloat3("##Scaling", scaling.data(), 0.005f)) {
                entityNode->entity()->setScaling(scaling);
                entityNode->entityConfig()->setValue("transformation", entityNode->entity()->transformation());
            }
        }

        /* Components */
        for(auto componentConfig: entityNode->entityConfig()->groups("component")) {
            std::string type = componentConfig->value("type");

            if(type == "rectangle_shape") {
                /* Rectangle shape */
                auto& components = entityNode->entity()->features();
                RectangleShape* rectangleShape = nullptr;

                for(auto& component: components) {
                    if((rectangleShape = dynamic_cast<RectangleShape*>(&component)))
                        break;
                }

                CORRADE_INTERNAL_ASSERT(rectangleShape != nullptr);

                bool componentIsOpen = true;

                if(ImGui::CollapsingHeader("Rectangle shape", &componentIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                    /* Size */
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Size");
                    ImGui::SameLine(columnWidth);
                    ImGui::SetNextItemWidth(-1);
                    Vector2 size = componentConfig->value<Vector2>("size");
                    if(ImGui::DragFloat2("##Size", size.data(), 0.5f)) {
                        rectangleShape->setSize(size);
                        componentConfig->setValue("size", size);
                    }

                    /* Color */
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Color");
                    ImGui::SameLine(columnWidth);
                    ImGui::SetNextItemWidth(-1);
                    Color4 color = componentConfig->value<Color4>("color");
                    if(ImGui::ColorEdit4("##Color", color.data())) {
                        rectangleShape->setColor(color);
                        componentConfig->setValue("color", color);
                    }
                }

                if(!componentIsOpen) {
                    delete rectangleShape;
                    entityNode->entityConfig()->removeGroup(componentConfig);
                }
           } else if(type == "script") {
                /* Script */
                auto& components = entityNode->entity()->features();
                Script* script = nullptr;

                for(auto& component: components) {
                    if((script = dynamic_cast<Script*>(&component)))
                        break;
                }

                CORRADE_INTERNAL_ASSERT(script != nullptr);

                bool componentIsOpen = true;

                if(ImGui::CollapsingHeader("Script", &componentIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                    /* Size */
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Script path");
                    ImGui::SameLine(columnWidth);
                    ImGui::SetNextItemWidth(-1);
                    std::string scriptPath = componentConfig->value("script_path");
                    if(ImGui::InputText("##ScriptPath", &scriptPath))
                        componentConfig->setValue("script_path", scriptPath);
                }

                if(!componentIsOpen) {
                    delete script;
                    entityNode->entityConfig()->removeGroup(componentConfig);
                }
           }
       }

        if(ImGui::Button("Add component"))
            ImGui::OpenPopup("ComponentPopup");

        if(ImGui::BeginPopup("ComponentPopup")) {
            if(ImGui::Selectable("Rectangle shape")) {
                Utility::ConfigurationGroup* componentConfig = entityNode->entityConfig()->addGroup("component");
                componentConfig->setValue("type", "rectangle_shape");

                _panel->addComponentToEntity(componentConfig, entityNode->entity());
            }

            if(ImGui::Selectable("Script")) {
                Utility::ConfigurationGroup* componentConfig = entityNode->entityConfig()->addGroup("component");
                componentConfig->setValue("type", "script");

                _panel->addComponentToEntity(componentConfig, entityNode->entity());
            }

            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

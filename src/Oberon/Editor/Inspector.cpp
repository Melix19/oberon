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
    if(!isVisible || !_panel || _panel->selectedNodes().empty()) {
        ImGui::End();
        return;
    }

    const Int columnWidth = 100;

    auto& selectedNodes = _panel->selectedNodes();
    ObjectNode* objectNode = selectedNodes.front();

    if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        /* Translation */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Translation");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Vector3 translation = objectNode->objectConfig()->value<Matrix4>("transformation").translation();
        if(ImGui::DragFloat3("##Translation", translation.data(), 0.5f)) {
            objectNode->object()->setTranslation(translation);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Rotation */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Rotation");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Vector3 rotationDegree = objectNode->rotationDegree();
        if(ImGui::DragFloat3("##Rotation", rotationDegree.data(), 0.5f)) {
            objectNode->object()->setRotation(
                Quaternion::rotation(Rad(Deg(rotationDegree.z())), Vector3::zAxis())*
                Quaternion::rotation(Rad(Deg(rotationDegree.y())), Vector3::yAxis())*
                Quaternion::rotation(Rad(Deg(rotationDegree.x())), Vector3::xAxis()));
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
            objectNode->setRotationDegree(rotationDegree);
        }

        /* Scaling */
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scaling");
        ImGui::SameLine(columnWidth);
        ImGui::SetNextItemWidth(-1);
        Vector3 scaling = objectNode->objectConfig()->value<Matrix4>("transformation").scaling();
        if(ImGui::DragFloat3("##Scaling", scaling.data(), 0.005f)) {
            objectNode->object()->setScaling(scaling);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }
    }

    /* Features */
    for(auto featureConfig: objectNode->objectConfig()->groups("feature")) {
        std::string type = featureConfig->value("type");

        if(type == "rectangle_shape") {
            /* Rectangle shape */
            auto& features = objectNode->object()->features();
            RectangleShape* rectangleShape = nullptr;

            for(auto& feature: features) {
                if((rectangleShape = dynamic_cast<RectangleShape*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(rectangleShape != nullptr);

            bool featureIsOpen = true;

            if(ImGui::CollapsingHeader("Rectangle shape", &featureIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                /* Size */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Size");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                Vector2 size = featureConfig->value<Vector2>("size");
                if(ImGui::DragFloat2("##Size", size.data(), 0.5f)) {
                    rectangleShape->setSize(size);
                    featureConfig->setValue("size", size);
                }

                /* Color */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Color");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                Color4 color = featureConfig->value<Color4>("color");
                if(ImGui::ColorEdit4("##Color", color.data())) {
                    rectangleShape->setColor(color);
                    featureConfig->setValue("color", color);
                }
            }

            if(!featureIsOpen) {
                delete rectangleShape;
                objectNode->objectConfig()->removeGroup(featureConfig);
            }
        } else if(type == "script") {
            /* Script */
            auto& features = objectNode->object()->features();
            Script* script = nullptr;

            for(auto& feature: features) {
                if((script = dynamic_cast<Script*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(script != nullptr);

            bool featureIsOpen = true;

            if(ImGui::CollapsingHeader("Script", &featureIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                /* Size */
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Script path");
                ImGui::SameLine(columnWidth);
                ImGui::SetNextItemWidth(-1);
                std::string scriptPath = featureConfig->value("script_path");
                if(ImGui::InputText("##ScriptPath", &scriptPath))
                    featureConfig->setValue("script_path", scriptPath);
            }

            if(!featureIsOpen) {
                delete script;
                objectNode->objectConfig()->removeGroup(featureConfig);
            }
        }
    }

    if(ImGui::Button("Add feature"))
        ImGui::OpenPopup("FeaturePopup");

    if(ImGui::BeginPopup("FeaturePopup")) {
        std::string newFeatureType;

        if(ImGui::Selectable("Rectangle shape"))
            newFeatureType = "rectangle_shape";

        if(ImGui::Selectable("Script"))
            newFeatureType = "script";

        if(!newFeatureType.empty()) {
            bool featureAlreadyPresent = false;

            for(auto featureConfig: objectNode->objectConfig()->groups("feature")) {
                std::string type = featureConfig->value("type");
                if(type == newFeatureType) featureAlreadyPresent = true;
            }

            if(!featureAlreadyPresent) {
                Utility::ConfigurationGroup* featureConfig = objectNode->objectConfig()->addGroup("feature");
                featureConfig->setValue("type", newFeatureType);

                _panel->addFeatureToObject(featureConfig, objectNode->object());
            }
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

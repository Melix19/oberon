/*
    This file is part of Oberon.

    Copyright (c) 2019-2020 Marco Melorio

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
#include <Magnum/Math/ConfigurationValue.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <climits>

#include "Themer.h"

void Inspector::newFrame() {
    bool isVisible = ImGui::Begin("Inspector");

    auto& selectedNodes = _panel->selectedNodes();
    ObjectNode* objectNode;

    if(_panel && !selectedNodes.empty())
        objectNode = selectedNodes.front();

    /* If the window is not visible, end the method here. */
    if(!isVisible || !_panel || selectedNodes.empty() || (objectNode == _panel->rootNode())) {
        ImGui::End();
        return;
    }

    if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        /* Translation */
        Themer::setNextItemRightAlign("Translation");
        Vector3 translation = objectNode->objectConfig()->value<Matrix4>("transformation").translation();
        if(ImGui::DragFloat3("##Translation", translation.data(), 0.002f)) {
            objectNode->object()->setTranslation(translation);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Rotation */
        Themer::setNextItemRightAlign("Rotation");
        Vector3 rotationDegree = objectNode->rotationDegree();
        if(ImGui::DragFloat3("##Rotation", rotationDegree.data(), 0.1f)) {
            objectNode->object()->setRotation(
                Quaternion::rotation(Rad{Deg{rotationDegree.z()}}, Vector3::zAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.y()}}, Vector3::yAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.x()}}, Vector3::xAxis()));
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
            objectNode->setRotationDegree(rotationDegree);
        }

        /* Scaling */
        Themer::setNextItemRightAlign("Scaling");
        Vector3 scaling = objectNode->objectConfig()->value<Matrix4>("transformation").scaling();
        if(ImGui::DragFloat3("##Scaling", scaling.data(), 0.001f)) {
            objectNode->object()->setScaling(scaling);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }
    }

    /* Features */
    for(auto featureConfig: objectNode->objectConfig()->groups("feature")) {
        std::string type = featureConfig->value("type");

        if(type == "mesh") {
            /* Mesh */
            auto& features = objectNode->object()->features();
            Mesh* mesh = nullptr;

            for(auto& feature: features) {
                if((mesh = dynamic_cast<Mesh*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(mesh != nullptr);

            bool featureIsOpen = true;

            if(ImGui::CollapsingHeader("Mesh", &featureIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
                Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");
                ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
                    ImGuiTreeNodeFlags_SpanFullWidth;

                if(ImGui::TreeNodeEx("Primitive", nodeFlags)) {
                    std::string primitiveType = "none";
                    bool updateMesh = false;

                    if(primitiveConfig && primitiveConfig->hasValue("type"))
                        primitiveType = primitiveConfig->value("type");

                    std::string primitiveString = primitiveType;
                    primitiveString[0] = toupper(primitiveString[0]);

                    Themer::setNextItemRightAlign("Type");
                    if(ImGui::BeginCombo("##Mesh.Primitive.Type", primitiveString.c_str())) {
                        const char* primitives[] = {"Circle", "Cube", "Plane", "Sphere", "Square"};

                        for(auto& type: primitives) {
                            bool isSelected = (primitiveString.c_str() == type);

                            if(ImGui::Selectable(type, isSelected)) {
                                primitiveType = type;
                                primitiveType[0] = tolower(primitiveType[0]);

                                featureConfig->removeGroup("primitive");
                                primitiveConfig = featureConfig->addGroup("primitive");
                                primitiveConfig->setValue("type", primitiveType);

                                if(!materialConfig)
                                    materialConfig = featureConfig->addGroup("material");

                                updateMesh = true;
                            }
                        }

                        ImGui::EndCombo();
                    }

                    if(primitiveConfig) {
                        Themer::setNextItemRightAlign("Size");
                        Vector3 size = primitiveConfig->value<Vector3>("size");
                        if(ImGui::DragFloat3("##Mesh.Primitive.Size", size.data(), 0.002f)) {
                            mesh->setSize(size);
                            primitiveConfig->setValue("size", size);
                        }

                        if(primitiveType == "sphere") {
                            Themer::setNextItemRightAlign("Rings");
                            Int rings = primitiveConfig->value<Int>("rings");
                            if(ImGui::DragInt("##Mesh.Primitive.Rings", &rings, 1.0f, 2, INT_MAX)) {
                                primitiveConfig->setValue("rings", rings);
                                updateMesh = true;
                            }
                        }

                        if(primitiveType == "circle" || primitiveType == "sphere") {
                            Themer::setNextItemRightAlign("Segments");
                            Int segments = primitiveConfig->value<Int>("segments");
                            if(ImGui::DragInt("##Mesh.Primitive.Segments", &segments, 1.0f, 3, INT_MAX)) {
                                primitiveConfig->setValue("segments", segments);
                                updateMesh = true;
                            }
                        }
                    }

                    if(updateMesh) _panel->loadMeshFeature(*mesh, primitiveConfig);

                    ImGui::TreePop();
                }

                if(materialConfig && ImGui::TreeNodeEx("Material", nodeFlags)) {
                    Themer::setNextItemRightAlign("Ambient color");
                    Color3 ambient = materialConfig->value<Color3>("ambient");
                    if(ImGui::ColorEdit3("##Mesh.Material.Ambient", ambient.data())) {
                        materialConfig->setValue("ambient", ambient);
                        mesh->setAmbientColor(ambient);
                    }

                    Themer::setNextItemRightAlign("Diffuse color");
                    Color3 diffuse = materialConfig->value<Color3>("diffuse");
                    if(ImGui::ColorEdit3("##Mesh.Material.Diffuse", diffuse.data())) {
                        materialConfig->setValue("diffuse", diffuse);
                        mesh->setDiffuseColor(diffuse);
                    }

                    Themer::setNextItemRightAlign("Specular color");
                    Color3 specular = materialConfig->value<Color3>("specular");
                    if(ImGui::ColorEdit3("##Mesh.Material.Specular", specular.data())) {
                        materialConfig->setValue("specular", specular);
                        mesh->setSpecularColor(specular);
                    }

                    Themer::setNextItemRightAlign("Shininess");
                    Float shininess = materialConfig->value<Float>("shininess");
                    if(ImGui::DragFloat("##Mesh.Material.Shininess", &shininess, 0.1f, 1.0f, FLT_MAX)) {
                        materialConfig->setValue("shininess", shininess);
                        mesh->setShininess(shininess);
                    }

                    ImGui::TreePop();
                }
            }

            if(!featureIsOpen) {
                delete mesh;
                objectNode->objectConfig()->removeGroup(featureConfig);

                _panel->removeDrawableNode(objectNode);
            }
        } else if(type == "light") {
            /* Light */
            auto& features = objectNode->object()->features();
            Light* light = nullptr;

            for(auto& feature: features) {
                if((light = dynamic_cast<Light*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(light != nullptr);

            bool featureIsOpen = true;

            if(ImGui::CollapsingHeader("Light", &featureIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                /* Color */
                Themer::setNextItemRightAlign("Color");
                Color3 color = featureConfig->value<Color3>("color");
                if(ImGui::ColorEdit3("##Light.Color", color.data())) {
                    light->setColor(color);
                    featureConfig->setValue("color", color);
                }
            }

            if(!featureIsOpen) {
                delete light;
                objectNode->objectConfig()->removeGroup(featureConfig);

                _panel->updateShader();
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
                /* Script name */
                Themer::setNextItemRightAlign("Script name");
                std::string name = featureConfig->value("name");
                if(ImGui::InputText("##Script.Name", &name, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    script->setname(name);
                    featureConfig->setValue("name", name);
                }
            }

            if(!featureIsOpen) {
                delete script;
                objectNode->objectConfig()->removeGroup(featureConfig);
            }
        } else if(type == "sprite") {
            /* Sprite */
            auto& features = objectNode->object()->features();
            Sprite* sprite = nullptr;

            for(auto& feature: features) {
                if((sprite = dynamic_cast<Sprite*>(&feature)))
                    break;
            }

            CORRADE_INTERNAL_ASSERT(sprite != nullptr);

            bool featureIsOpen = true;
            bool reloadFeature = false;

            if(ImGui::CollapsingHeader("Sprite", &featureIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
                /* Path */
                Themer::setNextItemRightAlign("Path");
                std::string path = featureConfig->value("path");
                if(ImGui::InputText("##Sprite.Path", &path, ImGuiInputTextFlags_EnterReturnsTrue)) {
                    featureConfig->setValue("path", path);
                    reloadFeature = true;
                }

                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        featureConfig->setValue("path", fileNode->resourcePath());
                        reloadFeature = true;
                    }
                    ImGui::EndDragDropTarget();
                }

                /* Pixel size */
                Themer::setNextItemRightAlign("Pixel size");
                Float pixelSize = featureConfig->value<Float>("pixel_size");
                if(ImGui::DragFloat("##Sprite.PixelSize", &pixelSize, 0.001f)) {
                    sprite->setPixelSize(pixelSize);
                    featureConfig->setValue("pixel_size", pixelSize);
                }
            }

            if(!featureIsOpen || reloadFeature) {
                delete sprite;

                if(reloadFeature) _panel->addFeatureToObject(objectNode, featureConfig);
                else objectNode->objectConfig()->removeGroup(featureConfig);
            }
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImVec2 featureButtonsize(100.0f, 0.0f);
    ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - featureButtonsize.x/2);
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

    if(ImGui::Button("Add feature", featureButtonsize))
        ImGui::OpenPopup("FeaturePopup");

    ImGui::PopStyleVar();

    if(ImGui::BeginPopup("FeaturePopup")) {
        std::string newFeatureType;

        if(ImGui::Selectable("Mesh"))   newFeatureType = "mesh";
        if(ImGui::Selectable("Light"))  newFeatureType = "light";
        if(ImGui::Selectable("Script")) newFeatureType = "script";
        if(ImGui::Selectable("Sprite")) newFeatureType = "sprite";

        if(!newFeatureType.empty()) {
            bool featureAlreadyPresent = false;

            for(auto featureConfig: objectNode->objectConfig()->groups("feature")) {
                std::string type = featureConfig->value("type");
                if(type == newFeatureType) featureAlreadyPresent = true;
            }

            if(!featureAlreadyPresent) {
                Utility::ConfigurationGroup* featureConfig = objectNode->objectConfig()->addGroup("feature");
                featureConfig->setValue("type", newFeatureType);

                _panel->addFeatureToObject(objectNode, featureConfig);
            }
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

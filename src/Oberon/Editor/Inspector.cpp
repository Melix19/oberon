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

#include <algorithm>
#include <climits>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>
#include <Oberon/Mesh.h>

#include "CollectionPanel.h"
#include "FileNode.h"
#include "ObjectNode.h"
#include "Themer.h"

namespace Oberon { namespace Editor {

namespace {

template<class T> T* getObjectFeature(Object3D* object) {
    Containers::LinkedList<SceneGraph::AbstractFeature3D>& features = object->features();
    T* feature = nullptr;
    for(SceneGraph::AbstractFeature3D& f: features) {
        if((feature = dynamic_cast<T*>(&f)))
            break;
    }
    return feature;
}

std::string snakeCaseToPhrase(const std::string& snakeCase) {
    std::string phrase = snakeCase;
    std::replace(phrase.begin(), phrase.end(), '_', ' ');
    phrase[0] = std::toupper(phrase[0]);
    return phrase;
}

}

void Inspector::newFrame() {
    bool isVisible = ImGui::Begin("Inspector");

    std::vector<ObjectNode*>& selectedNodes = _panel->selectedNodes();
    ObjectNode* objectNode = nullptr;

    if(_panel && !selectedNodes.empty())
        objectNode = selectedNodes.front();

    /* If the window is not visible or the object node is the root, end the method here. */
    if(!isVisible || !_panel || selectedNodes.empty() || objectNode == _panel->rootNode()) {
        ImGui::End();
        return;
    }

    _spacing = ImGui::GetWindowWidth()/2;

    showTransformationHeader(objectNode);
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    for(Utility::ConfigurationGroup* featureConfig: objectNode->objectConfig()->groups("feature")) {
        std::string type = featureConfig->value("type");
        if(type == "light") showLightHeader(objectNode, featureConfig);
        else if(type == "mesh") showMeshHeader(objectNode, featureConfig);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
    }

    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    /* Add feature button */
    const Float addFeatureButtonWidth = ImGui::GetWindowWidth()/2;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - addFeatureButtonWidth/2);

    if(ImGui::Button("Add feature", ImVec2(addFeatureButtonWidth, 0)))
        ImGui::OpenPopup("FeaturePopup");

    if(ImGui::BeginPopup("FeaturePopup")) {
        const char* features[] = {"light", "mesh"};
        for(const char* feature: features) {
            if(ImGui::Selectable(snakeCaseToPhrase(feature).c_str())) {
                bool featureAlreadyPresent = false;

                for(Utility::ConfigurationGroup* featureConfig: objectNode->objectConfig()->groups("feature")) {
                    std::string type = featureConfig->value("type");
                    if(type == feature) {
                        featureAlreadyPresent = true;
                        break;
                    }
                }

                if(!featureAlreadyPresent) {
                    Utility::ConfigurationGroup* featureConfig = objectNode->objectConfig()->addGroup("feature");
                    featureConfig->setValue("type", feature);
                    _panel->addFeatureToObject(objectNode, featureConfig);
                }
            }
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void Inspector::showTransformationHeader(ObjectNode* objectNode) {
    if(ImGui::CollapsingHeader("Transformation", ImGuiTreeNodeFlags_DefaultOpen)) {
        /* Translation */
        ImGui::Text("Translation");
        ImGui::SetNextItemWidth(-1);
        Vector3 translation = objectNode->object()->translation();
        if(ImGui::DragFloat3("##Transformation.Translation", translation.data(), 0.005f)) {
            objectNode->object()->setTranslation(translation);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Rotation */
        ImGui::Text("Rotation");
        ImGui::SetNextItemWidth(-1);
        Vector3 rotationDegree = objectNode->rotationDegree();
        if(ImGui::DragFloat3("##Transformation.Rotation", rotationDegree.data(), 0.1f)) {
            objectNode->object()->setRotation(
                Quaternion::rotation(Rad{Deg{rotationDegree.z()}}, Vector3::zAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.y()}}, Vector3::yAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.x()}}, Vector3::xAxis()));
            objectNode->setRotationDegree(rotationDegree);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Scaling */
        ImGui::Text("Scaling");
        ImGui::SetNextItemWidth(-1);
        Vector3 scaling = objectNode->object()->scaling();
        if(ImGui::DragFloat3("##Transformation.Scaling", scaling.data(), 0.002f)) {
            objectNode->object()->setScaling(scaling);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }
    }
}

void Inspector::showLightHeader(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    Light* light = getObjectFeature<Light>(objectNode->object());
    bool headerIsOpen = true;

    if(ImGui::CollapsingHeader("Light", &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Color");
        ImGui::SetNextItemWidth(-1);
        Color4 color = light->color();
        if(ImGui::ColorEdit4("##Light.Color", color.data())) {
            light->setColor(color);
            featureConfig->setValue("color", color);
        }

        Themer::setNextItemRightAlign("Constant", _spacing);
        Float constant = light->constant();
        if(ImGui::DragFloat("##Light.Contant", &constant, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setConstant(constant);
            featureConfig->setValue("constant", constant);
        }

        Themer::setNextItemRightAlign("Linear", _spacing);
        Float linear = light->linear();
        if(ImGui::DragFloat("##Light.Linear", &linear, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setLinear(linear);
            featureConfig->setValue("linear", linear);
        }

        Themer::setNextItemRightAlign("Quadratic", _spacing);
        Float quadratic = light->quadratic();
        if(ImGui::DragFloat("##Light.Quadratic", &quadratic, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setQuadratic(quadratic);
            featureConfig->setValue("quadratic", quadratic);
        }
    }

    if(!headerIsOpen) {
        delete light;
        objectNode->objectConfig()->removeGroup(featureConfig);

        _panel->recreateShaders();
        _panel->resetLightsId();
    }
}

void Inspector::showMeshHeader(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    Mesh* mesh = getObjectFeature<Mesh>(objectNode->object());
    bool headerIsOpen = true;

    if(ImGui::CollapsingHeader("Mesh", &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
        Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
        ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
        bool reloadPrimitive = false;

        if(ImGui::TreeNodeEx("Primitive", nodeFlags)) {
            std::string primitiveType = "none";
            if(primitiveConfig) primitiveType = primitiveConfig->value("type");

            Themer::setNextItemRightAlign("Type", _spacing);
            if(ImGui::BeginCombo("##Mesh.Primitive.Type", snakeCaseToPhrase(primitiveType).c_str())) {
                const char* primitives[] = {"circle", "cube", "plane", "sphere"};
                for(const char* type: primitives) {
                    bool isSelected = false;

                    if(ImGui::Selectable(snakeCaseToPhrase(type).c_str(), isSelected)) {
                        /* Recreate the group to remove superfluous values */
                        featureConfig->removeGroup("primitive");
                        primitiveConfig = featureConfig->addGroup("primitive");

                        primitiveConfig->setValue("type", type);
                        reloadPrimitive = true;
                    }
                    if(isSelected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if(primitiveConfig) {
                ImGui::Text("Size");
                ImGui::SetNextItemWidth(-1);
                Vector3 size = mesh->size();
                if(ImGui::DragFloat3("##Mesh.Primitive.Size", size.data(), 0.002f)) {
                    mesh->setSize(size);
                    primitiveConfig->setValue("size", size);
                }

                if(primitiveType == "sphere") {
                    Themer::setNextItemRightAlign("Rings", _spacing);
                    Int rings = primitiveConfig->value<Int>("rings");
                    if(ImGui::DragInt("##Mesh.Primitive.Rings", &rings, 1.0f, 2, INT_MAX)) {
                        primitiveConfig->setValue("rings", rings);
                        reloadPrimitive = true;
                    }
                }

                if(primitiveType == "circle" || primitiveType == "sphere") {
                    Themer::setNextItemRightAlign("Segments", _spacing);
                    Int segments = primitiveConfig->value<Int>("segments");
                    if(ImGui::DragInt("##Mesh.Primitive.Segments", &segments, 1.0f, 3, INT_MAX)) {
                        primitiveConfig->setValue("segments", segments);
                        reloadPrimitive = true;
                    }
                }
            }
        }

        if(primitiveConfig && ImGui::TreeNodeEx("Material", nodeFlags)) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            ImGui::Text("Ambient color");
            ImGui::SetNextItemWidth(-1);
            Color4 ambientColor = mesh->ambientColor();
            if(ImGui::ColorEdit4("##Mesh.Material.AmbientColor", ambientColor.data())) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("ambient_color", ambientColor);
                mesh->setAmbientColor(ambientColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                Themer::setNextItemRightAlign("Ambient texture", _spacing);
                std::string ambientTexturePath;
                if(materialConfig) ambientTexturePath = materialConfig->value("ambient_texture");
                ImGui::InputText("##Mesh.Material.AmbientTexture", &ambientTexturePath, ImGuiInputTextFlags_ReadOnly);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("ambient_texture", fileNode->resourcePath());

                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), _projectPath);
                        mesh->setAmbientTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::Text("Diffuse color");
            ImGui::SetNextItemWidth(-1);
            Color4 diffuseColor = mesh->diffuseColor();
            if(ImGui::ColorEdit4("##Mesh.Material.DiffuseColor", diffuseColor.data())) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("diffuse_color", diffuseColor);
                mesh->setDiffuseColor(diffuseColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                Themer::setNextItemRightAlign("Diffuse texture", _spacing);
                std::string diffuseTexturePath;
                if(materialConfig) diffuseTexturePath = materialConfig->value("diffuse_texture");
                ImGui::InputText("##Mesh.Material.DiffuseTexture", &diffuseTexturePath, ImGuiInputTextFlags_ReadOnly);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("diffuse_texture", fileNode->resourcePath());

                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), _projectPath);
                        mesh->setDiffuseTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }

                Themer::setNextItemRightAlign("Normal texture", _spacing);
                std::string normalTexturePath;
                if(materialConfig) normalTexturePath = materialConfig->value("normal_texture");
                ImGui::InputText("##Mesh.Material.NormalTexture", &normalTexturePath, ImGuiInputTextFlags_ReadOnly);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("normal_texture", fileNode->resourcePath());

                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), _projectPath);
                        mesh->setNormalTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            ImGui::Text("Specular color");
            ImGui::SetNextItemWidth(-1);
            Color4 specularColor = mesh->specularColor();
            if(ImGui::ColorEdit4("##Mesh.Material.SpecularColor", specularColor.data())) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("specular_color", specularColor);
                mesh->setSpecularColor(specularColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                Themer::setNextItemRightAlign("Specular texture", _spacing);
                std::string specularTexturePath;
                if(materialConfig) specularTexturePath = materialConfig->value("specular_texture");
                ImGui::InputText("##Mesh.Material.SpecularTexture", &specularTexturePath, ImGuiInputTextFlags_ReadOnly);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("specular_texture", fileNode->resourcePath());

                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), _projectPath);
                        mesh->setSpecularTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            Themer::setNextItemRightAlign("Shininess", _spacing);
            Float shininess = mesh->shininess();
            if(ImGui::DragFloat("##Mesh.Material.Shininess", &shininess, 0.1f, 1.0f, FLT_MAX)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("shininess", shininess);
                mesh->setShininess(shininess);
            }
        }

        if(reloadPrimitive)
            _importer.updateMeshPrimitive(*mesh, primitiveConfig);
    }

    if(!headerIsOpen) {
        delete mesh;
        objectNode->objectConfig()->removeGroup(featureConfig);
        _panel->removeDrawableNode(objectNode);
    }
}

void Inspector::addResource(const std::string& resourcePath, const std::string& resourceType) {
    Utility::ConfigurationGroup* resourcesGroup = _panel->collectionConfig().group("external_resources");
    if(!resourcesGroup)
        resourcesGroup = _panel->collectionConfig().addGroup("external_resources");

    bool resourceAlreadyDefined = false;

    /* Check if the resource is already defined */
    for(auto& r: resourcesGroup->groups("resource")) {
        if(r->value("path") == resourcePath) {
            resourceAlreadyDefined = true;
            break;
        }
    }

    if(!resourceAlreadyDefined) {
        Utility::ConfigurationGroup* resourceGroup = resourcesGroup->addGroup("resource");
        resourceGroup->addValue("path", resourcePath);
        resourceGroup->addValue("type", resourceType);
    }
}

}}

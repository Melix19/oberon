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
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>
#include <Oberon/Mesh.h>

#include "CollectionPanel.h"
#include "FileNode.h"
#include "ObjectNode.h"
#include "Theme.h"

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
    const bool isVisible = ImGui::Begin("Inspector");

    /* Get the first selected node in the CollectionPanel, if it exists */
    ObjectNode* objectNode = nullptr;
    if(_panel) {
        const std::vector<ObjectNode*>& selectedNodes = _panel->selectedNodes();
        if(!selectedNodes.empty()) objectNode = selectedNodes.front();
    }

    /* If the window is not visible or there is no object node or the object
       node is the root node, end the method here */
    if(!isVisible || !objectNode || objectNode == _panel->rootNode()) {
        ImGui::End();
        return;
    }

    /* Transformation header */
    showTransformationHeader(objectNode);
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    /* Feature headers */
    for(Utility::ConfigurationGroup* featureConfig: objectNode->objectConfig()->groups("feature")) {
        const std::string type = featureConfig->value("type");
        if(type == "light") showLightHeader(objectNode, featureConfig);
        else if(type == "mesh") showMeshHeader(objectNode, featureConfig);

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
    }

    /* Separator */
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    /* "Add feature" button with dynamic width */
    const Float addFeatureButtonWidth = ImGui::GetWindowWidth()/2;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - addFeatureButtonWidth/2);
    if(ImGui::Button("Add feature", ImVec2(addFeatureButtonWidth, 0)))
        ImGui::OpenPopup("FeaturesPopup");

    /* Features popup */
    if(ImGui::BeginPopup("FeaturesPopup")) {
        const char* features[] = {"light", "mesh"};
        for(const char* feature: features) {
            if(ImGui::Selectable(snakeCaseToPhrase(feature).c_str())) {
                bool featureAlreadyPresent = false;

                for(Utility::ConfigurationGroup* featureConfig: objectNode->objectConfig()->groups("feature")) {
                    const std::string type = featureConfig->value("type");
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
        Vector3 translation = objectNode->object()->translation();
        if(Theme::dragFloat3("Translation", "Transformation.Translation", translation, 0.005f)) {
            objectNode->object()->setTranslation(translation);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Rotation (degrees) */
        Vector3 rotationDegree = objectNode->rotationDegree();
        if(Theme::dragFloat3("Rotation", "Transformation.Rotation", rotationDegree, 0.1f)) {
            objectNode->object()->setRotation(
                Quaternion::rotation(Rad{Deg{rotationDegree.z()}}, Vector3::zAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.y()}}, Vector3::yAxis())*
                Quaternion::rotation(Rad{Deg{rotationDegree.x()}}, Vector3::xAxis()));
            objectNode->setRotationDegree(rotationDegree);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }

        /* Scaling */
        Vector3 scaling = objectNode->object()->scaling();
        if(Theme::dragFloat3("Scaling", "Transformation.Scaling", scaling, 0.002f)) {
            objectNode->object()->setScaling(scaling);
            objectNode->objectConfig()->setValue("transformation", objectNode->object()->transformation());
        }
    }
}

void Inspector::showLightHeader(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    Light* light = getObjectFeature<Light>(objectNode->object());
    bool headerIsOpen = true;

    if(ImGui::CollapsingHeader("Light", &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
        Color4 color = light->color();
        if(Theme::colorEdit4("Color", "##Light.Color", color)) {
            light->setColor(color);
            featureConfig->setValue("color", color);
        }

        Float constant = light->constant();
        if(Theme::dragFloat("Contant", "Light.Contant", constant, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setConstant(constant);
            featureConfig->setValue("constant", constant);
        }

        Float linear = light->linear();
        if(Theme::dragFloat("Linear", "Light.Linear", linear, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setLinear(linear);
            featureConfig->setValue("linear", linear);
        }

        Float quadratic = light->quadratic();
        if(Theme::dragFloat("Quadratic", "Light.Quadratic", quadratic, 0.0001f, 0.0f, 0.0f, "%f")) {
            light->setQuadratic(quadratic);
            featureConfig->setValue("quadratic", quadratic);
        }
    }

    if(!headerIsOpen) {
        delete light;
        objectNode->objectConfig()->removeGroup(featureConfig);

        /* A light is removed so the shaders have to be recreated */
        _panel->recreateShaders();
        /* Also reset all the other lights id so all the ids are sequential */
        _panel->resetLightsId();
    }
}

void Inspector::showMeshHeader(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    Mesh* mesh = getObjectFeature<Mesh>(objectNode->object());
    bool headerIsOpen = true;

    if(ImGui::CollapsingHeader("Mesh", &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
        Utility::ConfigurationGroup* primitiveConfig = featureConfig->group("primitive");
        const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
        bool reloadPrimitive = false;

        if(ImGui::TreeNodeEx("Primitive", nodeFlags)) {
            std::string primitiveType = "none";
            if(primitiveConfig) primitiveType = primitiveConfig->value("type");

            if(Theme::beginCombo("Type", "##Mesh.Primitive.Type", snakeCaseToPhrase(primitiveType))) {
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
                Vector3 size = mesh->size();
                if(Theme::dragFloat3("Size", "##Mesh.Primitive.Size", size, 0.002f)) {
                    mesh->setSize(size);
                    primitiveConfig->setValue("size", size);
                }

                if(primitiveType == "sphere") {
                    Int rings = primitiveConfig->value<Int>("rings");
                    if(Theme::dragInt("Rings", "##Mesh.Primitive.Rings", rings, 1.0f, 2, INT_MAX)) {
                        primitiveConfig->setValue("rings", rings);
                        reloadPrimitive = true;
                    }
                }

                if(primitiveType == "circle" || primitiveType == "sphere") {
                    Int segments = primitiveConfig->value<Int>("segments");
                    if(Theme::dragInt("Segments", "##Mesh.Primitive.Segments", segments, 1.0f, 3, INT_MAX)) {
                        primitiveConfig->setValue("segments", segments);
                        reloadPrimitive = true;
                    }
                }
            }
        }

        if(primitiveConfig && ImGui::TreeNodeEx("Material", nodeFlags)) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            Color4 ambientColor = mesh->ambientColor();
            if(Theme::colorEdit4("Ambient color", "##Mesh.Material.AmbientColor", ambientColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("ambient_color", ambientColor);
                mesh->setAmbientColor(ambientColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                std::string ambientTexturePath;
                if(materialConfig) ambientTexturePath = materialConfig->value("ambient_texture");
                Theme::inputText("Ambient texture", "##Mesh.Material.AmbientTexture", ambientTexturePath);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("ambient_texture", fileNode->resourcePath());

                        Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                        mesh->setAmbientTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            Color4 diffuseColor = mesh->diffuseColor();
            if(Theme::colorEdit4("Diffuse color", "##Mesh.Material.DiffuseColor", diffuseColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("diffuse_color", diffuseColor);
                mesh->setDiffuseColor(diffuseColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                std::string diffuseTexturePath;
                if(materialConfig) diffuseTexturePath = materialConfig->value("diffuse_texture");
                Theme::inputText("Diffuse texture", "##Mesh.Material.DiffuseTexture", diffuseTexturePath);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("diffuse_texture", fileNode->resourcePath());

                        Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                        mesh->setDiffuseTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }

                std::string normalTexturePath;
                if(materialConfig) normalTexturePath = materialConfig->value("normal_texture");
                Theme::inputText("Normal texture", "##Mesh.Material.NormalTexture", normalTexturePath);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("normal_texture", fileNode->resourcePath());

                        Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                        mesh->setNormalTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            Color4 specularColor = mesh->specularColor();
            if(Theme::colorEdit4("Specular color", "##Mesh.Material.SpecularColor", specularColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("specular_color", specularColor);
                mesh->setSpecularColor(specularColor);
            }

            if(primitiveConfig->value("type") != "cube") {
                std::string specularTexturePath;
                if(materialConfig) specularTexturePath = materialConfig->value("specular_texture");
                Theme::inputText("Specular texture", "##Mesh.Material.SpecularTexture", specularTexturePath);
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                        IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                        const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                        if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                        addResource(fileNode->resourcePath(), "Texture2D");
                        materialConfig->setValue("specular_texture", fileNode->resourcePath());

                        Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                        Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                        mesh->setSpecularTexture(textureResource);
                        _panel->updateShader(*mesh);

                        reloadPrimitive = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            Float shininess = mesh->shininess();
            if(Theme::dragFloat("Shininess", "Mesh.Material.Shininess", shininess, 0.1f, 1.0f, FLT_MAX)) {
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

        /* A drawable is removed so let's tell the CollectionPanel to remove the node to
           make the picking work */
        _panel->removeDrawableNode(objectNode);
    }
}

void Inspector::addResource(const std::string& resourcePath, const std::string& resourceType) {
    Utility::ConfigurationGroup* resourcesGroup = _panel->collectionConfig().group("external_resources");
    if(!resourcesGroup)
        resourcesGroup = _panel->collectionConfig().addGroup("external_resources");

    /* Check if the resource is already defined */
    bool resourceAlreadyDefined = false;
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

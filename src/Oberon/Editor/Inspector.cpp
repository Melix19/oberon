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
#include <Oberon/MeshRenderer.h>

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
        else if(type == "mesh_renderer") showMeshRendererHeader(objectNode, featureConfig);

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
        const char* features[] = {"light", "mesh_renderer"};
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
        if(Theme::colorEdit4("Color", "Light.Color", color)) {
            light->setColor(color);
            featureConfig->setValue("color", color);
        }

        Float constant = light->constant();
        if(Theme::dragFloat("Constant", "Light.Constant", constant, 0.0001f, 0.0f, 0.0f, "%f")) {
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

void Inspector::showMeshRendererHeader(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    MeshRenderer* meshRenderer = getObjectFeature<MeshRenderer>(objectNode->object());
    bool headerIsOpen = true;

    if(ImGui::CollapsingHeader("Mesh renderer", &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen)) {
        Utility::ConfigurationGroup* meshConfiguration = featureConfig->group("mesh");
        const ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;

        if(ImGui::TreeNodeEx("Mesh", nodeFlags)) {
            std::string primitiveType = meshConfiguration ? meshConfiguration->value("type") : "none";
            bool updatePrimitive = false;

            if(Theme::beginCombo("Type", "MeshRenderer.Mesh.Type", snakeCaseToPhrase(primitiveType))) {
                const char* primitives[] = {"capsule", "circle", "cone", "cylinder", "plane", "sphere"};
                for(const char* type: primitives) {
                    bool isSelected = false;

                    if(ImGui::Selectable(snakeCaseToPhrase(type).c_str(), isSelected)) {
                        primitiveType = type;

                        /* Recreate the group to remove superfluous values */
                        featureConfig->removeGroup("mesh");
                        meshConfiguration = featureConfig->addGroup("mesh");
                        meshConfiguration->setValue("type", primitiveType);

                        if(primitiveType == "capsule") {
                            meshConfiguration->setValue("radius", 1.0f);
                            meshConfiguration->setValue("length", 2.0f);
                            meshConfiguration->setValue("hemisphereRings", 8);
                            meshConfiguration->setValue("cylinderRings", 4);
                            meshConfiguration->setValue("segments", 64);
                        } else if(primitiveType == "circle") {
                            meshConfiguration->setValue("radius", 1.0f);
                            meshConfiguration->setValue("segments", 64);
                        } else if(primitiveType == "cone") {
                            meshConfiguration->setValue("radius", 1.0f);
                            meshConfiguration->setValue("length", 2.0f);
                            meshConfiguration->setValue("rings", 4);
                            meshConfiguration->setValue("segments", 64);
                            meshConfiguration->setValue("capEnd", true);
                        } else if(primitiveType == "cylinder") {
                            meshConfiguration->setValue("radius", 1.0f);
                            meshConfiguration->setValue("length", 2.0f);
                            meshConfiguration->setValue("rings", 4);
                            meshConfiguration->setValue("segments", 64);
                            meshConfiguration->setValue("capEnds", true);
                        } else if(primitiveType == "plane") {
                            meshConfiguration->setValue("size", Vector2{2.0f});
                        } else if(primitiveType == "sphere") {
                            meshConfiguration->setValue("radius", 1.0f);
                            meshConfiguration->setValue("rings", 32);
                            meshConfiguration->setValue("segments", 64);
                        }

                        updatePrimitive = true;
                    }
                    if(isSelected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if(primitiveType == "capsule") {
                Float radius = meshConfiguration->value<Float>("radius");
                if(Theme::dragFloat("Radius", "MeshRenderer.Mesh.Radius", radius, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("radius", radius);
                    updatePrimitive = true;
                }

                Float length = meshConfiguration->value<Float>("length");
                if(Theme::dragFloat("Length", "MeshRenderer.Mesh.Length", length, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("length", length);
                    updatePrimitive = true;
                }

                Int hemisphereRings = meshConfiguration->value<Int>("hemisphereRings");
                if(Theme::dragInt("Hemisphere rings", "MeshRenderer.Mesh.HemisphereRings", hemisphereRings, 1.0f, 1, INT_MAX)) {
                    meshConfiguration->setValue("hemisphereRings", hemisphereRings);
                    updatePrimitive = true;
                }

                Int cylinderRings = meshConfiguration->value<Int>("cylinderRings");
                if(Theme::dragInt("Cylinder rings", "MeshRenderer.Mesh.CylinderRings", cylinderRings, 1.0f, 1, INT_MAX)) {
                    meshConfiguration->setValue("cylinderRings", cylinderRings);
                    updatePrimitive = true;
                }

                Int segments = meshConfiguration->value<Int>("segments");
                if(Theme::dragInt("Segments", "MeshRenderer.Mesh.Segments", segments, 1.0f, 3, INT_MAX)) {
                    meshConfiguration->setValue("segments", segments);
                    updatePrimitive = true;
                }
            } else if(primitiveType == "circle") {
                Float radius = meshConfiguration->value<Float>("radius");
                if(Theme::dragFloat("Radius", "MeshRenderer.Mesh.Radius", radius, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("radius", radius);
                    updatePrimitive = true;
                }

                Int segments = meshConfiguration->value<Int>("segments");
                if(Theme::dragInt("Segments", "MeshRenderer.Mesh.Segments", segments, 1.0f, 3, INT_MAX)) {
                    meshConfiguration->setValue("segments", segments);
                    updatePrimitive = true;
                }
            } else if(primitiveType == "cone") {
                Float radius = meshConfiguration->value<Float>("radius");
                if(Theme::dragFloat("Radius", "MeshRenderer.Mesh.Radius", radius, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("radius", radius);
                    updatePrimitive = true;
                }

                Float length = meshConfiguration->value<Float>("length");
                if(Theme::dragFloat("Length", "MeshRenderer.Mesh.Length", length, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("length", length);
                    updatePrimitive = true;
                }

                Int rings = meshConfiguration->value<Int>("rings");
                if(Theme::dragInt("Rings", "MeshRenderer.Mesh.Rings", rings, 1.0f, 1, INT_MAX)) {
                    meshConfiguration->setValue("rings", rings);
                    updatePrimitive = true;
                }

                Int segments = meshConfiguration->value<Int>("segments");
                if(Theme::dragInt("Segments", "MeshRenderer.Mesh.Segments", segments, 1.0f, 3, INT_MAX)) {
                    meshConfiguration->setValue("segments", segments);
                    updatePrimitive = true;
                }

                bool capEnd = meshConfiguration->value<bool>("capEnd");
                if(Theme::checkbox("Cap end", "MeshRenderer.Mesh.CapEnd", capEnd)) {
                    meshConfiguration->setValue("capEnd", capEnd);
                    updatePrimitive = true;
                }
            } else if(primitiveType == "cylinder") {
                Float radius = meshConfiguration->value<Float>("radius");
                if(Theme::dragFloat("Radius", "MeshRenderer.Mesh.Radius", radius, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("radius", radius);
                    updatePrimitive = true;
                }

                Float length = meshConfiguration->value<Float>("length");
                if(Theme::dragFloat("Length", "MeshRenderer.Mesh.Length", length, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("length", length);
                    updatePrimitive = true;
                }

                Int rings = meshConfiguration->value<Int>("rings");
                if(Theme::dragInt("Rings", "MeshRenderer.Mesh.Rings", rings, 1.0f, 1, INT_MAX)) {
                    meshConfiguration->setValue("rings", rings);
                    updatePrimitive = true;
                }

                Int segments = meshConfiguration->value<Int>("segments");
                if(Theme::dragInt("Segments", "MeshRenderer.Mesh.Segments", segments, 1.0f, 3, INT_MAX)) {
                    meshConfiguration->setValue("segments", segments);
                    updatePrimitive = true;
                }

                bool capEnds = meshConfiguration->value<bool>("capEnds");
                if(Theme::checkbox("Cap ends", "MeshRenderer.Mesh.CapEnds", capEnds)) {
                    meshConfiguration->setValue("capEnds", capEnds);
                    updatePrimitive = true;
                }
            } else if(primitiveType == "plane") {
                Vector2 size = meshConfiguration->value<Vector2>("size");
                if(Theme::dragFloat2("Size", "MeshRenderer.Mesh.Size", size, 0.001f)) {
                    meshConfiguration->setValue("size", size);
                    updatePrimitive = true;
                }
            } else if(primitiveType == "sphere") {
                Float radius = meshConfiguration->value<Float>("radius");
                if(Theme::dragFloat("Radius", "MeshRenderer.Mesh.Radius", radius, 0.001f, 0.0f, FLT_MAX)) {
                    meshConfiguration->setValue("radius", radius);
                    updatePrimitive = true;
                }

                Int rings = meshConfiguration->value<Int>("rings");
                if(Theme::dragInt("Rings", "MeshRenderer.Mesh.Rings", rings, 1.0f, 2, INT_MAX)) {
                    meshConfiguration->setValue("rings", rings);
                    updatePrimitive = true;
                }

                Int segments = meshConfiguration->value<Int>("segments");
                if(Theme::dragInt("Segments", "MeshRenderer.Mesh.Segments", segments, 1.0f, 3, INT_MAX)) {
                    meshConfiguration->setValue("segments", segments);
                    updatePrimitive = true;
                }
            }

            if(updatePrimitive) _importer.generateMeshPrimitive(*meshRenderer, meshConfiguration);
        }

        if(meshConfiguration && ImGui::TreeNodeEx("Material", nodeFlags)) {
            Utility::ConfigurationGroup* materialConfig = featureConfig->group("material");

            Color4 ambientColor = meshRenderer->ambientColor();
            if(Theme::colorEdit4("Ambient color", "MeshRenderer.Material.AmbientColor", ambientColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("ambient_color", ambientColor);
                meshRenderer->setAmbientColor(ambientColor);
            }

            std::string ambientTexturePath;
            if(materialConfig) ambientTexturePath = materialConfig->value("ambient_texture");
            Theme::inputText("Ambient texture", "MeshRenderer.Material.AmbientTexture", ambientTexturePath);
            if(ImGui::BeginDragDropTarget()) {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                    IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                    const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                    if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                    addResource(fileNode->resourcePath(), "Texture2D");
                    materialConfig->setValue("ambient_texture", fileNode->resourcePath());

                    Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                    Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                    meshRenderer->setAmbientTexture(textureResource);
                    _panel->updateShader(*meshRenderer);
                }
                ImGui::EndDragDropTarget();
            }

            Color4 diffuseColor = meshRenderer->diffuseColor();
            if(Theme::colorEdit4("Diffuse color", "MeshRenderer.Material.DiffuseColor", diffuseColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("diffuse_color", diffuseColor);
                meshRenderer->setDiffuseColor(diffuseColor);
            }

            std::string diffuseTexturePath;
            if(materialConfig) diffuseTexturePath = materialConfig->value("diffuse_texture");
            Theme::inputText("Diffuse texture", "MeshRenderer.Material.DiffuseTexture", diffuseTexturePath);
            if(ImGui::BeginDragDropTarget()) {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                    IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                    const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                    if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                    addResource(fileNode->resourcePath(), "Texture2D");
                    materialConfig->setValue("diffuse_texture", fileNode->resourcePath());

                    Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                    Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                    meshRenderer->setDiffuseTexture(textureResource);
                    _panel->updateShader(*meshRenderer);
                }
                ImGui::EndDragDropTarget();
            }

            std::string normalTexturePath;
            if(materialConfig) normalTexturePath = materialConfig->value("normal_texture");
            Theme::inputText("Normal texture", "MeshRenderer.Material.NormalTexture", normalTexturePath);
            if(ImGui::BeginDragDropTarget()) {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                    IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                    const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                    if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                    addResource(fileNode->resourcePath(), "Texture2D");
                    materialConfig->setValue("normal_texture", fileNode->resourcePath());

                    Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                    Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                    meshRenderer->setNormalTexture(textureResource);
                    _panel->updateShader(*meshRenderer);
                }
                ImGui::EndDragDropTarget();
            }

            Color4 specularColor = meshRenderer->specularColor();
            if(Theme::colorEdit4("Specular color", "MeshRenderer.Material.SpecularColor", specularColor)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("specular_color", specularColor);
                meshRenderer->setSpecularColor(specularColor);
            }

            std::string specularTexturePath;
            if(materialConfig) specularTexturePath = materialConfig->value("specular_texture");
            Theme::inputText("Specular texture", "MeshRenderer.Material.SpecularTexture", specularTexturePath);
            if(ImGui::BeginDragDropTarget()) {
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Image")) {
                    IM_ASSERT(payload->DataSize == sizeof(FileNode*));
                    const FileNode* fileNode = *static_cast<const FileNode**>(payload->Data);

                    if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                    addResource(fileNode->resourcePath(), "Texture2D");
                    materialConfig->setValue("specular_texture", fileNode->resourcePath());

                    Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, fileNode->resourcePath()));
                    Resource<GL::Texture2D> textureResource = _importer.loadTexture(fileNode->resourcePath(), data);
                    meshRenderer->setSpecularTexture(textureResource);
                    _panel->updateShader(*meshRenderer);
                }
                ImGui::EndDragDropTarget();
            }

            Float shininess = meshRenderer->shininess();
            if(Theme::dragFloat("Shininess", "MeshRenderer.Material.Shininess", shininess, 0.1f, 1.0f, FLT_MAX)) {
                if(!materialConfig) materialConfig = featureConfig->addGroup("material");
                materialConfig->setValue("shininess", shininess);
                meshRenderer->setShininess(shininess);
            }
        }
    }

    if(!headerIsOpen) {
        delete meshRenderer;
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

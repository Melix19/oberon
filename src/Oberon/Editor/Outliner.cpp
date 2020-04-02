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

#include "Outliner.h"

#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Corrade/Utility/ConfigurationGroup.h>

#include "CollectionPanel.h"
#include "ObjectNode.h"

namespace {

Utility::ConfigurationGroup* moveObjectConfig(ObjectNode* source, ObjectNode* target) {
    Utility::ConfigurationGroup* newConfig = target->objectConfig()->addGroup("child");
    *newConfig = std::move(*source->objectConfig());
    source->parent()->objectConfig()->removeGroup(source->objectConfig());
    return newConfig;
}

}

void Outliner::newFrame() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool isVisible = ImGui::Begin("Outliner");
    ImGui::PopStyleVar();

    /* If the window is not visible, end the method here. */
    if(!isVisible || !_panel) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    displayNode(_panel->rootNode());
    ImGui::PopStyleVar();

    ImGui::End();

    if(_deleteSelectedNodes) {
        deleteSelectedNodes();
        _deleteSelectedNodes = false;
    }

    applyDragDrop();
}

void Outliner::displayNode(ObjectNode* node) {
    if(node == _editNode && _editNodeMode == EditMode::Rename)
        displayEditNode(node);
    else displayObjectNode(node);
}

void Outliner::displayObjectNode(ObjectNode* node) {
    std::string nodeName;
    if(node == _panel->rootNode()) nodeName = "scene";
    else nodeName = node->objectConfig()->value("name");

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_OpenOnArrow;

    bool hasChildren = !node->children().empty();
    if(!hasChildren) nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    if(node->isSelected()) nodeFlags |= ImGuiTreeNodeFlags_Selected;

    bool isOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

    if(ImGui::BeginDragDropSource()) {
        if(!_isDraggingNodes) {
            if(!node->isSelected()) {
                auto& selectedNodes = _panel->selectedNodes();
                if(!selectedNodes.empty()) deselectAllNodes();
                selectNode(node);
            }

            _isDraggingNodes = true;
        }

        ImGui::SetDragDropPayload("ObjectNode", &node, sizeof(FileNode*));
        ImGui::Text(nodeName.c_str());
        ImGui::EndDragDropSource();
    } else _isDraggingNodes = false;

    if(ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ObjectNode");
        if(payload) _dragDropTarget = node;

        ImGui::EndDragDropTarget();
    }

    if(!_dragDropTarget && ImGui::IsItemHovered() && (ImGui::IsMouseReleased(0) || ImGui::IsMouseReleased(1))) {
        auto& selectedNodes = _panel->selectedNodes();
        ImGuiIO& io = ImGui::GetIO();

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        const bool isShortcutKey = (io.ConfigMacOSXBehaviors ? (io.KeySuper &&
            !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if(isShortcutKey && ImGui::IsMouseReleased(0)) {
            if(node->isSelected()) {
                selectedNodes.erase(std::find_if(selectedNodes.begin(),
                    selectedNodes.end(), [&node](ObjectNode* n) { return n == node; }));
            } else selectedNodes.push_back(node);

            node->setSelected(!node->isSelected());
        } else if(!node->isSelected() || ImGui::IsMouseReleased(0)) {
            if(!selectedNodes.empty()) deselectAllNodes();
            selectNode(node);
        }
    }

    if(ImGui::BeginPopupContextItem()) {
        if(ImGui::MenuItem("Add child")) {
            _editNode = node;
            _editNodeMode = EditMode::ObjectCreation;
            _editNodeText = "";
            _editNodeNeedsFocus = true;
        }

        if(node != _panel->rootNode()) {
            if(ImGui::MenuItem("Rename")) {
                _editNode = node;
                _editNodeMode = EditMode::Rename;
                _editNodeText = nodeName;
                _editNodeNeedsFocus = true;
            }

            if(ImGui::MenuItem("Delete")) _deleteSelectedNodes = true;
        }

        ImGui::EndPopup();
    }

    if(isOpen) {
        for(auto& child: node->children())
            displayNode(child.get());

        if(node == _editNode && _editNodeMode != EditMode::Rename)
            displayEditNode(node);

        ImGui::TreePop();
    }
}

void Outliner::displayEditNode(ObjectNode* node) {
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() -
        ImGui::GetTreeNodeToLabelSpacing());

    if(_editNodeNeedsFocus) ImGui::SetKeyboardFocusHere();

    if(ImGui::InputText("##ObjectName", &_editNodeText, ImGuiInputTextFlags_EnterReturnsTrue)) {
        switch(_editNodeMode) {
            case EditMode::ObjectCreation: {
                Utility::ConfigurationGroup* childConfig = node->objectConfig()->addGroup("child");
                childConfig->setValue("name", _editNodeText);

                Object3D* child = new Object3D{node->object()};
                node->addChild(child, childConfig);
            } break;
            case EditMode::Rename: {
                node->objectConfig()->setValue("name", _editNodeText);
            } break;
        }
    }

    /* Delay the focus check on the next frame
        otherwise it will be deleted right away. */
    if(_editNodeNeedsFocus) _editNodeNeedsFocus = false;
    else if(!ImGui::IsItemActive()) _editNode = nullptr;

    if(_editNodeMode == EditMode::Rename)
        for(auto& child: node->children())
            displayNode(child.get());

    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
}

void Outliner::selectNode(ObjectNode* node) {
    auto& selectedNodes = _panel->selectedNodes();
    selectedNodes.push_back(node);
    node->setSelected(true);
}

void Outliner::deselectAllNodes() {
    auto& selectedNodes = _panel->selectedNodes();
    for(auto& selectedNode: selectedNodes)
        selectedNode->setSelected(false);
    selectedNodes.clear();
}

void Outliner::applyDragDrop() {
    if(_dragDropTarget) {
        auto& selectedNodes = _panel->selectedNodes();
        for(auto& selectedNode: selectedNodes) {
            if(selectedNode->parent() == _dragDropTarget) continue;

            auto& nodeParentChildren = selectedNode->parent()->children();
            auto found = std::find_if(nodeParentChildren.begin(),
                nodeParentChildren.end(), [&selectedNode](Containers::Pointer<ObjectNode>& n) {
                    return n.get() == selectedNode; });

            if(found != nodeParentChildren.end()) {
                Utility::ConfigurationGroup* childConfig = moveObjectConfig(selectedNode, _dragDropTarget);
                selectedNode->setObjectConfig(childConfig);
                selectedNode->object()->setParent(_dragDropTarget->object());
                selectedNode->setParent(_dragDropTarget);

                _dragDropTarget->children().push_back(std::move(*found));
                nodeParentChildren.erase(found);
            }
        }

        _dragDropTarget = nullptr;
    }
}

void Outliner::deleteSelectedNodes() {
    auto& selectedNodes = _panel->selectedNodes();
    if(!selectedNodes.empty()) {
        for(auto& selectedNode: selectedNodes) {
            delete selectedNode->object();
            selectedNode->parent()->objectConfig()->removeGroup(selectedNode->objectConfig());

            auto& parentChildren = selectedNode->parent()->children();
            parentChildren.erase(std::find_if(parentChildren.begin(), parentChildren.end(),
                [&selectedNode](Containers::Pointer<ObjectNode>& n) {
                    return n.get() == selectedNode;
                }));
        }

        selectedNodes.clear();
    }
}

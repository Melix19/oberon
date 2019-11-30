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

#include "Outliner.h"

#include <algorithm>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

void Outliner::newFrame() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool isVisible = ImGui::Begin("Outliner");
    ImGui::PopStyleVar();

    /* If the window is not visible, just end the method here. */
    if(!isVisible || !_panel) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    displayObjectTree(_panel->rootNode(), true);
    ImGui::PopStyleVar();

    ImGui::End();

    if(_deleteSelectedNodes) {
        auto& selectedNodes = _panel->selectedNodes();

        if(!selectedNodes.empty()) {
            for(auto& selectedNode : selectedNodes) {
                delete selectedNode->object();

                std::string name = selectedNode->objectConfig()->value("name");
                ObjectNode* parent =  selectedNode->parent();

                parent->objectConfig()->removeGroup(selectedNode->objectConfig());

                auto found = std::find_if(parent->children().begin(), parent->children().end(),
                    [&](Containers::Pointer<ObjectNode>& p) { return p.get() == selectedNode; });

                CORRADE_INTERNAL_ASSERT(found != parent->children().end());

                parent->children().erase(found);
            }

            selectedNodes.clear();
        }

        _deleteSelectedNodes = false;
    }
}

void Outliner::displayObjectTree(ObjectNode* node, bool isRoot) {
    if(node == _editNode && _editNodeMode == EditMode::Rename) {
        displayEditNode(node);
        return;
    }

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    std::string nodeName = node->objectConfig()->value("name");
    bool hasChildren = !node->children().empty();

    if(node->isSelected()) nodeFlags |= ImGuiTreeNodeFlags_Selected;

    if(!hasChildren) nodeFlags |= ImGuiTreeNodeFlags_Leaf;

    bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

    if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
        auto& selectedNodes = _panel->selectedNodes();
        ImGuiIO& io = ImGui::GetIO();

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        const bool isShortcutKey = (io.ConfigMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) :
            (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if(isShortcutKey && ImGui::IsItemClicked(0)) {
            if(node->isSelected()) {
                auto found = std::find_if(selectedNodes.begin(), selectedNodes.end(),
                    [&](ObjectNode* p) { return p == node; });

                CORRADE_INTERNAL_ASSERT(found != selectedNodes.end());

                selectedNodes.erase(found);
            } else selectedNodes.push_back(node);

            node->setSelected(!node->isSelected());
        } else {
            if(!node->isSelected() || ImGui::IsItemClicked(0)) {
                if(!selectedNodes.empty()) {
                    for(auto& selectedNode : selectedNodes)
                        selectedNode->setSelected(false);

                    selectedNodes.clear();
                }

                selectedNodes.push_back(node);
                node->setSelected(true);
            }
        }
    }

    if(ImGui::BeginPopupContextItem()) {
        if(ImGui::MenuItem("Add child")) {
            _editNode = node;
            _editNodeMode = EditMode::ObjectCreation;
            _editNodeText = "";
            _editNodeNeedsFocus = true;
        }

        if(ImGui::MenuItem("Rename")) {
            _editNode = node;
            _editNodeMode = EditMode::Rename;
            _editNodeText = nodeName;
            _editNodeNeedsFocus = true;
        }

        if(!isRoot && ImGui::MenuItem("Delete")) _deleteSelectedNodes = true;

        ImGui::EndPopup();
    }

    if(nodeOpen) {
        for(auto& child : node->children())
            displayObjectTree(child.get(), false);

        if(node == _editNode && _editNodeMode != EditMode::Rename)
            displayEditNode(node);

        ImGui::TreePop();
    }
}

void Outliner::displayEditNode(ObjectNode* node) {
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() -
        ImGui::GetTreeNodeToLabelSpacing());

    /* Set focus in it's first frame. */
    if(_editNodeNeedsFocus) ImGui::SetKeyboardFocusHere();

    if(ImGui::InputText("##ObjectName", &_editNodeText, ImGuiInputTextFlags_EnterReturnsTrue)) {
        switch (_editNodeMode) {
            case EditMode::ObjectCreation: {
                /* Add the new ObjectNode. */
                Utility::ConfigurationGroup* childConfig = node->objectConfig()->addGroup("child");
                childConfig->setValue("name", _editNodeText);

                Object3D* child = new Object3D{node->object()};
                ObjectNode* childNode = node->addChild(child, childConfig);
                _panel->resetObjectAndChildren(childNode);
            } break;
            case EditMode::Rename: {
                node->objectConfig()->setValue("name", _editNodeText);
            } break;
        }
    }

    /* In the first frame, delay the focus check on the next frame
        otherwise it will be deleted right away. */
    if(_editNodeNeedsFocus) _editNodeNeedsFocus = false;
    else if(!ImGui::IsItemActive()) _editNode = nullptr;

    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
}

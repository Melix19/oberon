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

#include "Hierarchy.hpp"

void Hierarchy::newFrame() {
    _clickedNode = nullptr;

    bool showTree = false;

    if(_panel) {
        EntityNode* rootNode = &_panel->rootNode();
        showTree = !rootNode->children().empty() || rootNode == _editNode;
    }

    if(showTree) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool isVisible = ImGui::Begin("Hierarchy");

    /* If the window is not visible, just end the method here. */
    if(!isVisible) {
        if(showTree) ImGui::PopStyleVar();
        ImGui::End();
        return;
    }

    if(_panel) {
        EntityNode* rootNode = &_panel->rootNode();

        if(showTree) {
            ImGui::PopStyleVar();

            if(rootNode == _editNode) displayEditNode(rootNode);
            else {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                displayEntityTree(rootNode->children().front().get());
                ImGui::PopStyleVar();
            }
        } else {
            ImGui::Text("Create root entity:");

            if(ImGui::Button("Entity")) {
                _editNode = rootNode;
                _editNodeMode = EditMode::EntityCreation;
                _editNodeText = "";
                _editNodeNeedsFocus = true;
            }
        }
    }

    ImGui::End();

    if(_deleteSelectedNodes) {
        if(!_selectedNodes.empty()) {
            for(auto& selectedNode : _selectedNodes) {
                auto& parentChildren = selectedNode->parent()->children();
                auto found = std::find_if(parentChildren.begin(), parentChildren.end(),
                    [&](Containers::Pointer<EntityNode>& p) { return p.get() == selectedNode; });

                CORRADE_INTERNAL_ASSERT(found != parentChildren.end());

                parentChildren.erase(found);
            }

            _selectedNodes.clear();
        }

        _deleteSelectedNodes = false;
    }
}

void Hierarchy::clearContent() {
    _panel = nullptr;

    for(auto& selectedNode : _selectedNodes)
        selectedNode->setSelected(false);

    _selectedNodes.clear();
}

void Hierarchy::displayEntityTree(EntityNode* node) {
    if(node == _editNode && _editNodeMode == EditMode::Rename) {
        displayEditNode(node);
        return;
    }

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth |
        ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    std::string nodeName = node->jsonEntity()["name"].GetString();
    bool hasChildren = !node->children().empty();

    if(node->isSelected()) nodeFlags |= ImGuiTreeNodeFlags_Selected;

    if(!hasChildren) nodeFlags |= ImGuiTreeNodeFlags_Leaf;

    bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

    if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
        ImGuiIO& io = ImGui::GetIO();

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        const bool isShortcutKey = (io.ConfigMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) :
            (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if(isShortcutKey && ImGui::IsItemClicked(0)) {
            if(node->isSelected()) {
                auto found = std::find_if(_selectedNodes.begin(), _selectedNodes.end(),
                    [&](EntityNode* p) { return p == node; });

                CORRADE_INTERNAL_ASSERT(found != _selectedNodes.end());

                _selectedNodes.erase(found);
            } else _selectedNodes.push_back(node);

            node->setSelected(!node->isSelected());
        } else {
            if(!node->isSelected() || ImGui::IsItemClicked(0)) {
                if(!_selectedNodes.empty()) {
                    for(auto& selectedNode : _selectedNodes)
                        selectedNode->setSelected(false);

                    _selectedNodes.clear();
                }

                _selectedNodes.push_back(node);
                node->setSelected(true);
            }

            if(ImGui::IsItemClicked(0)) _clickedNode = node;
        }
    }

    if(ImGui::BeginPopupContextItem()) {
        if(ImGui::MenuItem("Add child")) {
            _editNode = node;
            _editNodeMode = EditMode::EntityCreation;
            _editNodeText = "";
            _editNodeNeedsFocus = true;
        }

        if(ImGui::MenuItem("Rename")) {
            _editNode = node;
            _editNodeMode = EditMode::Rename;
            _editNodeText = nodeName;
            _editNodeNeedsFocus = true;
        }

        if(ImGui::MenuItem("Delete")) _deleteSelectedNodes = true;

        ImGui::EndPopup();
    }

    if(nodeOpen) {
        for(auto& child : node->children())
            displayEntityTree(child.get());

        if(node == _editNode && _editNodeMode != EditMode::Rename)
            displayEditNode(node);

        ImGui::TreePop();
    }
}

void Hierarchy::displayEditNode(EntityNode* node) {
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() -
        ImGui::GetTreeNodeToLabelSpacing());

    /* Set focus in it's first frame. */
    if(_editNodeNeedsFocus) ImGui::SetKeyboardFocusHere();

    if(ImGui::InputText("##FileName", &_editNodeText, ImGuiInputTextFlags_EnterReturnsTrue)) {
        switch (_editNodeMode) {
            case EditMode::EntityCreation:
                /* Add the new EntityNode. */
                _panel->addEntityNodeChild(_editNodeText, node);
                break;
            case EditMode::Rename: {
                Entity* entity_cast = static_cast<Entity*>(node->entity()->features().first());
                entity_cast->setName(_editNodeText);

                Document& document = _panel->jsonDocument();
                node->jsonEntity()["name"].SetString(_editNodeText.c_str(), document.GetAllocator());
            } break;
        }
    }

    /* In the first frame, delay the focus check on the next frame
        otherwise it will be deleted right away. */
    if(_editNodeNeedsFocus) _editNodeNeedsFocus = false;
    else if(!ImGui::IsItemActive()) _editNode = nullptr;

    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
}

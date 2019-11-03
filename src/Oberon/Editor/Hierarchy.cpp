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

Hierarchy::Hierarchy()
    : collection_panel_ptr(nullptr)
    , clicked_node_ptr(nullptr)
    , delete_selected_nodes(false)
    , edit_node_ptr(nullptr)
{
}

void Hierarchy::newFrame()
{
    if (collection_panel_ptr && !collection_panel_ptr->root_node.children.empty())
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Hierarchy");

    if (collection_panel_ptr) {
        if (!collection_panel_ptr->root_node.children.empty()) {
            ImGui::PopStyleVar();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            displayEntityTree(collection_panel_ptr->root_node.children.front().get());
            ImGui::PopStyleVar();
        } else {
            ImGui::Text("Create root entity:");

            if (ImGui::Button("Entity")) {
                edit_node_ptr = collection_panel_ptr->root_node.addChild();
                edit_node_mode = EditMode::EntityCreation;
                edit_node_string = "";
                edit_node_needs_focus = true;
            }
        }
    }

    ImGui::End();

    if (delete_selected_nodes) {
        if (!selected_nodes.empty()) {
            for (auto& selected_node : selected_nodes) {
                auto& parent_children = selected_node->parent->children;
                auto found = std::find_if(parent_children.begin(), parent_children.end(), [&](Containers::Pointer<EntityNode>& p) { return p.get() == selected_node; });
                assert(found != parent_children.end());

                parent_children.erase(found);
            }

            selected_nodes.clear();
        }

        delete_selected_nodes = false;
    }
}

void Hierarchy::clearContent()
{
    collection_panel_ptr = nullptr;
    clicked_node_ptr = nullptr;

    for (auto& selected_node_ptr : selected_nodes)
        selected_node_ptr->is_selected = false;

    selected_nodes.clear();
}

void Hierarchy::displayEntityTree(EntityNode* node_ptr)
{
    if (node_ptr == edit_node_ptr) {
        ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - ImGui::GetTreeNodeToLabelSpacing());
        bool success = false;

        if (edit_node_needs_focus)
            ImGui::SetKeyboardFocusHere(); // Set focus when it's created

        if (ImGui::InputText("##FileName", &edit_node_string, ImGuiInputTextFlags_EnterReturnsTrue)) {
            switch (edit_node_mode) {
                case EditMode::EntityCreation: {
                    EntityNode* parent = node_ptr->parent;

                    // Remove EditNode
                    auto& parent_children = parent->children;
                    parent_children.erase(parent_children.end() - 1);

                    // Add the new EntityNode
                    collection_panel_ptr->addEntityNodeChild(edit_node_string, parent);
                    success = true;
                    break;
                }
                case EditMode::Rename: {
                    Entity* entity_cast = static_cast<Entity*>(node_ptr->entity_ptr->features().first());
                    entity_cast->setName(edit_node_string);

                    (*node_ptr->j_entity_ptr)["name"].SetString(edit_node_string.c_str(), collection_panel_ptr->jsonDocument.GetAllocator());
                    success = true;
                    break;
                }
            }
        }

        if (edit_node_needs_focus)
            edit_node_needs_focus = false;
        else if (!ImGui::IsItemActive()) {
            if (!success && edit_node_mode != EditMode::Rename) {
                auto& parent_children = node_ptr->parent->children;
                parent_children.erase(parent_children.end() - 1); // The EditNode is always the last in creation mode
            }

            edit_node_ptr = nullptr;
        }

        ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
    } else {
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
        std::string node_name = (*node_ptr->j_entity_ptr)["name"].GetString();
        bool has_children = !node_ptr->children.empty();

        if (node_ptr->is_selected)
            node_flags |= ImGuiTreeNodeFlags_Selected;

        if (!has_children)
            node_flags |= ImGuiTreeNodeFlags_Leaf;

        bool node_open = ImGui::TreeNodeEx(node_name.c_str(), node_flags);

        if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
            ImGuiIO& io = ImGui::GetIO();

            // macOS style: Shortcuts using Cmd/Super instead of Ctrl
            const bool is_shortcut_key = (io.ConfigMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

            if (is_shortcut_key && ImGui::IsItemClicked(0)) {
                if (node_ptr->is_selected) {
                    auto found = std::find_if(selected_nodes.begin(), selected_nodes.end(), [&](EntityNode* p) { return p == node_ptr; });
                    assert(found != selected_nodes.end());

                    selected_nodes.erase(found);
                } else
                    selected_nodes.push_back(node_ptr);

                node_ptr->is_selected = !node_ptr->is_selected;
            } else {
                if (!node_ptr->is_selected || ImGui::IsItemClicked(0)) {
                    if (!selected_nodes.empty()) {
                        for (auto& selected_node_ptr : selected_nodes)
                            selected_node_ptr->is_selected = false;

                        selected_nodes.clear();
                    }

                    selected_nodes.push_back(node_ptr);
                    node_ptr->is_selected = true;
                }

                if (ImGui::IsItemClicked(0))
                    clicked_node_ptr = node_ptr;
            }
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Add child")) {
                edit_node_ptr = node_ptr->addChild();
                edit_node_mode = EditMode::EntityCreation;
                edit_node_string = "";
                edit_node_needs_focus = true;
            }

            if (ImGui::MenuItem("Rename")) {
                edit_node_ptr = node_ptr;
                edit_node_mode = EditMode::Rename;
                edit_node_string = node_name;
                edit_node_needs_focus = true;
            }

            if (ImGui::MenuItem("Delete"))
                delete_selected_nodes = true;

            ImGui::EndPopup();
        }

        if (node_open) {
            for (auto& child : node_ptr->children)
                displayEntityTree(child.get());

            ImGui::TreePop();
        }
    }
}

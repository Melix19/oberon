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
    : root_node_ptr(nullptr)
    , clicked_node_ptr(nullptr)
    , delete_selected_nodes(false)
{
}

void Hierarchy::newFrame()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Hierarchy");
    ImGui::PopStyleVar();

    if (root_node_ptr) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
        displayEntityTree(root_node_ptr);
        ImGui::PopStyleVar();
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

void Hierarchy::clear()
{
    root_node_ptr = nullptr;
    clicked_node_ptr = nullptr;

    for (auto& selected_node_ptr : selected_nodes)
        selected_node_ptr->is_selected = false;

    selected_nodes.clear();
}

void Hierarchy::displayEntityTree(EntityNode* node_ptr)
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
    std::string node_name = node_ptr->j_value["name"].GetString();
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

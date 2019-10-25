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

#include "CollectionPanel.hpp"

EntityNode::EntityNode(Value& j_value)
    : j_value(j_value)
    , is_selected(false)
{
}

EntityNode* EntityNode::addChild(Value& j_value)
{
    auto child = Containers::pointer<EntityNode>(j_value);
    child->parent = this;

    children.push_back(std::move(child));
    return children.back().get();
}

CollectionPanel::CollectionPanel(const std::string& path)
    : path(path)
    , is_open(true)
    , is_focused(false)
    , needs_focus(true)
    , needs_docking(true)
{
    std::string json = Utility::Directory::readString(path);
    j_document.Parse(json.c_str());

    root_node = Containers::pointer<EntityNode>(j_document);

    updateEntityNodeChildren(root_node.get());
}

void CollectionPanel::newFrame()
{
    if (needs_focus) {
        ImGui::SetNextWindowFocus();
        needs_focus = false;
    }

    std::string filename = Utility::Directory::filename(path);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(filename.c_str(), &is_open);
    ImGui::PopStyleVar();

    is_focused = ImGui::IsWindowFocused();

    displayEntity(root_node.get());

    ImGui::End();
}

void CollectionPanel::displayEntity(EntityNode* node_ptr)
{
    auto j_components = node_ptr->j_value["components"].GetArray();
    for (auto& j_component : j_components) {
        std::string type = j_component["type"].GetString();

        if (type == "rectangle_shape") {
            // Position
            float position_x = node_ptr->j_value["position"][0].GetFloat();
            float position_y = node_ptr->j_value["position"][1].GetFloat();

            // Size
            float size_x = j_component["size"][0].GetFloat();
            float size_y = j_component["size"][1].GetFloat();

            // Color
            float color_r = j_component["color"][0].GetFloat();
            float color_g = j_component["color"][1].GetFloat();
            float color_b = j_component["color"][2].GetFloat();
            float color_a = j_component["color"][3].GetFloat();

            ImVec2 p1 = ImVec2(position_x + ImGui::GetCursorScreenPos().x, position_y + ImGui::GetCursorScreenPos().y);
            ImVec2 p2 = ImVec2(p1.x + size_x, p1.y + size_y);

            ImGui::GetWindowDrawList()->AddRectFilled(p1, p2, IM_COL32(color_r * 255, color_g * 255, color_b * 255, color_a * 255));
        }
    }

    for (auto& child : node_ptr->children)
        displayEntity(child.get());
}

void CollectionPanel::updateEntityNodeChildren(EntityNode* node_ptr)
{
    auto j_children = node_ptr->j_value["children"].GetArray();

    for (auto& j_child : j_children) {
        bool has_children = j_children.Empty();
        EntityNode* child_node = node_ptr->addChild(j_child);

        if (has_children)
            updateEntityNodeChildren(child_node);
    }
}

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

    displayEntity(root_node.get(), Matrix3{});

    ImGui::End();
}

void CollectionPanel::displayEntity(EntityNode* node_ptr, const Matrix3& parent_transformation)
{
    // Position
    auto position = Vector2{ node_ptr->j_value["position"][0].GetFloat(), node_ptr->j_value["position"][1].GetFloat() };

    // Rotation
    auto rotation = Deg(node_ptr->j_value["rotation"].GetFloat());

    // Scale
    auto scale = Vector2{ node_ptr->j_value["scale"][0].GetFloat(), node_ptr->j_value["scale"][1].GetFloat() };

    auto j_components = node_ptr->j_value["components"].GetArray();
    for (auto& j_component : j_components) {
        std::string type = j_component["type"].GetString();

        if (type == "rectangle_shape") {
            auto abs_position = position + parent_transformation.translation();
            auto abs_rotation = rotation + Deg(Complex::fromMatrix(parent_transformation.rotation()).angle());
            auto abs_scale = scale * parent_transformation.scaling();

            // Size
            auto size = Vector2{ j_component["size"][0].GetFloat(), j_component["size"][1].GetFloat() };

            // Color
            float color_r = j_component["color"][0].GetFloat();
            float color_g = j_component["color"][1].GetFloat();
            float color_b = j_component["color"][2].GetFloat();
            float color_a = j_component["color"][3].GetFloat();

            float entity_cos = cos(Rad(abs_rotation));
            float entity_sin = sin(Rad(abs_rotation));

            auto cursor_pos = ImGui::GetCursorScreenPos();

            auto p1 = ImRotate(ImVec2(-abs_scale.x() * size.x() * 0.5f, -abs_scale.y() * size.y() * 0.5f), entity_cos, entity_sin);
            p1.x += cursor_pos.x + abs_position.x();
            p1.y += cursor_pos.y + abs_position.y();

            auto p2 = ImRotate(ImVec2(+abs_scale.x() * size.x() * 0.5f, -abs_scale.y() * size.y() * 0.5f), entity_cos, entity_sin);
            p2.x += cursor_pos.x + abs_position.x();
            p2.y += cursor_pos.y + abs_position.y();

            auto p3 = ImRotate(ImVec2(+abs_scale.x() * size.x() * 0.5f, +abs_scale.y() * size.y() * 0.5f), entity_cos, entity_sin);
            p3.x += cursor_pos.x + abs_position.x();
            p3.y += cursor_pos.y + abs_position.y();

            auto p4 = ImRotate(ImVec2(-abs_scale.x() * size.x() * 0.5f, +abs_scale.y() * size.y() * 0.5f), entity_cos, entity_sin);
            p4.x += cursor_pos.x + abs_position.x();
            p4.y += cursor_pos.y + abs_position.y();

            auto col = IM_COL32(color_r * 255, color_g * 255, color_b * 255, color_a * 255);

            ImGui::GetWindowDrawList()->AddQuadFilled(p1, p2, p3, p4, col);
        }
    }

    for (auto& child : node_ptr->children) {
        auto transform = Matrix3{ Matrix3::translation(position) * Matrix3::rotation(Rad(rotation)) * Matrix3::scaling(scale) };
        displayEntity(child.get(), transform);
    }
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

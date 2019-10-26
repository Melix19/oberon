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

    ImGui::Begin(filename.c_str(), &is_open);

    is_focused = ImGui::IsWindowFocused();

    ImGui::End();
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

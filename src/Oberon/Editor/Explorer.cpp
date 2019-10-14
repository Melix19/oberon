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

#include "Explorer.hpp"

FileNode::FileNode(const std::string& path, FileNode* parent)
    : path(path)
    , parent(parent)
    , is_selected(false)
{
}

FileNode* FileNode::addChild(const std::string& path)
{
    children.push_back(std::make_unique<FileNode>(path, this));
    return children.back().get();
}

Explorer::Explorer(const std::string& project_path)
    : root_node(project_path)
    , delete_selected_nodes(false)
{
    updateFileNodeChildren(&root_node);
}

void Explorer::newFrame()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Explorer");
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    for (auto& child : root_node.children)
        displayFileTree(child.get());
    ImGui::PopStyleVar();

    ImGui::End();

    if (delete_selected_nodes) {
        if (!selected_nodes.empty()) {
            for (auto& selected_node_ptr : selected_nodes) {
                auto& parent_children = selected_node_ptr->parent->children;
                auto found = std::find_if(parent_children.begin(), parent_children.end(), [&](FileNode::Ptr& p) { return p.get() == selected_node_ptr; });
                assert(found != parent_children.end());

                removeEntireFile((*found)->path);
                parent_children.erase(found);
            }

            selected_nodes.clear();
        }

        delete_selected_nodes = false;
    }
}

void Explorer::updateFileNodeChildren(FileNode* node)
{
    auto file_list = Utility::Directory::list(node->path, Utility::Directory::Flag::SkipDotAndDotDot);

    for (auto& filename : file_list) {
        bool is_directory = Utility::Directory::isDirectory(node->path);
        std::string child_path = Utility::Directory::join(node->path, filename);
        FileNode* child_node = node->addChild(child_path);

        if (is_directory)
            updateFileNodeChildren(child_node);
    }

    std::sort(node->children.begin(), node->children.end(), sortFileNodes);
}

void Explorer::displayFileTree(FileNode* node)
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth;
    std::string node_name = Utility::Directory::filename(node->path);
    bool is_directory = Utility::Directory::isDirectory(node->path);

    if (node->is_selected)
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if (!is_directory)
        node_flags |= ImGuiTreeNodeFlags_Leaf;

    bool node_open = ImGui::TreeNodeEx(node_name.c_str(), node_flags);

    if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
        ImGuiIO& io = ImGui::GetIO();

        // macOS style: Shortcuts using Cmd/Super instead of Ctrl
        const bool is_shortcut_key = (io.ConfigMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if (is_shortcut_key && ImGui::IsItemClicked(0)) {
            if (node->is_selected) {
                auto found = std::find_if(selected_nodes.begin(), selected_nodes.end(), [&](FileNode* p) { return p == node; });
                assert(found != selected_nodes.end());

                selected_nodes.erase(found);
            } else
                selected_nodes.push_back(node);

            node->is_selected = !node->is_selected;
        } else if (!node->is_selected) {
            if (!selected_nodes.empty()) {
                for (auto& selected_node : selected_nodes)
                    selected_node->is_selected = false;

                selected_nodes.clear();
            }

            selected_nodes.push_back(node);
            node->is_selected = true;
        }
    }

    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete"))
            delete_selected_nodes = true;

        ImGui::EndPopup();
    }

    if (node_open) {
        for (auto& child : node->children)
            displayFileTree(child.get());

        ImGui::TreePop();
    }
}

void Explorer::removeEntireFile(const std::string& path)
{
    bool is_directory = Utility::Directory::isDirectory(path);

    if (is_directory) {
        auto file_list = Utility::Directory::list(path, Utility::Directory::Flag::SkipDotAndDotDot);

        for (auto& filename : file_list)
            removeEntireFile(Utility::Directory::join(path, filename));
    }

    Utility::Directory::rm(path);
}

bool Explorer::sortFileNodes(const FileNode::Ptr& a, const FileNode::Ptr& b)
{
    bool a_is_directory = Utility::Directory::isDirectory(a->path);
    bool b_is_directory = Utility::Directory::isDirectory(b->path);

    // Show directories first
    if (a_is_directory && !b_is_directory) {
        return true;
    } else if (!a_is_directory && b_is_directory) {
        return false;
    } else {
        std::string a_name = Utility::Directory::filename(a->path);
        std::string b_name = Utility::Directory::filename(b->path);
        std::transform(a_name.begin(), a_name.end(), a_name.begin(), ::tolower);
        std::transform(b_name.begin(), b_name.end(), b_name.begin(), ::tolower);

        return a_name < b_name;
    }
}

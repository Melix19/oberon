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

#include "Explorer.h"

#include <Corrade/Utility/Directory.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>

namespace {

bool sortFileNodes(const Containers::Pointer<FileNode>& a, const Containers::Pointer<FileNode>& b) {
    bool aIsDirectory = Utility::Directory::isDirectory(a->path());
    bool bIsDirectory = Utility::Directory::isDirectory(b->path());

    /* Show directories first */
    if(aIsDirectory && !bIsDirectory) return true;
    else if(!aIsDirectory && bIsDirectory) return false;
    else {
        std::string aFileame = Utility::Directory::filename(a->path());
        std::string bFilename = Utility::Directory::filename(b->path());
        std::transform(aFileame.begin(), aFileame.end(), aFileame.begin(), ::tolower);
        std::transform(bFilename.begin(), bFilename.end(), bFilename.begin(), ::tolower);

        return aFileame < bFilename;
    }
}

void updateFileNodeChildren(FileNode* node) {
    auto directoryList = Utility::Directory::list(node->path(),
        Utility::Directory::Flag::SkipDotAndDotDot);
    for(auto& filename: directoryList) {
        std::string childPath = Utility::Directory::join(node->path(), filename);
        std::string childResourcePath = Utility::Directory::join(node->resourcePath(), filename);
        FileNode* childNode = node->addChild(childPath, childResourcePath);

        bool isDirectory = Utility::Directory::isDirectory(childPath);
        if(isDirectory) updateFileNodeChildren(childNode);
    }

    std::sort(node->children().begin(), node->children().end(), sortFileNodes);
}

void removeFileIterative(const std::string& path) {
    auto directoryList = Utility::Directory::list(path, Utility::Directory::Flag::SkipDotAndDotDot);
    for(auto& filename: directoryList)
        removeFileIterative(Utility::Directory::join(path, filename));

    Utility::Directory::rm(path);
}

}

Explorer::Explorer(const std::string& projectPath):
    _rootNode{projectPath}
{
    updateFileNodeChildren(&_rootNode);
}

void Explorer::newFrame() {
    _clickedNode = nullptr;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool isVisible = ImGui::Begin("Explorer");
    ImGui::PopStyleVar();

    /* If the window is not visible, end the method here. */
    if(!isVisible) {
        ImGui::End();
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    displayNode(&_rootNode);
    ImGui::PopStyleVar();

    ImGui::End();

    if(_deleteSelectedNodes) {
        deleteSelectedNodes();
        _deleteSelectedNodes = false;
    }

    applyDragDrop();
}

void Explorer::selectNode(FileNode* node) {
    _selectedNodes.push_back(node);
    node->setSelected(true);
}

void Explorer::deselectAllNodes() {
    for(auto& selectedNode: _selectedNodes)
        selectedNode->setSelected(false);
    _selectedNodes.clear();
}

void Explorer::displayNode(FileNode* node) {
    if(node == _editNode && _editNodeMode == EditMode::Rename)
        displayEditNode(node);
    else {
        displayFileNode(node);

        if(node == _editNode && _editNodeMode != EditMode::Rename)
            displayEditNode(node);
    }
}

void Explorer::displayFileNode(FileNode* node) {
    std::string nodeName = Utility::Directory::filename(node->path());
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_SpanFullWidth;

    bool isDirectory = Utility::Directory::isDirectory(node->path());
    if(!isDirectory) nodeFlags |= ImGuiTreeNodeFlags_Leaf;
    if(node->isSelected()) nodeFlags |= ImGuiTreeNodeFlags_Selected;
    if(node == &_rootNode) nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    bool isOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

    if(ImGui::BeginDragDropSource()) {
        std::string extension = Utility::Directory::splitExtension(node->path()).second;
        std::string typeName = "FileNode.";

        if(extension == ".png") typeName.append("Image");
        else typeName.append("Other");

        if(!_isDraggingNodes) {
            if(!node->isSelected()) {
                if(!_selectedNodes.empty()) deselectAllNodes();
                selectNode(node);
            }

            _isDraggingNodes = true;
        }

        ImGui::SetDragDropPayload(typeName.c_str(), &node, sizeof(FileNode*));
        ImGui::Text(nodeName.c_str());
        ImGui::EndDragDropSource();
    } else _isDraggingNodes = false;

    if(isDirectory && ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FileNode.Other");
        if(!payload) payload = ImGui::AcceptDragDropPayload("FileNode.Image");
        if(payload) _dragDropTarget = node;

        ImGui::EndDragDropTarget();
    }

    if(!_dragDropTarget && ImGui::IsItemHovered() && (ImGui::IsMouseReleased(0) || ImGui::IsMouseReleased(1))) {
        ImGuiIO& io = ImGui::GetIO();

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        const bool isShortcutKey = (io.ConfigMacOSXBehaviors ? (io.KeySuper &&
            !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if(isShortcutKey && ImGui::IsMouseReleased(0)) {
            if(node->isSelected()) {
                _selectedNodes.erase(std::find_if(_selectedNodes.begin(),
                    _selectedNodes.end(), [&node](FileNode* n) { return n == node; }));
            } else _selectedNodes.push_back(node);

            node->setSelected(!node->isSelected());
        } else if(!node->isSelected() || ImGui::IsMouseReleased(0)) {
            if(!_selectedNodes.empty()) deselectAllNodes();
            selectNode(node);

            if(!isDirectory && ImGui::IsMouseReleased(0))
                _clickedNode = node;
        }
    }

    if(ImGui::BeginPopupContextItem()) {
        if(isDirectory) {
            if(ImGui::BeginMenu("New file")) {
                if(ImGui::MenuItem("Collection")) {
                    _editNode = node;
                    _editNodeMode = EditMode::FileCreation;
                    _editNodeText = ".col";
                    _editNodeNeedsFocus = true;
                }

                ImGui::EndMenu();
            }

            if(ImGui::MenuItem("New folder")) {
                _editNode = node;
                _editNodeMode = EditMode::FolderCreation;
                _editNodeText = "";
                _editNodeNeedsFocus = true;
            }
        }

        if(node != &_rootNode) {
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

        ImGui::TreePop();
    }
}

void Explorer::displayEditNode(FileNode* node) {
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() -
        ImGui::GetTreeNodeToLabelSpacing());

    if(_editNodeNeedsFocus) ImGui::SetKeyboardFocusHere();

    if(ImGui::InputText("##FileName", &_editNodeText, ImGuiInputTextFlags_EnterReturnsTrue)) {
        switch(_editNodeMode) {
            case EditMode::FileCreation: {
                std::string newPath = Utility::Directory::join(node->path(), _editNodeText);
                bool success = Utility::Directory::writeString(newPath, "");
                if(success) {
                    std::string newResourcePath = Utility::Directory::join(node->resourcePath(),
                        _editNodeText);
                    node->addChild(newPath, newResourcePath);

                    std::sort(node->children().begin(), node->children().end(), sortFileNodes);
                }
            } break;
            case EditMode::FolderCreation: {
                std::string newPath = Utility::Directory::join(node->path(), _editNodeText);
                bool success = Utility::Directory::mkpath(newPath);
                if(success) {
                    std::string newResourcePath = Utility::Directory::join(node->resourcePath(),
                        _editNodeText);
                    node->addChild(newPath, newResourcePath);

                    std::sort(node->children().begin(), node->children().end(), sortFileNodes);
                }
            } break;
            case EditMode::Rename: {
                std::string newPath = Utility::Directory::join(node->parent()->path(), _editNodeText);
                bool success = Utility::Directory::move(node->path(), newPath);
                if(success) {
                    std::string newResourcePath = Utility::Directory::join(node->parent()->resourcePath(),
                        _editNodeText);
                    node->setPath(newPath);
                    node->setResourcePath(newResourcePath);

                    std::sort(node->parent()->children().begin(), node->parent()->children().end(),
                        sortFileNodes);
                }
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

void Explorer::applyDragDrop() {
    if(_dragDropTarget) {
        for(auto& selectedNode: _selectedNodes) {
            if(selectedNode->parent() == _dragDropTarget) continue;

            std::string filename = Utility::Directory::filename(selectedNode->path());
            std::string newPath = Utility::Directory::join(_dragDropTarget->path(), filename);

            bool success = Utility::Directory::move(selectedNode->path(), newPath);
            if(success) {
                std::string newResourcePath = Utility::Directory::join(_dragDropTarget->resourcePath(),
                    filename);
                FileNode* newNode = _dragDropTarget->addChild(newPath, newResourcePath);
                std::sort(_dragDropTarget->children().begin(), _dragDropTarget->children().end(),
                    sortFileNodes);

                bool isDirectory = Utility::Directory::isDirectory(newPath);
                if(isDirectory) updateFileNodeChildren(newNode);

                auto& nodeParentChildren = selectedNode->parent()->children();
                nodeParentChildren.erase(std::find_if(nodeParentChildren.begin(),
                    nodeParentChildren.end(), [&selectedNode](Containers::Pointer<FileNode>& n) {
                        return n.get() == selectedNode; }));
            }
        }

        _dragDropTarget = nullptr;
    }
}

void Explorer::deleteSelectedNodes() {
    if(!_selectedNodes.empty()) {
        for(auto& selectedNode: _selectedNodes) {
            removeFileIterative(selectedNode->path());

            auto& parentChildren = selectedNode->parent()->children();
            parentChildren.erase(std::find_if(parentChildren.begin(), parentChildren.end(),
                [&selectedNode](Containers::Pointer<FileNode>& n) {
                    return n.get() == selectedNode;
                }));
        }

        _selectedNodes.clear();
    }
}

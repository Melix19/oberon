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

#include "Explorer.h"

Explorer::Explorer(const std::string& rootPath): _rootNode(rootPath),
    _deleteSelectedNodes(false), _clickedNode(nullptr), _editNode(nullptr)
{
    updateFileNodeChildren(&_rootNode);
}

void Explorer::newFrame() {
    _clickedNode = nullptr;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool isVisible = ImGui::Begin("Explorer");
    ImGui::PopStyleVar();

    /* If the window is not visible, just end the method here. */
    if(!isVisible) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginPopupContextWindow()) {
        if(ImGui::BeginMenu("New file")) {
            if(ImGui::MenuItem("Collection")) {
                _editNode = &_rootNode;
                _editNodeMode = EditMode::FileCreation;
                _editNodeText = ".col";
                _editNodeNeedsFocus = true;
            }

            if(ImGui::MenuItem("Script")) {
                _editNode = &_rootNode;
                _editNodeMode = EditMode::FileCreation;
                _editNodeText = ".py";
                _editNodeNeedsFocus = true;
            }

            ImGui::EndMenu();
        }

        if(ImGui::MenuItem("New folder")) {
            _editNode = &_rootNode;
            _editNodeMode = EditMode::FolderCreation;
            _editNodeText = "";
            _editNodeNeedsFocus = true;
        }

        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    for(auto& child : _rootNode.children())
        displayFileTree(child.get());

    if(&_rootNode == _editNode && _editNodeMode != EditMode::Rename)
        displayEditNode(&_rootNode);

    ImGui::PopStyleVar();

    ImGui::End();

    if(_deleteSelectedNodes) {
        if(!_selectedNodes.empty()) {
            for(auto& selectedNode : _selectedNodes) {
                auto& parentChildren = selectedNode->parent()->children();
                auto found = std::find_if(parentChildren.begin(), parentChildren.end(),
                    [&](Containers::Pointer<FileNode>& p) { return p.get() == selectedNode; });

                CORRADE_INTERNAL_ASSERT(found != parentChildren.end());

                removeEntireFile((*found)->path());
                parentChildren.erase(found);
            }

            _selectedNodes.clear();
        }

        _deleteSelectedNodes = false;
    }
}

void Explorer::displayFileTree(FileNode* node) {
    if(node == _editNode && _editNodeMode == EditMode::Rename) {
        displayEditNode(node);
        return;
    }

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_FramePadding |
        ImGuiTreeNodeFlags_SpanFullWidth;
    std::string nodeName = Utility::Directory::filename(node->path());
    bool isDirectory = Utility::Directory::isDirectory(node->path());

    if(node->isSelected()) nodeFlags |= ImGuiTreeNodeFlags_Selected;

    if(!isDirectory) nodeFlags |= ImGuiTreeNodeFlags_Leaf;

    bool nodeOpen = ImGui::TreeNodeEx(nodeName.c_str(), nodeFlags);

    if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
        ImGuiIO& io = ImGui::GetIO();

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        const bool isShortcutKey = (io.ConfigMacOSXBehaviors ? (io.KeySuper && !io.KeyCtrl) :
            (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift;

        if(isShortcutKey && ImGui::IsItemClicked(0)) {
            if(node->isSelected()) {
                auto found = std::find_if(_selectedNodes.begin(), _selectedNodes.end(),
                    [&](FileNode* p) { return p == node; });

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
        if(isDirectory) {
            if(ImGui::BeginMenu("New file")) {
                if(ImGui::MenuItem("Collection")) {
                    _editNode = node;
                    _editNodeMode = EditMode::FileCreation;
                    _editNodeText = ".col";
                    _editNodeNeedsFocus = true;
                }

                if(ImGui::MenuItem("Script")) {
                    _editNode = node;
                    _editNodeMode = EditMode::FileCreation;
                    _editNodeText = ".py";
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
            displayFileTree(child.get());

        if(node == _editNode && _editNodeMode != EditMode::Rename)
            displayEditNode(node);

        ImGui::TreePop();
    }
}

void Explorer::displayEditNode(FileNode* node) {
    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() -
        ImGui::GetTreeNodeToLabelSpacing());
    bool success = false;

    /* Set focus in it's first frame. */
    if(_editNodeNeedsFocus) ImGui::SetKeyboardFocusHere();

    if(ImGui::InputText("##FileName", &_editNodeText, ImGuiInputTextFlags_EnterReturnsTrue)) {
        switch (_editNodeMode) {
            case EditMode::FileCreation: {
                std::string newPath = Utility::Directory::join(node->path(), _editNodeText);
                success = Utility::Directory::writeString(newPath, "");

                if(success) {
                    node->addChild(newPath);
                    auto& children = node->children();
                    std::sort(children.begin(), children.end(), sortFileNodes);
                }
            } break;
            case EditMode::FolderCreation: {
                std::string newPath = Utility::Directory::join(node->path(), _editNodeText);
                success = Utility::Directory::mkpath(newPath);

                if(success) {
                    node->addChild(newPath);
                    auto& children = node->children();
                    std::sort(children.begin(), children.end(), sortFileNodes);
                }
            } break;
            case EditMode::Rename: {
                std::string newPath = Utility::Directory::join(node->parent()->path(),
                    _editNodeText);
                success = Utility::Directory::move(node->path(), newPath);

                if(success) {
                    node->setPath(newPath);
                    auto& parentChildren = node->parent()->children();
                    std::sort(parentChildren.begin(), parentChildren.end(), sortFileNodes);
                }
            } break;
        }
    }

    /* In the first frame, delay the focus check on the next frame
        otherwise it will be deleted right away. */
    if(_editNodeNeedsFocus) _editNodeNeedsFocus = false;
    else if(!ImGui::IsItemActive()) {
        if(!success && _editNodeMode != EditMode::Rename) {
            /* The EditNode is always the last in creation mode. */
            auto& parentChildren = node->parent()->children();
            parentChildren.erase(parentChildren.end() - 1);
        }

        _editNode = nullptr;
    }

    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
}

void Explorer::updateFileNodeChildren(FileNode* node) {
    auto directoryList = Utility::Directory::list(node->path(), Utility::Directory::Flag::SkipDotAndDotDot);

    for(auto& filename : directoryList) {
        std::string childPath = Utility::Directory::join(node->path(), filename);
        FileNode* childNode = node->addChild(childPath);

        updateFileNodeChildren(childNode);
    }

    std::sort(node->children().begin(), node->children().end(), sortFileNodes);
}

void Explorer::removeEntireFile(const std::string& path) {
    auto directoryList = Utility::Directory::list(path, Utility::Directory::Flag::SkipDotAndDotDot);

    for(auto& filename : directoryList)
        removeEntireFile(Utility::Directory::join(path, filename));

    Utility::Directory::rm(path);
}

bool Explorer::sortFileNodes(const Containers::Pointer<FileNode>& a, const Containers::Pointer<FileNode>& b) {
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

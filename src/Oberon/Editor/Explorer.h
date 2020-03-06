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

#pragma once

#include "FileNode.h"

class Explorer {
    public:
        Explorer(const std::string& projectPath);
        void newFrame();

        void selectNode(FileNode* node);
        void deselectAllNodes();

        FileNode* clickedNode() const { return _clickedNode; }

    private:
        void displayNode(FileNode* node);
        void displayFileNode(FileNode* node);
        void displayEditNode(FileNode* node);

        void deleteSelectedNodes();
        void applyDragDrop();

    private:
        FileNode _rootNode;

        FileNode* _clickedNode{nullptr};
        std::vector<FileNode*> _selectedNodes;

        FileNode* _dragDropTarget{nullptr};
        bool _deleteSelectedNodes{false};

    private:
        enum class EditMode {
            FileCreation,
            FolderCreation,
            Rename
        };

        FileNode* _editNode{nullptr};
        EditMode _editNodeMode;
        std::string _editNodeText;
        bool _editNodeNeedsFocus;
};

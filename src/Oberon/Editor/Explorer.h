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

#pragma once

#include "FileNode.h"

class Explorer {
    public:
        Explorer(const std::string& rootPath);
        void newFrame();

        FileNode* clickedNode() const { return _clickedNode; }

    private:
        void displayFileTree(FileNode* node);
        void displayEditNode(FileNode* node);

        void updateFileNodeChildren(FileNode* node);
        void removeEntireFile(const std::string& path);

        static bool sortFileNodes(const Containers::Pointer<FileNode>& a, const Containers::Pointer<FileNode>& b);

        FileNode _rootNode;
        std::vector<FileNode*> _selectedNodes;
        bool _deleteSelectedNodes;
        FileNode* _clickedNode;

        enum class EditMode {
            FileCreation,
            FolderCreation,
            Rename
        };

        FileNode* _editNode;
        EditMode _editNodeMode;
        std::string _editNodeText;
        bool _editNodeNeedsFocus;
};

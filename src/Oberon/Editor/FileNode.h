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

#include <string>
#include <vector>
#include <Corrade/Containers/Pointer.h>

#include "Editor.h"

namespace Oberon { namespace Editor {

class FileNode {
    public:
        FileNode(const std::string& path, const std::string& resourcePath = ""):
            _path{path}, _resourcePath{resourcePath} {}

        std::string path() const { return _path; }
        FileNode& setPath(const std::string& path) {
            _path = path;
            return *this;
        }

        std::string resourcePath() const { return _resourcePath; }
        FileNode& setResourcePath(const std::string& resourcePath) {
            _resourcePath = resourcePath;
            return *this;
        }

        bool isSelected() const { return _isSelected; }
        FileNode& setSelected(bool selected) {
            _isSelected = selected;
            return *this;
        }

        FileNode* addChild(const std::string& path, const std::string& resourcePath) {
            auto child = Containers::pointer<FileNode>(path, resourcePath);
            child->_parent = this;

            _children.push_back(std::move(child));
            return _children.back().get();
        }

        FileNode* parent() const { return _parent; }

        std::vector<Containers::Pointer<FileNode>>& children() { return _children; }

    private:
        std::string _path;
        std::string _resourcePath;
        bool _isSelected{false};

        FileNode* _parent;
        std::vector<Containers::Pointer<FileNode>> _children;
};

}}

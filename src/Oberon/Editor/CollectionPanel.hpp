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

#include <Corrade/Containers/Pointer.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Complex.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/Math/Matrix3.h>
#include <document.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <vector>

using namespace Magnum;
using namespace rapidjson;

struct EntityNode {
    EntityNode(Value& j_value);
    EntityNode* addChild(Value& j_value);

    Value& j_value;
    bool is_selected;

    EntityNode* parent;
    std::vector<Containers::Pointer<EntityNode>> children;
};

class CollectionPanel {
public:
    CollectionPanel(const std::string& path);
    void newFrame();

    Containers::Pointer<EntityNode> root_node;
    std::string path;

    bool is_open;
    bool is_focused;
    bool needs_focus;
    bool needs_docking;

private:
    void displayEntity(EntityNode* node_ptr, const Matrix3& parent_transformation);
    void updateEntityNodeChildren(EntityNode* node_ptr);

    Document j_document;
};

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

#include "Editor.h"

namespace Oberon { namespace Editor {

class Outliner {
    public:
        void newFrame();

        Outliner& setPanel(CollectionPanel* panel) {
            _panel = panel;
            return *this;
        }

    private:
        void displayNode(ObjectNode* node);
        void displayObjectNode(ObjectNode* node);
        void displayEditNode(ObjectNode* node);

        void selectNode(ObjectNode* node);
        void deselectAllNodes();

        void deleteSelectedNodes();
        void applyDragDrop();

    private:
        CollectionPanel* _panel{nullptr};

        ObjectNode* _dragDropTarget{nullptr};
        bool _deleteSelectedNodes{false};
        bool _isDraggingNodes{false};

    private:
        enum class EditMode {
            ObjectCreation,
            Rename
        };

        ObjectNode* _editNode{nullptr};
        EditMode _editNodeMode;
        std::string _editNodeText;
        bool _editNodeNeedsFocus;
};

}}

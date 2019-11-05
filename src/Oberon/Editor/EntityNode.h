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
#include <Oberon/Core/EntitySerializer.hpp>

class EntityNode {
    public:
        EntityNode(Object2D* entity, Value& jsonEntity): _entity(entity),
            _jsonEntity(jsonEntity), _isSelected(false) {}

        Object2D* entity() const { return _entity; }

        Value& jsonEntity() const { return _jsonEntity; }

        bool isSelected() const { return _isSelected; }

        EntityNode& setSelected(bool select) {
            _isSelected = select;
            return *this;
        }

        EntityNode* addChild(Object2D* entity, Value& jsonEntity) {
            auto child = Containers::pointer<EntityNode>(entity, jsonEntity);
            child->_parent = this;

            _children.push_back(std::move(child));
            return _children.back().get();
        }

        EntityNode* parent() const { return _parent; }

        std::vector<Containers::Pointer<EntityNode>>& children() { return _children; }

    private:
        Object2D* _entity;
        Value& _jsonEntity;
        bool _isSelected;

        EntityNode* _parent;
        std::vector<Containers::Pointer<EntityNode>> _children;
};

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
#include <Oberon/Core/Serializer.h>

class ObjectNode {
    public:
        ObjectNode(Object3D* object, Utility::ConfigurationGroup* objectConfig): _object(object),
            _objectConfig(objectConfig), _isSelected(false) {}

        Object3D* object() const { return _object; }

        Utility::ConfigurationGroup* objectConfig() const { return _objectConfig; }

        bool isSelected() const { return _isSelected; }

        ObjectNode& setSelected(bool select) {
            _isSelected = select;
            return *this;
        }

        Vector3 rotationDegree() { return _rotationDegree; }

        ObjectNode& setRotationDegree(const Vector3& rotationDegree) {
            _rotationDegree = rotationDegree;
            return *this;
        }

        ObjectNode* addChild(Object3D* object, Utility::ConfigurationGroup* objectConfig) {
            auto child = Containers::pointer<ObjectNode>(object, objectConfig);
            child->_parent = this;

            _children.push_back(std::move(child));
            return _children.back().get();
        }

        ObjectNode* parent() const { return _parent; }

        std::vector<Containers::Pointer<ObjectNode>>& children() { return _children; }

    private:
        Object3D* _object;
        Utility::ConfigurationGroup* _objectConfig;
        bool _isSelected;

        Vector3 _rotationDegree;

        ObjectNode* _parent;
        std::vector<Containers::Pointer<ObjectNode>> _children;
};

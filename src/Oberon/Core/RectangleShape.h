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

#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Shaders/Flat.h>

using namespace Magnum;

class RectangleShape: public SceneGraph::Drawable2D {
    public:
        explicit RectangleShape(SceneGraph::AbstractObject2D& object, SceneGraph::DrawableGroup2D* drawables, GL::Mesh& mesh, Shaders::Flat2D& shader, const Vector2& size, const Color4& color): SceneGraph::Drawable2D{object, drawables}, _mesh(mesh), _shader(shader), _size{size}, _color{color} {}

        Vector2 size() const { return _size; }

        RectangleShape& setSize(const Vector2& size) {
            _size = size;
            return *this;
        }

        Color4 color() const { return _color; }

        RectangleShape& setColor(const Color4& color) {
            _color = color;
            return *this;
        }

    private:
        void draw(const Matrix3& transformationMatrix, SceneGraph::Camera2D& camera) override {
            _shader.setTransformationProjectionMatrix(camera.projectionMatrix()*transformationMatrix*Matrix3::scaling(_size/2))
                .setColor(_color);
            _mesh.draw(_shader);
        }

        GL::Mesh& _mesh;
        Shaders::Flat2D& _shader;

        Vector2 _size;
        Color4 _color;
};

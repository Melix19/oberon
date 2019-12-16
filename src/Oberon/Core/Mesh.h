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
#include <Magnum/Resource.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Shaders/Flat.h>

using namespace Magnum;

class Mesh: public SceneGraph::Drawable3D {
    public:
        explicit Mesh(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D* drawables, Resource<GL::Mesh>& mesh, Resource<GL::AbstractShaderProgram, Shaders::Flat3D>& shader, const Vector2& size, const Color4& color): SceneGraph::Drawable3D{object, drawables}, _mesh(mesh), _shader(shader), _size{size}, _color{color} {}

        Vector2 size() const { return _size; }

        Mesh& setSize(const Vector2& size) {
            _size = size;
            return *this;
        }

        Color4 color() const { return _color; }

        Mesh& setColor(const Color4& color) {
            _color = color;
            return *this;
        }

        Mesh& setObjectId(UnsignedByte id) {
            _id = id;
            return *this;
        }

    private:
        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) override {
            _shader->setTransformationProjectionMatrix(camera.projectionMatrix()*transformation*Matrix4::scaling({_size/2, 0}))
                .setColor(_color)
                .setObjectId(_id);
            _mesh->draw(*_shader);
        }

        Resource<GL::Mesh> _mesh;
        Resource<GL::AbstractShaderProgram, Shaders::Flat3D> _shader;

        UnsignedByte _id;
        Vector2 _size;
        Color4 _color;
};

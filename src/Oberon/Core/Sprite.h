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

#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Resource.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Shaders/Flat.h>

using namespace Magnum;

class Sprite: public SceneGraph::Drawable3D {
    public:
        explicit Sprite(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D* drawables, Resource<GL::Mesh>& mesh, Resource<GL::AbstractShaderProgram, Shaders::Flat3D>& shader):
            SceneGraph::Drawable3D{object, drawables}, _mesh{mesh}, _shader{shader} {}

        Sprite& setTexture(Resource<GL::Texture2D>& texture) {
            _texture = texture;
            return *this;
        }

        Sprite& setObjectId(UnsignedByte id) {
            _id = id;
            return *this;
        }

        Float pixelSize() const { return _pixelSize; }
        Sprite& setPixelSize(Float pixelSize) {
            _pixelSize = pixelSize;
            return *this;
        }

    private:
        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) override {
            if(!_texture) return;

            _shader->setTransformationProjectionMatrix(camera.projectionMatrix()*transformation*Matrix4::scaling(Vector3{Vector3i{_texture->imageSize(0)/2, 0}}*_pixelSize))
                .bindTexture(*_texture)
                .setObjectId(_id);
            _mesh->draw(*_shader);
        }

        Resource<GL::Texture2D> _texture;
        Resource<GL::Mesh> _mesh;
        Resource<GL::AbstractShaderProgram, Shaders::Flat3D> _shader;

        UnsignedByte _id{0};
        Float _pixelSize{0.01f};
};

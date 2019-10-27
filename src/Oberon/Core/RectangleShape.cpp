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

#include "RectangleShape.hpp"

RectangleShape::RectangleShape(SceneGraph::AbstractObject2D& object, SceneGraph::DrawableGroup2D* drawables, Shaders::Flat2D& shader, const Vector2& size, const Color4& color)
    : SceneGraph::Drawable2D{ object, drawables }
    , shader(shader)
    , size{ size }
    , color{ color }
{
    mesh = MeshTools::compile(Primitives::squareSolid());
}

void RectangleShape::setSize(const Vector2& size)
{
    this->size = size;
}

void RectangleShape::setColor(const Color4& color)
{
    this->color = color;
}

void RectangleShape::draw(const Matrix3& transformation_matrix, SceneGraph::Camera2D& camera)
{
    shader.setTransformationProjectionMatrix(camera.projectionMatrix() * transformation_matrix * Matrix3::scaling(size / 2))
        .setColor(color);
    mesh.draw(shader);
}

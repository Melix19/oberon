#ifndef Oberon_Editor_Im3dContext_h
#define Oberon_Editor_Im3dContext_h
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

#include <Magnum/Timeline.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Mesh.h>

#include "Oberon/Oberon.h"

namespace Oberon { namespace Editor {

class Im3dShader: public GL::AbstractShaderProgram {
    public:
        typedef GL::Attribute<0, Vector4> PositionSize;
        typedef GL::Attribute<1, Vector4> Color;

        explicit Im3dShader();

        Im3dShader& setTransformationProjectionMatrix(const Matrix4& matrix);

    private:
        Int _transformationProjectionMatrixUniform{0};
};

class Im3dContext {
    public:
        Im3dContext();

        void newFrame();
        void drawFrame();

        Im3dContext& setViewportSize(const Vector2i& size);
        Im3dContext& setTransformationProjectionMatrix(const Matrix4& matrix);
        Im3dContext& setCameraTranslation(const Vector3& translation);
        Im3dContext& setCameraDirection(const Vector3& direction);

    private:
        Im3dShader _shader;
        GL::Buffer _vertexBuffer{GL::Buffer::TargetHint::Array};
        Timeline _timeline;
        GL::Mesh _mesh;
};

}}

#endif

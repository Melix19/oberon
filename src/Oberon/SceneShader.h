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

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Shaders/Generic.h>

#include "Oberon.h"

class SceneShader: public GL::AbstractShaderProgram {
    public:
        typedef Shaders::Generic3D::Position Position;
        typedef Shaders::Generic3D::Normal Normal;
        typedef Shaders::Generic3D::Tangent Tangent;
        typedef Shaders::Generic3D::TextureCoordinates TextureCoordinates;
        #ifndef MAGNUM_TARGET_GLES2
        typedef Shaders::Generic3D::ObjectId ObjectId;
        #endif

        enum: UnsignedInt {
            ColorOutput = Shaders::Generic3D::ColorOutput,
            #ifndef MAGNUM_TARGET_GLES2
            ObjectIdOutput = Shaders::Generic3D::ObjectIdOutput
            #endif
        };

        enum class Flag: UnsignedShort {
            AmbientTexture = 1 << 0,
            DiffuseTexture = 1 << 1,
            SpecularTexture = 1 << 2,
            NormalTexture = 1 << 3,
            #ifndef MAGNUM_TARGET_GLES2
            ObjectId = 1 << 4
            #endif
        };

        typedef Containers::EnumSet<Flag> Flags;

        explicit SceneShader(Flags flags = {}, UnsignedInt lightCount = 0);

        SceneShader& setAmbientColor(const Color4& color);
        SceneShader& bindAmbientTexture(GL::Texture2D& texture);

        SceneShader& setDiffuseColor(const Color4& color);
        SceneShader& bindDiffuseTexture(GL::Texture2D& texture);

        SceneShader& bindNormalTexture(GL::Texture2D& texture);

        SceneShader& setSpecularColor(const Color4& color);
        SceneShader& bindSpecularTexture(GL::Texture2D& texture);

        SceneShader& setShininess(Float shininess);

        #ifndef MAGNUM_TARGET_GLES2
        SceneShader& setObjectId(UnsignedInt id);
        #endif

        SceneShader& setTransformationMatrix(const Matrix4& matrix);
        SceneShader& setNormalMatrix(const Matrix3x3& matrix);
        SceneShader& setProjectionMatrix(const Matrix4& matrix);

        SceneShader& setPointLight(UnsignedInt id, const Vector3& position, const Color4& color, Float constant, Float linear, Float quadratic);

    private:
        /* Prevent accidentally calling irrelevant functions */
        #ifndef MAGNUM_TARGET_GLES
        using GL::AbstractShaderProgram::drawTransformFeedback;
        #endif
        #if !defined(MAGNUM_TARGET_GLES2) && !defined(MAGNUM_TARGET_WEBGL)
        using GL::AbstractShaderProgram::dispatchCompute;
        #endif

        Flags _flags;
        UnsignedInt _lightCount;
        Int _transformationMatrixUniform{0},
            _projectionMatrixUniform{1},
            _normalMatrixUniform{2},
            _ambientColorUniform{3},
            _diffuseColorUniform{4},
            _specularColorUniform{5},
            _shininessUniform{6};
            #ifndef MAGNUM_TARGET_GLES2
            Int _objectIdUniform{7};
            #endif
        Int _pointLightsUniform{8};
};

CORRADE_ENUMSET_OPERATORS(SceneShader::Flags)

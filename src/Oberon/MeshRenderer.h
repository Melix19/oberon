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

#include <Magnum/Resource.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Drawable.h>

#include "Oberon.h"

namespace Oberon {

class MeshRenderer: public SceneGraph::Drawable3D {
    public:
        explicit MeshRenderer(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D& drawables);

        MeshRenderer& setMesh(const Resource<GL::Mesh>& mesh) {
            _mesh = mesh;
            return *this;
        }

        MeshRenderer& setShader(const Resource<GL::AbstractShaderProgram, SceneShader>& shader) {
            _shader = shader;
            return *this;
        }

        MeshRenderer& setObjectId(UnsignedInt id) {
            _id = id;
            return *this;
        }

        Color4 ambientColor() { return _ambientColor; }
        MeshRenderer& setAmbientColor(const Color4& color) {
            _ambientColor = color;
            return *this;
        }

        bool hasAmbientTexture() { return _ambientTexture; }
        MeshRenderer& setAmbientTexture(const Resource<GL::Texture2D>& texture) {
            _ambientTexture = texture;
            return *this;
        }

        Color4 diffuseColor() { return _diffuseColor; }
        MeshRenderer& setDiffuseColor(const Color4& color) {
            _diffuseColor = color;
            return *this;
        }

        bool hasDiffuseTexture() { return _diffuseTexture; }
        MeshRenderer& setDiffuseTexture(const Resource<GL::Texture2D>& texture) {
            _diffuseTexture = texture;
            return *this;
        }

        bool hasNormalTexture() { return _normalTexture; }
        MeshRenderer& setNormalTexture(const Resource<GL::Texture2D>& texture) {
            _normalTexture = texture;
            return *this;
        }

        Color4 specularColor() { return _specularColor; }
        MeshRenderer& setSpecularColor(const Color4& color) {
            _specularColor = color;
            return *this;
        }

        bool hasSpecularTexture() { return _specularTexture; }
        MeshRenderer& setSpecularTexture(const Resource<GL::Texture2D>& texture) {
            _specularTexture = texture;
            return *this;
        }

        Float shininess() { return _shininess; }
        MeshRenderer& setShininess(Float shininess) {
            _shininess = shininess;
            return *this;
        }

    private:
        void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

    private:
        Resource<GL::Mesh> _mesh;
        Resource<GL::AbstractShaderProgram, SceneShader> _shader;
        UnsignedInt _id{0};

        Color4 _ambientColor{0.0f};
        Color4 _diffuseColor{1.0f};
        Color4 _specularColor{1.0f};

        Resource<GL::Texture2D> _ambientTexture;
        Resource<GL::Texture2D> _diffuseTexture;
        Resource<GL::Texture2D> _normalTexture;
        Resource<GL::Texture2D> _specularTexture;

        Float _shininess{80.0f};
};

}

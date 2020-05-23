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
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>

#include "SceneShader.h"

class Mesh: public SceneGraph::Drawable3D {
    public:
        explicit Mesh(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D* drawables):
            SceneGraph::Drawable3D{object, drawables} {}

        Mesh& setMesh(const Resource<GL::Mesh>& mesh) {
            _mesh = mesh;
            return *this;
        }

        Mesh& setShader(const Resource<GL::AbstractShaderProgram, SceneShader>& shader) {
            _shader = shader;
            return *this;
        }

        Vector3 size() const { return _size; }
        Mesh& setSize(const Vector3& size) {
            _size = size;
            return *this;
        }

        Mesh& setObjectId(UnsignedInt id) {
            _id = id;
            return *this;
        }

        Color4 ambientColor() { return _ambientColor; }
        Mesh& setAmbientColor(const Color4& color) {
            _ambientColor = color;
            return *this;
        }

        bool hasAmbientTexture() {
            if(_ambientTexture) return true;
            else return false;
        }
        Mesh& setAmbientTexture(const Resource<GL::Texture2D>& texture) {
            _ambientTexture = texture;
            return *this;
        }

        Color4 diffuseColor() { return _diffuseColor; }
        Mesh& setDiffuseColor(const Color4& color) {
            _diffuseColor = color;
            return *this;
        }

        bool hasDiffuseTexture() {
            if(_diffuseTexture) return true;
            else return false;
        }
        Mesh& setDiffuseTexture(const Resource<GL::Texture2D>& texture) {
            _diffuseTexture = texture;
            return *this;
        }

        bool hasNormalTexture() {
            if(_normalTexture) return true;
            else return false;
        }
        Mesh& setNormalTexture(const Resource<GL::Texture2D>& texture) {
            _normalTexture = texture;
            return *this;
        }

        Color4 specularColor() { return _specularColor; }
        Mesh& setSpecularColor(const Color4& color) {
            _specularColor = color;
            return *this;
        }

        bool hasSpecularTexture() {
            if(_specularTexture) return true;
            else return false;
        }
        Mesh& setSpecularTexture(const Resource<GL::Texture2D>& texture) {
            _specularTexture = texture;
            return *this;
        }

        Float shininess() { return _shininess; }
        Mesh& setShininess(Float shininess) {
            _shininess = shininess;
            return *this;
        }

    private:
        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) override {
            if(!_shader || !_mesh) return;

            _shader->setTransformationMatrix(transformation*Matrix4::scaling(_size/2))
                .setNormalMatrix(transformation.normalMatrix())
                .setProjectionMatrix(camera.projectionMatrix())
                .setAmbientColor(_ambientColor)
                .setDiffuseColor(_diffuseColor)
                .setSpecularColor(_specularColor)
                .setShininess(_shininess);

            if(_id) _shader->setObjectId(_id);
            if(_ambientTexture) _shader->bindAmbientTexture(*_ambientTexture);
            if(_diffuseTexture) _shader->bindDiffuseTexture(*_diffuseTexture);
            if(_normalTexture) _shader->bindNormalTexture(*_normalTexture);
            if(_specularTexture) _shader->bindSpecularTexture(*_specularTexture);

            _shader->draw(*_mesh);
        }

        Resource<GL::Mesh> _mesh;
        Resource<GL::AbstractShaderProgram, SceneShader> _shader;

        Vector3 _size{2.0f};
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

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

#include "MeshRenderer.h"

#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Camera.h>

#include "SceneShader.h"

namespace Oberon {

MeshRenderer::MeshRenderer(SceneGraph::AbstractObject3D& object, SceneGraph::DrawableGroup3D& drawables):
    SceneGraph::Drawable3D{object, &drawables} {}

void MeshRenderer::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) {
    if(!_mesh || !_shader) return;

    _shader->setTransformationMatrix(transformationMatrix)
        .setNormalMatrix(transformationMatrix.normalMatrix())
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

}

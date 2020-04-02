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

#include "Shader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/FormatStl.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

static void importCoreResources() {
    CORRADE_RESOURCE_INITIALIZE(OberonCore_RCS)
}

namespace Oberon {

Shader::Shader(UnsignedInt lightCount): _lightCount{lightCount} {
    if(!Utility::Resource::hasGroup("OberonCore"))
        importCoreResources();

    Utility::Resource rs("OberonCore");

    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex},
        frag{GL::Version::GL330, GL::Shader::Type::Fragment};

    vert.addSource(Utility::formatString("#define NUM_POINT_LIGHTS {}\n", lightCount))
        .addSource(rs.get("Shader.vert"));

    frag.addSource(Utility::formatString("#define NUM_POINT_LIGHTS {}\n", lightCount))
        .addSource(rs.get("Shader.frag"));

    CORRADE_INTERNAL_ASSERT(GL::Shader::compile({vert, frag}));

    attachShaders({vert, frag});

    CORRADE_INTERNAL_ASSERT(link());

    _transformationMatrixUniform = uniformLocation("transformationMatrix");
    _projectionMatrixUniform = uniformLocation("projectionMatrix");
    _ambientColorUniform = uniformLocation("ambientColor");
    _objectIdUniform = uniformLocation("objectId");
    if(lightCount) {
        _normalMatrixUniform = uniformLocation("normalMatrix");
        _diffuseColorUniform = uniformLocation("diffuseColor");
        _specularColorUniform = uniformLocation("specularColor");
        _shininessUniform = uniformLocation("shininess");
    }
}

Shader& Shader::setAmbientColor(const Color3& color) {
    setUniform(_ambientColorUniform, color);
    return *this;
}

Shader& Shader::setDiffuseColor(const Color3& color) {
    if(_lightCount) setUniform(_diffuseColorUniform, color);
    return *this;
}

Shader& Shader::setSpecularColor(const Color3& color) {
    if(_lightCount) setUniform(_specularColorUniform, color);
    return *this;
}

Shader& Shader::setShininess(Float shininess) {
    if(_lightCount) setUniform(_shininessUniform, shininess);
    return *this;
}

Shader& Shader::setObjectId(UnsignedInt id) {
    setUniform(_objectIdUniform, id);
    return *this;
}

Shader& Shader::setTransformationMatrix(const Matrix4& matrix) {
    setUniform(_transformationMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setNormalMatrix(const Matrix3x3& matrix) {
    if(_lightCount) setUniform(_normalMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(_projectionMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setLightPosition(UnsignedInt id, const Vector3& position) {
    CORRADE_INTERNAL_ASSERT(id < _lightCount);
    setUniform(uniformLocation("pointLights[" + std::to_string(id) + "].position"), position);
    return *this;
}

Shader& Shader::setLightColor(UnsignedInt id, const Color3& color) {
    CORRADE_INTERNAL_ASSERT(id < _lightCount);
    setUniform(uniformLocation("pointLights[" + std::to_string(id) + "].color"), color);
    return *this;
}

Shader& Shader::setLightAttributes(UnsignedInt id, Float constant, Float linear, Float quadratic) {
    CORRADE_INTERNAL_ASSERT(id < _lightCount);
    setUniform(uniformLocation("pointLights[" + std::to_string(id) + "].constant"), constant);
    setUniform(uniformLocation("pointLights[" + std::to_string(id) + "].linear"), linear);
    setUniform(uniformLocation("pointLights[" + std::to_string(id) + "].quadratic"), quadratic);
    return *this;
}

}

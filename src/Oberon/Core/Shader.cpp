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
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Version.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

static void importCoreResources() {
    CORRADE_RESOURCE_INITIALIZE(OberonCore_RCS)
}

namespace Oberon {

Shader::Shader(const UnsignedInt lightCount): _lightCount{lightCount} {
    if(!Utility::Resource::hasGroup("OberonCore"))
        importCoreResources();

    Utility::Resource rs("OberonCore");

    GL::Shader vert{GL::Version::GL330, GL::Shader::Type::Vertex},
        frag{GL::Version::GL330, GL::Shader::Type::Fragment};

    vert.addSource(Utility::formatString("#define LIGHT_COUNT {}\n", lightCount))
        .addSource(rs.get("Shader.vert"));

    frag.addSource(Utility::formatString("#define LIGHT_COUNT {}\n", lightCount))
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
        _lightPositionsUniform = uniformLocation("lightPositions");
        _lightColorsUniform = uniformLocation("lightColors");
    }
}

Shader& Shader::setTransformationMatrix(const Matrix4& matrix) {
    setUniform(_transformationMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(_projectionMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setNormalMatrix(const Matrix3x3& matrix) {
    if(_lightCount) setUniform(_normalMatrixUniform, matrix);
    return *this;
}

Shader& Shader::setAmbientColor(const Color3& color) {
    setUniform(_ambientColorUniform, color);
    return *this;
}

Shader& Shader::setObjectId(UnsignedInt id) {
    setUniform(_objectIdUniform, id);
    return *this;
}

Shader& Shader::setLightPositions(const Containers::ArrayView<const Vector3> positions) {
    CORRADE_INTERNAL_ASSERT(_lightCount == positions.size());
    if(_lightCount) setUniform(_lightPositionsUniform, positions);
    return *this;
}

Shader& Shader::setLightColors(const Containers::ArrayView<const Magnum::Color4> colors) {
    CORRADE_INTERNAL_ASSERT(_lightCount == colors.size());
    if(_lightCount) setUniform(_lightColorsUniform, colors);
    return *this;
}

}

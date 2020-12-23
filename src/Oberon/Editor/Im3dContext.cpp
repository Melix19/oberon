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

#include "Im3dContext.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Vector3.h>

#include "Oberon/Editor/Im3dIntegration.h"

namespace Oberon { namespace Editor {

Im3dShader::Im3dShader() {
    Utility::Resource rs("OberonEditor");

    GL::Shader vert(GL::Version::GL320, GL::Shader::Type::Vertex);
    GL::Shader frag(GL::Version::GL320, GL::Shader::Type::Fragment);

    vert.addSource(rs.get("Im3d.vert"));
    frag.addSource(rs.get("Im3d.frag"));

    CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));

    attachShaders({vert, frag});

    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
}

Im3dShader& Im3dShader::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(_transformationProjectionMatrixUniform, matrix);
    return *this;
}

Im3dContext::Im3dContext() {
    _mesh.addVertexBuffer(_vertexBuffer, 0,
        Im3dShader::PositionSize{},
        Im3dShader::Color{
            Im3dShader::Color::DataType::UnsignedByte,
            Im3dShader::Color::DataOption::Normalized});

    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_worldUp = Im3d::Vec3(0.0f, 1.0f, 0.0f);
    ad.m_projOrtho = false;
    ad.m_projScaleY = 4.0f;

    _timeline.start();
}

void Im3dContext::newFrame() {
    _timeline.nextFrame();

    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_deltaTime = _timeline.previousFrameDuration();

    Im3d::NewFrame();
}

void Im3dContext::drawFrame() {
    Im3d::EndFrame();

    for(Im3d::U32 i = 0; i < Im3d::GetDrawListCount(); ++i) {
        const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

        switch(drawList.m_primType) {
            case Im3d::DrawPrimitive_Points:
                _mesh.setPrimitive(GL::MeshPrimitive::Points);
                break;
            case Im3d::DrawPrimitive_Lines:
                _mesh.setPrimitive(GL::MeshPrimitive::Lines);
                break;
            case Im3d::DrawPrimitive_Triangles:
                _mesh.setPrimitive(GL::MeshPrimitive::Triangles);
                break;
            default:
                return;
        }

        _vertexBuffer.setData(
            {drawList.m_vertexData, std::size_t(drawList.m_vertexCount)},
            GL::BufferUsage::StreamDraw);

        _mesh.setCount(drawList.m_vertexCount);

        _shader.draw(_mesh);
    }
}

Im3dContext& Im3dContext::setViewportSize(const Vector2i& size) {
    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_viewportSize = Im3d::Vec2(size.x(), size.y());
    return *this;
}

Im3dContext& Im3dContext::setTransformationProjectionMatrix(const Matrix4& matrix) {
    _shader.setTransformationProjectionMatrix(matrix);
    return *this;
}

Im3dContext& Im3dContext::setCameraTranslation(const Vector3& translation) {
    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_viewOrigin = Im3d::Vec3(translation);
    return *this;
}

Im3dContext& Im3dContext::setCameraDirection(const Vector3& direction) {
    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_viewDirection = Im3d::Vec3(direction);
    return *this;
}

}}

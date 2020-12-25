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
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/SceneGraph/Camera.h>

#include "Oberon/Editor/Im3dIntegration.h"
#include "OberonExternal/im3d/im3d_math.h"

namespace Oberon { namespace Editor {

Im3dShader::Im3dShader(Type type) {
    Utility::Resource rs("OberonEditor");

    GL::Shader vert(GL::Version::GL320, GL::Shader::Type::Vertex);
    GL::Shader frag(GL::Version::GL320, GL::Shader::Type::Fragment);

    switch(type) {
        case Type::Points:
            vert.addSource("#define POINTS\n");
            frag.addSource("#define POINTS\n");
            break;
        case Type::Lines:
            vert.addSource("#define LINES\n");
            frag.addSource("#define LINES\n");
            break;
        case Type::Triangles:
            vert.addSource("#define TRIANGLES\n");
            frag.addSource("#define TRIANGLES\n");
            break;
    }

    vert.addSource(rs.get("Im3d.vert"));
    frag.addSource(rs.get("Im3d.frag"));

    if(type == Type::Lines) {
        GL::Shader geom(GL::Version::GL320, GL::Shader::Type::Geometry);
        geom.addSource(rs.get("Im3d.geom"));

        CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, geom, frag}));
        attachShaders({vert, geom, frag});
    } else {
        CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));
        attachShaders({vert, frag});
    }

    CORRADE_INTERNAL_ASSERT_OUTPUT(link());
}

Im3dShader& Im3dShader::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(_transformationProjectionMatrixUniform, matrix);
    return *this;
}

Im3dShader& Im3dShader::setViewport(const Vector2& size) {
    setUniform(_viewportUniform, size);
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
    ad.m_viewOrigin = Im3d::Vec3(_cameraObject->translation());
    ad.m_viewDirection = Im3d::Vec3(-_cameraObject->transformation().backward());

    Im3d::NewFrame();
}

void Im3dContext::drawFrame() {
    Im3d::EndFrame();

    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::ProgramPointSize);
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    _trianglesShader.setTransformationProjectionMatrix(_camera->projectionMatrix()*_camera->cameraMatrix());
    _linesShader.setTransformationProjectionMatrix(_camera->projectionMatrix()*_camera->cameraMatrix());
    _pointsShader.setTransformationProjectionMatrix(_camera->projectionMatrix()*_camera->cameraMatrix());

    for(Im3d::U32 i = 0; i < Im3d::GetDrawListCount(); ++i) {
        const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];

        _vertexBuffer.setData(
            {drawList.m_vertexData, std::size_t(drawList.m_vertexCount)},
            GL::BufferUsage::StreamDraw);

        _mesh.setCount(drawList.m_vertexCount);

        switch(drawList.m_primType) {
            case Im3d::DrawPrimitive_Points:
                GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

                _mesh.setPrimitive(GL::MeshPrimitive::Points);
                _pointsShader.draw(_mesh);
                break;
            case Im3d::DrawPrimitive_Lines:
                GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);

                _mesh.setPrimitive(GL::MeshPrimitive::Lines);
                _linesShader.draw(_mesh);
                break;
            case Im3d::DrawPrimitive_Triangles:
                GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

                _mesh.setPrimitive(GL::MeshPrimitive::Triangles);
                _trianglesShader.draw(_mesh);
                break;
            default:
                return;
        }
    }

    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::Zero);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
    GL::Renderer::disable(GL::Renderer::Feature::ProgramPointSize);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
}

void Im3dContext::updateCursorRay(const Vector2& cursorPosition) {
    Im3d::AppData& ad = Im3d::GetAppData();
    Im3d::Vec2 cursorPos(cursorPosition);
    cursorPos = (cursorPos/ad.m_viewportSize)*2.0f - 1.0f;
    cursorPos.y = -cursorPos.y;

    Im3d::Vec3 rayOrigin, rayDirection;
    rayOrigin = ad.m_viewOrigin;
    rayDirection.x = cursorPos.x/_camera->projectionMatrix()[0][0];
    rayDirection.y = cursorPos.y/_camera->projectionMatrix()[1][1];
    rayDirection.z = -1.0f;
    rayDirection = Im3d::Mat4(_cameraObject->transformation())*Im3d::Vec4(Normalize(rayDirection), 0.0f);

    ad.m_cursorRayOrigin = rayOrigin;
    ad.m_cursorRayDirection = rayDirection;
}

Im3dContext& Im3dContext::setViewportSize(const Vector2i& size) {
    Im3d::AppData& ad = Im3d::GetAppData();
    ad.m_viewportSize = Im3d::Vec2(Vector2{size});
    _linesShader.setViewport(Vector2{size});
    return *this;
}

}}

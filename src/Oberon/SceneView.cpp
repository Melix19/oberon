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

#include "SceneView.h"

#include <algorithm>
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "Oberon/SceneImporter.h"

namespace Oberon {

SceneView::SceneView(const std::string& path, const Vector2i& viewportSize) {
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    SceneImporter::load(path, _data);

    _data.camera->setViewport(viewportSize);
}

void SceneView::draw() {
    /* Calculate light data and upload them to all shaders */
    arrayResize(_data.lightPositions, 0);
    arrayResize(_data.lightColors, 0);
    arrayResize(_data.lightRanges, 0);
    _data.camera->draw(_data.lightDrawables);
    CORRADE_INTERNAL_ASSERT(_data.lightPositions.size() == _data.lightCount);
    CORRADE_INTERNAL_ASSERT(_data.lightColors.size() == _data.lightCount);
    CORRADE_INTERNAL_ASSERT(_data.lightRanges.size() == _data.lightCount);
    for(const std::string& shaderKey: _data.phongShadersKeys) {
        Resource<GL::AbstractShaderProgram, Shaders::Phong> shader =
            _data.resourceManager.get<GL::AbstractShaderProgram, Shaders::Phong>(shaderKey);
        if(shader) {
            (*shader)
                .setLightPositions(_data.lightPositions)
                .setLightColors(_data.lightColors)
                .setLightRanges(_data.lightRanges);
        }
    }

    /* Draw opaque stuff */
    _data.camera->draw(_data.opaqueDrawables);

    /* Draw transparent stuff back-to-front with blending enabled */
    if(!_data.transparentDrawables.isEmpty()) {
        GL::Renderer::setDepthMask(false);
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

        std::vector<std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>>
            drawableTransformations = _data.camera->drawableTransformations(_data.transparentDrawables);
        std::sort(drawableTransformations.begin(), drawableTransformations.end(),
            [](const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& a,
                const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& b) {
                return a.second.translation().z() > b.second.translation().z();
            });
        _data.camera->draw(drawableTransformations);

        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::Zero);
        GL::Renderer::disable(GL::Renderer::Feature::Blending);
        GL::Renderer::setDepthMask(true);
    }
}

void SceneView::updateViewport(const Vector2i& size) {
    _data.camera->setViewport(size);
}

}

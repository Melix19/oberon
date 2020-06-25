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
#include <Magnum/SceneGraph/AbstractGroupedFeature.h>
#include <Magnum/SceneGraph/Camera.h>

#include "SceneShader.h"

namespace Oberon {

class Light: public SceneGraph::AbstractGroupedFeature3D<Light> {
    public:
        explicit Light(SceneGraph::AbstractObject3D& object, LightGroup& lights, OberonResourceManager& resourceManager):
            SceneGraph::AbstractGroupedFeature3D<Light>{object, &lights}, _resourceManager{resourceManager}, _id(lights.size() - 1) {}

        void updateShader(SceneGraph::Camera3D& camera, std::vector<std::pair<std::string, SceneShader::Flags>>& shaderKeys) {
            for(auto it = shaderKeys.begin(); it != shaderKeys.end();) {
                Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, SceneShader>(it->first);
                if(shaderResource) {
                    shaderResource->setPointLight(_id, camera.cameraMatrix().transformPoint(object().absoluteTransformationMatrix().translation()), _color, _constant, _linear, _quadratic);
                    ++it;
                } else {
                    it = shaderKeys.erase(it);
                }
            }
        }

        Light& setId(UnsignedInt id) {
            _id = id;
            return *this;
        }

        Color4 color() { return _color; }
        Light& setColor(const Color4& color) {
            _color = color;
            return *this;
        }

        Float constant() { return _constant; }
        Light& setConstant(Float constant) {
            _constant = constant;
            return *this;
        }

        Float linear() { return _linear; }
        Light& setLinear(Float linear) {
            _linear = linear;
            return *this;
        }

        Float quadratic() { return _quadratic; }
        Light& setQuadratic(Float quadratic) {
            _quadratic = quadratic;
            return *this;
        }

    private:
        OberonResourceManager& _resourceManager;

        UnsignedInt _id;
        Color4 _color{1.0f};
        Float _constant{1.0f};
        Float _linear{0.09f};
        Float _quadratic{0.032f};
};

}

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

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Resource.h>
#include <Magnum/SceneGraph/AbstractGroupedFeature.h>

#include "Shader.h"

using namespace Magnum;

class Light;
typedef SceneGraph::FeatureGroup3D<Light> LightGroup;

class Light: public SceneGraph::AbstractGroupedFeature3D<Light> {
    public:
        explicit Light(SceneGraph::AbstractObject3D& object, LightGroup* lights, Resource<GL::AbstractShaderProgram, Oberon::Shader>& shader, Color3& color):
            SceneGraph::AbstractGroupedFeature3D<Light>{object, lights}, _id(lights->size() - 1), _shader(shader), _color(color) {}

        Light& setId(UnsignedInt id) {
            _id = id;
            return *this;
        }

        Light& setColor(Color3& color) {
            _color = color;
            return *this;
        }

        void applyShader() {
            _shader->setLightPosition(_id, SceneGraph::AbstractGroupedFeature3D<Light>::object().transformationMatrix().translation());
            _shader->setLightColor(_id, _color);
        }

    private:
        UnsignedInt _id;
        Resource<GL::AbstractShaderProgram, Oberon::Shader> _shader;

        Color3 _color;
};

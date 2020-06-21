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

#include <Magnum/ResourceManager.h>
#include <Magnum/Timeline.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Oberon/SceneShader.h>

namespace Oberon { namespace ExportTemplate {

class AbstractApplication {
    protected:
        AbstractApplication();
        void loadCompiledReources(Utility::Configuration& collectionConfig, Utility::Resource& resources, Importer& importer);

        Scene3D _scene;
        Object3D* _cameraObject;
        SceneGraph::Camera3D* _camera;

        Timeline _timeline;

        SceneGraph::DrawableGroup3D _drawables;
        LightGroup _lights;

        OberonResourceManager _resourceManager;
        std::vector<std::pair<std::string, SceneShader::Flags>> shaderKeys;
};

}}

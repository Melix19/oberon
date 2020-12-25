#ifndef Oberon_SceneData_h
#define Oberon_SceneData_h
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

#include <Corrade/Containers/Array.h>
#include <Corrade/PluginManager/Manager.h>
#include <Magnum/ResourceManager.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>
#include <Magnum/Trade/Trade.h>

#include "Oberon/Oberon.h"

namespace Oberon {

typedef ResourceManager<GL::AbstractShaderProgram, GL::Mesh, GL::Texture2D> SceneResourceManager;

struct ObjectInfo {
    Object3D* object;
    std::string name;
    std::vector<UnsignedInt> children;

    enum class FeatureTypes: UnsignedInt {
        PhongDrawable,
        LightDrawable
    };
    Containers::Array<SceneGraph::AbstractFeature3D*> features{Containers::ValueInit, 2};
};

struct SceneData {
    PluginManager::Manager<Trade::AbstractImporter> manager;

    SceneResourceManager resourceManager;

    Scene3D scene;
    Object3D* cameraObject{};
    SceneGraph::Camera3D* camera;
    SceneGraph::DrawableGroup3D opaqueDrawables, transparentDrawables,
        lightDrawables;

    Containers::Array<ObjectInfo> objects;
    UnsignedInt sceneObjectId{};

    UnsignedInt lightCount{};
    Containers::Array<Vector4> lightPositions;
    Containers::Array<Color3> lightColors;
    Containers::Array<Float> lightRanges;

    Containers::Array<std::string> phongShadersKeys;
};

}

#endif

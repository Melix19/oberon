/*
    MIT License

    Copyright (c) 2019 Marco Melorio

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

#include "Entity.h"
#include "RectangleShape.h"

#include <Corrade/Utility/ConfigurationGroup.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Square.h>
#include <Magnum/ResourceManager.h>
#include <Magnum/SceneGraph/TranslationRotationScalingTransformation2D.h>
#include <Magnum/Trade/MeshData2D.h>

typedef SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation2D> Object2D;
typedef SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation2D> Scene2D;

typedef ResourceManager<GL::Mesh, Shaders::Flat2D> OberonResourceManager;

namespace EntitySerializer {

Object2D* createEntityFromConfig(Utility::ConfigurationGroup* entityGroup, Object2D* parent, SceneGraph::DrawableGroup2D* drawables, OberonResourceManager& resourceManager);
void addComponentFromConfig(Utility::ConfigurationGroup* componentGroup, Object2D* object, SceneGraph::DrawableGroup2D* drawables, OberonResourceManager& resourceManager);

}

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

#include <Corrade/PluginManager/Manager.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "SceneShader.h"

class Importer {
    public:
        Importer(OberonResourceManager& resourceManager): _resourceManager(resourceManager) {}

        Object3D* loadObject(Utility::ConfigurationGroup* objectConfig, Object3D* parent, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights);
        Object3D* loadChildrenObject(Utility::ConfigurationGroup* parentConfig, Object3D* parent, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights);

        SceneGraph::AbstractFeature3D* loadFeature(Utility::ConfigurationGroup* featureConfig, Object3D* object, SceneGraph::DrawableGroup3D* drawables, ScriptGroup* scripts, LightGroup* lights);
        void resetObject(Object3D* object, Utility::ConfigurationGroup* objectConfig);

        void updateMeshPrimitive(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig);

        Resource<GL::AbstractShaderProgram, SceneShader> createShader(Mesh& mesh, UnsignedInt lightCount, std::vector<std::pair<std::string, SceneShader::Flags>>& shaderKeys, bool useObjectId);
        void createShaders(SceneGraph::DrawableGroup3D* drawables, UnsignedInt lightCount, std::vector<std::pair<std::string, SceneShader::Flags>>& shaderKeys, bool useObjectId = false);

        GL::Texture2D loadTexture(Containers::ArrayView<const char> data);

    private:
        std::pair<std::string, SceneShader::Flags> calculateShaderKey(Mesh& mesh, bool useObjectId);

    private:
        OberonResourceManager& _resourceManager;
        PluginManager::Manager<Trade::AbstractImporter> _importerManager;
};

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

namespace Oberon {

class Importer {
    public:
        Importer(OberonResourceManager& resourceManager): _resourceManager(resourceManager) {}

        Object3D* loadObject(Utility::ConfigurationGroup* objectConfig, Object3D* parent, GameData& gameData);
        Object3D* loadChildrenObject(Utility::ConfigurationGroup* parentConfig, Object3D* parent, GameData& gameData);

        SceneGraph::AbstractFeature3D* loadFeature(Utility::ConfigurationGroup* featureConfig, Object3D* object, GameData& gameData);
        void resetObject(Object3D* object, Utility::ConfigurationGroup* objectConfig);

        void generateMeshPrimitive(MeshRenderer& meshRenderer, Utility::ConfigurationGroup* meshConfiguration);

        Resource<GL::AbstractShaderProgram, SceneShader> createShader(MeshRenderer& meshRenderer, GameData& gameData, bool useObjectId);
        void createShaders(GameData& gameData, bool useObjectId = false);

        Resource<GL::Texture2D> loadTexture(const std::string& resourcePath, Containers::ArrayView<const char> data);

    private:
        std::pair<std::string, SceneShader::Flags> calculateShaderKey(MeshRenderer& meshRenderer, bool useObjectId);

    private:
        OberonResourceManager& _resourceManager;
        PluginManager::Manager<Trade::AbstractImporter> _importerManager;
};

}

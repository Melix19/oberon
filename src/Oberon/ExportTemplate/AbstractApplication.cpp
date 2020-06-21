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

#include "AbstractApplication.h"

#include <sstream>
#include <physfs.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>

namespace Oberon { namespace ExportTemplate {

namespace {

Containers::Array<char> read(const std::string& filename) {
    PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
    std::size_t fileLength = PHYSFS_fileLength(file);
    Containers::Array<char> data{fileLength};

    PHYSFS_readBytes(file, data, data.size());
    return data;
}

std::string readString(const std::string& filename) {
    PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
    std::size_t fileLength = PHYSFS_fileLength(file);
    Containers::Array<char> data{fileLength + 1};

    PHYSFS_readBytes(file, data, data.size());
    /* Insert final null character */
    data[data.size() - 1] = 0;
    return std::string(data);
}

void loadCollectionResources(Utility::Configuration& collectionConfiguration, Importer& importer) {
    Utility::ConfigurationGroup* resourcesGroup = collectionConfiguration.group("external_resources");
    if(!resourcesGroup)
        return;

    for(Utility::ConfigurationGroup* resourceGroup: resourcesGroup->groups("resource")) {
        const std::string resourceType = resourceGroup->value("type");
        const std::string resourcePath = resourceGroup->value("path");

        if(resourceType == "Texture2D") {
            Containers::Array<char> data = read(resourcePath);
            importer.loadTexture(resourcePath, data);
        }
    }
}

}

AbstractApplication::AbstractApplication(const char* argv0) {
    const std::string executableLocation = Utility::Directory::executableLocation();
    const std::string executableParentPath = Utility::Directory::path(executableLocation);
    const std::string executableName = Utility::Directory::filename(executableLocation);
    const std::string dataPath = Utility::Directory::join(executableParentPath, executableName + "-data.zip");

    PHYSFS_init(argv0);
    PHYSFS_mount(dataPath.c_str(), nullptr, 1);

    /* Load project configuration */
    std::istringstream projectConfigurationStream(readString("project.oberon"));
    Utility::Configuration projectConfiguration{projectConfigurationStream};
    std::string mainCollection = projectConfiguration.value("main_collection");

    /* Load main collection configuration */
    std::istringstream collectionConfigurationStream(readString(mainCollection));
    Utility::Configuration collectionConfiguration{collectionConfigurationStream};

    /* Load main collection resources */
    Importer importer{_resourceManager};
    loadCollectionResources(collectionConfiguration, importer);

    PHYSFS_deinit();

    /* Load scene */
    _collectionObject = new Object3D{&_scene};
    Utility::ConfigurationGroup* sceneGroup = collectionConfiguration.group("scene");
    importer.loadChildrenObject(sceneGroup, _collectionObject, &_drawables, &_lights);

    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    _cameraObject = new Object3D{_collectionObject};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(Deg{90.0f}, 4.0f/3.0f, 0.001f, 100.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    importer.createShaders(&_drawables, _lights.size(), _shaderKeys);

    _timeline.start();
}

AbstractApplication::~AbstractApplication() {
    /* Needed to destroy all the referenced resources so the ResourceManager
       does not complain that it's been destroyed while there are referenced
       data. */
    delete _collectionObject;
}

}}

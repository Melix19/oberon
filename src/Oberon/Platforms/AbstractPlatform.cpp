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

#include "AbstractPlatform.h"

#include <sstream>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>

static void importApplicationResources() {
    CORRADE_RESOURCE_INITIALIZE(OberonApplication_RCS)
}

AbstractPlatform::AbstractPlatform() {
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    using namespace Math::Literals;

    _cameraObject = new Object3D{&_scene};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 4.0f/3.0f, 0.001f, 100.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    if(!Utility::Resource::hasGroup("OberonApplication"))
        importApplicationResources();

    Utility::Resource resources("OberonApplication");

    std::istringstream projectConfigurationStream(resources.get("project.oberon"));
    Utility::Configuration projectConfiguration(projectConfigurationStream);

    std::string mainCollection = projectConfiguration.value("main_collection");
    std::istringstream collectionStream(resources.get(mainCollection));
    Utility::Configuration collectionConfig{collectionStream};
    Utility::ConfigurationGroup* sceneConfig = collectionConfig.group("scene");

    Importer importer{_resourceManager};
    loadCompiledReources(collectionConfig, resources, importer);

    importer.loadChildrenObject(sceneConfig, &_scene, &_drawables, &_lights);
    importer.createShaders(&_drawables, _lights.size(), shaderKeys);

    _timeline.start();
}

void AbstractPlatform::loadCompiledReources(Utility::Configuration& collectionConfig, Utility::Resource& resources, Importer& importer) {
    Utility::ConfigurationGroup* resourcesGroup = collectionConfig.group("external_resources");
    if(!resourcesGroup)
        return;

    for(Utility::ConfigurationGroup* resource: resourcesGroup->groups("resource")) {
        std::string resourceType = resource->value("type");
        std::string resourcePath = resource->value("path");

        if(resourceType == "Texture2D")
            importer.loadTexture(resourcePath, resources);
    }
}

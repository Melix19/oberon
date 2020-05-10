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

#include "GlfwPlatform.h"

#include <sstream>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Directory.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Oberon/Core/Importer.h>
#include <Oberon/Core/Light.h>
#include <Oberon/Core/Script.h>

static void importApplicationResources() {
    CORRADE_RESOURCE_INITIALIZE(OberonApplication_RCS)
}

GlfwPlatform::GlfwPlatform(const Arguments& arguments): Platform::Application{arguments,
    Configuration{}.setTitle("Application")
                   .setSize({1024, 576})}
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

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

    std::string mainCollectionPath = projectConfiguration.value("main-collection-path");
    std::istringstream collectionStream(resources.get(mainCollectionPath));
    Utility::Configuration collection(collectionStream);
    Utility::ConfigurationGroup* sceneConfiguration = collection.group("scene");

    Importer importer;
    importer.loadChildrenObject(sceneConfiguration, &_scene, _resourceManager, &_drawables, &_scripts, &_lights);

    _scriptManager.loadScripts(_scripts);

    _timeline.start();
}

void GlfwPlatform::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    _scriptManager.update(_timeline.previousFrameDuration());

    for(std::size_t i = 0; i != _lights.size(); ++i)
        _lights[i].updateShader(*_camera);

    _camera->draw(_drawables);

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

MAGNUM_APPLICATION_MAIN(GlfwPlatform)

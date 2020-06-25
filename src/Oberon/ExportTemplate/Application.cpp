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

#include "Application.h"

#include <sstream>
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/Configuration.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>

#include "PackingUtility.h"

namespace Oberon { namespace ExportTemplate {

namespace {

void loadCollectionResources(Utility::Configuration& collectionConfiguration, Importer& importer) {
    Utility::ConfigurationGroup* resourcesGroup = collectionConfiguration.group("external_resources");
    if(!resourcesGroup)
        return;

    for(Utility::ConfigurationGroup* resourceGroup: resourcesGroup->groups("resource")) {
        const std::string resourceType = resourceGroup->value("type");
        const std::string resourcePath = resourceGroup->value("path");

        if(resourceType == "Texture2D") {
            Containers::Array<char> data = PackingUtility::read(resourcePath);
            importer.loadTexture(resourcePath, data);
        }
    }
}

}

Application::Application(const Arguments& arguments, const Configuration& configuration, const Utility::Configuration& projectConfiguration):
    Platform::Application{arguments, configuration}
{
    /* Load main collection configuration */
    std::string mainCollection = projectConfiguration.value("main_collection");
    std::istringstream collectionConfigurationStream(PackingUtility::readString(mainCollection));
    Utility::Configuration collectionConfiguration{collectionConfigurationStream};

    /* Load main collection resources */
    Importer importer{_resourceManager};
    loadCollectionResources(collectionConfiguration, importer);

    /* Load scene */
    _collectionObject = new Object3D{&_scene};
    Utility::ConfigurationGroup* sceneGroup = collectionConfiguration.group("scene");
    importer.loadChildrenObject(sceneGroup, _collectionObject, _gameData);

    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    Object3D* cameraObject = new Object3D{_collectionObject};
    SceneGraph::Camera3D* camera = new SceneGraph::Camera3D{*cameraObject};
    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(Deg{90.0f}, 4.0f/3.0f, 0.001f, 100.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    _gameData.setCamera(camera);

    importer.createShaders(_gameData);

    _timeline.start();
}

Application::~Application() {
    /* Needed to destroy all the referenced resources so the ResourceManager
       does not complain that it's been destroyed while there are referenced
       data. */
    delete _collectionObject;
}

void Application::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    for(std::size_t i = 0; i != _gameData.lights().size(); ++i)
        _gameData.lights()[i].updateShader(*_gameData.camera(), _gameData.shaderKeys());

    _gameData.camera()->draw(_gameData.drawables());

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

}}

using namespace Magnum;
using namespace Oberon::ExportTemplate;

int main(int argc, char** argv) {
    PackingUtility::initialize(argv[0]);

    /* Load project configuration */
    std::istringstream projectConfigurationStream(PackingUtility::readString("project.oberon"));
    const Utility::Configuration projectConfiguration{projectConfigurationStream};
    const std::string title = projectConfiguration.value("name");
    const Vector2i windowSize = projectConfiguration.value<Vector2i>("window_size");

    Platform::Application::Configuration configuration{};
    configuration
        .setTitle(title)
        .setSize(windowSize);

    Application app({argc, argv}, configuration, projectConfiguration);

    PackingUtility::deinitialize();

    return app.exec();
}

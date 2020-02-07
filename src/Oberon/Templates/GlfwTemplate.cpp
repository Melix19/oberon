#include "GlfwTemplate.h"

#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/Renderer.h>
#include <Oberon/Bindings/Oberon/Python.h>

GlfwTemplate::GlfwTemplate(const Arguments& arguments): Platform::Application{arguments,
    Configuration{}, GLConfiguration{}.setColorBufferSize({8, 8, 8, 8})}
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    using namespace Math::Literals;

    _cameraObject = new Object3D{&_scene};
    _cameraObject->translate(Vector3::zAxis(5.0f));
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 4.0f/3.0f, 0.001f, 100.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    std::string projectPath = Utility::Directory::executableLocation();

    Utility::Configuration collectionConfig{Utility::Directory::join(projectPath, "")};
    Utility::ConfigurationGroup* sceneConfig = collectionConfig.group("scene");

    Importer::loadChildrenObject(sceneConfig, &_scene, _resourceManager, &_drawables, &_scripts, &_lights);

    setup(projectPath);

    try {
        for(std::size_t i = 0; i != _scripts.size(); ++i) {
            Script& script = _scripts[i];

            script.pyModule() = py::module::import(script.path().c_str());
            script.pyModule().attr("init")(&script.object());
        }
    } catch (py::error_already_set const &pythonErr) {
        py::print(pythonErr.what());
    }

    _timeline.start();
}

void GlfwTemplate::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    for(std::size_t i = 0; i != _scripts.size(); ++i) {
        Script& script = _scripts[i];
        script.pyModule().attr("update")(&script.object(), _timeline.previousFrameDuration());
    }

    for(std::size_t i = 0; i != _lights.size(); ++i)
        _lights[i].updateShader();

    _camera->draw(_drawables);

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

MAGNUM_APPLICATION_MAIN(GlfwTemplate)

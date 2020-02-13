#pragma once

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Timeline.h>

#include <Oberon/Core/Importer.h>

class GlfwTemplate: public Platform::Application {
    public:
        explicit GlfwTemplate(const Arguments& arguments);

    private:
        void drawEvent() override;

        Scene3D _scene;
        Object3D* _cameraObject;
        SceneGraph::Camera3D* _camera;

        Timeline _timeline;

        SceneGraph::DrawableGroup3D _drawables;
        ScriptGroup _scripts;
        LightGroup _lights;

        OberonResourceManager _resourceManager;
};

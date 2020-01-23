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

#include "CollectionPanel.h"

#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/ImGuiIntegration/Integration.h>
#include <Magnum/Math/ConfigurationValue.h>

CollectionPanel::CollectionPanel(const std::string& collectionPath, OberonResourceManager& resourceManager, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio):
    _collectionPath(collectionPath), _resourceManager(resourceManager), _viewportTextureSize(viewportTextureSize), _dpiScaleRatio(dpiScaleRatio), _isOrthographicCamera(true),
    _collectionConfig{_collectionPath}, _isOpen(true), _isFocused(false), _isVisible(true), _isDragging(false), _needsFocus(true), _needsDocking(true), _isSimulating(false)
{
    _name = Utility::Directory::filename(_collectionPath);

    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, _viewportTextureSize*_dpiScaleRatio);
    _depth.setStorage(GL::RenderbufferFormat::Depth24Stencil8, _viewportTextureSize*_dpiScaleRatio);
    _objectId.setStorage(GL::RenderbufferFormat::R32UI, _viewportTextureSize*_dpiScaleRatio);

    _framebuffer = GL::Framebuffer{{}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0)
        .attachRenderbuffer(GL::Framebuffer::ColorAttachment{1}, _objectId)
        .attachRenderbuffer(GL::Framebuffer::BufferAttachment::Depth, _depth)
        .mapForDraw({
            {Oberon::Shader::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
            {Oberon::Shader::ObjectIdOutput, GL::Framebuffer::ColorAttachment{1}}});

    _cameraObject = new Object3D{&_scene};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);

    if(!_collectionConfig.hasGroup("scene")) {
        Utility::ConfigurationGroup* sceneConfig = _collectionConfig.addGroup("scene");
        sceneConfig->setValue("name", "root");
    }

    Object3D* rootObject = Serializer::createObjectFromConfig(_collectionConfig.group("scene"),
        &_scene, resourceManager, &_drawables, &_scripts, &_lights, _drawablesNodes.size() + 1);
    _rootNode = Containers::pointer<ObjectNode>(rootObject, _collectionConfig.group("scene"));

    if(!_drawables.isEmpty() && rootObject == &_drawables[_drawables.size() - 1].object())
        _drawablesNodes.push_back(_rootNode.get());

    updateObjectNodeChildren(_rootNode.get());
}

void CollectionPanel::drawViewport(Float deltaTime) {
    /* If the window is not visible, just end the method here. */
    if(!_isVisible || !_isOpen)
        return;

    _framebuffer
        .clearColor(0, Color3{0.12f})
        .clearColor(1, Vector4ui{0})
        .clearDepth(1.0f)
        .bind();

    if(_isSimulating) {
        for(std::size_t i = 0; i != _scripts.size(); ++i) {
            Script& script = _scripts[i];
            script.pyModule().attr("update")(&script.object(), deltaTime);
        }
    }

    for(std::size_t i = 0; i != _lights.size(); ++i)
        _lights[i].applyShader();

    _camera->draw(_drawables);
}

void CollectionPanel::newFrame() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(), ImVec2(_viewportTextureSize.x(),
        _viewportTextureSize.y()));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    _isVisible = ImGui::Begin(_name.c_str(), &_isOpen, ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar);
    ImGui::PopStyleVar();

    _isFocused = ImGui::IsWindowFocused();
    _isHovered = ImGui::IsWindowHovered();

    /* If the window is not visible, just end the method here. */
    if(!_isVisible || !_isOpen) {
        ImGui::End();
        return;
    }

    if(ImGui::BeginMenuBar()) {
        std::string buttonStr;
        if(_isOrthographicCamera)
            buttonStr = "Perspective";
        else
            buttonStr = "Orthographic";

        if(ImGui::Button(buttonStr.c_str())) {
            _isOrthographicCamera = !_isOrthographicCamera;

            Matrix4 currentCameraTransformation = _cameraObject->transformation();
            _cameraObject->setTransformation(_prevCameraTransformation);
            _prevCameraTransformation = currentCameraTransformation;
        }

        ImGui::EndMenuBar();
    }

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();

    _viewportPos = Vector2{windowPos.x, windowPos.y - (_viewportTextureSize.y() - windowSize.y)};
    ImGui::GetWindowDrawList()->AddImage(static_cast<ImTextureID>(&_viewportTexture), ImVec2(_viewportPos),
        ImVec2(_viewportPos + Vector2{_viewportTextureSize}), ImVec2(0, 1), ImVec2(1, 0));

    _viewportSize = {ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y};

    _framebuffer.setViewport({{}, Vector2i{_viewportSize*_dpiScaleRatio}});

    if(_isOrthographicCamera)
        _camera->setProjectionMatrix(Matrix4::orthographicProjection(_viewportSize, -1000.0f, 1000.0f));
    else
        _camera->setProjectionMatrix(Matrix4::perspectiveProjection(Deg{70.0f}, _viewportSize.x()/
            _viewportSize.y(), 0.05, 500.0f));

    _camera->setViewport(Vector2i{_viewportSize});

    ImGui::End();
}

void CollectionPanel::addFeatureToObject(Utility::ConfigurationGroup* objectConfig, Object3D* object) {
    Serializer::addFeatureFromConfig(objectConfig, object, _resourceManager, &_drawables, &_scripts, &_lights,
        _drawablesNodes.size() + 1);
}

void CollectionPanel::save() {
    _collectionConfig.save();
}

void CollectionPanel::updateObjectNodeChildren(ObjectNode* node) {
    for(auto childConfig : node->objectConfig()->groups("child")) {
        Object3D* child = Serializer::createObjectFromConfig(childConfig, node->object(),
            _resourceManager, &_drawables, &_scripts, &_lights, _drawablesNodes.size() + 1);
        ObjectNode* childNode = node->addChild(child, childConfig);

        if(!_drawables.isEmpty() && child == &_drawables[_drawables.size() - 1].object())
            _drawablesNodes.push_back(childNode);

        Math::Vector3<Rad> rotationRadians = Quaternion::fromMatrix(childConfig->value<Matrix4>("transformation").
            rotation()).toEuler();

        childNode->setRotationDegree(Vector3{Float(Deg{rotationRadians.x()}), Float(Deg{rotationRadians.y()}),
            Float(Deg{rotationRadians.z()})});

        updateObjectNodeChildren(childNode);
    }
}

CollectionPanel& CollectionPanel::startSimulation() {
    try {
        for(std::size_t i = 0; i != _scripts.size(); ++i) {
            Script& script = _scripts[i];

            if(script.pyModule())
                script.pyModule().reload();
            else
                script.pyModule() = py::module::import(script.path().c_str());

            script.pyModule().attr("init")(&script.object());
        }

        _isSimulating = true;
    } catch (py::error_already_set const &pythonErr) {
        py::print(pythonErr.what());
    }


    return *this;
}

CollectionPanel& CollectionPanel::stopSimulation() {
    resetObjectAndChildren(_rootNode.get());

    _isSimulating = false;
    return *this;
}

void CollectionPanel::resetObjectAndChildren(ObjectNode* node) {
    Serializer::resetObjectFromConfig(node->object(), node->objectConfig());

    for(auto& child : node->children())
        resetObjectAndChildren(child.get());
}

void CollectionPanel::updateShader() {
    Resource<GL::AbstractShaderProgram, Oberon::Shader> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, Oberon::Shader>("shader");
    _resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Oberon::Shader{UnsignedInt(_lights.size())}, ResourceDataState::Mutable, ResourcePolicy::Resident);

    for(std::size_t i = 0; i != _lights.size(); ++i)
        _lights[i].setId(i);
}

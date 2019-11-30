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

#include "CollectionPanel.h"

#include <algorithm>

#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/ImGuiIntegration/Widgets.h>
#include <pybind11/embed.h>

namespace py = pybind11;

CollectionPanel::CollectionPanel(const std::string& path, OberonResourceManager& resourceManager, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio):
    _path(path), _resourceManager(resourceManager), _viewportTextureSize(viewportTextureSize), _dpiScaleRatio(dpiScaleRatio), _collectionConfig{_path},
    _rootNode(&_scene, &_collectionConfig), _isOpen(true), _isVisible(true), _isFocused(false), _needsFocus(true), _needsDocking(true), _isSimulating(false)
{
    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, _viewportTextureSize * _dpiScaleRatio);
    _framebuffer = GL::Framebuffer{{}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0);

    _cameraObject = new Object3D{&_scene};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);

    if(_collectionConfig.hasGroup("child"))
        addEntityNodeChild(_collectionConfig.group("child"), &_rootNode);
}

void CollectionPanel::drawViewport(Float deltaTime) {
    /* If the window is not visible, just end the method here. */
    if(!_isVisible || !_isOpen)
        return;

    _framebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    if(_isSimulating) {
        for(std::size_t i = 0; i != _scripts.size(); ++i) {
            Script& script = _scripts[i];
            py::module module = py::module::import(script.scriptPath().c_str());
            module.attr("update")(&script.object(), deltaTime);
        }
    }

    std::vector<std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>>
        drawableTransformations = _camera->drawableTransformations(_drawables);

    std::sort(drawableTransformations.begin(), drawableTransformations.end(),
        [](const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& a,
        const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& b) {
            return a.second.translation().z() < b.second.translation().z();
        });

    _camera->draw(drawableTransformations);
}

void CollectionPanel::newFrame() {
    std::string filename = Utility::Directory::filename(_path);

    ImGui::SetNextWindowSizeConstraints(ImVec2(), ImVec2(_viewportTextureSize.x(), _viewportTextureSize.y()));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    _isVisible = ImGui::Begin(filename.c_str(), &_isOpen, ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    _isFocused = ImGui::IsWindowFocused();

    /* If the window is not visible, just end the method here. */
    if(!_isVisible || !_isOpen) {
        ImGui::End();
        return;
    }

    ImGui::SetScrollY(_viewportTextureSize.y());
    ImGuiIntegration::image(_viewportTexture, Vector2{_viewportTextureSize});

    Vector2i windowContentSize{Int(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x),
        Int(ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y)};

    _framebuffer.setViewport({{}, windowContentSize * _dpiScaleRatio});
    _camera->setProjectionMatrix(Matrix4::orthographicProjection(Vector2{windowContentSize}, -1000.0f, 1000.0f))
        .setViewport(windowContentSize);

    ImGui::End();
}

void CollectionPanel::addComponentToEntity(Utility::ConfigurationGroup* entityConfig, Object3D* object) {
    EntitySerializer::addComponentFromConfig(entityConfig, object, _resourceManager, &_drawables, &_scripts);
}

void CollectionPanel::save() {
    _collectionConfig.save();
}

void CollectionPanel::addEntityNodeChild(Utility::ConfigurationGroup* entityConfig, EntityNode* parentNode) {
    Object3D* entity = EntitySerializer::createEntityFromConfig(entityConfig, parentNode->entity(),
        _resourceManager, &_drawables, &_scripts);
    EntityNode* node = parentNode->addChild(entity, entityConfig);

    Math::Vector3<Rad> rotationRadians = Quaternion::fromMatrix(entityConfig->value<Matrix4>("transformation").
        rotation()).toEuler();

    node->setRotationDegree(Vector3{Float(Deg(rotationRadians.x())), Float(Deg(rotationRadians.y())),
        Float(Deg(rotationRadians.z()))});

    for(auto childGroup : entityConfig->groups("child"))
        addEntityNodeChild(childGroup, node);
}

CollectionPanel& CollectionPanel::startSimulation() {
    for(std::size_t i = 0; i != _scripts.size(); ++i) {
        Script& script = _scripts[i];
        py::module module = py::module::import(script.scriptPath().c_str());
        module.attr("init")(&script.object());
    }

    _isSimulating = true;
    return *this;
}

CollectionPanel& CollectionPanel::stopSimulation() {
    resetEntity(&_rootNode);

    _isSimulating = false;
    return *this;
}

void CollectionPanel::resetEntity(EntityNode* node) {
    EntitySerializer::resetEntityFromConfig(node->entity(), node->entityConfig());

    for(auto& child : node->children())
        resetEntity(child.get());
}

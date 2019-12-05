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

CollectionPanel::CollectionPanel(const std::string& path, OberonResourceManager& resourceManager, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio):
    _path(path), _resourceManager(resourceManager), _viewportTextureSize(viewportTextureSize), _dpiScaleRatio(dpiScaleRatio), _collectionConfig{_path},
    _isOpen(true), _isFocused(false), _isSimulating(false), _isVisible(true), _isDragging(false), _needsFocus(true), _needsDocking(true)
{
    _name = Utility::Directory::filename(_path);

    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, _viewportTextureSize*_dpiScaleRatio);
    _framebuffer = GL::Framebuffer{{}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0);

    _cameraObject = new Object3D{&_scene};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);

    if(!_collectionConfig.hasGroup("scene")) {
        Utility::ConfigurationGroup* sceneConfig = _collectionConfig.addGroup("scene");
        sceneConfig->setValue("name", "root");
    }

    Object3D* rootObject = Serializer::createObjectFromConfig(_collectionConfig.group("scene"), &_scene, resourceManager, &_drawables, &_scripts);
    _rootNode = Containers::pointer<ObjectNode>(rootObject, _collectionConfig.group("scene"));

    updateObjectNodeChildren(_rootNode.get());
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
            script.pyModule().attr("update")(&script.object(), deltaTime);
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
    ImGui::SetNextWindowSizeConstraints(ImVec2(), ImVec2(_viewportTextureSize.x(), _viewportTextureSize.y()));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    _isVisible = ImGui::Begin(_name.c_str(), &_isOpen, ImGuiWindowFlags_NoScrollbar |
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

    _framebuffer.setViewport({{}, windowContentSize*_dpiScaleRatio});
    _camera->setProjectionMatrix(Matrix4::orthographicProjection(Vector2{windowContentSize}, -1000.0f, 1000.0f))
        .setViewport(windowContentSize);

    if(ImGui::IsWindowHovered() && ImGui::IsMouseClicked(2))
        _isDragging = true;
    else if(ImGui::IsMouseReleased(2))
        _isDragging = false;

    if(_isDragging) {
        ImGuiIO& io = ImGui::GetIO();
        _cameraObject->translate({-io.MouseDelta.x, io.MouseDelta.y, 0.0f});
    }

    ImGui::End();
}

void CollectionPanel::addFeatureToObject(Utility::ConfigurationGroup* objectConfig, Object3D* object) {
    Serializer::addFeatureFromConfig(objectConfig, object, _resourceManager, &_drawables, &_scripts);
}

void CollectionPanel::save() {
    _collectionConfig.save();
}

void CollectionPanel::updateObjectNodeChildren(ObjectNode* node) {
    for(auto childConfig : node->objectConfig()->groups("child")) {
        Object3D* child = Serializer::createObjectFromConfig(childConfig, node->object(),
            _resourceManager, &_drawables, &_scripts);
        ObjectNode* childNode = node->addChild(child, childConfig);

        Math::Vector3<Rad> rotationRadians = Quaternion::fromMatrix(childConfig->value<Matrix4>("transformation").
            rotation()).toEuler();

        childNode->setRotationDegree(Vector3{Float(Deg(rotationRadians.x())), Float(Deg(rotationRadians.y())),
            Float(Deg(rotationRadians.z()))});

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
                script.pyModule() = py::module::import(script.scriptPath().c_str());

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

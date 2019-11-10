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

CollectionPanel::CollectionPanel(const std::string& path, OberonResourceManager& resourceManager): _path(path),
    _resourceManager(resourceManager), _collectionConfig{_path}, _rootNode(&_scene, &_collectionConfig),
    _isOpen(true), _isVisible(true), _isFocused(false), _needsFocus(true), _needsDocking(true)
{
    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, {2560, 1440});
    _framebuffer = GL::Framebuffer{{{}, _viewportTexture.imageSize(0)}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0);

    _cameraObject = new Object2D{&_scene};
    _camera = new SceneGraph::Camera2D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix3::projection(Vector2{_viewportTexture.imageSize(0)}))
        .setViewport(_viewportTexture.imageSize(0));

    if(_collectionConfig.hasGroup("child"))
        addEntityNodeChild(_collectionConfig.group("child"), &_rootNode);
}

void CollectionPanel::drawViewport() {
    /* If the window is not visible, just end the method here. */
    if(!_isVisible || !_isOpen)
        return;

    _framebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    _camera->draw(_drawables);
}

void CollectionPanel::newFrame() {
    std::string filename = Utility::Directory::filename(_path);

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

    Vector2 imageSize{_viewportTexture.imageSize(0)};
    ImVec2 contentSize(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);

    ImGuiIntegration::image(_viewportTexture, imageSize);
    ImGui::SetScrollX((imageSize.x() - contentSize.x)/2);
    ImGui::SetScrollY((imageSize.y() - contentSize.y)/2);

    ImGui::End();
}

void CollectionPanel::addComponentToEntity(Utility::ConfigurationGroup* entityGroup, Object2D* object) {
    EntitySerializer::addComponentFromConfig(entityGroup, object, &_drawables, _resourceManager);
}

void CollectionPanel::save() {
    _collectionConfig.save();
}

void CollectionPanel::addEntityNodeChild(Utility::ConfigurationGroup* entityGroup, EntityNode* parentNode) {
    Object2D* entity = EntitySerializer::createEntityFromConfig(entityGroup, parentNode->entity(),
        &_drawables, _resourceManager);
    EntityNode* node = parentNode->addChild(entity, entityGroup);

    for(auto childGroup : entityGroup->groups("child"))
        addEntityNodeChild(childGroup, node);
}

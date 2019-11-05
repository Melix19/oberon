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

#include "CollectionPanel.hpp"

CollectionPanel::CollectionPanel(const std::string& path): _path(path),
    _rootNode(&_scene, _jsonDocument), _isOpen(true), _isFocused(false), _needsFocus(true),
    _needsDocking(true)
{
    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, {2560, 1440});
    _framebuffer = GL::Framebuffer{{{}, _viewportTexture.imageSize(0)}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0);

    _cameraObject = new Object2D{&_scene};
    _camera = new SceneGraph::Camera2D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix3::projection(Vector2{_viewportTexture.imageSize(0)}))
        .setViewport(_viewportTexture.imageSize(0));

    std::string json = Utility::Directory::readString(_path);
    _jsonDocument.Parse(json.c_str());

    if(_jsonDocument.IsObject())
        addEntityNodeChild(_jsonDocument, &_rootNode);
}

void CollectionPanel::drawViewport() {
    _framebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    _camera->draw(_drawables);
}

void CollectionPanel::newFrame() {
    std::string filename = Utility::Directory::filename(_path);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin(filename.c_str(), &_isOpen, ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    _isFocused = ImGui::IsWindowFocused();

    Vector2 imageSize{_viewportTexture.imageSize(0)};
    ImVec2 contentSize(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y);

    ImGuiIntegration::image(_viewportTexture, imageSize);
    ImGui::SetScrollX((imageSize.x() - contentSize.x)/2);
    ImGui::SetScrollY((imageSize.y() - contentSize.y)/2);

    ImGui::End();
}

void CollectionPanel::addEntityNodeChild(const std::string& name, EntityNode* parentNode) {
    auto& allocator = _jsonDocument.GetAllocator();
    Value jsonEntity(kObjectType);
    jsonEntity.SetObject();

    jsonEntity.AddMember("name", Value(name.c_str(), allocator).Move(), allocator);

    Value positionValue(kArrayType);
    positionValue.PushBack(0, allocator).PushBack(0, allocator);
    jsonEntity.AddMember("position", positionValue, allocator);

    jsonEntity.AddMember("rotation", 0, allocator);

    Value scaleValue(kArrayType);
    scaleValue.PushBack(1, allocator).PushBack(1, allocator);
    jsonEntity.AddMember("scale", scaleValue, allocator);

    jsonEntity.AddMember("components", Value(kArrayType).Move(), allocator);
    jsonEntity.AddMember("children", Value(kArrayType).Move(), allocator);

    Object2D* entity = EntitySerializer::createEntityFromJson(jsonEntity, parentNode->entity(),
        &_drawables, _shader);

    if(parentNode->entity() == &_scene) {
        _jsonDocument.CopyFrom(jsonEntity, allocator);
        parentNode->addChild(entity, _jsonDocument);
    } else {
        auto& parentNodeChildren = parentNode->jsonEntity()["children"];
        parentNodeChildren.PushBack(jsonEntity, allocator);
        parentNode->addChild(entity, parentNodeChildren[parentNodeChildren.Size() - 1]);
    }
}

void CollectionPanel::addEntityNodeChild(Value& jsonEntity, EntityNode* parentNode) {
    Object2D* entity = EntitySerializer::createEntityFromJson(jsonEntity, parentNode->entity(),
        &_drawables, _shader);
    EntityNode* node = parentNode->addChild(entity, jsonEntity);

    auto jsonChildren = jsonEntity["children"].GetArray();
    for(auto& jsonChild : jsonChildren) {
        if(!jsonChildren.Empty())
            addEntityNodeChild(jsonChild, node);
    }
}

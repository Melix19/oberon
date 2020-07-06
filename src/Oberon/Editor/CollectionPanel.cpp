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

#include <algorithm>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/ImGuiIntegration/Integration.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/Trade/MeshData.h>
#include <Oberon/Importer.h>
#include <Oberon/Light.h>
#include <Oberon/MeshRenderer.h>

#include "FileNode.h"
#include "ObjectNode.h"

namespace Oberon { namespace Editor {

CollectionPanel::CollectionPanel(FileNode* fileNode, OberonResourceManager& resourceManager, Importer& importer, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio, const std::string& projectPath):
    AbstractPanel{fileNode}, _collectionConfig{fileNode->path()}, _resourceManager{resourceManager}, _importer{importer}, _viewportTextureSize{viewportTextureSize}, _dpiScaleRatio{dpiScaleRatio}, _projectPath(projectPath)
{
    _name = Utility::Directory::filename(_fileNode->path());

    _viewportTexture.setStorage(1, GL::TextureFormat::RGBA8, _viewportTextureSize*_dpiScaleRatio);
    _depth.setStorage(GL::RenderbufferFormat::Depth24Stencil8, _viewportTextureSize*_dpiScaleRatio);
    _objectId.setStorage(GL::RenderbufferFormat::R32UI, _viewportTextureSize*_dpiScaleRatio);

    _framebuffer = GL::Framebuffer{{}};
    _framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{0}, _viewportTexture, 0)
        .attachRenderbuffer(GL::Framebuffer::ColorAttachment{1}, _objectId)
        .attachRenderbuffer(GL::Framebuffer::BufferAttachment::Depth, _depth)
        .mapForDraw({
            {SceneShader::ColorOutput, GL::Framebuffer::ColorAttachment{0}},
            {SceneShader::ObjectIdOutput, GL::Framebuffer::ColorAttachment{1}}});

    _cameraObject = new Object3D{&_scene};
    _camera = new SceneGraph::Camera3D{*_cameraObject};
    _camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend);

    createGrid();

    loadResources();

    if(!_collectionConfig.hasGroup("scene"))
        _collectionConfig.addGroup("scene");

    _rootNode = Containers::pointer<ObjectNode>(&_scene, _collectionConfig.group("scene"));
    updateObjectNodeChildren(_rootNode.get());
    resetDrawablesId();

    _importer.createShaders(_gameData, true);
}

void CollectionPanel::drawViewport() {
    /* If the window is not visible, end the method here. */
    if(!_isVisible || !_isOpen)
        return;

    _framebuffer
        .clearColor(0, Color3{0.12f})
        .clearColor(1, Vector4ui{0})
        .clearDepth(1.0f)
        .bind();

    for(std::size_t i = 0; i != _gameData.lights().size(); ++i)
        _gameData.lights()[i].updateShader(*_camera, _gameData.shaderKeys());

    _camera->draw(_gameData.drawables());
    _camera->draw(_editorDrawables);
}

void CollectionPanel::newFrame() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(), ImVec2(_viewportTextureSize.x(),
        _viewportTextureSize.y()));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    _isVisible = ImGui::Begin(_name.c_str(), &_isOpen, ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    _isFocused = ImGui::IsWindowFocused();
    _isHovered = ImGui::IsWindowHovered();

    /* If the window is not visible, end the method here. */
    if(!_isVisible || !_isOpen) {
        ImGui::End();
        return;
    }

    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();

    _viewportPos = Vector2{windowPos.x, windowPos.y - (_viewportTextureSize.y() - windowSize.y)};
    ImGui::GetWindowDrawList()->AddImage(static_cast<ImTextureID>(&_viewportTexture), ImVec2(_viewportPos),
        ImVec2(_viewportPos + Vector2{_viewportTextureSize}), ImVec2(0, 1), ImVec2(1, 0));

    _viewportSize = Vector2{ImGui::GetContentRegionAvail()};

    _framebuffer.setViewport({{}, Vector2i{_viewportSize*_dpiScaleRatio}});

    if(_isOrthographicCamera)
        _camera->setProjectionMatrix(Matrix4::orthographicProjection(_viewportSize, -1000.0f, 1000.0f));
    else
        _camera->setProjectionMatrix(Matrix4::perspectiveProjection(Deg{70.0f}, _viewportSize.aspectRatio(),
            0.05, 500.0f));

    _camera->setViewport(Vector2i{_viewportSize});

    /* Simulate window padding. We need this because we have to set the real ImGui's WindowPadding
       to 0 because of the viewport. */
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + ImGui::GetStyle().WindowPadding.x,
        ImGui::GetCursorPos().y + ImGui::GetStyle().WindowPadding.y));

    const char* projections[] = {"Orthographic", "Perspective"};
    const char* currentProjection = projections[!_isOrthographicCamera];

    ImGui::PushItemWidth(120);
    if(ImGui::BeginCombo("##CameraProjection", currentProjection)) {
        for(auto& projection: projections) {
            bool isSelected = (currentProjection == projection);

            if(ImGui::Selectable(projection, isSelected)) {
                _isOrthographicCamera = !_isOrthographicCamera;

                Matrix4 currentCameraTransformation = _cameraObject->transformation();
                _cameraObject->setTransformation(_prevCameraTransformation);
                _prevCameraTransformation = currentCameraTransformation;
            }
            if(isSelected) ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::End();
}

void CollectionPanel::save() {
    _collectionConfig.save();
}

void CollectionPanel::loadResources() {
    Utility::ConfigurationGroup* resourcesGroup = _collectionConfig.group(
        "external_resources");
    if(!resourcesGroup)
        return;

    for(Utility::ConfigurationGroup* resource: resourcesGroup->groups("resource")) {
        std::string resourceType = resource->value("type");
        std::string resourcePath = resource->value("path");

        if(resourceType == "Texture2D") {
            Containers::Array<char> data = Utility::Directory::read(Utility::Directory::join(_projectPath, resourcePath));
            _importer.loadTexture(resourcePath, data);
        }
    }
}

void CollectionPanel::updateObjectNodeChildren(ObjectNode* node) {
    for(auto childConfig: node->objectConfig()->groups("child")) {
        Object3D* child = _importer.loadObject(childConfig, node->object(), _gameData);
        ObjectNode* childNode = node->addChild(child, childConfig);

        if(!_gameData.drawables().isEmpty() && child == &_gameData.drawables()[_gameData.drawables().size() - 1].object())
            _drawablesNodes.push_back(childNode);

        Math::Vector3<Rad> rotationRadians = child->rotation().toEuler();
        childNode->setRotationDegree({Float{Deg{rotationRadians.x()}}, Float{Deg{rotationRadians.y()}},
            Float{Deg{rotationRadians.z()}}});

        updateObjectNodeChildren(childNode);
    }
}

void CollectionPanel::createGrid() {
    const Int size = 20;
    _gridObject = new Object3D{&_scene};
    _gridObject->rotateX({Deg{90}});

    Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, SceneShader>("editor");
    if(!shaderResource)
        _resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new SceneShader{SceneShader::Flag::ObjectId}, ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);

    Resource<GL::Mesh> meshResource = _resourceManager.get<GL::Mesh>("grid");
    if(!meshResource) {
        Trade::MeshData grid = Primitives::grid3DWireframe({size - 1, size - 1});
        MeshTools::transformPointsInPlace(Matrix4::scaling({size/2, size/2, 0.0f}),
            grid.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(grid);
        _resourceManager.set(meshResource.key(), std::move(mesh));
    }

    MeshRenderer& meshRenderer = _gridObject->addFeature<MeshRenderer>(_editorDrawables);
    meshRenderer.setMesh(meshResource);
    meshRenderer.setShader(shaderResource);
    meshRenderer.setAmbientColor(Color3{0.3f});
}

void CollectionPanel::resetObjectAndChildren(ObjectNode* node) {
    _importer.resetObject(node->object(), node->objectConfig());

    for(auto& child: node->children())
        resetObjectAndChildren(child.get());
}

void CollectionPanel::updateShader(MeshRenderer& meshRenderer) {
    Resource<Magnum::GL::AbstractShaderProgram, SceneShader> shaderResource = _importer.createShader(meshRenderer, _gameData, true);
    meshRenderer.setShader(shaderResource);
}

void CollectionPanel::recreateShaders() {
    for(std::pair<std::string, SceneShader::Flags>& key: _gameData.shaderKeys()) {
        _resourceManager.set<GL::AbstractShaderProgram>(key.first, new SceneShader{key.second, UnsignedInt(_gameData.lights().size())},
            ResourceDataState::Mutable, ResourcePolicy::ReferenceCounted);
    }
}

void CollectionPanel::resetLightsId() {
    for(std::size_t i = 0; i != _gameData.lights().size(); ++i)
        _gameData.lights()[i].setId(i);
}

void CollectionPanel::addFeatureToObject(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig) {
    SceneGraph::AbstractFeature3D* newFeature = _importer.loadFeature(featureConfig, objectNode->object(), _gameData);

    if(featureConfig->value("type") == "light")
        recreateShaders();
    else if(featureConfig->value("type") == "mesh_renderer") {
        MeshRenderer& meshRenderer = reinterpret_cast<MeshRenderer&>(*newFeature);
        Resource<GL::AbstractShaderProgram, SceneShader> shaderResource = _importer.createShader(meshRenderer, _gameData, true);
        meshRenderer.setObjectId(_gameData.drawables().size());
        meshRenderer.setShader(shaderResource);
        _drawablesNodes.push_back(objectNode);
    }
}

void CollectionPanel::removeDrawableNode(ObjectNode* objectNode) {
    _drawablesNodes.erase(std::find_if(_drawablesNodes.begin(), _drawablesNodes.end(),
        [&objectNode](ObjectNode* n) { return n == objectNode; }));

    resetDrawablesId();
}

void CollectionPanel::resetDrawablesId() {
    for(std::size_t i = 0; i != _gameData.drawables().size(); ++i) {
        MeshRenderer* meshRenderer = dynamic_cast<MeshRenderer*>(&_gameData.drawables()[i]);
        if(meshRenderer) { meshRenderer->setObjectId(i + 1); }
    }
}

void CollectionPanel::startSimulation() {
    _isSimulating = true;
}

void CollectionPanel::stopSimulation() {
    resetObjectAndChildren(_rootNode.get());
    _isSimulating = false;
}

}}

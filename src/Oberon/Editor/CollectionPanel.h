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

#pragma once

#include <Corrade/Utility/Configuration.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Oberon/Core/Importer.h>

#include "AbstractPanel.h"
#include "ObjectNode.h"

class CollectionPanel: public AbstractPanel {
    public:
        CollectionPanel(FileNode* fileNode, OberonResourceManager& resourceManager, Importer& importer, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio);
        void drawViewport(Float deltaTime);
        void newFrame();

    private:
        OberonResourceManager& _resourceManager;
        Importer& _importer;

        Vector2i _viewportTextureSize;
        Vector2 _dpiScaleRatio;

    public:
        template<class KeyEvent> void handleKeyPressEvent(KeyEvent& event);
        template<class MouseEvent> void handleMousePressEvent(MouseEvent& event);
        template<class MouseEvent> void handleMouseReleaseEvent(MouseEvent& event);
        template<class MouseMoveEvent> void handleMouseMoveEvent(MouseMoveEvent& event);

    private:
        Vector2i _previousMousePosition;

        bool _isOrthographicCamera{false}; /* Otherwise it's perspective */
        Matrix4 _prevCameraTransformation;

    public:
        void resetObjectAndChildren(ObjectNode* node);
        void save();

        ObjectNode* rootNode() { return _rootNode.get(); }

        std::vector<ObjectNode*>& selectedNodes() { return _selectedNodes; }

        void loadMeshFeature(Mesh& mesh, Utility::ConfigurationGroup* primitiveConfig);

        CollectionPanel& updateShader();

        CollectionPanel& addFeatureToObject(ObjectNode* objectNode, Utility::ConfigurationGroup* featureConfig);
        CollectionPanel& removeDrawableNode(ObjectNode* objectNode);
        CollectionPanel& updateDrawablesId();

    private:
        void updateObjectNodeChildren(ObjectNode* node);
        void createGrid();

        Vector2 _viewportPos;
        Vector2 _viewportSize;

        Containers::Pointer<Utility::Configuration> _collectionConfig;
        Containers::Pointer<ObjectNode> _rootNode;
        std::vector<ObjectNode*> _selectedNodes;

        bool _isDragging{false};

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Texture2D _viewportTexture;
        GL::Renderbuffer _depth;
        GL::Renderbuffer _objectId;

        SceneGraph::DrawableGroup3D _drawables;
        SceneGraph::DrawableGroup3D _editorDrawables;
        std::vector<ObjectNode*> _drawablesNodes;
        ScriptGroup _scripts;
        LightGroup _lights;

        Scene3D _scene;
        Object3D* _cameraObject;
        Object3D* _gridObject;
        SceneGraph::Camera3D* _camera;

    private:
        bool _isHovered{false};

    public:
        bool isSimulating() const { return _isSimulating; }

        CollectionPanel& startSimulation();
        CollectionPanel& stopSimulation();

    private:
        bool _isSimulating{false};
};

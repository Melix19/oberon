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

#pragma once

#include <Corrade/Utility/Configuration.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Scene.h>

#include "ObjectNode.h"

class CollectionPanel: public Containers::LinkedListItem<CollectionPanel> {
    public:
        CollectionPanel(const std::string& path, OberonResourceManager& resourceManager, const Vector2i& viewportTextureSize, const Vector2& dpiScaleRatio);
        void drawViewport(Float deltaTime);
        void newFrame();

        void resetObjectAndChildren(ObjectNode* node);
        void addFeatureToObject(Utility::ConfigurationGroup* objectConfig, Object3D* object);
        void save();

        const std::string& path() const { return _path; }

        ObjectNode* rootNode() { return _rootNode.get(); }

        std::vector<ObjectNode*>& selectedNodes() { return _selectedNodes; }

        bool isOpen() const { return _isOpen; }
        bool isFocused() const { return _isFocused; }

        bool needsFocus() const { return _needsFocus; }
        CollectionPanel& setNeedsFocus(bool needsFocus) {
            _needsFocus = needsFocus;
            return *this;
        }

        bool needsDocking() const { return _needsDocking; }
        CollectionPanel& setNeedsDocking(bool needsDocking) {
            _needsDocking = needsDocking;
            return *this;
        }

        bool isSimulating() const { return _isSimulating; }

        CollectionPanel& startSimulation();
        CollectionPanel& stopSimulation();

    private:
        void updateObjectNodeChildren(ObjectNode* node);

        std::string _path;
        OberonResourceManager& _resourceManager;

        Vector2i _viewportTextureSize;
        Vector2 _dpiScaleRatio;

        Utility::Configuration _collectionConfig;
        Containers::Pointer<ObjectNode> _rootNode;
        std::vector<ObjectNode*> _selectedNodes;

        bool _isOpen;
        bool _isVisible;
        bool _isFocused;
        bool _needsFocus;
        bool _needsDocking;
        bool _isSimulating;

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Texture2D _viewportTexture;

        SceneGraph::DrawableGroup3D _drawables;
        ScriptGroup _scripts;

        Scene3D _scene;
        Object3D* _cameraObject;
        SceneGraph::Camera3D* _camera;
};

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

#include <Corrade/Containers/Pointer.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/ImGuiIntegration/Widgets.h>
#include <Magnum/SceneGraph/Scene.h>

#include "EntityNode.h"

class CollectionPanel: public Containers::LinkedListItem<CollectionPanel> {
    public:
        CollectionPanel(const std::string& path);
        void drawViewport();
        void newFrame();
        void addEntityNodeChild(const std::string& name, EntityNode* parentNode);

        const std::string& path() { return _path; }

        EntityNode& rootNode() { return _rootNode; }
        Document& jsonDocument() { return _jsonDocument; }

        bool isOpen() { return _isOpen; }
        bool isFocused() { return _isFocused; }

        bool needsFocus() { return _needsFocus; }
        CollectionPanel& setNeedsFocus(bool needsFocus) {
            _needsFocus = needsFocus;
            return *this;
        }

        bool needsDocking() { return _needsDocking; }
        CollectionPanel& setNeedsDocking(bool needsDocking) {
            _needsDocking = needsDocking;
            return *this;
        }

    private:
        void addEntityNodeChild(Value& jsonEntity, EntityNode* parentNode);

        std::string _path;

        EntityNode _rootNode;
        Document _jsonDocument;

        bool _isOpen;
        bool _isFocused;
        bool _needsFocus;
        bool _needsDocking;

        GL::Framebuffer _framebuffer{NoCreate};
        GL::Texture2D _viewportTexture;

        SceneGraph::DrawableGroup2D _drawables;
        Shaders::Flat2D _shader;

        Scene2D _scene;
        Object2D* _cameraObject;
        SceneGraph::Camera2D* _camera;
};

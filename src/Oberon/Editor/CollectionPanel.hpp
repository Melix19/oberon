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
#include <Oberon/Core/EntitySerializer.hpp>

struct EntityNode {
    EntityNode(Entity* entity_ptr, Value* j_entity_ptr);
    EntityNode* addChild(Entity* entity_ptr = nullptr, Value* j_entity_ptr = nullptr);

    Entity* entity_ptr;
    Value* j_entity_ptr;
    bool is_selected;

    EntityNode* parent;
    std::vector<Containers::Pointer<EntityNode>> children;
};

class CollectionPanel {
public:
    CollectionPanel(const std::string& path);
    void drawContent();
    void newFrame();

    std::string path;
    Document j_document;
    Containers::Pointer<EntityNode> root_node;

    bool is_open;
    bool is_focused;
    bool needs_focus;
    bool needs_docking;

private:
    void addEntityNodeChild(Value* j_entity_ptr, EntityNode* parent_node_ptr = nullptr);

    GL::Framebuffer framebuffer{ NoCreate };
    GL::Texture2D content_texture;
    Shaders::Flat2D shader;

    Scene2D scene;
    Object2D* camera_object;
    SceneGraph::Camera2D* camera;
    SceneGraph::DrawableGroup2D drawables;
};

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

EntityNode::EntityNode(Entity* entity_ptr, Value& j_entity)
    : entity_ptr(entity_ptr)
    , j_entity(j_entity)
    , is_selected(false)
{
}

EntityNode* EntityNode::addChild(Entity* entity_ptr, Value& j_entity)
{
    auto child = Containers::pointer<EntityNode>(entity_ptr, j_entity);
    child->parent = this;

    children.push_back(std::move(child));
    return children.back().get();
}

CollectionPanel::CollectionPanel(const std::string& path)
    : path(path)
    , is_open(true)
    , is_focused(false)
    , needs_focus(true)
    , needs_docking(true)
{
    camera_object = new Object2D{ &scene };
    camera = new SceneGraph::Camera2D{ *camera_object };
    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix3::projection({ 4.0f / 3.0f, 1.0f }))
        .setViewport(framebuffer.viewport().size());

    framebuffer = GL::Framebuffer{ { {}, content_size } };

    std::string json = Utility::Directory::readString(path);
    j_document.Parse(json.c_str());

    addEntityNodeChild(j_document);
}

void CollectionPanel::drawContent()
{
    content_texture.setStorage(1, GL::TextureFormat::RGBA8, content_size);
    framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{ 0 }, content_texture, 0);

    framebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    camera->draw(drawables);
}

void CollectionPanel::newFrame()
{
    if (needs_focus) {
        ImGui::SetNextWindowFocus();
        needs_focus = false;
    }

    std::string filename = Utility::Directory::filename(path);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::Begin(filename.c_str(), &is_open);
    ImGui::PopStyleVar();

    is_focused = ImGui::IsWindowFocused();

    ImVec2 content_min = ImGui::GetWindowContentRegionMin();
    ImVec2 content_max = ImGui::GetWindowContentRegionMax();

    content_size = { int(content_max.x - content_min.x), int(content_max.y - content_min.y) };

    ImGuiIntegration::image(content_texture, Vector2{ content_size });

    ImGui::End();
}

void CollectionPanel::addEntityNodeChild(Value& j_entity, EntityNode* parent_node_ptr)
{
    EntityNode* node_ptr;

    if (parent_node_ptr) {
        Entity* entity_ptr = createEntityFromJson(j_entity, parent_node_ptr->entity_ptr);
        node_ptr = parent_node_ptr->addChild(entity_ptr, j_entity);
    } else {
        Entity* entity_ptr = createEntityFromJson(j_entity, &scene);
        root_node = Containers::pointer<EntityNode>(entity_ptr, j_entity);
        node_ptr = root_node.get();
    }

    auto j_children = j_entity["children"].GetArray();
    for (auto& j_child : j_children) {
        if (!j_children.Empty())
            addEntityNodeChild(j_child, node_ptr);
    }
}

Entity* CollectionPanel::createEntityFromJson(Value& j_entity, Object2D* parent)
{
    std::string entity_name = j_entity["name"].GetString();
    Entity* entity_ptr = new Entity{ entity_name, parent };

    return entity_ptr;
}

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

EntityNode::EntityNode(Entity* entity_ptr, Value* j_entity_ptr)
    : entity_ptr(entity_ptr)
    , j_entity_ptr(j_entity_ptr)
    , is_selected(false)
{
}

EntityNode* EntityNode::addChild(Entity* entity_ptr, Value* j_entity_ptr)
{
    auto child = Containers::pointer<EntityNode>(entity_ptr, j_entity_ptr);
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
    content_texture.setStorage(1, GL::TextureFormat::RGBA8, Vector2i{ 2560, 1440 }); // FIXME: Maybe get the screen resolution and use that instead of using a fixed resolution
    framebuffer = GL::Framebuffer{ { {}, content_texture.imageSize(0) } };
    framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{ 0 }, content_texture, 0);

    camera_object = new Object2D{ &scene };
    camera = new SceneGraph::Camera2D{ *camera_object };
    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix3::projection(Vector2{ content_texture.imageSize(0) }))
        .setViewport(content_texture.imageSize(0));

    std::string json = Utility::Directory::readString(path);
    j_document.Parse(json.c_str());

    if (j_document.IsObject())
        addEntityNodeChild(&j_document);
}

void CollectionPanel::drawContent()
{
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
    ImGui::Begin(filename.c_str(), &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    is_focused = ImGui::IsWindowFocused();

    auto image_size = Vector2{ content_texture.imageSize(0) };
    auto content_min = ImGui::GetWindowContentRegionMin();
    auto content_max = ImGui::GetWindowContentRegionMax();
    auto content_size = ImVec2(content_max.x - content_min.x, content_max.y - content_min.y);

    ImGuiIntegration::image(content_texture, image_size);
    ImGui::SetScrollX((image_size.x() - content_size.x) / 2);
    ImGui::SetScrollY((image_size.y() - content_size.y) / 2);

    ImGui::End();
}

void CollectionPanel::addEntityNodeChild(Value* j_entity_ptr, EntityNode* parent_node_ptr)
{
    EntityNode* node_ptr;

    if (parent_node_ptr) {
        Entity* entity_ptr = EntitySerializer::createEntityFromJson(j_entity_ptr, parent_node_ptr->entity_ptr, &drawables, shader);
        node_ptr = parent_node_ptr->addChild(entity_ptr, j_entity_ptr);
    } else {
        Entity* entity_ptr = EntitySerializer::createEntityFromJson(j_entity_ptr, &scene, &drawables, shader);
        root_node = Containers::pointer<EntityNode>(entity_ptr, j_entity_ptr);
        node_ptr = root_node.get();
    }

    auto j_children = (*j_entity_ptr)["children"].GetArray();
    for (auto& j_child : j_children) {
        if (!j_children.Empty())
            addEntityNodeChild(&j_child, node_ptr);
    }
}

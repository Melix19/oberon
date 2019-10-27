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
    content_texture.setStorage(1, GL::TextureFormat::RGBA8, GL::Texture2D::maxSize());
    framebuffer = GL::Framebuffer{ { {}, content_texture.imageSize(0) } };
    framebuffer.attachTexture(GL::Framebuffer::ColorAttachment{ 0 }, content_texture, 0);

    camera_object = new Object2D{ &scene };
    camera = new SceneGraph::Camera2D{ *camera_object };
    camera->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix3::projection(Vector2{ content_texture.imageSize(0) }))
        .setViewport(content_texture.imageSize(0));

    std::string json = Utility::Directory::readString(path);
    j_document.Parse(json.c_str());

    addEntityNodeChild(j_document);
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

    // Position
    float position_x = j_entity["position"][0].GetFloat();
    float position_y = j_entity["position"][1].GetFloat();
    entity_ptr->setTranslation({ position_x, position_y });

    // Rotation
    float rotation = j_entity["rotation"].GetFloat();
    entity_ptr->setRotation(Complex::rotation(Deg(rotation)));

    // Scale
    float scale_x = j_entity["scale"][0].GetFloat();
    float scale_y = j_entity["scale"][1].GetFloat();
    entity_ptr->setScaling({ scale_x, scale_y });

    auto j_components = j_entity["components"].GetArray();
    for (auto& j_component : j_components) {
        std::string type = j_component["type"].GetString();

        if (type == "rectangle_shape") {
            // Size
            float size_x = j_component["size"][0].GetFloat();
            float size_y = j_component["size"][1].GetFloat();

            // Color
            float color_r = j_component["color"][0].GetFloat();
            float color_g = j_component["color"][1].GetFloat();
            float color_b = j_component["color"][2].GetFloat();
            float color_a = j_component["color"][3].GetFloat();

            entity_ptr->addFeature<RectangleShape>(&drawables, shader, Vector2{ size_x, size_y }, Color4{ color_r, color_g, color_b, color_a });
        }
    }

    return entity_ptr;
}

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

#include "Editor.hpp"

Editor::Editor(const Arguments& arguments, const std::string& project_path)
    : Platform::Application{
        arguments,
        Configuration{}.setTitle("Oberon").setWindowFlags(Configuration::WindowFlag::Maximized | Configuration::WindowFlag::Resizable)
    }
    , explorer(project_path)
{
    ImGui::CreateContext();
    Themer::styleColorsDark();

    const Vector2 size = Vector2{ windowSize() } / dpiScaling();

    {
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;

        const Containers::ArrayView<const char> font = Utility::Resource{ "OberonEditor" }.getRaw("NotoSans-Regular.ttf");

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.Fonts->AddFontFromMemoryTTF(const_cast<char*>(font.data()), font.size(), 18.0f * framebufferSize().x() / size.x(), &fontConfig);
    }

    imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(), size, windowSize(), framebufferSize());

    /* Set up proper blending to be used by ImGui. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setMinimalLoopPeriod(16);
}

void Editor::drawEvent()
{
    for (auto& panel : collection_panels)
        panel->drawContent();

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    imgui.newFrame();

    /* Enable text input, if needed */
    if (ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Editor", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");

    if (!ImGui::DockBuilderGetNode(dockspace_id)) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
        ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
        ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.7f, nullptr, &dock_id_right);

        ImGui::DockBuilderDockWindow("Explorer", dock_id_left);
        ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Hierarchy", dock_id_right);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_right_bottom);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    console.newFrame();
    explorer.newFrame();

    if (explorer.clicked_node_ptr) {
        std::string path = explorer.clicked_node_ptr->path;
        std::string extension = Utility::Directory::splitExtension(path).second;

        if (extension == ".col") {
            auto panel_found = std::find_if(collection_panels.begin(), collection_panels.end(), [&](Containers::Pointer<CollectionPanel>& p) { return p->path == path; });

            if (panel_found != collection_panels.end())
                (*panel_found)->needs_focus = true;
            else
                collection_panels.push_back(Containers::pointer<CollectionPanel>(path));
        }
    }

    for (auto panel_it = collection_panels.begin(); panel_it != collection_panels.end();) {
        if ((*panel_it)->needs_docking) {
            ImGui::SetNextWindowDockID(dockspace_id);
            (*panel_it)->needs_docking = false;
        }

        (*panel_it)->newFrame();

        if ((*panel_it)->is_open) {
            if ((*panel_it)->is_focused && hierarchy.collection_panel_ptr != panel_it->get()) {
                hierarchy.clearContent();
                hierarchy.collection_panel_ptr = panel_it->get();
            }

            ++panel_it;
        } else {
            if (panel_it->get() == hierarchy.collection_panel_ptr)
                hierarchy.clearContent();

            panel_it = collection_panels.erase(panel_it);
        }
    }

    hierarchy.newFrame();

    inspector.entity_node_ptr = hierarchy.clicked_node_ptr;
    inspector.newFrame();

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    imgui.drawFrame();

    /* Reset state. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    swapBuffers();
    redraw();
}

void Editor::viewportEvent(ViewportEvent& event)
{
    GL::defaultFramebuffer.setViewport({ {}, event.framebufferSize() });

    imgui.relayout(Vector2{ event.windowSize() } / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void Editor::keyPressEvent(KeyEvent& event)
{
    if (imgui.handleKeyPressEvent(event))
        return;
}

void Editor::keyReleaseEvent(KeyEvent& event)
{
    if (imgui.handleKeyReleaseEvent(event))
        return;
}

void Editor::mousePressEvent(MouseEvent& event)
{
    if (imgui.handleMousePressEvent(event))
        return;
}

void Editor::mouseReleaseEvent(MouseEvent& event)
{
    if (imgui.handleMouseReleaseEvent(event))
        return;
}

void Editor::mouseMoveEvent(MouseMoveEvent& event)
{
    if (imgui.handleMouseMoveEvent(event))
        return;
}

void Editor::mouseScrollEvent(MouseScrollEvent& event)
{
    if (imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void Editor::textInputEvent(TextInputEvent& event)
{
    if (imgui.handleTextInputEvent(event))
        return;
}

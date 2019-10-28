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

#include "ProjectManager.hpp"

ProjectManager::ProjectManager(const Arguments& arguments)
    : Platform::Application{
        arguments,
        Configuration{}.setSize(Vector2i(1024, 576)).setTitle("Oberon - Project Manager").setWindowFlags(Configuration::WindowFlag::Resizable)
    }
    , project_path("")
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

void ProjectManager::drawEvent()
{
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

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
    ImGui::Begin("Project Manager", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");

    if (!ImGui::DockBuilderGetNode(dockspace_id)) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.4f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("Main", dock_main_id);
        ImGui::DockBuilderDockWindow("List", dock_id_right);

        ImGui::DockBuilderFinish(dockspace_id);
    }

    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    ImGui::Begin("Main");

    if (ImGui::Button("Open")) {
        project_path = pfd::select_folder("").result();

        if (!project_path.empty()) {
            project_path.pop_back();
            exit();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Create")) {
        project_path = pfd::select_folder("").result();

        if (!project_path.empty()) {
            project_path.pop_back();
            exit();
        }
    }

    ImGui::End();

    ImGui::Begin("List");
    ImGui::End();

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

void ProjectManager::viewportEvent(ViewportEvent& event)
{
    GL::defaultFramebuffer.setViewport({ {}, event.framebufferSize() });

    imgui.relayout(Vector2{ event.windowSize() } / event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void ProjectManager::keyPressEvent(KeyEvent& event)
{
    if (imgui.handleKeyPressEvent(event))
        return;
}

void ProjectManager::keyReleaseEvent(KeyEvent& event)
{
    if (imgui.handleKeyReleaseEvent(event))
        return;
}

void ProjectManager::mousePressEvent(MouseEvent& event)
{
    if (imgui.handleMousePressEvent(event))
        return;
}

void ProjectManager::mouseReleaseEvent(MouseEvent& event)
{
    if (imgui.handleMouseReleaseEvent(event))
        return;
}

void ProjectManager::mouseMoveEvent(MouseMoveEvent& event)
{
    if (imgui.handleMouseMoveEvent(event))
        return;
}

void ProjectManager::mouseScrollEvent(MouseScrollEvent& event)
{
    if (imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void ProjectManager::textInputEvent(TextInputEvent& event)
{
    if (imgui.handleTextInputEvent(event))
        return;
}

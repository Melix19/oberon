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

#include "ProjectManager.h"

#include <imgui_internal.h>
#include <portable-file-dialogs.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>

#include "Themer.h"

namespace Oberon { namespace Editor {

ProjectManager::ProjectManager(const Arguments& arguments): Platform::Application{arguments,
    Configuration{}.setTitle("Oberon - Project Manager")
                   .setSize({1024, 576})
                   .setWindowFlags(Configuration::WindowFlag::Resizable)}
{
    ImGui::CreateContext();
    Themer::styleColorsDark();

    const Vector2 size = Vector2{windowSize()}/dpiScaling();

    {
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;

        const Containers::ArrayView<const char> font =
            Utility::Resource{"OberonEditor"}.getRaw("NotoSans-Regular.ttf");

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.Fonts->AddFontFromMemoryTTF(const_cast<char*>(font.data()), font.size(),
            18.0f*framebufferSize().x()/size.x(), &fontConfig);
    }

    _imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(), size, windowSize(),
        framebufferSize());

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    /* Set up proper blending to be used by ImGui. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setSwapInterval(1);
}

void ProjectManager::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

    _imgui.newFrame();

    /* Enable text input, if needed */
    if(ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Project Manager", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockSpaceId = ImGui::GetID("ProjectManager DockSpace");

    if(!ImGui::DockBuilderGetNode(dockSpaceId)) {
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->Size);

        ImGuiID dockMainId = dockSpaceId;
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.4f, nullptr, &dockMainId);

        ImGui::DockBuilderDockWindow("Main", dockMainId);
        ImGui::DockBuilderDockWindow("List", dockRightId);

        ImGui::DockBuilderFinish(dockMainId);
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    ImGui::Begin("Main");

    if(ImGui::Button("Open")) {
        _projectPath = Utility::Directory::fromNativeSeparators(pfd::select_folder("").result());

        if(!_projectPath.empty())
            exit();
    }

    ImGui::SameLine();

    if(ImGui::Button("Create")) {
        _projectPath = Utility::Directory::fromNativeSeparators(pfd::select_folder("").result());

        if(!_projectPath.empty()) {
            createProject(_projectPath);
            exit();
        }
    }

    ImGui::End();

    ImGui::Begin("List");
    ImGui::End();

    /* Update application cursor */
    _imgui.updateApplicationCursor(*this);

    _imgui.drawFrame();

    swapBuffers();
    redraw();
}

void ProjectManager::createProject(const std::string& path) {
    std::string settingsPath = Utility::Directory::join(path, "project.oberon");
    Utility::Directory::writeString(settingsPath, "");
}

void ProjectManager::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void ProjectManager::keyPressEvent(KeyEvent& event) {
    _imgui.handleKeyPressEvent(event);
}

void ProjectManager::keyReleaseEvent(KeyEvent& event) {
    _imgui.handleKeyReleaseEvent(event);
}

void ProjectManager::mousePressEvent(MouseEvent& event) {
    _imgui.handleMousePressEvent(event);
}

void ProjectManager::mouseReleaseEvent(MouseEvent& event) {
    _imgui.handleMouseReleaseEvent(event);
}

void ProjectManager::mouseMoveEvent(MouseMoveEvent& event) {
    _imgui.handleMouseMoveEvent(event);
}

void ProjectManager::mouseScrollEvent(MouseScrollEvent& event) {
    if(_imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void ProjectManager::textInputEvent(TextInputEvent& event) {
    _imgui.handleTextInputEvent(event);
}

}}

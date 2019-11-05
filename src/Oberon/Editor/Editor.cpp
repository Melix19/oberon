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

Editor::Editor(const Arguments& arguments, const std::string& projectPath): Platform::Application{arguments,
    Configuration{}.setTitle("Oberon")
                   .setWindowFlags(Configuration::WindowFlag::Maximized|Configuration::WindowFlag::Resizable)},
    _explorer(projectPath)
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

    /* Set up proper blending to be used by ImGui. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setMinimalLoopPeriod(16);
}

void Editor::drawEvent() {
    for(auto& panel: _collectionPanels)
        panel.drawViewport();

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    _imgui.newFrame();

    /* Enable text input, if needed */
    if(ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Editor", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockSpaceId = ImGui::GetID("Editor DockSpace");

    if(!ImGui::DockBuilderGetNode(dockSpaceId)) {
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->Size);

        ImGuiID dockMainId = dockSpaceId;
        ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, nullptr, &dockMainId);
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, nullptr, &dockMainId);
        ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.3f, nullptr, &dockMainId);
        ImGuiID dockRightBottomId = ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.7f, nullptr, &dockRightId);

        ImGui::DockBuilderDockWindow("Explorer", dockLeftId);
        ImGui::DockBuilderDockWindow("Console", dockBottomId);
        ImGui::DockBuilderDockWindow("Hierarchy", dockRightId);
        ImGui::DockBuilderDockWindow("Inspector", dockRightBottomId);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    _console.newFrame();
    _explorer.newFrame();

    if(_explorer.clickedNode()) {
        std::string path = _explorer.clickedNode()->path();
        std::string extension = Utility::Directory::splitExtension(path).second;

        if(extension == ".col") {
            CollectionPanel* found = nullptr;

            for(auto& panel: _collectionPanels)
                if(panel.path() == path) found = &panel;

            if(found) found->setNeedsFocus(true);
            else _collectionPanels.insert(new CollectionPanel(path));
        }
    }

    for(auto& panel: _collectionPanels) {
        if(panel.needsFocus()) {
            ImGui::SetNextWindowFocus();
            panel.setNeedsFocus(false);
        }

        if(panel.needsDocking()) {
            ImGui::SetNextWindowDockID(dockSpaceId);
            panel.setNeedsDocking(false);
        }

        panel.newFrame();

        if(panel.isOpen()) {
            if(panel.isFocused() && _hierarchy.panel() != &panel) {
                _inspector.clearContent();
                _hierarchy.clearContent();
                _hierarchy.setPanel(&panel);
            }
        } else {
            if(_hierarchy.panel() == &panel) {
                _inspector.clearContent();
                _hierarchy.clearContent();
            }

            _collectionPanels.erase(&panel);
        }
    }

    _hierarchy.newFrame();

    if(_hierarchy.clickedNode()) _inspector.setEntityNode(_hierarchy.clickedNode());

    _inspector.newFrame();

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    _imgui.drawFrame();

    /* Reset state. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    swapBuffers();
    redraw();
}

void Editor::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void Editor::keyPressEvent(KeyEvent& event) {
    if(_imgui.handleKeyPressEvent(event)) return;
}

void Editor::keyReleaseEvent(KeyEvent& event) {
    if(_imgui.handleKeyReleaseEvent(event)) return;
}

void Editor::mousePressEvent(MouseEvent& event) {
    if(_imgui.handleMousePressEvent(event)) return;
}

void Editor::mouseReleaseEvent(MouseEvent& event) {
    if(_imgui.handleMouseReleaseEvent(event)) return;
}

void Editor::mouseMoveEvent(MouseMoveEvent& event) {
    if(_imgui.handleMouseMoveEvent(event)) return;
}

void Editor::mouseScrollEvent(MouseScrollEvent& event) {
    if(_imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void Editor::textInputEvent(TextInputEvent& event) {
    if(_imgui.handleTextInputEvent(event)) return;
}

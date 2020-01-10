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

#include "Editor.h"

#include <Corrade/Utility/Directory.h>
#include <imgui_internal.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Oberon/Bindings/Oberon/Python.h>

#include "Themer.h"

Editor::Editor(const Arguments& arguments, const std::string& projectPath): Platform::Application{arguments,
    Configuration{}.setTitle("Oberon")
                   .setWindowFlags(Configuration::WindowFlag::Maximized|Configuration::WindowFlag::Resizable)},
    _explorer(projectPath), _inspector(_resourceManager), _activePanel(nullptr)
{
    _maximizedWindowSize = windowSize();
    _dpiScaleRatio = Vector2{framebufferSize()}/(Vector2{windowSize()}/dpiScaling());

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

    setSwapInterval(1);

    setup(projectPath);
    initResourceManager();

    _timeline.start();
}

void Editor::initResourceManager() {
    Resource<GL::AbstractShaderProgram, Shaders::Flat3D> shaderResource = _resourceManager.get<GL::AbstractShaderProgram, Shaders::Flat3D>("flat3d");
    if(!shaderResource)
        _resourceManager.set<GL::AbstractShaderProgram>(shaderResource.key(), new Shaders::Flat3D{Shaders::Flat3D::Flag::ObjectId});
}

void Editor::drawEvent() {
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    for(auto& panel: _collectionPanels)
        panel.drawViewport(_timeline.previousFrameDuration());

    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color)
        .bind();

    _imgui.newFrame();

    /* Enable text input, if needed */
    if(ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if(!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGuiID dockSpaceId = ImGui::GetID("Editor DockSpace");
    ImVec2 menuBarSize;

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));
    if(ImGui::BeginMainMenuBar()) {
        ImGui::PopStyleVar();

        if(ImGui::BeginMenu("Editor")) {
            if(ImGui::MenuItem("Reset layout")) ImGui::DockBuilderRemoveNode(dockSpaceId);

            ImGui::EndMenu();
        }

        ImVec2 size(40.0f, 0.0f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - size.x/2);
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

        if(!_activePanel || !_activePanel->isSimulating()) {
            if(ImGui::Selectable("Play", false, 0, size) && _activePanel) {
                _console.resetStrings();
                _activePanel->startSimulation();
            }
        } else {
            if(ImGui::Selectable("Stop", false, 0, size)) {
                _activePanel->stopSimulation();
            }
        }

        ImGui::PopStyleVar();

        menuBarSize = ImGui::GetWindowSize();

        ImGui::EndMainMenuBar();
    }

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 pos = viewport->Pos;
    ImVec2 size = viewport->Size;

    pos.y += menuBarSize.y;
    size.y -= menuBarSize.y;

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Editor", nullptr, windowFlags);
    ImGui::PopStyleVar(3);

    if(!ImGui::DockBuilderGetNode(dockSpaceId)) {
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, size);

        ImGuiID dockMainId = dockSpaceId;
        ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, nullptr, &dockMainId);
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, nullptr, &dockMainId);
        ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.3f, nullptr, &dockMainId);
        ImGuiID dockRightBottomId = ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.7f, nullptr, &dockRightId);

        ImGui::DockBuilderDockWindow("Explorer", dockLeftId);
        ImGui::DockBuilderDockWindow("Console", dockBottomId);
        ImGui::DockBuilderDockWindow("Outliner", dockRightId);
        ImGui::DockBuilderDockWindow("Inspector", dockRightBottomId);

        for(auto& panel: _collectionPanels)
            ImGui::DockBuilderDockWindow(panel.name().c_str(), dockMainId);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    std::string pyOut = _pyOutputRedirect.stdoutString();
    if(!pyOut.empty())
        _console.addString(pyOut);

    std::string pyErr = _pyOutputRedirect.stderrString();
    if(!pyErr.empty())
        _console.addString(pyErr);

    _console.newFrame();
    _explorer.newFrame();

    if(_explorer.clickedNode()) {
        std::string path = _explorer.clickedNode()->path();
        std::string extension = Utility::Directory::splitExtension(path).second;

        if(extension == ".col") {
            CollectionPanel* found = nullptr;

            for(auto& panel: _collectionPanels)
                if(panel.collectionPath() == path) found = &panel;

            if(found) found->setNeedsFocus(true);
            else _collectionPanels.insert(new CollectionPanel(path, _resourceManager, _maximizedWindowSize, _dpiScaleRatio));
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
            if(panel.isFocused() && _activePanel != &panel) {
                _activePanel = &panel;

                _outliner.setPanel(&panel);
                _inspector.setPanel(&panel);
            }
        } else {
            if(_activePanel == &panel) {
                _outliner.setPanel(nullptr);
                _inspector.setPanel(nullptr);

                _activePanel = nullptr;
            }

            _collectionPanels.erase(&panel);
        }
    }

    _outliner.newFrame();
    _inspector.newFrame();

    /* Update application cursor */
    _imgui.updateApplicationCursor(*this);

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    _imgui.drawFrame();

    /* Reset state. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

void Editor::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void Editor::keyPressEvent(KeyEvent& event) {
    if(_activePanel) {
        const bool altPressed = event.modifiers() >= KeyEvent::Modifier::Alt;
        const bool ctrlPressed = event.modifiers() >= KeyEvent::Modifier::Ctrl;
        const bool shiftPressed = event.modifiers() >= KeyEvent::Modifier::Shift;
        const bool superPressed = event.modifiers() >= KeyEvent::Modifier::Super;

        /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
        #ifdef CORRADE_TARGET_APPLE
        const bool isShortcutKey = superPressed && !ctrlPressed && !altPressed && !shiftPressed;
        #else
        const bool isShortcutKey = !superPressed && ctrlPressed && !altPressed && !shiftPressed;
        #endif

        if(isShortcutKey && event.key() == KeyEvent::Key::S) _activePanel->save();
    }

    for(auto& panel: _collectionPanels) panel.handleKeyPressEvent(event);

    if(_imgui.handleKeyPressEvent(event)) return;
}

void Editor::keyReleaseEvent(KeyEvent& event) {
    if(_imgui.handleKeyReleaseEvent(event)) return;
}

void Editor::mousePressEvent(MouseEvent& event) {
    for(auto& panel: _collectionPanels) panel.handleMousePressEvent(event);

    if(_imgui.handleMousePressEvent(event)) return;
}

void Editor::mouseReleaseEvent(MouseEvent& event) {
    for(auto& panel: _collectionPanels) panel.handleMouseReleaseEvent(event);

    if(_imgui.handleMouseReleaseEvent(event)) return;
}

void Editor::mouseMoveEvent(MouseMoveEvent& event) {
    for(auto& panel: _collectionPanels) panel.handleMouseMoveEvent(event);

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

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
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <imgui_internal.h>

#include "Themer.h"

Editor::Editor(const Arguments& arguments, const std::string& projectPath): Platform::Application{arguments,
    Configuration{}.setTitle("Oberon")
                   .setWindowFlags(Configuration::WindowFlag::Maximized|Configuration::WindowFlag::Resizable),
    GLConfiguration{}.setColorBufferSize({8, 8, 8, 8})}, _projectPath{projectPath}, _importer{projectPath},
    _scriptManager{projectPath}, _explorer{projectPath}
{
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    _screenResolution = Vector2i{videoMode->width, videoMode->height};
    _dpiScaleRatio = Vector2{framebufferSize()}/(Vector2{windowSize()}/dpiScaling());

    ImGui::CreateContext();
    Themer::styleColorsDark();

    const Vector2 size = Vector2{windowSize()}/dpiScaling();

    {
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        const Containers::ArrayView<const char> font1 =
            Utility::Resource{"OberonEditor"}.getRaw("NotoSans-Regular.ttf");
        io.Fonts->AddFontFromMemoryTTF(const_cast<char*>(font1.data()), font1.size(),
            18.0f*framebufferSize().x()/size.x(), &fontConfig);

        const Containers::ArrayView<const char> font2 =
            Utility::Resource{"OberonEditor"}.getRaw("JetBrainsMono-Regular.ttf");
        io.Fonts->AddFontFromMemoryTTF(const_cast<char*>(font2.data()), font2.size(),
            15.0f*framebufferSize().x()/size.x(), &fontConfig);
    }

    _imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(), size, windowSize(),
        framebufferSize());

    /* Set up proper blending to be used by ImGui. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setSwapInterval(1);

    _timeline.start();
}

void Editor::drawEvent() {
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->drawViewport(_timeline.previousFrameDuration());

    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
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
            if(ImGui::MenuItem("Reset layout"))
                ImGui::DockBuilderRemoveNode(dockSpaceId);

            ImGui::EndMenu();
        }

        if(_activePanel) {
            CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(_activePanel);
            if(collectionPanel) {
                std::string label = "Play";
                if(collectionPanel->isSimulating())
                    label = "Stop";

                ImVec2 size = ImGui::CalcTextSize(label.c_str());
                ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - size.x/2);
                if(ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, size)) {
                    if(collectionPanel->isSimulating())
                        collectionPanel->stopSimulation();
                    else {
                        _console.resetStrings();
                        collectionPanel->startSimulation();
                    }
                }
            }
        }

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

        for(auto& panel: _panels)
            ImGui::DockBuilderDockWindow(panel->name().c_str(), dockMainId);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    _console.newFrame();
    _explorer.newFrame();

    if(_explorer.clickedNode()) {
        bool isDirectory = Utility::Directory::isDirectory(_explorer.clickedNode()->path());
        if(!isDirectory)
            openFile(_explorer.clickedNode());
    }

    showPanels(dockSpaceId);

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

void Editor::showPanels(ImGuiID dockSpaceId) {
    for(auto it = _panels.begin(); it != _panels.end();) {
        Containers::Pointer<AbstractPanel>& panel = *it;

        if(panel->needsDocking()) {
            ImGui::SetNextWindowDockID(dockSpaceId);
            panel->setNeedsDocking(false);
        }

        bool wasFocused = panel->isFocused();
        panel->newFrame();

        if(!wasFocused && panel->isFocused()) {
            _explorer.deselectAllNodes();
            _explorer.selectNode(panel->fileNode());
        }

        if(panel->isOpen()) {
            if(panel->isFocused() && _activePanel != panel.get()) {
                _activePanel = panel.get();

                /* If the focused panel is the collection panel, update the outline and the inspector
                   (otherwise set them to nullptr) */
                CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(panel.get());
                _outliner.setPanel(collectionPanel);
                _inspector.setPanel(collectionPanel);
            }

            ++it;
        } else {
            CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(panel.get());

            if(_activePanel == panel.get()) {
                _activePanel = nullptr;

                if(collectionPanel) {
                    _outliner.setPanel(nullptr);
                    _inspector.setPanel(nullptr);
                }
            }

            if(collectionPanel) {
                /* Remove panel from collection panels' vector */
                _collectionPanels.erase(std::find_if(_collectionPanels.begin(), _collectionPanels.end(),
                    [&panel](CollectionPanel* p) { return p == panel.get(); }));
            }

            it = _panels.erase(it);
        }
    }
}

void Editor::openFile(FileNode* fileNode) {
    /* Check if the file is already open in a panel */
    auto found = std::find_if(_panels.begin(), _panels.end(),
        [&](Containers::Pointer<AbstractPanel>& p) {
            CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(p.get());
            if(collectionPanel && collectionPanel->fileNode() == fileNode)
                return true;

            CodePanel* codePanel = dynamic_cast<CodePanel*>(p.get());
            if(codePanel && codePanel->fileNode() == fileNode)
                return true;

            return false;
        }
    );

    if(found != _panels.end()) { /* If the file is already open in a panel, focus it */
        ImGui::SetWindowFocus((*found)->name().c_str());
    } else { /* Open the file with the appropriate panel */
        std::string extension = Utility::Directory::splitExtension(fileNode->path()).second;

        if(extension == ".col") {
            _panels.push_back(Containers::pointer<CollectionPanel>(fileNode, _resourceManager,
                _importer, _scriptManager, _screenResolution, _dpiScaleRatio));
            _collectionPanels.push_back(reinterpret_cast<CollectionPanel*>(_panels.back().get()));
        } else {
            _panels.push_back(Containers::pointer<CodePanel>(fileNode));
            _codePanels.push_back(reinterpret_cast<CodePanel*>(_panels.back().get()));
        }
    }
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

        if(isShortcutKey && event.key() == KeyEvent::Key::S) {
            CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(_activePanel);
            if(collectionPanel) collectionPanel->save();
        }
    }

    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleKeyPressEvent(event);

    _imgui.handleKeyPressEvent(event);
}

void Editor::keyReleaseEvent(KeyEvent& event) {
    _imgui.handleKeyReleaseEvent(event);
}

void Editor::mousePressEvent(MouseEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMousePressEvent(event);

    for(auto& codePanel: _codePanels)
        codePanel->handleMousePressEvent(event);

    _imgui.handleMousePressEvent(event);
}

void Editor::mouseReleaseEvent(MouseEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMouseReleaseEvent(event);

    _imgui.handleMouseReleaseEvent(event);
}

void Editor::mouseMoveEvent(MouseMoveEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMouseMoveEvent(event);

    _imgui.handleMouseMoveEvent(event);
}

void Editor::mouseScrollEvent(MouseScrollEvent& event) {
    if(_imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void Editor::textInputEvent(TextInputEvent& event) {
    _imgui.handleTextInputEvent(event);
}

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

#include "EditorApplication.h"

#include <algorithm>
#include <imgui_internal.h>
#include <portable-file-dialogs.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Oberon/Core/Light.h>
#include <Oberon/Core/Script.h>

#include "CollectionPanel.hpp"
#include "FileNode.h"
#include "PropertiesPanel.h"
#include "Themer.h"

EditorApplication::EditorApplication(const Arguments& arguments, const std::string& projectPath): Platform::Application{arguments,
    Configuration{}.setTitle("Oberon")
                   .setWindowFlags(Configuration::WindowFlag::Maximized|Configuration::WindowFlag::Resizable)},
    _projectPath{projectPath}, _importer{_resourceManager}, _scriptManager{projectPath}, _explorer{projectPath},
    _inspector{_resourceManager, _importer}
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

    GL::Renderer::enable(GL::Renderer::Feature::Blending);

    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setSwapInterval(1);

    _timeline.start();
}

void EditorApplication::drawEvent() {
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

        if(ImGui::BeginMenu("Project")) {
            if(ImGui::MenuItem("Export"))
                exportProject();

            ImGui::EndMenu();
        }

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

    if(_explorer.clickedNode()) openFile(_explorer.clickedNode());

    showPanels(dockSpaceId);

    _outliner.newFrame();
    _inspector.newFrame();

    /* Update application cursor */
    _imgui.updateApplicationCursor(*this);

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    _imgui.drawFrame();

    /* Reset state. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

void EditorApplication::showPanels(ImGuiID dockSpaceId) {
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

void EditorApplication::openFile(FileNode* fileNode) {
    /* Check if the file is already open in a panel */
    auto found = std::find_if(_panels.begin(), _panels.end(),
        [&](Containers::Pointer<AbstractPanel>& p) {
            CollectionPanel* collectionPanel = dynamic_cast<CollectionPanel*>(p.get());
            if(collectionPanel && collectionPanel->fileNode() == fileNode)
                return true;

            PropertiesPanel* propertiesPanel = dynamic_cast<PropertiesPanel*>(p.get());
            if(propertiesPanel && propertiesPanel->fileNode() == fileNode)
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
                _importer, _scriptManager, _screenResolution, _dpiScaleRatio, _projectPath));
            _collectionPanels.push_back(reinterpret_cast<CollectionPanel*>(_panels.back().get()));
        } else if(extension == ".oberon") {
            _panels.push_back(Containers::pointer<PropertiesPanel>(fileNode));
        } else {
            /* Open every other file with Visual Studio Code */
            std::string command = "code \"";
            command.append(fileNode->path());
            command.append("\"");

            system(command.c_str());
        }
    }
}

void EditorApplication::exportProject() {
    /* Get the libraries and export path */
    std::string libFolderPath = Utility::Directory::fromNativeSeparators(pfd::select_folder("").result());
    std::string exportPath = Utility::Directory::fromNativeSeparators(pfd::select_folder("").result());
    if(libFolderPath.empty() || exportPath.empty())
        return;

    /* Create build directory */
    Utility::Directory::mkpath(Utility::Directory::join(_projectPath, "build"));

    /* Add required modules for finding the platform */
    Utility::Directory::mkpath(Utility::Directory::join(_projectPath, "build/modules"));
    std::string moduleText = Utility::Resource("OberonEditor").get("FindGLFW.cmake");
    Utility::Directory::writeString(Utility::Directory::join(_projectPath, "build/modules/FindGLFW.cmake"), moduleText);

    /* Create resources configuration */
    Utility::Configuration resourcesConfig{Utility::Directory::join(_projectPath, "build/resources.conf")};
    resourcesConfig.addValue("group", "OberonApplication");

    /* Add the project settings into the resources */
    Utility::ConfigurationGroup* projectFile = resourcesConfig.addGroup("file");
    projectFile->addValue("filename", "../project.oberon");
    projectFile->addValue("alias", "project.oberon");

    /* Add the main collection into the resources */
    Utility::Configuration projectConfig{Utility::Directory::join(_projectPath, "project.oberon")};
    std::string mainCollection = projectConfig.value("main_collection");
    Utility::ConfigurationGroup* mainCollectionFile = resourcesConfig.addGroup("file");
    mainCollectionFile->addValue("filename", "../" + mainCollection);
    mainCollectionFile->addValue("alias", mainCollection);

    /* Add all the main collection resources */
    Utility::Configuration mainCollectionConfig{Utility::Directory::join(_projectPath, mainCollection)};
    for(Utility::ConfigurationGroup* resourceGroup: mainCollectionConfig.group("external_resources")->groups("resource")) {
        Utility::ConfigurationGroup* resourceFile = resourcesConfig.addGroup("file");
        resourceFile->addValue("filename", "../" + resourceGroup->value("path"));
        resourceFile->addValue("alias", resourceGroup->value("path"));
    }

    /* Save the resources configuration */
    resourcesConfig.save();

    /* Create the CMakeLists for the export */
    Utility::Directory::writeString(Utility::Directory::join(_projectPath, "build/CMakeLists.txt"), "\
cmake_minimum_required(VERSION 3.4)\n\
project(Application CXX)\n\
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY " + exportPath + ")\n\
set(CMAKE_MODULE_PATH \"${PROJECT_SOURCE_DIR}/modules/\" ${CMAKE_MODULE_PATH})\n\
link_directories(" + libFolderPath + ")\n\
find_package(Magnum REQUIRED\n\
    GlfwApplication\n\
    MeshTools\n\
    Primitives\n\
    SceneGraph\n\
    Shaders)\n\
find_package(MagnumPlugins REQUIRED PngImporter)\n\
corrade_add_resource(OberonApplication_RCS resources.conf)\n\
add_executable(Application MACOSX_BUNDLE ${OberonApplication_RCS})\n\
target_link_libraries(Application PRIVATE OberonGlfwPlatform OberonCore\n\
    Magnum::Application\n\
    Magnum::MeshTools\n\
    Magnum::Primitives\n\
    Magnum::SceneGraph\n\
    Magnum::Shaders\n\
    MagnumPlugins::PngImporter)"
    );

    /* Execute the command to export */
    std::string command = "cd \"";
    command.append(Utility::Directory::join(_projectPath, "build"));
    command.append("\" && cmake . && cmake --build .");
    system(command.c_str());
}

void EditorApplication::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void EditorApplication::keyPressEvent(KeyEvent& event) {
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

void EditorApplication::keyReleaseEvent(KeyEvent& event) {
    _imgui.handleKeyReleaseEvent(event);
}

void EditorApplication::mousePressEvent(MouseEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMousePressEvent(event);

    _imgui.handleMousePressEvent(event);
}

void EditorApplication::mouseReleaseEvent(MouseEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMouseReleaseEvent(event);

    _imgui.handleMouseReleaseEvent(event);
}

void EditorApplication::mouseMoveEvent(MouseMoveEvent& event) {
    for(auto& collectionPanel: _collectionPanels)
        collectionPanel->handleMouseMoveEvent(event);

    _imgui.handleMouseMoveEvent(event);
}

void EditorApplication::mouseScrollEvent(MouseScrollEvent& event) {
    if(_imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void EditorApplication::textInputEvent(TextInputEvent& event) {
    _imgui.handleTextInputEvent(event);
}

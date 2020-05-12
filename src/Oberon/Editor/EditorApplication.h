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

#pragma once

#include <Magnum/ResourceManager.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/Platform/GlfwApplication.h>
#include <Oberon/Core/Importer.h>
#include <Oberon/Core/ScriptManager.h>

#include "Console.h"
#include "Explorer.h"
#include "Inspector.h"
#include "Outliner.h"

class EditorApplication: public Platform::Application {
    public:
        explicit EditorApplication(const Arguments& arguments, const std::string& projectPath);

    private:
        void drawEvent() override;

        void showPanels(ImGuiID dockSpaceId);
        void openFile(FileNode* fileNode);
        void exportProject();

        void viewportEvent(ViewportEvent& event) override;

        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent& event) override;

        void mousePressEvent(MouseEvent& event) override;
        void mouseReleaseEvent(MouseEvent& event) override;
        void mouseMoveEvent(MouseMoveEvent& event) override;
        void mouseScrollEvent(MouseScrollEvent& event) override;
        void textInputEvent(TextInputEvent& event) override;

        Vector2i _screenResolution;
        Vector2 _dpiScaleRatio;
        Timeline _timeline;

        std::string _projectPath;

        ImGuiIntegration::Context _imgui{NoCreate};
        OberonResourceManager _resourceManager;
        Importer _importer;
        ScriptManager _scriptManager;

        Console _console;
        Explorer _explorer;
        Inspector _inspector;
        Outliner _outliner;

        std::vector<Containers::Pointer<AbstractPanel>> _panels;
        std::vector<CollectionPanel*> _collectionPanels;
        AbstractPanel* _activePanel{nullptr};
};

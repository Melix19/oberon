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

#include <Corrade/Utility/Resource.h>
#include <Magnum/Platform/GLContext.h>

#include "Oberon/Editor/EditorWindow.h"
#include "Oberon/Editor/FileBrowser.h"
#include "Oberon/Editor/Viewport.h"

int main(int argc, char** argv) {
    Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};

    Glib::RefPtr<Gtk::Application> app =
        Gtk::Application::create(argc, argv, "org.melix.OberonEditor");

    Corrade::Utility::Resource rs("OberonEditor");
    Glib::RefPtr<Gtk::Builder> builder =
        Gtk::Builder::create_from_string(rs.get("EditorWindow.ui"));

    Oberon::Editor::Viewport* viewport;
    builder->get_widget_derived("Viewport", viewport, context);

    Oberon::Editor::FileBrowser* fileBrowser;
    builder->get_widget_derived("FileBrowser", fileBrowser, viewport);

    Oberon::Editor::EditorWindow* editorWindow;
    builder->get_widget_derived("EditorWindow", editorWindow, fileBrowser);

    return app->run(*editorWindow);
}

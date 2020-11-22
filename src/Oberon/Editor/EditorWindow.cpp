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

#include "EditorWindow.h"

#include <gtkmm/button.h>
#include <gtkmm/filechooserdialog.h>

#include "Oberon/Editor/ProjectTree.h"

namespace Oberon { namespace Editor {

EditorWindow::EditorWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, ProjectTree* projectTree):
    Gtk::Window(cobject), _projectTree(projectTree)
{
    maximize();

    Gtk::Button* openButton;
    builder->get_widget("open-button", openButton);
    openButton->signal_clicked().connect(sigc::mem_fun(*this, &EditorWindow::onButtonOpen));
}

void EditorWindow::onButtonOpen() {
    Gtk::FileChooserDialog dialog("Please choose a folder",
        Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_transient_for(*this);

    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Select", Gtk::RESPONSE_OK);

    int result = dialog.run();
    if(result == Gtk::RESPONSE_OK)
        _projectTree->setRootPath(dialog.get_filename());
}

}}

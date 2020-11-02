#ifndef Oberon_Editor_FileBrowser_h
#define Oberon_Editor_FileBrowser_h
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

#include <giomm/fileenumerator.h>
#include <gtkmm/builder.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>
#include <Magnum/Platform/Platform.h>

#include "Oberon/Oberon.h"

namespace Oberon { namespace Editor {

class FileBrowser: public Gtk::TreeView {
    public:
        explicit FileBrowser(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Platform::GLContext& context);

        void setRootPath(const std::string& path);

    private:
        void onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void onRowExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path);

        void loadDirectory(const Gtk::TreeModel::Row& row);

        std::string getPathFromRow(const Gtk::TreeModel::iterator& iter);

        void onEnumerateChildren(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Gtk::TreeModel::Row& row);

        void requestNextFiles(const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row);
        void onNextFiles(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row);

    private:
        struct ModelColumns: public Gtk::TreeModel::ColumnRecord {
            explicit ModelColumns() { add(filename); }

            Gtk::TreeModelColumn<Glib::ustring> filename;
        };

        Platform::GLContext& _context;

        Gtk::Notebook* _notebook;

        ModelColumns _columns;

        Glib::RefPtr<Gtk::TreeStore> _treeStore;

        std::string _rootPath;
};

}}

#endif

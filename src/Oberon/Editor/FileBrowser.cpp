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

#include "FileBrowser.h"

#include <giomm/file.h>

namespace Oberon { namespace Editor {

FileBrowser::FileBrowser(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::TreeView(cobject) {
    _fileTree = Gtk::TreeStore::create(_columns);
    _cancellable = Gio::Cancellable::create();

    set_model(_fileTree);
    append_column("", _columns.filename);
}

void FileBrowser::setRootFromPath(const std::string& path) {
    /* Cancel any current async operation and then
       reset its state */
    _cancellable->cancel();
    _cancellable->reset();

    /* Clear the current tree */
    _fileTree->clear();

    loadDirectory(Gio::File::create_for_path(path));
}

void FileBrowser::loadDirectory(const Glib::RefPtr<Gio::File>& directory) {
    directory->enumerate_children_async(sigc::bind(sigc::mem_fun(*this,
        &FileBrowser::onDirectoryEnumerateChildren), directory), _cancellable);
}

void FileBrowser::onDirectoryEnumerateChildren(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory) {
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children_finish(result);
    requestNextFiles(directory, enumerator);
}

void FileBrowser::requestNextFiles(const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator) {
    enumerator->next_files_async(sigc::bind(sigc::mem_fun(*this, &FileBrowser::onDirectoryNextFiles),
        directory, enumerator), _cancellable);
}

void FileBrowser::onDirectoryNextFiles(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator) {
    Glib::ListHandle<Glib::RefPtr<Gio::FileInfo>> listInfo = enumerator->next_files_finish(result);

    if(listInfo.empty()) {
        _cancellable->cancel();
    } else {
        for(Glib::RefPtr<Gio::FileInfo> info: listInfo) {
            Gtk::TreeModel::Row row = *(_fileTree->append());
            row[_columns.filename] = info->get_name();
        }

        requestNextFiles(directory, enumerator);
    }
}

}}

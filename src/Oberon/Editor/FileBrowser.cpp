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
#include <Corrade/Utility/Directory.h>

#include "Oberon/Editor/Viewport.h"

namespace Oberon { namespace Editor {

FileBrowser::FileBrowser(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Platform::GLContext& context):
    Gtk::TreeView(cobject), _context(context)
{
    builder->get_widget("notebook", _notebook);

    _treeStore = Gtk::TreeStore::create(_columns);
    set_model(_treeStore);

    append_column("", _columns.filename);

    signal_row_activated().connect(sigc::mem_fun(*this, &FileBrowser::onRowActivated));
    signal_row_expanded().connect(sigc::mem_fun(*this, &FileBrowser::onRowExpanded));
}

void FileBrowser::setRootPath(const std::string& path) {
    _rootPath = path;

    /* Clear the current tree */
    _treeStore->clear();

    /* Create root project node */
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(path);
    Gtk::TreeModel::Row row = *(_treeStore->append());
    row[_columns.filename] = file->get_basename();

    /* Make the node expandable with a placeholder child node */
    _treeStore->append(row->children());
}

void FileBrowser::onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
    Gtk::TreeModel::iterator iter = _treeStore->get_iter(path);
    Gtk::Label* label = Gtk::make_managed<Gtk::Label>(iter->get_value(_columns.filename));
    Viewport* viewport = Gtk::make_managed<Viewport>(_context);

    label->show();
    viewport->show();

    _notebook->append_page(*viewport, *label);
}

void FileBrowser::onRowExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path) {
    /* If the first node is the placeholder (empty node),
       the directory needs to be loaded */
    if(iter->children().begin()->get_value(_columns.filename).empty())
        loadDirectory(*iter);
}

void FileBrowser::loadDirectory(const Gtk::TreeModel::Row& row) {
    /* Get the directory path from the node */
    std::string path = getPathFromRow(Gtk::TreeModel::iterator(row));
    Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(path);

    directory->enumerate_children_async(sigc::bind(sigc::mem_fun(*this,
        &FileBrowser::onEnumerateChildren), directory, row));
}

std::string FileBrowser::getPathFromRow(const Gtk::TreeModel::iterator& iter) {
    std::string path;

    /* Iterate with each parent except the root node */
    for(Gtk::TreeModel::iterator parentIter(iter); parentIter && parentIter->parent(); parentIter = parentIter->parent())
        path = Utility::Directory::join(parentIter->get_value(_columns.filename), path);

    /* Join with the root path after the node iteration is done */
    path = Utility::Directory::join(_rootPath, path);

    return path;
}

void FileBrowser::onEnumerateChildren(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Gtk::TreeModel::Row& row) {
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children_finish(result);
    requestNextFiles(directory, enumerator, row);
}

void FileBrowser::requestNextFiles(const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row) {
    enumerator->next_files_async(sigc::bind(sigc::mem_fun(*this, &FileBrowser::onNextFiles),
        directory, enumerator, row));
}

void FileBrowser::onNextFiles(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row) {
    Glib::ListHandle<Glib::RefPtr<Gio::FileInfo>> listInfo = enumerator->next_files_finish(result);

    if(listInfo.empty()) { /* If we're done with the loading */
        /* Get the placeholder node (it's always the last children) */
        Gtk::TreeModel::iterator placeholder = --(row.children().end());

        /* If the folder only has 1 child it means that it's empty
           (only the placeholder is present), so set it's child name
           to "empty". Otherwise just remove the placeholder. */
        if(row.children().size() == 1)
            (*placeholder)[_columns.filename] = "(Empty)";
        else
            _treeStore->erase(placeholder);
    } else {
        for(Glib::RefPtr<Gio::FileInfo> info: listInfo) {
            Gtk::TreeModel::Row childRow = *(_treeStore->prepend(row.children()));
            childRow[_columns.filename] = info->get_name();

            /* Add a child placeholder if the file is a directory */
            if(info->get_file_type() == Gio::FileType::FILE_TYPE_DIRECTORY)
                _treeStore->append(childRow.children());
        }

        requestNextFiles(directory, enumerator, row);
    }
}

}}

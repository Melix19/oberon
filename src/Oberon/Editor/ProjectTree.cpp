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

#include "ProjectTree.h"

#include <giomm/file.h>
#include <Corrade/Utility/Directory.h>
#include <Corrade/Utility/String.h>

#include "Oberon/Editor/Viewport.h"

namespace Oberon { namespace Editor {

ProjectTree::ProjectTree(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Viewport* viewport):
    Gtk::TreeView(cobject), _viewport(viewport)
{
    _treeStore = Gtk::TreeStore::create(_columns);
    set_model(_treeStore);

    append_column("", _columns.filename);

    signal_row_activated().connect(sigc::mem_fun(*this, &ProjectTree::onRowActivated));
    signal_row_expanded().connect(sigc::mem_fun(*this, &ProjectTree::onRowExpanded));
}

void ProjectTree::setRootPath(const std::string& path) {
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

void ProjectTree::onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
    const Gtk::TreeModel::iterator iter = _treeStore->get_iter(path);
    const std::string filePath = getPathFromRow(iter);
    const std::string normalized = Utility::String::lowercase(filePath);

    if(Utility::String::endsWith(normalized, ".gltf") ||
       Utility::String::endsWith(normalized, ".glb"))
        _viewport->loadScene(filePath);
}

void ProjectTree::onRowExpanded(const Gtk::TreeModel::iterator& iter, const Gtk::TreeModel::Path& path) {
    /* If the first node is the placeholder (empty node),
       the directory needs to be loaded */
    if(iter->children().begin()->get_value(_columns.filename).empty())
        loadDirectory(*iter);
}

void ProjectTree::loadDirectory(const Gtk::TreeModel::Row& row) {
    /* Get the directory path from the node */
    std::string path = getPathFromRow(Gtk::TreeModel::iterator(row));
    Glib::RefPtr<Gio::File> directory = Gio::File::create_for_path(path);

    directory->enumerate_children_async(sigc::bind(sigc::mem_fun(*this,
        &ProjectTree::onEnumerateChildren), directory, row));
}

std::string ProjectTree::getPathFromRow(const Gtk::TreeModel::iterator& iter) {
    std::string path;

    if(iter->parent()) {
        path = iter->get_value(_columns.filename);

        /* Iterate with each parent except the root node */
        for(Gtk::TreeModel::iterator parentIter = iter->parent(); parentIter && parentIter->parent(); parentIter = parentIter->parent())
            path = Utility::Directory::join(parentIter->get_value(_columns.filename), path);

        /* Join with the root path after the node iteration is done */
        path = Utility::Directory::join(_rootPath, path);
    } else {
        path = _rootPath;
    }

    return path;
}

void ProjectTree::onEnumerateChildren(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Gtk::TreeModel::Row& row) {
    Glib::RefPtr<Gio::FileEnumerator> enumerator = directory->enumerate_children_finish(result);
    requestNextFiles(directory, enumerator, row);
}

void ProjectTree::requestNextFiles(const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row) {
    enumerator->next_files_async(sigc::bind(sigc::mem_fun(*this, &ProjectTree::onNextFiles),
        directory, enumerator, row));
}

void ProjectTree::onNextFiles(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& directory, const Glib::RefPtr<Gio::FileEnumerator>& enumerator, const Gtk::TreeModel::Row& row) {
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

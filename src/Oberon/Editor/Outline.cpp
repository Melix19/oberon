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

#include "Outline.h"

#include "Oberon/SceneData.h"
#include "Oberon/Editor/Properties.h"

namespace Oberon { namespace Editor {

Outline::Outline(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&, Properties* properties):
    Gtk::TreeView(cobject), _properties(properties)
{
    _treeStore = Gtk::TreeStore::create(_columns);
    set_model(_treeStore);

    append_column("", _columns.name);

    signal_row_activated().connect(sigc::mem_fun(*this, &Outline::onRowActivated));
}

void Outline::updateWithScene(const SceneData& data) {
    /* Clear the current tree */
    _treeStore->clear();

    /* Create scene node */
    Gtk::TreeModel::Row row = *(_treeStore->append());
    row[_columns.name] = "scene";

    /* Load all objects rows */
    for(UnsignedInt id = 0; id < data.objects.size(); ++id) {
        addObjectRow(row, data, id);
    }
}

void Outline::onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
    const Gtk::TreeModel::iterator iter = _treeStore->get_iter(path);
    _properties->showObjectProperties(iter->get_value(_columns.objectInfo));
}

void Outline::addObjectRow(const Gtk::TreeModel::Row& parentRow, const SceneData& data, UnsignedInt& id) {
    Gtk::TreeModel::Row childRow = *(_treeStore->append(parentRow.children()));
    childRow[_columns.name] = data.objects[id].name;
    childRow[_columns.objectInfo] = &data.objects[id];

    UnsignedInt childCount = data.objects[id].childCount;
    for(UnsignedInt childId = 0; childId < childCount; ++childId) {
        addObjectRow(childRow, data, ++id);
    }
}

}}

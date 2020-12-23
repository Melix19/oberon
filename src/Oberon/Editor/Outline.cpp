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

Outline::Outline(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Properties* properties):
    Gtk::TreeView(cobject), _properties(properties)
{
    builder->get_widget("outlineMenuPopup", _menuPopup);

    Gtk::MenuItem* deleteItem;
    builder->get_widget("outlineDeleteItem", deleteItem);
    deleteItem->signal_activate().connect(sigc::mem_fun(*this, &Outline::onDeleteItemActivate));

    _treeStore = Gtk::TreeStore::create(_columns);
    set_model(_treeStore);

    append_column("", _columns.name);

    signal_row_activated().connect(sigc::mem_fun(*this, &Outline::onRowActivated));
    signal_button_press_event().connect_notify(sigc::mem_fun(*this, &Outline::onButtonPressEvent));
}

void Outline::updateWithSceneData(SceneData& data) {
    _sceneData = &data;

    /* Clear the current tree */
    _treeStore->clear();

    /* Empty selected objects array */
    _selectedObjects.clear();

    /* Create scene node (without objectInfo because the scene
       cannot have a non-default transformation or features) */
    Gtk::TreeModel::Row row = *(_treeStore->append());
    row[_columns.name] = _sceneData->objects[_sceneData->sceneObjectId].name;
    row[_columns.objectId] = _sceneData->sceneObjectId;

    /* Load scene children */
    for(UnsignedInt objectId: _sceneData->objects[_sceneData->sceneObjectId].children)
        addObjectRow(row, objectId);

    /* Expand all nodes */
    expand_all();
}

void Outline::onRowActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*) {
    const Gtk::TreeModel::iterator iter = _treeStore->get_iter(path);
    UnsignedInt objectId = iter->get_value(_columns.objectId);
    _properties->showObjectProperties(_sceneData->objects[objectId]);

    /* Empty selected objects array and insert the
       selected object */
    _selectedObjects.clear();
    _selectedObjects.push_back(objectId);
}

void Outline::onButtonPressEvent(GdkEventButton* buttonEvent) {
    if(buttonEvent->type == GDK_BUTTON_PRESS && buttonEvent->button == GDK_BUTTON_SECONDARY)
        _menuPopup->popup_at_pointer(reinterpret_cast<GdkEvent*>(buttonEvent));
}

void Outline::onDeleteItemActivate() {
    Glib::RefPtr<Gtk::TreeSelection> treeSelection = get_selection();
    if(treeSelection) {
        Gtk::TreeModel::iterator iter = treeSelection->get_selected();
        if(iter) {
            UnsignedInt objectId = iter->get_value(_columns.objectId);
            UnsignedInt parentObjectId = iter->parent()->get_value(_columns.objectId);

            /* Delete the object id from the parent's children array */
            std::vector<UnsignedInt>::iterator childIdIter = std::find(
                _sceneData->objects[parentObjectId].children.begin(),
                _sceneData->objects[parentObjectId].children.end(), objectId);
            _sceneData->objects[parentObjectId].children.erase(childIdIter);

            /* Delete object from the scene */
            /* TODO: also delete the objectInfo from the sceneData when
               corrade will have arbitrary deletion of the Arrays. */
            delete _sceneData->objects[objectId].object;

            /* Delete row from the tree */
            _treeStore->erase(iter);
        }
    }
}

void Outline::addObjectRow(const Gtk::TreeModel::Row& parentRow, UnsignedInt objectId) {
    Gtk::TreeModel::Row childRow = *(_treeStore->append(parentRow.children()));
    childRow[_columns.name] = _sceneData->objects[objectId].name;
    childRow[_columns.objectId] = objectId;

    for(UnsignedInt childObjectId: _sceneData->objects[objectId].children)
        addObjectRow(childRow, childObjectId);
}

}}

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

#include "Properties.h"

namespace Oberon { namespace Editor {

Properties::Properties(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::Box(cobject)
{
    builder->get_widget_derived("TransformationEditor", _transformationEditor);
    add(*_transformationEditor);
    signalShowEditor().connect(sigc::mem_fun(_transformationEditor, &TransformationEditor::showEditor));

    PhongDrawableEditor* phongDrawableEditor;
    builder->get_widget_derived("PhongDrawableEditor", phongDrawableEditor);
    add(*phongDrawableEditor);
    signalShowEditor().connect(sigc::mem_fun(phongDrawableEditor, &PhongDrawableEditor::showEditor));
}

void Properties::showObjectProperties(const ObjectInfo& objectInfo) {
    _signalShowEditor.emit(objectInfo);
    show();
}

void Properties::updateTransformation() {
    _transformationEditor->updateEditor();
}

}}

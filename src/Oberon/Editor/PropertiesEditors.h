#ifndef Oberon_Editor_PropertiesEditors_h
#define Oberon_Editor_PropertiesEditors_h
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

#include <gtkmm/builder.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/expander.h>
#include <gtkmm/spinbutton.h>

#include "Oberon/Oberon.h"

namespace Oberon { namespace Editor {

class TransformationEditor: public Gtk::Expander {
    public:
        explicit TransformationEditor(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

        void showEditor(const ObjectInfo& objectInfo);
        void updateEditor();

    private:
        void onTranslationChanged();
        void onRotationChanged();
        void onScalingChanged();

    private:
        Gtk::SpinButton* _translationX;
        Gtk::SpinButton* _translationY;
        Gtk::SpinButton* _translationZ;

        Gtk::SpinButton* _rotationX;
        Gtk::SpinButton* _rotationY;
        Gtk::SpinButton* _rotationZ;

        Gtk::SpinButton* _scalingX;
        Gtk::SpinButton* _scalingY;
        Gtk::SpinButton* _scalingZ;

        Object3D* _object;
};

class PhongDrawableEditor: public Gtk::Expander {
    public:
        explicit PhongDrawableEditor(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

        void showEditor(const ObjectInfo& objectInfo);
        void updateEditor();

    private:
        void onColorChanged();

    private:
        Gtk::ColorButton* _colorButton;

        PhongDrawable* _phongDrawable;
};

}}

#endif

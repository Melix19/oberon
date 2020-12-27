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

#include "PropertiesEditors.h"

#include "Oberon/PhongDrawable.h"
#include "Oberon/SceneData.h"

namespace Oberon { namespace Editor {

TransformationEditor::TransformationEditor(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::Expander(cobject)
{
    /* Translation adjustaments */
    builder->get_widget("transformation_translation_x", _translationX);
    builder->get_widget("transformation_translation_y", _translationY);
    builder->get_widget("transformation_translation_z", _translationZ);
    _translationX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationX->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onTranslationChanged));
    _translationY->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onTranslationChanged));
    _translationZ->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onTranslationChanged));

    /* Rotation adjustaments */
    builder->get_widget("transformation_rotation_x", _rotationX);
    builder->get_widget("transformation_rotation_y", _rotationY);
    builder->get_widget("transformation_rotation_z", _rotationZ);
    _rotationX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationX->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onRotationChanged));
    _rotationY->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onRotationChanged));
    _rotationZ->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onRotationChanged));

    /* Scaling adjustaments */
    builder->get_widget("transformation_scaling_x", _scalingX);
    builder->get_widget("transformation_scaling_y", _scalingY);
    builder->get_widget("transformation_scaling_z", _scalingZ);
    _scalingX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingX->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onScalingChanged));
    _scalingY->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onScalingChanged));
    _scalingZ->signal_value_changed().connect(sigc::mem_fun(*this, &TransformationEditor::onScalingChanged));
}

void TransformationEditor::showEditor(const ObjectInfo& objectInfo) {
    _object = objectInfo.object;
    updateEditor();
}

void TransformationEditor::updateEditor() {
    Vector3 translation = _object->translation();
    _translationX->set_value(double(translation.x()));
    _translationY->set_value(double(translation.y()));
    _translationZ->set_value(double(translation.z()));

    Math::Vector3<Rad> rotation = _object->rotation().toEuler();
    _rotationX->set_value(double(float(Deg(rotation.x()))));
    _rotationY->set_value(double(float(Deg(rotation.y()))));
    _rotationZ->set_value(double(float(Deg(rotation.z()))));

    Vector3 scaling = _object->scaling();
    _scalingX->set_value(double(scaling.x()));
    _scalingY->set_value(double(scaling.y()));
    _scalingZ->set_value(double(scaling.z()));
}

void TransformationEditor::onTranslationChanged() {
    _object->setTranslation({Float(_translationX->get_value()),
        Float(_translationY->get_value()),
        Float(_translationZ->get_value())});
}

void TransformationEditor::onRotationChanged() {
    Math::Vector3<Rad> euler{Rad(Deg(_rotationX->get_value())),
        Rad(Deg(_rotationY->get_value())),
        Rad(Deg(_rotationZ->get_value()))};

    _object->setRotation(Quaternion::rotation(Rad(euler.z()), Vector3::zAxis())*
        Quaternion::rotation(Rad(euler.y()), Vector3::yAxis())*
        Quaternion::rotation(Rad(euler.x()), Vector3::xAxis()));
}

void TransformationEditor::onScalingChanged() {
    _object->setScaling({Float(_scalingX->get_value()),
        Float(_scalingY->get_value()),
        Float(_scalingZ->get_value())});
}

PhongDrawableEditor::PhongDrawableEditor(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder):
    Gtk::Expander(cobject)
{
    builder->get_widget("phong_drawable_color", _colorButton);
    _colorButton->signal_color_set().connect(sigc::mem_fun(*this, &PhongDrawableEditor::onColorChanged));
}

void PhongDrawableEditor::showEditor(const ObjectInfo& objectInfo) {
    SceneGraph::AbstractFeature3D* feature = objectInfo.features[UnsignedByte(ObjectInfo::FeatureType::PhongDrawable)];
    if(feature) {
        _phongDrawable = reinterpret_cast<PhongDrawable*>(feature);
        updateEditor();
        show();
    } else {
        hide();
    }
}

void PhongDrawableEditor::updateEditor() {
    Gdk::RGBA gdkColor;
    Color4 color = _phongDrawable->color();
    gdkColor.set_rgba(double(color.r()), double(color.g()), double(color.b()), double(color.a()));
    _colorButton->set_rgba(gdkColor);
}

void PhongDrawableEditor::onColorChanged() {
    Gdk::RGBA gdkColor = _colorButton->get_rgba();
    _phongDrawable->setColor({Float(gdkColor.get_red()), Float(gdkColor.get_green()), Float(gdkColor.get_blue()), Float(gdkColor.get_alpha())});
}

}}

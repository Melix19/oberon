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

#include <gtkmm/expander.h>
#include <Corrade/Utility/Resource.h>

#include "Oberon/SceneData.h"

namespace Oberon { namespace Editor {

Properties::Properties(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&):
    Gtk::Box(cobject)
{
    Corrade::Utility::Resource rs("OberonEditor");
    Glib::RefPtr<Gtk::Builder> propertiesBuilder =
        Gtk::Builder::create_from_string(rs.get("Properties.ui"));

    /* Translation adjustaments */
    propertiesBuilder->get_widget("translation_x", _translationX);
    propertiesBuilder->get_widget("translation_y", _translationY);
    propertiesBuilder->get_widget("translation_z", _translationZ);
    _translationX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.1));
    _translationX->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateTranslation));
    _translationY->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateTranslation));
    _translationZ->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateTranslation));

    /* Rotation adjustaments */
    propertiesBuilder->get_widget("rotation_x", _rotationX);
    propertiesBuilder->get_widget("rotation_y", _rotationY);
    propertiesBuilder->get_widget("rotation_z", _rotationZ);
    _rotationX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX)));
    _rotationX->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateRotation));
    _rotationY->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateRotation));
    _rotationZ->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateRotation));

    /* Scaling adjustaments */
    propertiesBuilder->get_widget("scaling_x", _scalingX);
    propertiesBuilder->get_widget("scaling_y", _scalingY);
    propertiesBuilder->get_widget("scaling_z", _scalingZ);
    _scalingX->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingY->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingZ->set_adjustment(Gtk::Adjustment::create(0, double(-FLT_MAX), double(FLT_MAX), 0.01));
    _scalingX->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateScaling));
    _scalingY->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateScaling));
    _scalingZ->signal_value_changed().connect(sigc::mem_fun(*this, &Properties::updateScaling));

    Gtk::Expander* transformation;
    propertiesBuilder->get_widget("transformation", transformation);
    add(*transformation);
}

void Properties::showObjectProperties(const ObjectInfo* objectInfo) {
    _object = objectInfo->object;

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

    show();
}

void Properties::updateTranslation() {
    _object->setTranslation({Float(_translationX->get_value()),
        Float(_translationY->get_value()),
        Float(_translationZ->get_value())});
}

void Properties::updateRotation() {
    Math::Vector3<Rad> euler{Rad(Deg(_rotationX->get_value())),
        Rad(Deg(_rotationY->get_value())),
        Rad(Deg(_rotationZ->get_value()))};

    _object->setRotation(Quaternion::rotation(Rad(euler.z()), Vector3::zAxis())*
        Quaternion::rotation(Rad(euler.y()), Vector3::yAxis())*
        Quaternion::rotation(Rad(euler.x()), Vector3::xAxis()));
}

void Properties::updateScaling() {
    _object->setScaling({Float(_scalingX->get_value()),
        Float(_scalingY->get_value()),
        Float(_scalingZ->get_value())});
}

}}

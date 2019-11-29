/*
    MIT License

    Copyright (c) 2019 Marco Melorio

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

#include <Magnum/SceneGraph/TranslationRotationScalingTransformation3D.h>

#include "magnum/scenegraph.h"
#include "oberon/bootstrap.h"

namespace magnum { namespace {

template<UnsignedInt dimensions, class T> void abstractObject(py::class_<SceneGraph::AbstractObject<dimensions, T>, SceneGraph::PyObjectHolder<SceneGraph::AbstractObject<dimensions, T>>>& c) {
    c
        /* Matrix transformation APIs */
        .def("transformation_matrix", &SceneGraph::AbstractObject<dimensions, T>::transformationMatrix,
            "Transformation matrix")
        .def("absolute_transformation_matrix", &SceneGraph::AbstractObject<dimensions, T>::absoluteTransformationMatrix,
             "Transformation matrix relative to the root object");
}

template<class Transformation> void objectTrs(py::class_<SceneGraph::Object<Transformation>, SceneGraph::PyObject<SceneGraph::Object<Transformation>>, SceneGraph::AbstractObject<Transformation::Dimensions, typename Transformation::Type>, SceneGraph::PyObjectHolder<SceneGraph::Object<Transformation>>>& c) {
    c
        .def_property("translation",
            &SceneGraph::Object<Transformation>::translation,
            &SceneGraph::Object<Transformation>::setTranslation,
            "Object translation")
        .def_property("rotation",
            &SceneGraph::Object<Transformation>::rotation,
            &SceneGraph::Object<Transformation>::setRotation,
            "Object rotation")
        .def_property("scaling",
            &SceneGraph::Object<Transformation>::scaling,
            &SceneGraph::Object<Transformation>::setScaling,
            "Object scaling");
}

}}

namespace oberon_ {

void scenegraph(py::module& m) {
    m.doc() = "Scene graph library";

    /* Abstract objects. Returned from feature.object, so need to be registered
       as well. */
    {
        py::class_<SceneGraph::AbstractObject3D, SceneGraph::PyObjectHolder<SceneGraph::AbstractObject3D>> abstractObject{m, "AbstractObject", "Base object for three-dimensional scenes"};
        magnum::abstractObject(abstractObject);
    }

    py::class_<SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation3D>, SceneGraph::PyObject<SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation3D>>, SceneGraph::AbstractObject3D, SceneGraph::PyObjectHolder<SceneGraph::Object<SceneGraph::TranslationRotationScalingTransformation3D>>> object_{m, "Object", "Three-dimensional object"};
    magnum::object(object_);
    magnum::object3D(object_);
    magnum::objectScale(object_);
    magnum::objectTrs(object_);

    py::class_<SceneGraph::Scene<SceneGraph::TranslationRotationScalingTransformation3D>> scene_{m, "Scene", "Three-dimensional scene", object_};
    magnum::scene(scene_);
}

}

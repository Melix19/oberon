#ifndef Oberon_LightDrawable_h
#define Oberon_LightDrawable_h
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

#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Drawable.h>

#include "Oberon/Oberon.h"

namespace Oberon {

class LightDrawable: public SceneGraph::Drawable3D {
    public:
        explicit LightDrawable(SceneGraph::AbstractObject3D& object, bool directional, const Color3& color, Float range, Containers::Array<Vector4>& positions, Containers::Array<Color3>& colors, Containers::Array<Float>& ranges, SceneGraph::DrawableGroup3D& group): SceneGraph::Drawable3D{object, &group}, _directional{directional}, _color{color}, _range{range}, _positions(positions), _colors(colors), _ranges(ranges) {}

    private:
        void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D&) override;

        bool _directional;
        Color3 _color;
        Float _range;
        Containers::Array<Vector4>& _positions;
        Containers::Array<Color3>& _colors;
        Containers::Array<Float>& _ranges;
};

}

#endif

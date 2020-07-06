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

#pragma once

#include <Magnum/Resource.h>

#include "Oberon.h"

namespace Oberon {

namespace MeshPrimitives {

Resource<GL::Mesh> capsule(Float radius, Float length, UnsignedInt hemisphereRings, UnsignedInt cylinderRings, UnsignedInt segments, OberonResourceManager& resourceManager);
Resource<GL::Mesh> circle(Float radius, UnsignedInt segments, OberonResourceManager& resourceManager);
Resource<GL::Mesh> cone(Float radius, Float length, UnsignedInt rings, UnsignedInt segments, bool capEnd, OberonResourceManager& resourceManager);
Resource<GL::Mesh> cylinder(Float radius, Float length, UnsignedInt rings, UnsignedInt segments, bool capEnds, OberonResourceManager& resourceManager);
Resource<GL::Mesh> plane(Vector2 size, OberonResourceManager& resourceManager);
Resource<GL::Mesh> sphere(Float radius, UnsignedInt rings, UnsignedInt segments, OberonResourceManager& resourceManager);

}

}

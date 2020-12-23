#ifndef Oberon_Editor_Im3dIntegration_h
#define Oberon_Editor_Im3dIntegration_h
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

#include <Magnum/Math/Matrix.h>
#include <Magnum/Math/Vector.h>

#include "OberonExternal/im3d/im3d.h"

namespace Magnum { namespace Math { namespace Implementation {

/* Im3d::Vec2 */
template<> struct VectorConverter<2, Float, Im3d::Vec2> {
    static Vector<2, Float> from(const Im3d::Vec2& other) {
        return {other.x, other.y};
    }

    static Im3d::Vec2 to(const Vector<2, Float>& other) {
        return {other[0], other[1]};
    }
};

/* Im3d::Vec3 */
template<> struct VectorConverter<3, Float, Im3d::Vec3> {
    static Vector<3, Float> from(const Im3d::Vec3& other) {
        return {other.x, other.y, other.z};
    }

    static Im3d::Vec3 to(const Vector<3, Float>& other) {
        return {other[0], other[1], other[2]};
    }
};

/* Im3d::Mat4 */
template<> struct RectangularMatrixConverter<4, 4, Float, Im3d::Mat4> {
    static Matrix<4, Float> from(const Im3d::Mat4& other) {
        return {Vector<4, Float>{other(0, 0), other(1, 0), other(2, 0), other(3, 0)},
                Vector<4, Float>{other(0, 1), other(1, 1), other(2, 1), other(3, 1)},
                Vector<4, Float>{other(0, 2), other(1, 2), other(2, 2), other(3, 2)},
                Vector<4, Float>{other(0, 3), other(1, 3), other(2, 3), other(3, 3)}};
    }

    static Im3d::Mat4 to(const Matrix<4, Float>& other) {
        return {other[0][0], other[1][0], other[2][0], other[3][0],
                other[0][1], other[1][1], other[2][1], other[3][1],
                other[0][2], other[1][2], other[2][2], other[3][2],
                other[0][3], other[1][3], other[2][3], other[3][3]};
    }
};

}}}

#endif

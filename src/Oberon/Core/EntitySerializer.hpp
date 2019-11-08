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

#pragma once

#include <Corrade/Utility/ConfigurationGroup.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector2.h>
#include <sstream>

namespace Corrade { namespace Utility {

template<> struct ConfigurationValue<Vector2> {
    static std::string toString(const Vector2& value, ConfigurationValueFlags flags) {
        return
            ConfigurationValue<Float>::toString(value.x(), flags) + ' ' +
            ConfigurationValue<Float>::toString(value.y(), flags);
    }

    static Vector2 fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::istringstream i{stringValue};
        std::string x, y;

        Vector2 vec;
        (i >> x) && (vec.x() = ConfigurationValue<Float>::fromString(x, flags));
        (i >> y) && (vec.y() = ConfigurationValue<Float>::fromString(y, flags));
        return vec;
    }
};

template<> struct ConfigurationValue<Color4> {
    static std::string toString(const Color4& value, ConfigurationValueFlags flags) {
        return
            ConfigurationValue<Float>::toString(value.r(), flags) + ' ' +
            ConfigurationValue<Float>::toString(value.g(), flags) + ' ' +
            ConfigurationValue<Float>::toString(value.b(), flags) + ' ' +
            ConfigurationValue<Float>::toString(value.a(), flags);
    }

    static Color4 fromString(const std::string& stringValue, ConfigurationValueFlags flags) {
        std::istringstream i{stringValue};
        std::string r, g, b, a;

        Color4 col;
        (i >> r) && (col.r() = ConfigurationValue<Float>::fromString(r, flags));
        (i >> g) && (col.g() = ConfigurationValue<Float>::fromString(g, flags));
        (i >> b) && (col.b() = ConfigurationValue<Float>::fromString(b, flags));
        (i >> a) && (col.a() = ConfigurationValue<Float>::fromString(a, flags));
        return col;
    }
};

}}

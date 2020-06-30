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

#include <string>
#include <Oberon/Oberon.h>

namespace Oberon { namespace Editor {

namespace Theme {

enum class Color: UnsignedByte {
    Dark,
    Light
};

void setStyle();
void setStyleColor(Color color);

bool inputText(const std::string& label, const std::string& id, std::string& text);

bool dragFloat(const std::string& label, const std::string& id, Float& value, Float speed = 1.0f, Float min = 0.0f, Float max = 0.0f, const std::string& format = "%.3f");
bool dragFloat3(const std::string& label, const std::string& id, Vector3& value, Float speed = 1.0f, Float min = 0.0f, Float max = 0.0f, const std::string& format = "%.3f");

bool colorEdit4(const std::string& label, const std::string& id, Color4& value);

bool dragInt(const std::string& label, const std::string& id, Int& value, Float speed = 1.0f, Int min = 0, Int max = 0, const std::string& format = "%d");
bool dragInt2(const std::string& label, const std::string& id, Vector2i& value, Float speed = 1.0f, Int min = 0, Int max = 0, const std::string& format = "%d");

bool beginCombo(const std::string& label, const std::string& id, const std::string& value);

}}}

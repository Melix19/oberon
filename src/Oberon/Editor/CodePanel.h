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

#include "AbstractPanel.h"

#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Magnum.h>
#include <imgui.h>

using namespace Magnum;

class CodePanel: public AbstractPanel {
    public:
        CodePanel(FileNode* fileNode);

        void newFrame() override;

    private:
        enum class PaletteIndex: UnsignedInt {
            Default,
            LineNumber
        };

        ImU32 getColorFromIndex(PaletteIndex index);

        struct Glyph {
            char _character;
            PaletteIndex _colorIndex = PaletteIndex::Default;

            Glyph(char character, PaletteIndex colorIndex):
                _character(character), _colorIndex(colorIndex) {}
    };

    typedef Containers::Array<Glyph> Line;
    typedef Containers::Array<Line> Lines;

    Lines _lines;
};

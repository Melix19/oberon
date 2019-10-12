#
#   MIT License
#
#   Copyright (c) 2019 Marco Melorio
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"), to deal
#   in the Software without restriction, including without limitation the rights
#   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#   copies of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in all
#   copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#   SOFTWARE.
#

find_package(ImGui REQUIRED SourcesMiscCpp)
find_package(Magnum REQUIRED GL Sdl2Application)

set(OberonEditor_SRCS
    Console.cpp
    Editor.cpp
    Explorer.cpp
    Hierarchy.cpp
    Inspector.cpp)

set(OberonEditor_HEADERS
    Console.hpp
    Editor.hpp
    Explorer.hpp
    Hierarchy.hpp
    Inspector.hpp)

add_library(OberonEditor ${SHARED_OR_STATIC}
    ${OberonEditor_SRCS}
    ${OberonEditor_HEADERS})

target_link_libraries(OberonEditor PUBLIC
    ImGui::SourcesMiscCpp
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    MagnumIntegration::ImGui)

add_library(Oberon::Editor ALIAS OberonEditor)

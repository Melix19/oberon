#
#   This file is part of Oberon.
#
#   Copyright (c) 2019-2020 Marco Melorio
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

find_package(Magnum REQUIRED Sdl2Application)
find_package(PhysFS REQUIRED)

set(OberonExportTemplate_SRCS
    AbstractApplication.cpp
    PackingUtility.cpp)

set(OberonExportTemplate_HEADERS
    AbstractApplication.h
    PackingUtility.h)

add_executable(OberonSdl2Application
    ${OberonExportTemplate_SRCS}
    ${OberonExportTemplate_HEADERS}
    Sdl2Application.cpp Sdl2Application.h)
target_link_libraries(OberonSdl2Application PRIVATE
    Oberon
    Magnum::Sdl2Application
    PhysFS::PhysFS)

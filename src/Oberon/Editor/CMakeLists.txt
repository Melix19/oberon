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

if(MAGNUM_TARGET_GLES AND NOT MAGNUM_TARGET_DESKTOP_GLES)
    find_package(Magnum REQUIRED EglContext)
elseif(CORRADE_TARGET_WINDOWS)
    find_package(Magnum REQUIRED WglContext)
elseif(CORRADE_TARGET_APPLE)
    find_package(Magnum REQUIRED CglContext)
elseif(CORRADE_TARGET_UNIX)
    find_package(Magnum REQUIRED GlxContext)
else()
    message(FATAL_ERROR "Magnum context creation is not supported on this platform")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0)
include_directories(SYSTEM ${GTKMM_INCLUDE_DIRS})

corrade_add_resource(OberonEditor_RCS resources.conf)

set(OberonEditor_SRCS
    EditorWindow.cpp
    main.cpp
    ProjectTree.cpp
    Viewport.cpp

    ${OberonEditor_RCS})

set(OberonEditor_HEADERS
    Editor.h
    EditorWindow.h
    ProjectTree.h
    Viewport.h)

add_executable(OberonEditor
    ${OberonEditor_SRCS}
    ${OberonEditor_HEADERS})
target_link_libraries(OberonEditor PRIVATE
    Magnum::GLContext
    Oberon
    ${GTKMM_LIBRARIES})

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OberonEditor)

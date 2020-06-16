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

find_package(Magnum REQUIRED
    AnyImageImporter
    MeshTools
    Primitives
    SceneGraph
    Shaders)
find_package(MagnumPlugins REQUIRED PngImporter)

corrade_add_resource(Oberon_RCS resources.conf)

set(Oberon_SRCS
    Importer.cpp
    SceneShader.cpp

    ${Oberon_RCS})

set(Oberon_HEADERS
    Importer.h
    Light.h
    Mesh.h
    Oberon.h
    SceneShader.h)

add_library(Oberon
    ${Oberon_SRCS}
    ${Oberon_HEADERS})

target_include_directories(Oberon PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)

target_link_libraries(Oberon PRIVATE
    Magnum::AnyImageImporter
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    MagnumPlugins::PngImporter)

if(WITH_EDITOR)
    add_subdirectory(Editor)
endif()

if(WITH_PLATFORMS)
    add_subdirectory(Platforms)
endif()

# Oberon library target alias for superprojects
add_library(Oberon::Oberon ALIAS Oberon)

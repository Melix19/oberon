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

find_package(Magnum REQUIRED GL MeshTools SceneGraph Shaders Trade)

set(Oberon_SRCS
    LightDrawable.cpp
    PhongDrawable.cpp
    SceneImporter.cpp
    SceneView.cpp)

set(Oberon_HEADERS
    LightDrawable.h
    Oberon.h
    PhongDrawable.h
    SceneData.h
    SceneImporter.h
    SceneView.h)

add_library(Oberon
    ${Oberon_SRCS}
    ${Oberon_HEADERS})
target_include_directories(Oberon PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src)
target_link_libraries(Oberon PUBLIC
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Trade)

install(TARGETS Oberon
    RUNTIME DESTINATION ${OBERON_BINARY_INSTALL_DIR}
    LIBRARY DESTINATION ${OBERON_LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION ${OBERON_LIBRARY_INSTALL_DIR})

add_subdirectory(Editor)

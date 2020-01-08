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
find_package(Magnum REQUIRED GL GlfwApplication)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(OberonEditor_RESOURCES resources.conf)

set(OberonEditor_SRCS
    CollectionPanel.cpp
    Console.cpp
    Editor.cpp
    Explorer.cpp
    Inspector.cpp
    Oberon.cpp
    Outliner.cpp
    ProjectManager.cpp
    Themer.cpp

    ${OberonEditor_RESOURCES})

set(OberonEditor_HEADERS
    CollectionPanel.h
    CollectionPanel.hpp
    Console.h
    Editor.h
    Explorer.h
    FileNode.h
    Inspector.h
    ObjectNode.h
    Outliner.h
    ProjectManager.h
    Themer.h)

add_executable(Oberon MACOSX_BUNDLE
    ${OberonEditor_SRCS}
    ${OberonEditor_HEADERS})

target_link_libraries(Oberon PRIVATE
    ImGui::SourcesMiscCpp
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    MagnumIntegration::ImGui
    Oberon::Core
    portable_file_dialogs)

if(CORRADE_TARGET_APPLE)
    set_target_properties(Oberon PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Resources/MacOSXBundleInfo.plist.in
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
endif()

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Oberon)

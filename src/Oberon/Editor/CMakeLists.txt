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

find_package(ImGui REQUIRED SourcesMiscCpp)
find_package(Magnum REQUIRED Sdl2Application)
find_package(MagnumIntegration REQUIRED ImGui)

corrade_add_resource(OberonEditor_RCS resources.conf)

set(OberonEditor_SRCS
    CollectionPanel.cpp
    Console.cpp
    EditorApplication.cpp
    Explorer.cpp
    Inspector.cpp
    Outliner.cpp
    ProjectManager.cpp
    PropertiesPanel.cpp
    Themer.cpp
    main.cpp

    ${OberonEditor_RCS})

set(OberonEditor_HEADERS
    AbstractPanel.h
    CollectionPanel.h
    CollectionPanel.hpp
    Console.h
    Editor.h
    EditorApplication.h
    Explorer.h
    FileNode.h
    Inspector.h
    ObjectNode.h
    Outliner.h
    ProjectManager.h
    PropertiesPanel.h
    Themer.h)

add_executable(OberonEditor MACOSX_BUNDLE
    ${OberonEditor_SRCS}
    ${OberonEditor_HEADERS})

set_target_properties(OberonEditor PROPERTIES OUTPUT_NAME "Oberon")

# Windows generates import libraries that will result in the same name
# as the Oberon library so we output them in a different directory
if(CORRADE_TARGET_WINDOWS)
    set_target_properties(OberonEditor PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/temp)
endif()

target_link_libraries(OberonEditor PRIVATE
    Oberon
    ImGui::SourcesMiscCpp
    Magnum::Application
    MagnumIntegration::ImGui
    portable_file_dialogs)

if(CORRADE_TARGET_APPLE)
    set_target_properties(OberonEditor PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
endif()

# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Oberon)

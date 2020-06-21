#.rst:
# Find PhysFS
# ---------
#
# Finds the PhysFS library. This module defines:
#
#  PhysFS_FOUND                 - True if PhysFS library is found
#  PhysFS::PhysFS               - PhysFS imported target
#
# Additionally these variables are defined for internal usage:
#
#  PHYSFS_LIBRARY               - PhysFS library, if found
#  PHYSFS_INCLUDE_DIR           - Root include dir
#

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

find_library(PHYSFS_LIBRARY
    NAMES physfs)

# Include dir
find_path(PHYSFS_INCLUDE_DIR
    NAMES physfs.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PhysFS DEFAULT_MSG
    PHYSFS_LIBRARY
    PHYSFS_INCLUDE_DIR)

if(NOT TARGET PhysFS::PhysFS)
    add_library(PhysFS::PhysFS UNKNOWN IMPORTED)

    set_property(TARGET PhysFS::PhysFS PROPERTY IMPORTED_LOCATION ${PHYSFS_LIBRARY})

    set_property(TARGET PhysFS::PhysFS PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${PHYSFS_INCLUDE_DIR})
endif()

mark_as_advanced(PHYSFS_LIBRARY PHYSFS_INCLUDE_DIR)

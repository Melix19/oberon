#.rst:
# Find LibZip
# ---------
#
# Finds the LibZip library. This module defines:
#
#  LibZip_FOUND                 - True if LibZip library is found
#  LibZip::LibZip               - LibZip imported target
#
# Additionally these variables are defined for internal usage:
#
#  LIBZIP_LIBRARY               - LibZip library, if found
#  LIBZIP_INCLUDE_DIR           - Root include dir
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

find_library(LIBZIP_LIBRARY
    NAMES zip)

# Include dir
find_path(LIBZIP_INCLUDE_DIR
    NAMES zip.h)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibZip DEFAULT_MSG
    LIBZIP_LIBRARY
    LIBZIP_INCLUDE_DIR)

if(NOT TARGET LibZip::LibZip)
    add_library(LibZip::LibZip UNKNOWN IMPORTED)

    set_property(TARGET LibZip::LibZip PROPERTY IMPORTED_LOCATION ${LIBZIP_LIBRARY})

    set_property(TARGET LibZip::LibZip PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${LIBZIP_INCLUDE_DIR})
endif()

mark_as_advanced(LIBZIP_LIBRARY LIBZIP_INCLUDE_DIR)

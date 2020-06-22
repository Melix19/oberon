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

#include "PackingUtility.h"

#include <physfs.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/Directory.h>

namespace Oberon { namespace ExportTemplate { namespace PackingUtility {

void initialize(char* argv0) {
    const std::string executableLocation = Utility::Directory::executableLocation();
    const std::string executableParentPath = Utility::Directory::path(executableLocation);
    const std::string executableName = Utility::Directory::filename(executableLocation);
    const std::string dataPath = Utility::Directory::join(executableParentPath, executableName + "-data.zip");

    PHYSFS_init(argv0);
    PHYSFS_mount(dataPath.c_str(), nullptr, 1);
}

void deinitialize() {
    PHYSFS_deinit();
}

Containers::Array<char> read(const std::string& filename) {
    PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
    std::size_t fileLength = PHYSFS_fileLength(file);
    Containers::Array<char> data{fileLength};

    PHYSFS_readBytes(file, data, data.size());
    return data;
}

std::string readString(const std::string& filename) {
    PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
    std::size_t fileLength = PHYSFS_fileLength(file);
    Containers::Array<char> data{fileLength + 1};

    PHYSFS_readBytes(file, data, data.size());
    /* Insert final null character */
    data[data.size() - 1] = 0;
    return std::string(data);
}

}}}

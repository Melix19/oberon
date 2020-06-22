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

#include "Export.h"

#include <zip.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Directory.h>

namespace Oberon { namespace Editor {

namespace Export {

namespace {

void addFileToPack(zip_t* pack, const std::string& path, const std::string& localPath) {
    zip_source_t* source = zip_source_file(pack, path.c_str(), 0, 0);
    zip_file_add(pack, localPath.c_str(), source, 0);
}

}

void exportProject(const std::string& projectPath, const std::string& exportPath, const std::string& exportTemplatesPath) {
    /* Calculate the right export template to use based on the application type */
    const std::string applicationType = "OberonSdl2Application";
    const std::string exportTemplatePath = Utility::Directory::join(exportTemplatesPath, applicationType);

    /* Calculate the final application path based on the application name setted in the project
       configuration */
    const std::string projectConfigurationPath = Utility::Directory::join(projectPath, "project.oberon");
    const Utility::Configuration projectConfiguration{projectConfigurationPath};
    const std::string applicationName = projectConfiguration.value("name");
    const std::string applicationExportPath = Utility::Directory::join(exportPath, applicationName);

    /* Copy the export template to the export path */
    Utility::Directory::copy(exportTemplatePath, applicationExportPath);

    /* Get the main collection to know which resources should be packed */
    const std::string mainCollection = projectConfiguration.value("main_collection");
    const std::string mainCollectionPath = Utility::Directory::join(projectPath, mainCollection);
    const Utility::Configuration collectionConfiguration{mainCollectionPath};
    const Utility::ConfigurationGroup* resourcesGroup = collectionConfiguration.group("external_resources");

    /* Create the data pack to be used to pack the resources */
    const std::string packPath = Utility::Directory::join(exportPath, applicationName + "-data.zip");
    zip_t* pack = zip_open(packPath.c_str(), ZIP_CREATE | ZIP_EXCL, nullptr);
    if(!pack) return;

    /* Add the project configuration file and the main collection to the data pack */
    addFileToPack(pack, projectConfigurationPath, "project.oberon");
    addFileToPack(pack, mainCollectionPath, mainCollection);

    /* Add all the main collection external resources to the data pack */
    for(const Utility::ConfigurationGroup* resourceGroup: resourcesGroup->groups("resource")) {
        const std::string resourceLocalPath = resourceGroup->value("path");
        const std::string resourcePath = Utility::Directory::join(projectPath, resourceLocalPath);
        addFileToPack(pack, resourcePath, resourceLocalPath);
    }

    /* Close the data pack */
    zip_close(pack);
}

}}}

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

#include "Sdl2Application.h"

#include <sstream>
#include <Corrade/Utility/Configuration.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/ConfigurationValue.h>
#include <Oberon/Light.h>

#include "PackingUtility.h"

namespace Oberon { namespace ExportTemplate {

Sdl2Application::Sdl2Application(const Arguments& arguments, const Configuration& configuration, const Utility::Configuration& projectConfiguration):
    Platform::Application{arguments, configuration}, AbstractApplication{projectConfiguration} {}

void Sdl2Application::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    for(std::size_t i = 0; i != _lights.size(); ++i)
        _lights[i].updateShader(*_camera, _shaderKeys);

    _camera->draw(_drawables);

    swapBuffers();
    redraw();

    _timeline.nextFrame();
}

}}

using namespace Corrade;
using namespace Magnum;
using namespace Oberon::ExportTemplate;

int main(int argc, char** argv) {
    PackingUtility::initialize(argv[0]);

    /* Load project configuration */
    std::istringstream projectConfigurationStream(PackingUtility::readString("project.oberon"));
    const Utility::Configuration projectConfiguration{projectConfigurationStream};
    const std::string title = projectConfiguration.value("name");
    const Vector2i windowSize = projectConfiguration.value<Vector2i>("window_size");

    Platform::Application::Configuration configuration{};
    configuration
        .setTitle(title)
        .setSize(windowSize);

    Sdl2Application app({argc, argv}, configuration, projectConfiguration);

    PackingUtility::deinitialize();

    return app.exec();
}

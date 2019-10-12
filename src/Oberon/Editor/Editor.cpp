/*
    MIT License

    Copyright (c) 2019 Marco Melorio

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

#include "Editor.hpp"

Editor::Editor(const Arguments& arguments)
    : Platform::Application{
        arguments,
        Configuration{}.setTitle("Oberon").setWindowFlags(Configuration::WindowFlag::Maximized | Configuration::WindowFlag::Resizable)
    }
{
    ImGui::CreateContext();

    const Vector2 size = Vector2{ windowSize() } / dpiScaling();

    _imgui = ImGuiIntegration::Context(*ImGui::GetCurrentContext(), size, windowSize(), framebufferSize());

    /* Set up proper blending to be used by ImGui. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    setMinimalLoopPeriod(16);
}

void Editor::drawEvent()
{
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

    _imgui.newFrame();

    /* Enable text input, if needed */
    if (ImGui::GetIO().WantTextInput && !isTextInputActive())
        startTextInput();
    else if (!ImGui::GetIO().WantTextInput && isTextInputActive())
        stopTextInput();

    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::ShowTestWindow();

    /* Set appropriate states. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    _imgui.drawFrame();

    /* Reset state. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);

    swapBuffers();
    redraw();
}

void Editor::viewportEvent(ViewportEvent& event)
{
    GL::defaultFramebuffer.setViewport({ {}, event.framebufferSize() });

    _imgui.relayout(Vector2{ event.windowSize() } / event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void Editor::keyPressEvent(KeyEvent& event)
{
    if (_imgui.handleKeyPressEvent(event))
        return;
}

void Editor::keyReleaseEvent(KeyEvent& event)
{
    if (_imgui.handleKeyReleaseEvent(event))
        return;
}

void Editor::mousePressEvent(MouseEvent& event)
{
    if (_imgui.handleMousePressEvent(event))
        return;
}

void Editor::mouseReleaseEvent(MouseEvent& event)
{
    if (_imgui.handleMouseReleaseEvent(event))
        return;
}

void Editor::mouseMoveEvent(MouseMoveEvent& event)
{
    if (_imgui.handleMouseMoveEvent(event))
        return;
}

void Editor::mouseScrollEvent(MouseScrollEvent& event)
{
    if (_imgui.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

void Editor::textInputEvent(TextInputEvent& event)
{
    if (_imgui.handleTextInputEvent(event))
        return;
}

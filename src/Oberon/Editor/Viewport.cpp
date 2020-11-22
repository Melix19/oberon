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

#include "Viewport.h"

#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Platform/GLContext.h>

#include "Oberon/SceneView.h"
#include "Oberon/Editor/Outline.h"

namespace Oberon { namespace Editor {

Viewport::Viewport(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Outline* outline, Platform::GLContext& context):
    Gtk::GLArea(cobject), _outline(outline), _context(context)
{
    /* Automatically re-render everything every time it needs to be drawn */
    set_auto_render();

    /* Set size requests and scaling behavior */
    set_hexpand();
    set_vexpand();
    set_halign(Gtk::ALIGN_FILL);
    set_valign(Gtk::ALIGN_FILL);

    /* Set desired OpenGL version */
    set_required_version(4, 5);

    /* Connect signals to their respective handlers */
    signal_realize().connect(sigc::mem_fun(this, &Viewport::onRealize));
    signal_render().connect(sigc::mem_fun(this, &Viewport::onRender));
    signal_resize().connect(sigc::mem_fun(this, &Viewport::onResize));
}

void Viewport::loadScene(const std::string& path) {
    /* Make sure the OpenGL context is current then load the scene */
    make_current();
    _sceneView = Containers::pointer<SceneView>(path, _viewportSize);

    _outline->updateWithScene(_sceneView->data());
}

void Viewport::onRealize() {
    /* Make sure the OpenGL context is current then configure it */
    make_current();
    _context.create();
}

bool Viewport::onRender(const Glib::RefPtr<Gdk::GLContext>& context) {
    /* Reset state to avoid Gtkmm affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);

    /* Retrieve the ID of the relevant framebuffer */
    GLint framebufferID;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebufferID);

    /* Attach Magnum's framebuffer manager to the framebuffer provided by Gtkmm */
    auto gtkmmDefaultFramebuffer = GL::Framebuffer::wrap(framebufferID, {{}, {get_width(), get_height()}});

    /* Clear the frame */
    gtkmmDefaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    /* Draw the scene if there is one loaded */
    if(_sceneView) _sceneView->draw();

    /* Clean up Magnum state and back to Gtkmm */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
    return true;
}

void Viewport::onResize(int width, int height) {
    _viewportSize = {width, height};

    /* Update the scene viewport if there is one loaded */
    if(_sceneView) _sceneView->updateViewport(_viewportSize);
}

}}

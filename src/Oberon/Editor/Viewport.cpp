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
#include <Magnum/GL/Renderer.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/SceneGraph/Camera.h>

#include "Oberon/SceneView.h"
#include "Oberon/Editor/Im3dIntegration.h"
#include "Oberon/Editor/Outline.h"
#include "Oberon/Editor/Properties.h"

namespace Oberon { namespace Editor {

Viewport::Viewport(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&, Outline& outline, Properties& properties, Platform::GLContext& context):
    Gtk::GLArea(cobject), _outline(outline), _properties(properties), _context(context), _isDragging{false}
{
    /* Set size requests and scaling behavior */
    set_hexpand();
    set_vexpand();
    set_halign(Gtk::ALIGN_FILL);
    set_valign(Gtk::ALIGN_FILL);

    /* Set desired OpenGL version */
    set_required_version(3, 2);

    /* Connect signals for scene rendering */
    signal_realize().connect(sigc::mem_fun(this, &Viewport::onRealize));
    signal_render().connect(sigc::mem_fun(this, &Viewport::onRender));
    signal_resize().connect(sigc::mem_fun(this, &Viewport::onResize));

    /* Set masks for event handling */
    add_events(Gdk::POINTER_MOTION_MASK|Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|
        Gdk::KEY_PRESS_MASK);

    /* Connect signals for event handling */
    signal_motion_notify_event().connect(sigc::mem_fun(this, &Viewport::onMotionNotifyEvent));
    signal_button_press_event().connect(sigc::mem_fun(this, &Viewport::onButtonPressEvent));
    signal_button_release_event().connect(sigc::mem_fun(this, &Viewport::onButtonReleaseEvent));
    signal_key_press_event().connect(sigc::mem_fun(this, &Viewport::onKeyPressEvent));
}

void Viewport::loadScene(const std::string& path) {
    /* Make sure the OpenGL context is current then load the scene */
    make_current();
    _sceneView = Containers::pointer<SceneView>(path, _viewportSize);

    _outline.updateWithSceneData(_sceneView->data());

    (*_im3d)
        .setCameraObject(_sceneView->data().cameraObject)
        .setCamera(_sceneView->data().camera);

    /* Force queue redraw */
    queue_render();
}

void Viewport::onRealize() {
    /* Make sure the OpenGL context is current then configure it */
    make_current();
    _context.create();

    _im3d = Containers::pointer<Im3dContext>();
}

bool Viewport::onRender(const Glib::RefPtr<Gdk::GLContext>&) {
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
    if(_sceneView) {
        _sceneView->draw();

        if(_outline.selectedObjects().size() > 0) {
            /* Configure im3d gizmo */
            (*_im3d)
                .setViewportSize(_viewportSize)
                .newFrame();

            /* Show gizmo for the first selected object */
            /* TODO: make the gizmo work for multiple selected objects */
            Object3D* object = _sceneView->data().objects[*_outline.selectedObjects().begin()].object;
            Im3d::Mat4 gizmoTransformation(object->transformation());
            if(Im3d::Gizmo("ViewportGizmo", gizmoTransformation)) {
                object->setTransformation(Matrix4{gizmoTransformation});
                _properties.updateTransformation();
            }

            /* Render gizmo without depth test and with blending enabled */
            GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
            GL::Renderer::enable(GL::Renderer::Feature::Blending);
            GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
            _im3d->drawFrame();
            GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::Zero);
            GL::Renderer::disable(GL::Renderer::Feature::Blending);
            GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        }

        /* Force queue redraw */
        /* TODO: force the redraw only when necessary */
        queue_render();
    }

    /* Clean up Magnum state and back to Gtkmm */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
    return true;
}

void Viewport::onResize(int width, int height) {
    _viewportSize = {width, height};

    /* Update the scene viewport if there is one loaded */
    if(_sceneView) _sceneView->updateViewport(_viewportSize);
}

bool Viewport::onMotionNotifyEvent(GdkEventMotion* motionEvent) {
    if(_sceneView) {
        const Vector2 eventPosition{Float(motionEvent->x), Float(motionEvent->y)};

        if(_isDragging) {
            const Vector2 delta = 2.0f*
                Vector2{eventPosition - _previousMousePosition}/
                Vector2{Float(get_width()), Float(get_height())};

            (*_sceneView->data().cameraObject)
                .rotate(Rad{-delta.y()}, _sceneView->data().cameraObject->transformation().right().normalized())
                .rotateY(Rad{-delta.x()});

            _previousMousePosition = eventPosition;
        } else if(_outline.selectedObjects().size() > 0) {
            _im3d->updateCursorRay(eventPosition);
        }
    }

    return true;
}

bool Viewport::onButtonPressEvent(GdkEventButton* buttonEvent) {
    if(_sceneView) {
        if(buttonEvent->button == GDK_BUTTON_PRIMARY) {
            /* Grab focus so that key events work */
            grab_focus();

            Im3d::AppData& ad = Im3d::GetAppData();
            ad.m_keyDown[Im3d::Mouse_Left] = true;
        } else if(buttonEvent->button == GDK_BUTTON_SECONDARY) {
            /* Grab focus so that key events work */
            grab_focus();

            _isDragging = true;
            _previousMousePosition = Vector2{Float(buttonEvent->x), Float(buttonEvent->y)};
        }
    }

    return true;
}

bool Viewport::onButtonReleaseEvent(GdkEventButton* releaseEvent) {
    if(_sceneView) {
        if(releaseEvent->button == GDK_BUTTON_PRIMARY) {
            Im3d::AppData& ad = Im3d::GetAppData();
            ad.m_keyDown[Im3d::Mouse_Left] = false;
        } else if(releaseEvent->button == GDK_BUTTON_SECONDARY) {
            _isDragging = false;
        }
    }

    return true;
}

bool Viewport::onKeyPressEvent(GdkEventKey* keyEvent) {
    if(_sceneView) {
        if(_isDragging) {
            const Float speed = 0.1f;

            if(keyEvent->keyval == GDK_KEY_w || keyEvent->keyval == GDK_KEY_W)
                _sceneView->data().cameraObject->translate(-_sceneView->data().cameraObject->transformation().backward()*speed);
            else if(keyEvent->keyval == GDK_KEY_s || keyEvent->keyval == GDK_KEY_S)
                _sceneView->data().cameraObject->translate(_sceneView->data().cameraObject->transformation().backward()*speed);
            else if(keyEvent->keyval == GDK_KEY_a || keyEvent->keyval == GDK_KEY_A)
                _sceneView->data().cameraObject->translate(-_sceneView->data().cameraObject->transformation().right()*speed);
            else if(keyEvent->keyval == GDK_KEY_d || keyEvent->keyval == GDK_KEY_D)
                _sceneView->data().cameraObject->translate(_sceneView->data().cameraObject->transformation().right()*speed);
            else if(keyEvent->keyval == GDK_KEY_q || keyEvent->keyval == GDK_KEY_Q)
                _sceneView->data().cameraObject->translate(-_sceneView->data().cameraObject->transformation().up()*speed);
            else if(keyEvent->keyval == GDK_KEY_e || keyEvent->keyval == GDK_KEY_E)
                _sceneView->data().cameraObject->translate(_sceneView->data().cameraObject->transformation().up()*speed);
        } else {
            if(keyEvent->keyval == GDK_KEY_g || keyEvent->keyval == GDK_KEY_G)
                Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Translation;
            if(keyEvent->keyval == GDK_KEY_r || keyEvent->keyval == GDK_KEY_R)
                Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Rotation;
            if(keyEvent->keyval == GDK_KEY_s || keyEvent->keyval == GDK_KEY_S)
                Im3d::GetContext().m_gizmoMode = Im3d::GizmoMode_Scale;
        }
    }

    return true;
}

}}

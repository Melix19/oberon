#ifndef Oberon_Editor_Viewport_h
#define Oberon_Editor_Viewport_h
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

#include <gtkmm/builder.h>
#include <gtkmm/glarea.h>
#include <Corrade/Containers/Pointer.h>
#include <Magnum/Math/Vector2.h>
#include <Magnum/Platform/Platform.h>

#include "Oberon/Editor/Editor.h"
#include "Oberon/Editor/Im3dContext.h"

namespace Oberon { namespace Editor {

class Viewport: public Gtk::GLArea {
    public:
        explicit Viewport(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>&, Outline& outline, Platform::GLContext& context);

        void loadScene(const std::string& path);

    private:
        void onRealize();
        bool onRender(const Glib::RefPtr<Gdk::GLContext>&);
        void onResize(int width, int height);

        bool onMotionNotifyEvent(GdkEventMotion* motionEvent);
        bool onButtonPressEvent(GdkEventButton* buttonEvent);
        bool onButtonReleaseEvent(GdkEventButton* releaseEvent);

        bool onKeyPressEvent(GdkEventKey* keyEvent);

        Outline& _outline;
        Platform::GLContext& _context;

        Containers::Pointer<Im3dContext> _im3d;

        Vector2i _viewportSize;
        Containers::Pointer<SceneView> _sceneView;

        bool _isDragging;
        Vector2 _previousMousePosition;
};

}}

#endif

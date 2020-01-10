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

#pragma once

#include "CollectionPanel.h"

#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>

template<class MouseEvent> void CollectionPanel::handleMousePressEvent(MouseEvent& event) {
    if(event.button() == MouseEvent::Button::Right && _isHovered) {
        _previousMousePosition = event.position();
        _isDragging = true;
    }
}

template<class MouseEvent> void CollectionPanel::handleMouseReleaseEvent(MouseEvent& event) {
    switch(event.button()) {
        case MouseEvent::Button::Left: {
            const Vector2i mouseViewportPos = (event.position() - Vector2i{_viewportPos})*_dpiScaleRatio;
            const Vector2i fbMouseViewportPos{mouseViewportPos.x(), _viewportTextureSize.y()*
                Int(_dpiScaleRatio.y()) - mouseViewportPos.y() - 1};

            /* Read object ID at given click position */
            _framebuffer.mapForRead(GL::Framebuffer::ColorAttachment{1});
            Image2D data = _framebuffer.read(Range2Di::fromSize(fbMouseViewportPos, {1, 1}),
                {PixelFormat::R32UI});

            UnsignedByte id = Containers::arrayCast<UnsignedByte>(data.data())[0];

            const bool altPressed = event.modifiers() >= MouseEvent::Modifier::Alt;
            const bool ctrlPressed = event.modifiers() >= MouseEvent::Modifier::Ctrl;
            const bool shiftPressed = event.modifiers() >= MouseEvent::Modifier::Shift;
            const bool superPressed = event.modifiers() >= MouseEvent::Modifier::Super;

            /* Use the macOS style shortcuts (Cmd/Super instead of Ctrl) for macOS. */
            #ifdef CORRADE_TARGET_APPLE
            const bool isShortcutKey = superPressed && !ctrlPressed && !altPressed && !shiftPressed;
            #else
            const bool isShortcutKey = !superPressed && ctrlPressed && !altPressed && !shiftPressed;
            #endif

            if(!isShortcutKey) {
                for(auto& selectedNode: _selectedNodes)
                    selectedNode->setSelected(false);
                _selectedNodes.clear();
            }

            if(id > 0 && id < _drawablesNodes.size() + 1) {
                ObjectNode* pickedNode = _drawablesNodes[id - 1];

                if(isShortcutKey && pickedNode->isSelected()) {
                    auto found = std::find_if(_selectedNodes.begin(), _selectedNodes.end(),
                        [&](ObjectNode* p) { return p == pickedNode; });

                    CORRADE_INTERNAL_ASSERT(found != _selectedNodes.end());

                    _selectedNodes.erase(found);
                    pickedNode->setSelected(false);
                } else {
                    _selectedNodes.push_back(pickedNode);
                    pickedNode->setSelected(true);
                }
            }
        } break;
        case MouseEvent::Button::Right: {
            _isDragging = false;
        } break;
        default:
            break;
    }
}

template<class MouseMoveEvent> void CollectionPanel::handleMouseMoveEvent(MouseMoveEvent& event) {
    if(_isDragging) {
        if(_isOrthographicCamera) {
            const Vector2 delta{event.position() - _previousMousePosition};
            _cameraObject->translate({-delta.x(), delta.y(), 0});
        } else {
            const Vector2 delta = 3.0f*
                Vector2{event.position() - _previousMousePosition}/_viewportSize;
            _cameraObject->rotate(Rad{-delta.y()}, _cameraObject->transformation().right().normalized())
                .rotateY(Rad{-delta.x()});
        }

        _previousMousePosition = event.position();
    }
}

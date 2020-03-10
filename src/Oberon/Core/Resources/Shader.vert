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

uniform highp mat4 transformationMatrix;
uniform highp mat4 projectionMatrix;
#if LIGHT_COUNT
uniform mediump mat3 normalMatrix;
#endif

layout(location = 0) in highp vec4 position;
#if LIGHT_COUNT
layout(location = 2) in mediump vec3 normal;
#endif

#if LIGHT_COUNT
out highp vec3 transformedPosition;
out mediump vec3 transformedNormal;
#endif

void main() {
    highp vec4 transformedPosition4 = transformationMatrix*position;

    #if LIGHT_COUNT
    transformedPosition = transformedPosition4.xyz/transformedPosition4.w;
    transformedNormal = normalMatrix*normal;
    #endif

    gl_Position = projectionMatrix*transformedPosition4;
}

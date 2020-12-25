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

#define VertexData _VertexData { \
    noperspective highp float edgeDistance; \
    noperspective highp float size; \
    smooth lowp vec4 color; \
}

uniform vec2 viewport;

layout(lines) in;
in VertexData data[];

layout(triangle_strip, max_vertices = 4) out;
out VertexData dataOut;

void main() {
    vec2 pos0 = gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
    vec2 pos1 = gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;

    vec2 dir = pos0 - pos1;
    dir = normalize(vec2(dir.x, dir.y*viewport.y/viewport.x));
    vec2 tng0 = vec2(-dir.y, dir.x);
    vec2 tng1 = tng0*data[1].size/viewport;
    tng0 = tng0*data[0].size/viewport;

    gl_Position = vec4((pos0 - tng0)*gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
    dataOut.edgeDistance = -data[0].size;
    dataOut.size = data[0].size;
    dataOut.color = data[0].color;
    EmitVertex();

    gl_Position = vec4((pos0 + tng0)*gl_in[0].gl_Position.w, gl_in[0].gl_Position.zw);
    dataOut.color = data[0].color;
    dataOut.edgeDistance = data[0].size;
    dataOut.size = data[0].size;
    EmitVertex();

    gl_Position = vec4((pos1 - tng1)*gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
    dataOut.edgeDistance = -data[1].size;
    dataOut.size = data[1].size;
    dataOut.color = data[1].color;
    EmitVertex();

    gl_Position = vec4((pos1 + tng1)*gl_in[1].gl_Position.w, gl_in[1].gl_Position.zw);
    dataOut.color = data[1].color;
    dataOut.size = data[1].size;
    dataOut.edgeDistance = data[1].size;
    EmitVertex();
}

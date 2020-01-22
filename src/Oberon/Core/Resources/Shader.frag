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

uniform lowp vec3 ambientColor;
uniform highp uint objectId;
#if LIGHT_COUNT
uniform lowp vec3 diffuseColor;
uniform lowp vec3 specularColor;
uniform mediump float shininess;
uniform lowp vec3 lightColors[LIGHT_COUNT];
#endif

#if LIGHT_COUNT
in mediump vec3 transformedNormal;
in highp vec3 lightDirections[LIGHT_COUNT];
in highp vec3 cameraDirection;
#endif

layout(location = 0) out lowp vec4 fragmentColor;
layout(location = 1) out highp uint fragmentObjectId;

void main() {
    fragmentColor.rgb = ambientColor;

    #if LIGHT_COUNT
    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    for(int i = 0; i < LIGHT_COUNT; ++i) {
        highp vec3 normalizedLightDirection = normalize(lightDirections[i]);
        lowp float intensity = max(0.0, dot(normalizedTransformedNormal, normalizedLightDirection));
        fragmentColor.rgb += diffuseColor*lightColors[i]*intensity;

        if(intensity > 0.001) {
            highp vec3 reflection = reflect(-normalizedLightDirection, normalizedTransformedNormal);
            mediump float specularity = pow(max(0.0, dot(normalize(cameraDirection), reflection)), shininess);
            fragmentColor.rgb += specularColor*specularity;
        }
    }
    #endif

    fragmentColor.a = 1.0;
    fragmentObjectId = objectId;
}

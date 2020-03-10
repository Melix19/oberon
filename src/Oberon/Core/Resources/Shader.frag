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

uniform highp uint objectId;
uniform lowp vec3 ambientColor;
#if LIGHT_COUNT
uniform lowp vec3 diffuseColor;
uniform lowp vec3 specularColor;
uniform mediump float shininess;

struct Light {
    highp vec3 position;
    lowp vec3 color;

    mediump float constant;
    mediump float linear;
    mediump float quadratic;
};

uniform Light lights[LIGHT_COUNT];
#endif

#if LIGHT_COUNT
in highp vec3 transformedPosition;
in mediump vec3 transformedNormal;
#endif

layout(location = 0) out lowp vec4 fragmentColor;
layout(location = 1) out highp uint fragmentObjectId;

void main() {
    /* Ambient */
    fragmentColor.rgb = ambientColor;

    #if LIGHT_COUNT
    /* Normal */
    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    for(int i = 0; i < LIGHT_COUNT; ++i) {
        /* Point light attenuation */
        mediump float distance = length(lights[i].position - transformedPosition);
        mediump float attenuation = 1.0/(lights[i].constant + lights[i].linear*distance +
            lights[i].quadratic*(distance*distance));

        /* Diffuse */
        highp vec3 normalizedLightDirection = normalize(lights[i].position - transformedPosition);
        lowp float intensity = max(0.0, dot(normalizedTransformedNormal, normalizedLightDirection));
        fragmentColor.rgb += diffuseColor*lights[i].color*intensity*attenuation;

        /* Specular, if needed */
        if(intensity > 0.001) {
            highp vec3 reflection = reflect(-normalizedLightDirection, normalizedTransformedNormal);
            mediump float specularity = clamp(pow(max(0.0, dot(normalize(-transformedPosition), reflection)), shininess), 0.0, 1.0);
            fragmentColor.rgb += specularColor*specularity*attenuation;
        }
    }
    #endif

    fragmentColor.a = 1.0;
    fragmentObjectId = objectId;
}

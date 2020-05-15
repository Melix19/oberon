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

#if defined(OBJECT_ID) && !defined(GL_ES) && !defined(NEW_GLSL)
#extension GL_EXT_gpu_shader4: require
#endif

#ifndef NEW_GLSL
#define in varying
#define fragmentColor gl_FragColor
#define texture texture2D
#endif

#ifndef RUNTIME_CONST
#define const
#endif

#ifdef TEXTURED
#ifdef EXPLICIT_TEXTURE_LAYER
layout(binding = 0)
#endif
uniform lowp sampler2D ambientTexture;
#endif

#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 3)
#endif
uniform lowp vec4 ambientColor;

#if LIGHT_COUNT
#ifdef TEXTURED
#ifdef EXPLICIT_TEXTURE_LAYER
layout(binding = 1)
#endif
uniform lowp sampler2D diffuseTexture;
#endif

#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 4)
#endif
uniform lowp vec4 diffuseColor;

#ifdef TEXTURED
#ifdef EXPLICIT_TEXTURE_LAYER
layout(binding = 2)
#endif
uniform lowp sampler2D specularTexture;
#endif

#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 5)
#endif
uniform lowp vec4 specularColor;

#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 6)
#endif
uniform mediump float shininess;
#endif

#ifdef OBJECT_ID
#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 7)
#endif
uniform highp uint objectId;
#endif

#if LIGHT_COUNT
struct PointLight {
    highp vec3 position;
    lowp vec4 color;

    mediump float constant;
    mediump float linear;
    mediump float quadratic;
};

/* Needs to be last because it uses locations 8 to 8 + LIGHT_COUNT - 1 */
#ifdef EXPLICIT_UNIFORM_LOCATION
layout(location = 8)
#endif
uniform PointLight pointLights[LIGHT_COUNT];
#endif

#if LIGHT_COUNT
in highp vec3 transformedPosition;
in mediump vec3 transformedNormal;
#endif

#if defined(TEXTURED)
in mediump vec2 interpolatedTextureCoordinates;
#endif

#ifdef NEW_GLSL
#ifdef EXPLICIT_ATTRIB_LOCATION
layout(location = COLOR_OUTPUT_ATTRIBUTE_LOCATION)
#endif
out lowp vec4 fragmentColor;
#endif
#ifdef OBJECT_ID
#ifdef EXPLICIT_ATTRIB_LOCATION
layout(location = OBJECT_ID_OUTPUT_ATTRIBUTE_LOCATION)
#endif
out highp uint fragmentObjectId;
#endif

void main() {
    lowp const vec4 finalAmbientColor =
        #ifdef TEXTURED
        texture(ambientTexture, interpolatedTextureCoordinates)*
        #endif
        ambientColor;
    #if LIGHT_COUNT
    lowp const vec4 finalDiffuseColor =
        #ifdef TEXTURED
        texture(diffuseTexture, interpolatedTextureCoordinates)*
        #endif
        diffuseColor;
    lowp const vec4 finalSpecularColor =
        #ifdef TEXTURED
        texture(specularTexture, interpolatedTextureCoordinates)*
        #endif
        specularColor;
    #endif

    /* Ambient color */
    fragmentColor = finalAmbientColor;

    #if LIGHT_COUNT
    highp vec3 cameraDirection = -transformedPosition;

    /* Normal */
    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    /* Add diffuse color for each light */
    for(int i = 0; i < LIGHT_COUNT; ++i) {
        /* Attenuation */
        float distance = length(pointLights[i].position - transformedPosition);
        float attenuation = 1.0/(pointLights[i].constant + pointLights[i].linear*distance + pointLights[i].quadratic*(distance*distance));

        /* Diffuse */
        highp vec3 lightDirection = normalize(pointLights[i].position - transformedPosition);
        highp vec3 normalizedLightDirection = normalize(lightDirection);
        lowp float intensity = max(0.0, dot(normalizedTransformedNormal, normalizedLightDirection));
        fragmentColor += vec4(finalDiffuseColor.rgb*pointLights[i].color.rgb*intensity*attenuation, pointLights[i].color.a*finalDiffuseColor.a/float(LIGHT_COUNT));

        /* Add specular color, if needed */
        if(intensity > 0.001) {
            highp vec3 reflection = reflect(-normalizedLightDirection, normalizedTransformedNormal);
            mediump float specularity = clamp(pow(max(0.0, dot(normalize(cameraDirection), reflection)), shininess), 0.0, 1.0);
            fragmentColor += vec4(finalSpecularColor.rgb*specularity*attenuation, finalSpecularColor.a);
        }
    }
    #endif

    #ifdef OBJECT_ID
    fragmentObjectId = objectId;
    #endif
}

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

#include "SceneShader.h"

#include <Corrade/Containers/Reference.h>
#include <Corrade/Utility/FormatStl.h>
#include <Corrade/Utility/Resource.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Extensions.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

static void importCoreResources() {
    CORRADE_RESOURCE_INITIALIZE(OberonCore_RCS)
}

namespace {

inline GL::Shader createCompatibilityShader(const Utility::Resource& rs, GL::Version version, GL::Shader::Type type) {
    GL::Shader shader(version, type);

    #ifndef MAGNUM_TARGET_GLES
    if(GL::Context::current().isExtensionDisabled<GL::Extensions::ARB::explicit_attrib_location>(version))
        shader.addSource("#define DISABLE_GL_ARB_explicit_attrib_location\n");
    if(GL::Context::current().isExtensionDisabled<GL::Extensions::ARB::shading_language_420pack>(version))
        shader.addSource("#define DISABLE_GL_ARB_shading_language_420pack\n");
    if(GL::Context::current().isExtensionDisabled<GL::Extensions::ARB::explicit_uniform_location>(version))
        shader.addSource("#define DISABLE_GL_ARB_explicit_uniform_location\n");
    #endif

    #ifndef MAGNUM_TARGET_GLES2
    if(type == GL::Shader::Type::Vertex && GL::Context::current().isExtensionDisabled<GL::Extensions::MAGNUM::shader_vertex_id>(version))
        shader.addSource("#define DISABLE_GL_MAGNUM_shader_vertex_id\n");
    #endif

    /* My Android emulator (running on NVidia) doesn't define GL_ES
       preprocessor macro, thus *all* the stock shaders fail to compile */
    /** @todo remove this when Android emulator is sane */
    #ifdef CORRADE_TARGET_ANDROID
    shader.addSource("#ifndef GL_ES\n#define GL_ES 1\n#endif\n");
    #endif

    shader.addSource(rs.get("compatibility.glsl"));
    return shader;
}

enum: Int {
    AmbientTextureUnit = 0,
    DiffuseTextureUnit = 1,
    SpecularTextureUnit = 2,
    NormalTextureUnit = 3
};

}

SceneShader::SceneShader(const Flags flags, const UnsignedInt lightCount): _flags{flags}, _lightCount{lightCount} {
    if(!Utility::Resource::hasGroup("OberonCore"))
        importCoreResources();

    Utility::Resource rs("OberonCore");
    Utility::Resource magnumRs("MagnumShaders");

    #ifndef MAGNUM_TARGET_GLES
    const GL::Version version = GL::Context::current().supportedVersion({GL::Version::GL320, GL::Version::GL310, GL::Version::GL300, GL::Version::GL210});
    #else
    const GL::Version version = GL::Context::current().supportedVersion({GL::Version::GLES300, GL::Version::GLES200});
    #endif

    GL::Shader vert = createCompatibilityShader(magnumRs, version, GL::Shader::Type::Vertex);
    GL::Shader frag = createCompatibilityShader(magnumRs, version, GL::Shader::Type::Fragment);

    vert.addSource(flags & (Flag::AmbientTexture|Flag::DiffuseTexture|Flag::SpecularTexture|Flag::NormalTexture) ? "#define TEXTURED\n" : "")
        .addSource(flags & Flag::NormalTexture ? "#define NORMAL_TEXTURE\n" : "")
        .addSource(Utility::formatString("#define LIGHT_COUNT {}\n", lightCount))
        .addSource(magnumRs.get("generic.glsl"))
        .addSource(rs.get("SceneShader.vert"));
    frag.addSource(flags & Flag::AmbientTexture ? "#define AMBIENT_TEXTURE\n" : "")
        .addSource(flags & Flag::DiffuseTexture ? "#define DIFFUSE_TEXTURE\n" : "")
        .addSource(flags & Flag::SpecularTexture ? "#define SPECULAR_TEXTURE\n" : "")
        .addSource(flags & Flag::NormalTexture ? "#define NORMAL_TEXTURE\n" : "")
        #ifndef MAGNUM_TARGET_GLES2
        .addSource(flags & Flag::ObjectId ? "#define OBJECT_ID\n" : "")
        #endif
        .addSource(Utility::formatString("#define LIGHT_COUNT {}\n", lightCount))
        .addSource(magnumRs.get("generic.glsl"))
        .addSource(rs.get("SceneShader.frag"));

    CORRADE_INTERNAL_ASSERT_OUTPUT(GL::Shader::compile({vert, frag}));

    attachShaders({vert, frag});

    /* ES3 has this done in the shader directly and doesn't even provide
       bindFragmentDataLocation() */
    #if !defined(MAGNUM_TARGET_GLES) || defined(MAGNUM_TARGET_GLES2)
    #ifndef MAGNUM_TARGET_GLES
    if(!GL::Context::current().isExtensionSupported<GL::Extensions::ARB::explicit_attrib_location>(version))
    #endif
    {
        bindAttributeLocation(Position::Location, "position");
        if(lightCount)
            bindAttributeLocation(Normal::Location, "normal");
        if((flags & Flag::NormalTexture) && lightCount)
            bindAttributeLocation(Tangent::Location, "tangent");
        if(flags & (Flag::AmbientTexture|Flag::DiffuseTexture|Flag::SpecularTexture))
            bindAttributeLocation(TextureCoordinates::Location, "textureCoordinates");
        #ifndef MAGNUM_TARGET_GLES2
        if(flags & Flag::ObjectId) {
            bindFragmentDataLocation(ColorOutput, "color");
            bindFragmentDataLocation(ObjectIdOutput, "objectId");
        }
        #endif
    }
    #endif

    CORRADE_INTERNAL_ASSERT_OUTPUT(link());

    #ifndef MAGNUM_TARGET_GLES
    if(!GL::Context::current().isExtensionSupported<GL::Extensions::ARB::explicit_uniform_location>(version))
    #endif
    {
        _transformationMatrixUniform = uniformLocation("transformationMatrix");
        _projectionMatrixUniform = uniformLocation("projectionMatrix");
        _ambientColorUniform = uniformLocation("ambientColor");
        if(lightCount) {
            _normalMatrixUniform = uniformLocation("normalMatrix");
            _diffuseColorUniform = uniformLocation("diffuseColor");
            _specularColorUniform = uniformLocation("specularColor");
            _shininessUniform = uniformLocation("shininess");
            _pointLightsUniform = uniformLocation("pointLights");
        }
        #ifndef MAGNUM_TARGET_GLES2
        if(flags & Flag::ObjectId) _objectIdUniform = uniformLocation("objectId");
        #endif
    }

    #ifndef MAGNUM_TARGET_GLES
    if(flags && !GL::Context::current().isExtensionSupported<GL::Extensions::ARB::shading_language_420pack>(version))
    #endif
    {
        if(flags & Flag::AmbientTexture) setUniform(uniformLocation("ambientTexture"), AmbientTextureUnit);
        if(lightCount) {
            if(flags & Flag::DiffuseTexture) setUniform(uniformLocation("diffuseTexture"), DiffuseTextureUnit);
            if(flags & Flag::SpecularTexture) setUniform(uniformLocation("specularTexture"), SpecularTextureUnit);
            if(flags & Flag::NormalTexture) setUniform(uniformLocation("normalTexture"), NormalTextureUnit);
        }
    }
}

SceneShader& SceneShader::setAmbientColor(const Color4& color) {
    setUniform(_ambientColorUniform, color);
    return *this;
}

SceneShader& SceneShader::bindAmbientTexture(GL::Texture2D& texture) {
    CORRADE_ASSERT(_flags & Flag::AmbientTexture,
        "SceneShader::bindAmbientTexture(): the shader was not created with ambient texture enabled", *this);
    texture.bind(AmbientTextureUnit);
    return *this;
}

SceneShader& SceneShader::setDiffuseColor(const Color4& color) {
    if(_lightCount) setUniform(_diffuseColorUniform, color);
    return *this;
}

SceneShader& SceneShader::bindDiffuseTexture(GL::Texture2D& texture) {
    CORRADE_ASSERT(_flags & Flag::DiffuseTexture,
        "SceneShader::bindDiffuseTexture(): the shader was not created with diffuse texture enabled", *this);
    if(_lightCount) texture.bind(DiffuseTextureUnit);
    return *this;
}

SceneShader& SceneShader::setSpecularColor(const Color4& color) {
    if(_lightCount) setUniform(_specularColorUniform, color);
    return *this;
}

SceneShader& SceneShader::bindSpecularTexture(GL::Texture2D& texture) {
    CORRADE_ASSERT(_flags & Flag::SpecularTexture,
        "SceneShader::bindSpecularTexture(): the shader was not created with specular texture enabled", *this);
    if(_lightCount) texture.bind(SpecularTextureUnit);
    return *this;
}

SceneShader& SceneShader::bindNormalTexture(GL::Texture2D& texture) {
    CORRADE_ASSERT(_flags & Flag::NormalTexture,
        "SceneShader::bindNormalTexture(): the shader was not created with normal texture enabled", *this);
    if(_lightCount) texture.bind(NormalTextureUnit);
    return *this;
}

SceneShader& SceneShader::setShininess(Float shininess) {
    if(_lightCount) setUniform(_shininessUniform, shininess);
    return *this;
}

#ifndef MAGNUM_TARGET_GLES2
SceneShader& SceneShader::setObjectId(UnsignedInt id) {
    CORRADE_ASSERT(_flags & Flag::ObjectId,
        "SceneShader::setObjectId(): the shader was not created with object ID enabled", *this);
    setUniform(_objectIdUniform, id);
    return *this;
}
#endif

SceneShader& SceneShader::setTransformationMatrix(const Matrix4& matrix) {
    setUniform(_transformationMatrixUniform, matrix);
    return *this;
}

SceneShader& SceneShader::setNormalMatrix(const Matrix3x3& matrix) {
    if(_lightCount) setUniform(_normalMatrixUniform, matrix);
    return *this;
}

SceneShader& SceneShader::setProjectionMatrix(const Matrix4& matrix) {
    setUniform(_projectionMatrixUniform, matrix);
    return *this;
}

SceneShader& SceneShader::setPointLight(UnsignedInt id, const Vector3& position, const Color4& color, Float constant, Float linear, Float quadratic) {
    CORRADE_ASSERT(id < _lightCount,
        "SceneShader::setPointLight(): light ID" << id << "is out of bounds for" << _lightCount << "lights", *this);
    setUniform(_pointLightsUniform + id, position);
    setUniform(_pointLightsUniform + id + 1, color);
    setUniform(_pointLightsUniform + id + 2, constant);
    setUniform(_pointLightsUniform + id + 3, linear);
    setUniform(_pointLightsUniform + id + 4, quadratic);
    return *this;
}

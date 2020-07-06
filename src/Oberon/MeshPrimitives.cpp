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

#include "MeshPrimitives.h"

#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#include <Magnum/Primitives/Capsule.h>
#include <Magnum/Primitives/Circle.h>
#include <Magnum/Primitives/Cone.h>
#include <Magnum/Primitives/Cylinder.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData.h>

namespace Oberon {

namespace MeshPrimitives {

Resource<GL::Mesh> capsule(Float radius, Float length, UnsignedInt hemisphereRings, UnsignedInt cylinderRings, UnsignedInt segments, OberonResourceManager& resourceManager) {
    std::string meshKey = "capsule";
    meshKey.append("-radius=" + std::to_string(radius));
    meshKey.append("-length=" + std::to_string(length));
    meshKey.append("-hemisphereRings=" + hemisphereRings);
    meshKey.append("-cylinderRings=" + cylinderRings);
    meshKey.append("-segments=" + segments);

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        const Primitives::CapsuleFlags flags = Primitives::CapsuleFlag::TextureCoordinates |
            Primitives::CapsuleFlag::Tangents;
        Trade::MeshData capsule = Primitives::capsule3DSolid(hemisphereRings, cylinderRings,
            segments, 0.5f*length/radius, flags);

        if(radius != 1.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling(Vector3{radius}),
                capsule.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(capsule);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

Resource<GL::Mesh> circle(Float radius, UnsignedInt segments, OberonResourceManager& resourceManager) {
    std::string meshKey = "circle";
    meshKey.append("-radius=" + std::to_string(radius));
    meshKey.append("-segments=" + segments);

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        const Primitives::Circle3DFlags flags = Primitives::Circle3DFlag::TextureCoordinates |
            Primitives::Circle3DFlag::Tangents;
        Trade::MeshData circle = Primitives::circle3DSolid(segments, flags);

        if(radius != 1.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling({Vector2{radius}, 0.0f}),
                circle.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(circle);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

Resource<GL::Mesh> cone(Float radius, Float length, UnsignedInt rings, UnsignedInt segments, bool capEnd, OberonResourceManager& resourceManager) {
    std::string meshKey = "cone";
    meshKey.append("-radius=" + std::to_string(radius));
    meshKey.append("-length=" + std::to_string(length));
    meshKey.append("-rings=" + rings);
    meshKey.append("-segments=" + segments);
    meshKey.append("-capEnd=" + capEnd);

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        Primitives::ConeFlags flags = Primitives::ConeFlag::TextureCoordinates |
            Primitives::ConeFlag::Tangents;
        if(capEnd) flags |= Primitives::ConeFlag::CapEnd;
        Trade::MeshData cone = Primitives::coneSolid(rings, segments, 0.5f*length/radius, flags);

        if(radius != 1.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling(Vector3{radius}),
                cone.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(cone);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

Resource<GL::Mesh> cylinder(Float radius, Float length, UnsignedInt rings, UnsignedInt segments, bool capEnds, OberonResourceManager& resourceManager) {
    std::string meshKey = "cylinder";
    meshKey.append("-radius=" + std::to_string(radius));
    meshKey.append("-length=" + std::to_string(length));
    meshKey.append("-rings=" + rings);
    meshKey.append("-segments=" + segments);
    meshKey.append("-capEnds=" + capEnds);

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        Primitives::CylinderFlags flags = Primitives::CylinderFlag::TextureCoordinates |
            Primitives::CylinderFlag::Tangents;
        if(capEnds) flags |= Primitives::CylinderFlag::CapEnds;
        Trade::MeshData cylinder = Primitives::cylinderSolid(rings, segments, 0.5f*length/radius, flags);

        if(radius != 1.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling(Vector3{radius}),
                cylinder.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(cylinder);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

Resource<GL::Mesh> plane(Vector2 size, OberonResourceManager& resourceManager) {
    std::string meshKey = "plane";
    meshKey.append("-size=" + std::to_string(size.x()) + " " + std::to_string(size.y()));

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        const Primitives::PlaneFlags flags = Primitives::PlaneFlag::TextureCoordinates |
            Primitives::PlaneFlag::Tangents;
        Trade::MeshData plane = Primitives::planeSolid(flags);

        if(size.x() != 2.0f || size.y() != 2.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling({size/2, 0.0f}),
                plane.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(plane);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

Resource<GL::Mesh> sphere(Float radius, UnsignedInt rings, UnsignedInt segments, OberonResourceManager& resourceManager) {
    std::string meshKey = "sphere";
    meshKey.append("-radius=" + std::to_string(radius));
    meshKey.append("-rings=" + rings);
    meshKey.append("-segments=" + segments);

    Resource<GL::Mesh> meshResource = resourceManager.get<GL::Mesh>(meshKey);
    if(!meshResource) {
        const Primitives::UVSphereFlags flags = Primitives::UVSphereFlag::TextureCoordinates |
            Primitives::UVSphereFlag::Tangents;
        Trade::MeshData sphere = Primitives::uvSphereSolid(rings, segments, flags);

        if(radius != 1.0f)
            MeshTools::transformPointsInPlace(Matrix4::scaling(Vector3{radius}),
                sphere.mutableAttribute<Vector3>(Trade::MeshAttribute::Position));

        GL::Mesh mesh = MeshTools::compile(sphere);
        resourceManager.set(meshResource.key(), std::move(mesh));
    }

    return meshResource;
}

}

}

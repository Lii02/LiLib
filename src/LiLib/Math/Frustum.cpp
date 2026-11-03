#include "LiLib/Math/Frustum.h"

namespace LiLib {

namespace {

float Element(const Mat4& matrix, int row, int column) {
    return matrix.columns[column][row];
}

// Combines rows of the matrix as a*rowA + b*rowB and normalizes the resulting plane
Plane MakePlane(const Mat4& matrix, int rowA, float a, int rowB, float b) {
    const Vec3 normal = {a * Element(matrix, rowA, 0) + b * Element(matrix, rowB, 0),
                         a * Element(matrix, rowA, 1) + b * Element(matrix, rowB, 1),
                         a * Element(matrix, rowA, 2) + b * Element(matrix, rowB, 2)};
    const float distance = a * Element(matrix, rowA, 3) + b * Element(matrix, rowB, 3);
    const float length = normal.Length();
    if(length <= 1e-6f) {
        return {normal, distance};
    }
    return {normal / length, distance / length};
}

} // namespace

Frustum Frustum::FromMatrix(const Mat4& viewProjection) {
    constexpr int rowX = 0;
    constexpr int rowY = 1;
    constexpr int rowZ = 2;
    constexpr int rowW = 3;

    Frustum frustum;
    frustum.planes[0] = MakePlane(viewProjection, rowW, 1.0f, rowX, 1.0f);  // Left: x >= -w
    frustum.planes[1] = MakePlane(viewProjection, rowW, 1.0f, rowX, -1.0f); // Right: x <= w
    frustum.planes[2] = MakePlane(viewProjection, rowW, 1.0f, rowY, 1.0f);  // Bottom: y >= -w
    frustum.planes[3] = MakePlane(viewProjection, rowW, 1.0f, rowY, -1.0f); // Top: y <= w
    frustum.planes[4] = MakePlane(viewProjection, rowZ, 1.0f, rowZ, 0.0f);  // Near: z >= 0
    frustum.planes[5] = MakePlane(viewProjection, rowW, 1.0f, rowZ, -1.0f); // Far: z <= w
    return frustum;
}

bool Frustum::Intersects(const AABB& box) const {
    if(box.IsEmpty()) {
        return false;
    }
    for(const Plane& plane : planes) {
        // The corner furthest along the plane's normal; if that one is behind, the whole box is
        const Vec3 corner = {plane.normal.x >= 0.0f ? box.max.x : box.min.x, plane.normal.y >= 0.0f ? box.max.y : box.min.y,
                             plane.normal.z >= 0.0f ? box.max.z : box.min.z};
        if(plane.normal.Dot(corner) + plane.distance < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum::IntersectsSphere(Vec3 center, float radius) const {
    for(const Plane& plane : planes) {
        if(plane.normal.Dot(center) + plane.distance < -radius) {
            return false;
        }
    }
    return true;
}

} // namespace LiLib

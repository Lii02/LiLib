#pragma once
#include "LiLib/Math/AABB.h"
#include "LiLib/Math/Mat4.h"
#include "LiLib/Math/Vec3.h"

namespace LiLib {

/*
 * @brief An infinite plane: every point p on it satisfies normal.Dot(p) + distance == 0. The normal side is positive.
 */
struct Plane {
    Vec3 normal = {0.0f, 1.0f, 0.0f};
    float distance = 0.0f;
};

/*
 * @brief Six inward-facing planes, used to test whether objects are visible to a camera or a shadow-casting light.
 */
class Frustum {
public:
    /*
     * @brief Extracts the planes of a view-projection matrix with clip-space z in [0, 1], as Mat4's projections produce.
     */
    static Frustum FromMatrix(const Mat4& viewProjection);

    /*
     * @brief Conservative tests: a shape near a corner may be reported as intersecting when it is just outside.
     */
    bool Intersects(const AABB& box) const;
    bool IntersectsSphere(Vec3 center, float radius) const;

    Plane planes[6];
};

} // namespace LiLib

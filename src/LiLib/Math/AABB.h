#pragma once
#include "LiLib/Math/Mat4.h"
#include "LiLib/Math/Vec3.h"
#include <cfloat>

namespace LiLib {

/*
 * @brief Axis-aligned bounding box. A default-constructed AABB is empty: expanding it by a point gives a box around
 *        that point, and it intersects nothing.
 */
class AABB {
public:
    bool IsEmpty() const { return min.x > max.x || min.y > max.y || min.z > max.z; }
    Vec3 Center() const { return (min + max) * 0.5f; }
    Vec3 Extents() const { return (max - min) * 0.5f; }

    void Expand(Vec3 point);
    void Expand(const AABB& other);

    bool Contains(Vec3 point) const;
    bool Intersects(const AABB& other) const;

    /*
     * @brief The smallest AABB around this box after a transform. An empty box stays empty.
     */
    AABB Transformed(const Mat4& transform) const;

    Vec3 min = {FLT_MAX, FLT_MAX, FLT_MAX};
    Vec3 max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
};

} // namespace LiLib

#include "LiLib/Math/AABB.h"
#include "LiLib/Core/Typedefs.h"

namespace LiLib {

void AABB::Expand(Vec3 point) {
    min = {Min(min.x, point.x), Min(min.y, point.y), Min(min.z, point.z)};
    max = {Max(max.x, point.x), Max(max.y, point.y), Max(max.z, point.z)};
}

void AABB::Expand(const AABB& other) {
    if(other.IsEmpty()) {
        return;
    }
    Expand(other.min);
    Expand(other.max);
}

bool AABB::Contains(Vec3 point) const {
    return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y && point.z >= min.z && point.z <= max.z;
}

bool AABB::Intersects(const AABB& other) const {
    if(IsEmpty() || other.IsEmpty()) {
        return false;
    }
    return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y && max.y >= other.min.y && min.z <= other.max.z &&
           max.z >= other.min.z;
}

AABB AABB::Transformed(const Mat4& transform) const {
    if(IsEmpty()) {
        return {};
    }
    AABB result;
    for(int corner = 0; corner < 8; ++corner) {
        const Vec3 point = {(corner & 1) ? max.x : min.x, (corner & 2) ? max.y : min.y, (corner & 4) ? max.z : min.z};
        result.Expand(transform.TransformPoint3D(point));
    }
    return result;
}

} // namespace LiLib

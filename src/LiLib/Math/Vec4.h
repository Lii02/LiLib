#pragma once
#include "LiLib/Math/Vec3.h"

namespace LiLib {

class Vec4 {
public:
    Vec4 operator+(Vec4 other) const { return {x + other.x, y + other.y, z + other.z, w + other.w}; }
    Vec4 operator-(Vec4 other) const { return {x - other.x, y - other.y, z - other.z, w - other.w}; }
    Vec4 operator-() const { return {-x, -y, -z, -w}; }
    Vec4 operator*(float s) const { return {x * s, y * s, z * s, w * s}; }
    Vec4 operator/(float s) const { return {x / s, y / s, z / s, w / s}; }

    friend Vec4 operator*(float s, Vec4 v) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }

    bool operator==(Vec4 other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(Vec4 other) const { return !(*this == other); }

    float Dot(Vec4 other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
    float Length() const { return std::sqrt(Dot(*this)); }
    Vec3 XYZ() const { return {x, y, z}; }

    Vec4 Lerp(Vec4 to, float t) const { return {x + (to.x - x) * t, y + (to.y - y) * t, z + (to.z - z) * t, w + (to.w - w) * t}; }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

} // namespace LiLib

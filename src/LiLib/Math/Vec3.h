#pragma once
#include <cmath>

namespace LiLib {

class Vec3 {
public:
    Vec3 operator+(Vec3 other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(Vec3 other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator-() const { return {-x, -y, -z}; }
    Vec3 operator*(Vec3 other) const { return {x * other.x, y * other.y, z * other.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3 operator/(Vec3 other) const { return {x / other.x, y / other.y, z / other.z}; }

    friend Vec3 operator*(float s, Vec3 v) { return {v.x * s, v.y * s, v.z * s}; }

    Vec3& operator+=(Vec3 other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3& operator-=(Vec3 other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3& operator*=(float s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    bool operator==(Vec3 other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(Vec3 other) const { return !(*this == other); }

    float Dot(Vec3 other) const { return x * other.x + y * other.y + z * other.z; }
    Vec3 Cross(Vec3 other) const { return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x}; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }
    float Distance(Vec3 other) const { return (*this - other).Length(); }

    /*
     * @brief Returns this vector scaled to unit length, or zero if it is too short to normalize.
     */
    Vec3 Normalize() const {
        const float length = Length();
        return length > 1e-6f ? *this / length : Vec3{0.0f, 0.0f, 0.0f};
    }

    Vec3 Lerp(Vec3 to, float t) const { return {x + (to.x - x) * t, y + (to.y - y) * t, z + (to.z - z) * t}; }

    float x;
    float y;
    float z;
};

} // namespace LiLib

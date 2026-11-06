#pragma once
#include <cmath>

namespace LiLib {

class Vec2 {
public:
    Vec2 operator+(Vec2 other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator*(Vec2 other) const { return {x * other.x, y * other.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2 operator/(Vec2 other) const { return {x / other.x, y / other.y}; }

    friend Vec2 operator*(float s, Vec2 v) { return {v.x * s, v.y * s}; }

    Vec2& operator+=(Vec2 other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(Vec2 other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float s) {
        x *= s;
        y *= s;
        return *this;
    }

    bool operator==(Vec2 other) const { return x == other.x && y == other.y; }
    bool operator!=(Vec2 other) const { return !(*this == other); }

    float Dot(Vec2 other) const { return x * other.x + y * other.y; }
    float Cross(Vec2 other) const { return x * other.y - y * other.x; }
    float Length() const { return std::sqrt(x * x + y * y); }
    float LengthSquared() const { return x * x + y * y; }
    float Distance(Vec2 other) const { return (*this - other).Length(); }

    /*
     * @brief Returns this vector scaled to unit length, or zero if it is too short to normalize.
     */
    Vec2 Normalize() const {
        const float length = Length();
        return length > 1e-6f ? *this / length : Vec2{0.0f, 0.0f};
    }

    /*
     * @brief Returns this vector rotated counter-clockwise by angle radians.
     */
    Vec2 Rotate(float angle) const {
        const float c = std::cos(angle);
        const float s = std::sin(angle);
        return {x * c - y * s, x * s + y * c};
    }

    Vec2 Lerp(Vec2 to, float t) const { return {x + (to.x - x) * t, y + (to.y - y) * t}; }

    float x = 0.0f;
    float y = 0.0f;
};

} // namespace LiLib

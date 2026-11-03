#pragma once
#include "LiLib/Math/Mat4.h"
#include "LiLib/Math/Vec3.h"

namespace LiLib {

/*
 * @brief Rotation quaternion, stored as (x, y, z, w). Follows Mat4's conventions: right-handed, angles in radians,
 *        and a * b applies b first, then a. A default-constructed Quat is the identity.
 */
class Quat {
public:
    static Quat Identity() { return {}; }

    /*
     * @brief Rotation around an axis. The axis does not need to be unit length; a zero axis gives the identity.
     */
    static Quat FromAxisAngle(Vec3 axis, float angle);

    /*
     * @brief Builds a rotation from angles around x (pitch), y (yaw) and z (roll), applied in the order z, x, y.
     */
    static Quat FromEuler(Vec3 angles);

    /*
     * @brief Rotation that points -z along forward, keeping +y as close to up as possible.
     *        The result is degenerate if forward is zero; up may be parallel to forward.
     */
    static Quat LookRotation(Vec3 forward, Vec3 up);

    /*
     * @brief The angles that FromEuler turns back into this rotation. Pitch is in [-pi/2, pi/2].
     */
    Vec3 ToEuler() const;

    Mat4 ToMat4() const;

    Quat operator*(Quat other) const;
    bool operator==(Quat other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(Quat other) const { return !(*this == other); }

    /*
     * @brief Rotates a vector. The quaternion must be unit length.
     */
    Vec3 Rotate(Vec3 vector) const;

    float Dot(Quat other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

    /*
     * @brief Returns this rotation scaled to unit length, or the identity if it is too short to normalize.
     */
    Quat Normalize() const;

    /*
     * @brief The opposite rotation. The quaternion must be unit length.
     */
    Quat Inverse() const { return {-x, -y, -z, w}; }

    /*
     * @brief Spherical interpolation along the shortest path. The result is unit length.
     */
    Quat Slerp(Quat to, float t) const;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

} // namespace LiLib

#include "LiLib/Math/Quat.h"
#include "LiLib/Math/MathUtil.h"
#include <cmath>

namespace LiLib {

Quat Quat::FromAxisAngle(Vec3 axis, float angle) {
    const Vec3 unit = axis.Normalize();
    if(unit.LengthSquared() == 0.0f) {
        return {};
    }
    const float halfSin = std::sin(angle * 0.5f);
    return {unit.x * halfSin, unit.y * halfSin, unit.z * halfSin, std::cos(angle * 0.5f)};
}

Quat Quat::FromEuler(Vec3 angles) {
    const Quat pitch = FromAxisAngle({1.0f, 0.0f, 0.0f}, angles.x);
    const Quat yaw = FromAxisAngle({0.0f, 1.0f, 0.0f}, angles.y);
    const Quat roll = FromAxisAngle({0.0f, 0.0f, 1.0f}, angles.z);
    return yaw * pitch * roll;
}

Quat Quat::LookRotation(Vec3 forward, Vec3 up) {
    const Vec3 back = (-forward).Normalize();
    Vec3 right = up.Cross(back).Normalize();
    if(right.LengthSquared() == 0.0f) {
        // up is parallel to forward, so any perpendicular axis will do
        const Vec3 fallback = std::fabs(back.x) < 0.9f ? Vec3{1.0f, 0.0f, 0.0f} : Vec3{0.0f, 1.0f, 0.0f};
        right = fallback.Cross(back).Normalize();
    }
    const Vec3 trueUp = back.Cross(right);

    // Standard matrix-to-quaternion conversion, picking the largest component to stay numerically stable
    const float m00 = right.x;
    const float m11 = trueUp.y;
    const float m22 = back.z;
    const float trace = m00 + m11 + m22;
    Quat result;
    if(trace > 0.0f) {
        const float s = std::sqrt(trace + 1.0f) * 2.0f;
        result = {(trueUp.z - back.y) / s, (back.x - right.z) / s, (right.y - trueUp.x) / s, s * 0.25f};
    } else if(m00 > m11 && m00 > m22) {
        const float s = std::sqrt(1.0f + m00 - m11 - m22) * 2.0f;
        result = {s * 0.25f, (trueUp.x + right.y) / s, (back.x + right.z) / s, (trueUp.z - back.y) / s};
    } else if(m11 > m22) {
        const float s = std::sqrt(1.0f + m11 - m00 - m22) * 2.0f;
        result = {(trueUp.x + right.y) / s, s * 0.25f, (back.y + trueUp.z) / s, (back.x - right.z) / s};
    } else {
        const float s = std::sqrt(1.0f + m22 - m00 - m11) * 2.0f;
        result = {(back.x + right.z) / s, (back.y + trueUp.z) / s, s * 0.25f, (right.y - trueUp.x) / s};
    }
    return result.Normalize();
}

Vec3 Quat::ToEuler() const {
    // Read back from the rotation matrix of yaw * pitch * roll
    const float sinPitch = 2.0f * (w * x - y * z);
    const float rollSinCosPitch = 2.0f * (x * y + w * z);
    const float rollCosCosPitch = 1.0f - 2.0f * (x * x + z * z);
    const float cosPitch = std::sqrt(rollSinCosPitch * rollSinCosPitch + rollCosCosPitch * rollCosCosPitch);

    // atan2 rather than asin, which loses most of its precision near +-90 degrees
    const float pitch = std::atan2(sinPitch, cosPitch);
    if(cosPitch < 1e-3f) {
        // Gimbal lock: yaw and roll turn around the same axis, so all of it is reported as yaw
        const float yaw = std::atan2(-(2.0f * (x * z - w * y)), 1.0f - 2.0f * (y * y + z * z));
        return {pitch, yaw, 0.0f};
    }
    const float yaw = std::atan2(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y));
    const float roll = std::atan2(rollSinCosPitch, rollCosCosPitch);
    return {pitch, yaw, roll};
}

Mat4 Quat::ToMat4() const {
    Mat4 result;
    result.columns[0][0] = 1.0f - 2.0f * (y * y + z * z);
    result.columns[0][1] = 2.0f * (x * y + w * z);
    result.columns[0][2] = 2.0f * (x * z - w * y);
    result.columns[1][0] = 2.0f * (x * y - w * z);
    result.columns[1][1] = 1.0f - 2.0f * (x * x + z * z);
    result.columns[1][2] = 2.0f * (y * z + w * x);
    result.columns[2][0] = 2.0f * (x * z + w * y);
    result.columns[2][1] = 2.0f * (y * z - w * x);
    result.columns[2][2] = 1.0f - 2.0f * (x * x + y * y);
    return result;
}

Quat Quat::operator*(Quat other) const {
    return {w * other.x + x * other.w + y * other.z - z * other.y, w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w, w * other.w - x * other.x - y * other.y - z * other.z};
}

Vec3 Quat::Rotate(Vec3 vector) const {
    const Vec3 axis = {x, y, z};
    const Vec3 t = axis.Cross(vector) * 2.0f;
    return vector + t * w + axis.Cross(t);
}

Quat Quat::Normalize() const {
    const float length = std::sqrt(Dot(*this));
    if(length <= 1e-6f) {
        return {};
    }
    return {x / length, y / length, z / length, w / length};
}

Quat Quat::Slerp(Quat to, float t) const {
    float cosAngle = Dot(to);
    Quat target = to;
    if(cosAngle < 0.0f) {
        // q and -q are the same rotation; negate one to take the shorter way around
        cosAngle = -cosAngle;
        target = {-to.x, -to.y, -to.z, -to.w};
    }
    float fromWeight = 1.0f - t;
    float toWeight = t;
    if(cosAngle < 0.9995f) {
        const float angle = std::acos(cosAngle);
        const float inverseSin = 1.0f / std::sin(angle);
        fromWeight = std::sin((1.0f - t) * angle) * inverseSin;
        toWeight = std::sin(t * angle) * inverseSin;
    }
    const Quat blended = {x * fromWeight + target.x * toWeight, y * fromWeight + target.y * toWeight, z * fromWeight + target.z * toWeight,
                          w * fromWeight + target.w * toWeight};
    return blended.Normalize();
}

} // namespace LiLib

#include "LiLib/Math/Mat4.h"
#include <cmath>

namespace LiLib {

Mat4::Mat4() {
    columns[0][0] = 1.0f;
    columns[1][1] = 1.0f;
    columns[2][2] = 1.0f;
    columns[3][3] = 1.0f;
}

Mat4 Mat4::Identity() {
    return {};
}

Mat4 Mat4::Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Mat4 result;
    const float invWidth = 1.0f / (right - left);
    const float invHeight = 1.0f / (top - bottom);
    const float invDepth = -1.0f / (farPlane - nearPlane);

    // Right-handed, with clip-space z in [0, 1] (the Vulkan and Direct3D convention)
    result.columns[0][0] = 2.0f * invWidth;
    result.columns[1][1] = 2.0f * invHeight;
    result.columns[2][2] = invDepth;
    result.columns[3][0] = -(right + left) * invWidth;
    result.columns[3][1] = -(top + bottom) * invHeight;
    result.columns[3][2] = nearPlane * invDepth;
    return result;
}

Mat4 Mat4::Transform2D(Vec2 position, float rotation, Vec2 scale) {
    Mat4 result;
    const float c = std::cos(rotation);
    const float s = std::sin(rotation);

    result.columns[0][0] = c * scale.x;
    result.columns[0][1] = s * scale.x;
    result.columns[1][0] = -s * scale.y;
    result.columns[1][1] = c * scale.y;
    result.columns[3][0] = position.x;
    result.columns[3][1] = position.y;
    return result;
}

Mat4 Mat4::Translation(Vec3 offset) {
    Mat4 result;
    result.columns[3][0] = offset.x;
    result.columns[3][1] = offset.y;
    result.columns[3][2] = offset.z;
    return result;
}

Mat4 Mat4::Scaling(Vec3 scale) {
    Mat4 result;
    result.columns[0][0] = scale.x;
    result.columns[1][1] = scale.y;
    result.columns[2][2] = scale.z;
    return result;
}

Mat4 Mat4::RotationX(float angle) {
    Mat4 result;
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    result.columns[1][1] = c;
    result.columns[1][2] = s;
    result.columns[2][1] = -s;
    result.columns[2][2] = c;
    return result;
}

Mat4 Mat4::RotationY(float angle) {
    Mat4 result;
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    result.columns[0][0] = c;
    result.columns[0][2] = -s;
    result.columns[2][0] = s;
    result.columns[2][2] = c;
    return result;
}

Mat4 Mat4::RotationZ(float angle) {
    Mat4 result;
    const float c = std::cos(angle);
    const float s = std::sin(angle);

    result.columns[0][0] = c;
    result.columns[0][1] = s;
    result.columns[1][0] = -s;
    result.columns[1][1] = c;
    return result;
}

Mat4 Mat4::Rotation(Vec3 axis, float angle) {
    Mat4 result;
    const Vec3 n = axis.Normalize();
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    // Rodrigues' rotation formula
    result.columns[0][0] = t * n.x * n.x + c;
    result.columns[0][1] = t * n.x * n.y + s * n.z;
    result.columns[0][2] = t * n.x * n.z - s * n.y;
    result.columns[1][0] = t * n.x * n.y - s * n.z;
    result.columns[1][1] = t * n.y * n.y + c;
    result.columns[1][2] = t * n.y * n.z + s * n.x;
    result.columns[2][0] = t * n.x * n.z + s * n.y;
    result.columns[2][1] = t * n.y * n.z - s * n.x;
    result.columns[2][2] = t * n.z * n.z + c;
    return result;
}

Mat4 Mat4::Perspective(float fovY, float aspect, float nearPlane, float farPlane) {
    Mat4 result;
    const float focalLength = 1.0f / std::tan(fovY * 0.5f);
    const float invDepth = 1.0f / (nearPlane - farPlane);

    result.columns[0][0] = focalLength / aspect;
    result.columns[1][1] = focalLength;
    result.columns[2][2] = farPlane * invDepth;
    result.columns[2][3] = -1.0f;
    result.columns[3][2] = nearPlane * farPlane * invDepth;
    result.columns[3][3] = 0.0f;
    return result;
}

Mat4 Mat4::LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Mat4 result;
    const Vec3 forward = (target - eye).Normalize();
    const Vec3 right = forward.Cross(up).Normalize();
    const Vec3 cameraUp = right.Cross(forward);

    result.columns[0][0] = right.x;
    result.columns[1][0] = right.y;
    result.columns[2][0] = right.z;
    result.columns[0][1] = cameraUp.x;
    result.columns[1][1] = cameraUp.y;
    result.columns[2][1] = cameraUp.z;
    result.columns[0][2] = -forward.x;
    result.columns[1][2] = -forward.y;
    result.columns[2][2] = -forward.z;
    result.columns[3][0] = -right.Dot(eye);
    result.columns[3][1] = -cameraUp.Dot(eye);
    result.columns[3][2] = forward.Dot(eye);
    return result;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;
    for(int col = 0; col < 4; ++col) {
        for(int row = 0; row < 4; ++row) {
            result.columns[col][row] = columns[0][row] * other.columns[col][0] + columns[1][row] * other.columns[col][1] +
                                       columns[2][row] * other.columns[col][2] + columns[3][row] * other.columns[col][3];
        }
    }
    return result;
}

Mat4 Mat4::Inverse() const {
    // Cofactor expansion via 2x2 sub-determinants. inverse(transpose(M)) == transpose(inverse(M)),
    // so the same formula works whether m[i][j] is read as column-major or row-major.
    const float (*m)[4] = columns;

    const float s0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    const float s1 = m[0][0] * m[1][2] - m[0][2] * m[1][0];
    const float s2 = m[0][0] * m[1][3] - m[0][3] * m[1][0];
    const float s3 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    const float s4 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
    const float s5 = m[0][2] * m[1][3] - m[0][3] * m[1][2];
    const float c0 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    const float c1 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    const float c2 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    const float c3 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    const float c4 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    const float c5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];

    const float invDet = 1.0f / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

    Mat4 result;
    float (*r)[4] = result.columns;
    r[0][0] = (m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * invDet;
    r[0][1] = (m[0][2] * c4 - m[0][1] * c5 - m[0][3] * c3) * invDet;
    r[0][2] = (m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * invDet;
    r[0][3] = (m[2][2] * s4 - m[2][1] * s5 - m[2][3] * s3) * invDet;
    r[1][0] = (m[1][2] * c2 - m[1][0] * c5 - m[1][3] * c1) * invDet;
    r[1][1] = (m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * invDet;
    r[1][2] = (m[3][2] * s2 - m[3][0] * s5 - m[3][3] * s1) * invDet;
    r[1][3] = (m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * invDet;
    r[2][0] = (m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * invDet;
    r[2][1] = (m[0][1] * c2 - m[0][0] * c4 - m[0][3] * c0) * invDet;
    r[2][2] = (m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * invDet;
    r[2][3] = (m[2][1] * s2 - m[2][0] * s4 - m[2][3] * s0) * invDet;
    r[3][0] = (m[1][1] * c1 - m[1][0] * c3 - m[1][2] * c0) * invDet;
    r[3][1] = (m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * invDet;
    r[3][2] = (m[3][1] * s1 - m[3][0] * s3 - m[3][2] * s0) * invDet;
    r[3][3] = (m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * invDet;
    return result;
}

Vec2 Mat4::TransformPoint(Vec2 point) const {
    return {columns[0][0] * point.x + columns[1][0] * point.y + columns[3][0],
            columns[0][1] * point.x + columns[1][1] * point.y + columns[3][1]};
}

Vec3 Mat4::TransformPoint3D(Vec3 point) const {
    const float invW = 1.0f / (columns[0][3] * point.x + columns[1][3] * point.y + columns[2][3] * point.z + columns[3][3]);
    return {(columns[0][0] * point.x + columns[1][0] * point.y + columns[2][0] * point.z + columns[3][0]) * invW,
            (columns[0][1] * point.x + columns[1][1] * point.y + columns[2][1] * point.z + columns[3][1]) * invW,
            (columns[0][2] * point.x + columns[1][2] * point.y + columns[2][2] * point.z + columns[3][2]) * invW};
}

Vec3 Mat4::TransformDirection(Vec3 direction) const {
    return {columns[0][0] * direction.x + columns[1][0] * direction.y + columns[2][0] * direction.z,
            columns[0][1] * direction.x + columns[1][1] * direction.y + columns[2][1] * direction.z,
            columns[0][2] * direction.x + columns[1][2] * direction.y + columns[2][2] * direction.z};
}

Mat4 Mat4::Transpose() const {
    Mat4 result;
    for(int col = 0; col < 4; ++col) {
        for(int row = 0; row < 4; ++row) {
            result.columns[col][row] = columns[row][col];
        }
    }
    return result;
}

} // namespace LiLib

#pragma once
#include "LiLib/Math/Vec2.h"
#include "LiLib/Math/Vec3.h"

namespace LiLib {

/*
 * @brief Column-major 4x4 matrix, laid out so it can be uploaded to shaders directly.
 *        Uses a right-handed coordinate system. Projections produce clip space with z in [0, 1] and y pointing up.
 *        Angles are in radians, and positive rotations are counter-clockwise when looking down the axis toward the origin.
 */
class Mat4 {
public:
    Mat4();

    static Mat4 Identity();

    /*
     * @brief Orthographic projection for the given view bounds and depth range.
     */
    static Mat4 Ortho(float left, float right, float bottom, float top, float nearPlane = -1.0f, float farPlane = 1.0f);

    /*
     * @brief Builds translate * rotate(z) * scale for a 2D transform.
     */
    static Mat4 Transform2D(Vec2 position, float rotation, Vec2 scale);

    static Mat4 Translation(Vec3 offset);
    static Mat4 Scaling(Vec3 scale);
    static Mat4 RotationX(float angle);
    static Mat4 RotationY(float angle);
    static Mat4 RotationZ(float angle);

    /*
     * @brief Rotation around an arbitrary axis. The axis does not need to be unit length, but must not be zero.
     */
    static Mat4 Rotation(Vec3 axis, float angle);

    /*
     * @brief Perspective projection. The camera looks down -z; nearPlane maps to z = 0 and farPlane to z = 1.
     * @param fovY Full vertical field of view in radians.
     * @param aspect Viewport width divided by height.
     * @param nearPlane Distance to the near plane. Must be greater than zero.
     * @param farPlane Distance to the far plane. Must be greater than nearPlane.
     */
    static Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane);

    /*
     * @brief View matrix for a camera at eye looking toward target. The result is degenerate if
     *        eye equals target or if up is parallel to the view direction.
     */
    static Mat4 LookAt(Vec3 eye, Vec3 target, Vec3 up);

    Mat4 operator*(const Mat4& other) const;

    /*
     * @brief Returns the inverse. A singular matrix has none, and the result contains infinities or NaNs.
     */
    Mat4 Inverse() const;
    Mat4 Transpose() const;

    /*
     * @brief Transforms a 2D point as (x, y, 0, 1) and returns the resulting x and y.
     */
    Vec2 TransformPoint(Vec2 point) const;

    /*
     * @brief Transforms a 3D point as (x, y, z, 1) and divides the result by w, so it also works with
     *        projections. A point with w = 0 (in the camera's plane) produces infinities or NaNs.
     */
    Vec3 TransformPoint3D(Vec3 point) const;

    /*
     * @brief Transforms a direction as (x, y, z, 0), which ignores translation. The result is not normalized.
     *        To transform normals under non-uniform scale, use Inverse().Transpose().TransformDirection(normal).
     */
    Vec3 TransformDirection(Vec3 direction) const;

    float* Data() { return &columns[0][0]; }
    const float* Data() const { return &columns[0][0]; }

    alignas(16) float columns[4][4] = {};
};

} // namespace LiLib

#include "LiLib/Math/AABB.h"
#include "LiLib/Math/Frustum.h"
#include "LiLib/Math/MathUtil.h"
#include "LiLib/Math/Quat.h"
#include "LiLib/Math/Vec4.h"
#include "TestFramework.h"

using namespace LiLib;
using LiLibTest::Near;

namespace {

bool NearVec(Vec3 a, Vec3 b, float epsilon = 1e-4f) {
    return Near(a.x, b.x, epsilon) && Near(a.y, b.y, epsilon) && Near(a.z, b.z, epsilon);
}

// q and -q are the same rotation, so compare what they do instead of their components
bool SameRotation(Quat a, Quat b) {
    return NearVec(a.Rotate({1.0f, 0.0f, 0.0f}), b.Rotate({1.0f, 0.0f, 0.0f})) &&
           NearVec(a.Rotate({0.0f, 1.0f, 0.0f}), b.Rotate({0.0f, 1.0f, 0.0f}));
}

} // namespace

// Vec4

LI_TEST(Vec4, ArithmeticAndDot) {
    const Vec4 a = {1.0f, 2.0f, 3.0f, 4.0f};
    const Vec4 b = {4.0f, 3.0f, 2.0f, 1.0f};
    LI_CHECK(a + b == (Vec4{5.0f, 5.0f, 5.0f, 5.0f}));
    LI_CHECK(a * 2.0f == (Vec4{2.0f, 4.0f, 6.0f, 8.0f}));
    LI_CHECK(Near(a.Dot(b), 20.0f));
    LI_CHECK(a.XYZ() == (Vec3{1.0f, 2.0f, 3.0f}));
}

// Quat

LI_TEST(Quat, DefaultIsIdentity) {
    const Vec3 v = {1.0f, 2.0f, 3.0f};
    LI_CHECK(NearVec(Quat().Rotate(v), v));
    LI_CHECK(Quat() == Quat::Identity());
}

LI_TEST(Quat, ZeroAxisGivesIdentity) {
    LI_CHECK(Quat::FromAxisAngle({0.0f, 0.0f, 0.0f}, 1.0f) == Quat::Identity());
}

LI_TEST(Quat, RotateMatchesMat4) {
    const Vec3 axis = {1.0f, 2.0f, -0.5f};
    const float angle = 0.8f;
    const Quat rotation = Quat::FromAxisAngle(axis, angle);
    const Mat4 matrix = Mat4::Rotation(axis, angle);
    const Vec3 v = {0.3f, -1.2f, 2.0f};
    LI_CHECK(NearVec(rotation.Rotate(v), matrix.TransformDirection(v)));
    LI_CHECK(NearVec(rotation.ToMat4().TransformDirection(v), matrix.TransformDirection(v)));
}

LI_TEST(Quat, CompositionAppliesRightFirst) {
    const Quat first = Quat::FromAxisAngle({0.0f, 1.0f, 0.0f}, Math::Pi * 0.5f);
    const Quat second = Quat::FromAxisAngle({1.0f, 0.0f, 0.0f}, Math::Pi * 0.5f);
    const Vec3 v = {1.0f, 0.0f, 0.0f};
    LI_CHECK(NearVec((second * first).Rotate(v), second.Rotate(first.Rotate(v))));
}

LI_TEST(Quat, InverseUndoesRotation) {
    const Quat rotation = Quat::FromAxisAngle({1.0f, 1.0f, 0.0f}, 1.1f);
    const Vec3 v = {0.5f, 2.0f, -1.0f};
    LI_CHECK(NearVec(rotation.Inverse().Rotate(rotation.Rotate(v)), v));
}

LI_TEST(Quat, EulerRoundTrip) {
    const Vec3 angles[] = {{0.3f, 0.5f, -0.2f}, {-1.2f, 2.5f, 1.0f}, {0.0f, -3.0f, 0.0f}, {1.5f, 0.0f, 0.0f}};
    for(const Vec3 input : angles) {
        const Quat rotation = Quat::FromEuler(input);
        LI_CHECK(SameRotation(Quat::FromEuler(rotation.ToEuler()), rotation));
    }
}

LI_TEST(Quat, EulerRoundTripAtGimbalLock) {
    const Quat lookingUp = Quat::FromEuler({Math::Pi * 0.5f, 0.7f, 0.0f});
    LI_CHECK(SameRotation(Quat::FromEuler(lookingUp.ToEuler()), lookingUp));
    const Quat lookingDown = Quat::FromEuler({-Math::Pi * 0.5f, -1.1f, 0.0f});
    LI_CHECK(SameRotation(Quat::FromEuler(lookingDown.ToEuler()), lookingDown));
}

LI_TEST(Quat, LookRotationPointsMinusZAlongForward) {
    const Vec3 directions[] = {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {-1.0f, -2.0f, 0.5f}};
    for(const Vec3 direction : directions) {
        const Quat rotation = Quat::LookRotation(direction, {0.0f, 1.0f, 0.0f});
        LI_CHECK(NearVec(rotation.Rotate({0.0f, 0.0f, -1.0f}), direction.Normalize()));
    }
}

LI_TEST(Quat, LookRotationHandlesParallelUp) {
    const Quat rotation = Quat::LookRotation({0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    LI_CHECK(NearVec(rotation.Rotate({0.0f, 0.0f, -1.0f}), {0.0f, 1.0f, 0.0f}));
}

LI_TEST(Quat, SlerpEndpointsAndMiddle) {
    const Quat from = Quat::Identity();
    const Quat to = Quat::FromAxisAngle({0.0f, 1.0f, 0.0f}, Math::Pi * 0.5f);
    LI_CHECK(SameRotation(from.Slerp(to, 0.0f), from));
    LI_CHECK(SameRotation(from.Slerp(to, 1.0f), to));
    LI_CHECK(SameRotation(from.Slerp(to, 0.5f), Quat::FromAxisAngle({0.0f, 1.0f, 0.0f}, Math::Pi * 0.25f)));
}

LI_TEST(Quat, SlerpTakesShortestPath) {
    const Quat from = Quat::Identity();
    const Quat to = Quat::FromAxisAngle({0.0f, 1.0f, 0.0f}, 0.2f);
    const Quat negated = {-to.x, -to.y, -to.z, -to.w};
    LI_CHECK(SameRotation(from.Slerp(negated, 0.5f), Quat::FromAxisAngle({0.0f, 1.0f, 0.0f}, 0.1f)));
}

LI_TEST(Quat, NormalizeHandlesZero) {
    const Quat zero = {0.0f, 0.0f, 0.0f, 0.0f};
    LI_CHECK(zero.Normalize() == Quat::Identity());
}

// AABB

LI_TEST(AABB, DefaultIsEmpty) {
    const AABB box;
    LI_CHECK(box.IsEmpty());
    LI_CHECK(!box.Contains({0.0f, 0.0f, 0.0f}));
    LI_CHECK(!box.Intersects(box));
}

LI_TEST(AABB, ExpandAndContains) {
    AABB box;
    box.Expand(Vec3{1.0f, 2.0f, 3.0f});
    box.Expand(Vec3{-1.0f, 0.0f, 5.0f});
    LI_CHECK(!box.IsEmpty());
    LI_CHECK(box.min == (Vec3{-1.0f, 0.0f, 3.0f}));
    LI_CHECK(box.max == (Vec3{1.0f, 2.0f, 5.0f}));
    LI_CHECK(box.Center() == (Vec3{0.0f, 1.0f, 4.0f}));
    LI_CHECK(box.Extents() == (Vec3{1.0f, 1.0f, 1.0f}));
    LI_CHECK(box.Contains({0.0f, 1.0f, 4.0f}));
    LI_CHECK(!box.Contains({2.0f, 1.0f, 4.0f}));
}

LI_TEST(AABB, ExpandByEmptyBoxChangesNothing) {
    AABB box;
    box.Expand(Vec3{1.0f, 1.0f, 1.0f});
    box.Expand(AABB{});
    LI_CHECK(box.min == (Vec3{1.0f, 1.0f, 1.0f}));
    LI_CHECK(box.max == (Vec3{1.0f, 1.0f, 1.0f}));
}

LI_TEST(AABB, Intersects) {
    const AABB a = {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}};
    const AABB overlapping = {{0.5f, 0.5f, 0.5f}, {2.0f, 2.0f, 2.0f}};
    const AABB apart = {{2.0f, 0.0f, 0.0f}, {3.0f, 1.0f, 1.0f}};
    LI_CHECK(a.Intersects(overlapping));
    LI_CHECK(!a.Intersects(apart));
}

LI_TEST(AABB, TransformedRotatesAndMoves) {
    const AABB box = {{-1.0f, -1.0f, -2.0f}, {1.0f, 1.0f, 2.0f}};
    const Mat4 transform = Mat4::Translation({10.0f, 0.0f, 0.0f}) * Mat4::RotationY(Math::Pi * 0.5f);
    const AABB result = box.Transformed(transform);
    LI_CHECK(NearVec(result.min, {8.0f, -1.0f, -1.0f}));
    LI_CHECK(NearVec(result.max, {12.0f, 1.0f, 1.0f}));
    LI_CHECK(AABB{}.Transformed(transform).IsEmpty());
}

// Frustum

LI_TEST(Frustum, AABBInsideAndOutside) {
    const Mat4 viewProjection = Mat4::Perspective(Math::Pi * 0.5f, 1.0f, 1.0f, 100.0f);
    const Frustum frustum = Frustum::FromMatrix(viewProjection);
    const AABB ahead = {{-1.0f, -1.0f, -11.0f}, {1.0f, 1.0f, -9.0f}};
    const AABB behind = {{-1.0f, -1.0f, 9.0f}, {1.0f, 1.0f, 11.0f}};
    const AABB tooFar = {{-1.0f, -1.0f, -202.0f}, {1.0f, 1.0f, -200.0f}};
    const AABB toTheSide = {{50.0f, -1.0f, -11.0f}, {52.0f, 1.0f, -9.0f}};
    const AABB nearerThanNearPlane = {{-0.1f, -0.1f, -0.5f}, {0.1f, 0.1f, -0.2f}};
    LI_CHECK(frustum.Intersects(ahead));
    LI_CHECK(!frustum.Intersects(behind));
    LI_CHECK(!frustum.Intersects(tooFar));
    LI_CHECK(!frustum.Intersects(toTheSide));
    LI_CHECK(!frustum.Intersects(nearerThanNearPlane));
    LI_CHECK(!frustum.Intersects(AABB{}));
}

LI_TEST(Frustum, FollowsTheView) {
    const Mat4 projection = Mat4::Perspective(Math::Pi * 0.5f, 1.0f, 1.0f, 100.0f);
    const Mat4 view = Mat4::LookAt({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    const Frustum frustum = Frustum::FromMatrix(projection * view);
    LI_CHECK(frustum.IntersectsSphere({10.0f, 0.0f, 0.0f}, 1.0f));
    LI_CHECK(!frustum.IntersectsSphere({-10.0f, 0.0f, 0.0f}, 1.0f));
}

LI_TEST(Frustum, SphereTouchingPlaneCounts) {
    const Frustum frustum = Frustum::FromMatrix(Mat4::Perspective(Math::Pi * 0.5f, 1.0f, 1.0f, 100.0f));
    LI_CHECK(frustum.IntersectsSphere({0.0f, 0.0f, 0.5f}, 2.0f));
    LI_CHECK(!frustum.IntersectsSphere({0.0f, 0.0f, 5.0f}, 2.0f));
}

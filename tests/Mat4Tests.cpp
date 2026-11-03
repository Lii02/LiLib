#include "LiLib/Math/Mat4.h"
#include "LiLib/Math/MathUtil.h"
#include "TestFramework.h"
#include <cmath>
#include <type_traits>

using namespace LiLib;
using LiLibTest::Near;

namespace {

static_assert(alignof(Mat4) == 16);
static_assert(sizeof(Mat4) == 64);

const float HalfPi = Math::Pi / 2.0f;

bool NearVec(Vec3 a, Vec3 b, float tolerance = 1e-4f) {
    return Near(a.x, b.x, tolerance) && Near(a.y, b.y, tolerance) && Near(a.z, b.z, tolerance);
}

bool NearVec(Vec2 a, Vec2 b, float tolerance = 1e-4f) {
    return Near(a.x, b.x, tolerance) && Near(a.y, b.y, tolerance);
}

bool NearMat(const Mat4& a, const Mat4& b, float tolerance = 1e-4f) {
    for(int i = 0; i < 16; ++i) {
        if(!Near(a.Data()[i], b.Data()[i], tolerance)) {
            return false;
        }
    }
    return true;
}

Mat4 MakeGeneral() {
    Mat4 general;
    const float values[16] = {2, 0, 1, 3, 1, 4, 0, 2, 0, 1, 5, 1, 3, 2, 1, 6};
    for(int i = 0; i < 16; ++i) {
        general.Data()[i] = values[i];
    }
    return general;
}

} // namespace

LI_TEST(Mat4, DefaultIsIdentity) {
    const Mat4 matrix;
    for(int column = 0; column < 4; ++column) {
        for(int row = 0; row < 4; ++row) {
            LI_CHECK(matrix.columns[column][row] == (column == row ? 1.0f : 0.0f));
        }
    }
    LI_CHECK(NearMat(Mat4::Identity(), matrix));
}

LI_TEST(Mat4, DataIsColumnMajor) {
    const Mat4 matrix = Mat4::Translation({1.0f, 2.0f, 3.0f});
    LI_CHECK(matrix.Data() == &matrix.columns[0][0]);
    LI_CHECK(matrix.Data()[12] == 1.0f);
    LI_CHECK(matrix.Data()[13] == 2.0f);
    LI_CHECK(matrix.Data()[14] == 3.0f);
}

LI_TEST(Mat4, MultiplyingByIdentityChangesNothing) {
    const Mat4 general = MakeGeneral();
    LI_CHECK(NearMat(general * Mat4::Identity(), general));
    LI_CHECK(NearMat(Mat4::Identity() * general, general));
}

LI_TEST(Mat4, MultiplicationMatchesAHandCalculatedResult) {
    Mat4 a;
    Mat4 b;
    for(int i = 0; i < 16; ++i) {
        a.Data()[i] = static_cast<float>(i + 1);
        b.Data()[i] = static_cast<float>(i + 17);
    }
    // Column-major a * b: element (row r, column c) = sum over k of a(r, k) * b(k, c)
    const Mat4 result = a * b;
    LI_CHECK(result.columns[0][0] == 538.0f);
    LI_CHECK(result.columns[0][1] == 612.0f);
    LI_CHECK(result.columns[1][0] == 650.0f);
    LI_CHECK(result.columns[3][3] == 1240.0f);
}

LI_TEST(Mat4, Translation) {
    const Mat4 matrix = Mat4::Translation({1.0f, 2.0f, 3.0f});
    LI_CHECK(NearVec(matrix.TransformPoint3D({1.0f, 1.0f, 1.0f}), Vec3{2.0f, 3.0f, 4.0f}));
    LI_CHECK(NearVec(matrix.TransformDirection({1.0f, 1.0f, 1.0f}), Vec3{1.0f, 1.0f, 1.0f}));
}

LI_TEST(Mat4, Scaling) {
    const Mat4 matrix = Mat4::Scaling({2.0f, 3.0f, 4.0f});
    LI_CHECK(NearVec(matrix.TransformPoint3D({1.0f, 1.0f, 1.0f}), Vec3{2.0f, 3.0f, 4.0f}));
    LI_CHECK(NearVec(matrix.TransformDirection({1.0f, 1.0f, 1.0f}), Vec3{2.0f, 3.0f, 4.0f}));
}

LI_TEST(Mat4, SingleAxisRotationsAreRightHanded) {
    LI_CHECK(NearVec(Mat4::RotationX(HalfPi).TransformPoint3D({0.0f, 1.0f, 0.0f}), Vec3{0.0f, 0.0f, 1.0f}));
    LI_CHECK(NearVec(Mat4::RotationY(HalfPi).TransformPoint3D({0.0f, 0.0f, 1.0f}), Vec3{1.0f, 0.0f, 0.0f}));
    LI_CHECK(NearVec(Mat4::RotationZ(HalfPi).TransformPoint3D({1.0f, 0.0f, 0.0f}), Vec3{0.0f, 1.0f, 0.0f}));
}

LI_TEST(Mat4, RotationAroundAnAxisMatchesSingleAxisVersions) {
    LI_CHECK(NearMat(Mat4::Rotation({1.0f, 0.0f, 0.0f}, 0.7f), Mat4::RotationX(0.7f)));
    LI_CHECK(NearMat(Mat4::Rotation({0.0f, 1.0f, 0.0f}, 0.7f), Mat4::RotationY(0.7f)));
    LI_CHECK(NearMat(Mat4::Rotation({0.0f, 0.0f, 1.0f}, 0.7f), Mat4::RotationZ(0.7f)));
}

LI_TEST(Mat4, RotationNormalizesItsAxis) {
    LI_CHECK(NearMat(Mat4::Rotation({0.0f, 0.0f, 9.0f}, 0.7f), Mat4::RotationZ(0.7f)));
}

LI_TEST(Mat4, RotationAroundADiagonalCyclesTheAxes) {
    const Mat4 matrix = Mat4::Rotation({1.0f, 1.0f, 1.0f}, Math::TwoPi / 3.0f);
    LI_CHECK(NearVec(matrix.TransformPoint3D({1.0f, 0.0f, 0.0f}), Vec3{0.0f, 1.0f, 0.0f}));
    LI_CHECK(NearVec(matrix.TransformPoint3D({0.0f, 1.0f, 0.0f}), Vec3{0.0f, 0.0f, 1.0f}));
}

LI_TEST(Mat4, RotationsAreOrthonormal) {
    const Mat4 matrix = Mat4::Rotation({1.0f, 2.0f, -3.0f}, 1.1f);
    LI_CHECK(NearMat(matrix * matrix.Transpose(), Mat4::Identity()));
}

LI_TEST(Mat4, TransformsApplyRightToLeft) {
    const Mat4 matrix = Mat4::Translation({10.0f, 0.0f, 0.0f}) * Mat4::RotationZ(HalfPi) * Mat4::Scaling({2.0f, 2.0f, 2.0f});
    LI_CHECK(NearVec(matrix.TransformPoint3D({1.0f, 0.0f, 0.0f}), Vec3{10.0f, 2.0f, 0.0f}));
}

LI_TEST(Mat4, Transform2DMatchesTheEquivalent3DChain) {
    const Mat4 chained = Mat4::Translation({3.0f, 4.0f, 0.0f}) * Mat4::RotationZ(0.6f) * Mat4::Scaling({2.0f, 5.0f, 1.0f});
    LI_CHECK(NearMat(chained, Mat4::Transform2D({3.0f, 4.0f}, 0.6f, {2.0f, 5.0f})));
}

LI_TEST(Mat4, TransformPoint2D) {
    const Mat4 matrix = Mat4::Transform2D({10.0f, 20.0f}, HalfPi, {2.0f, 2.0f});
    LI_CHECK(NearVec(matrix.TransformPoint({1.0f, 0.0f}), Vec2{10.0f, 22.0f}));
}

LI_TEST(Mat4, OrthoMapsTheViewBoundsToClipSpace) {
    const Mat4 ortho = Mat4::Ortho(0.0f, 800.0f, 0.0f, 600.0f);
    LI_CHECK(NearVec(ortho.TransformPoint({800.0f, 600.0f}), Vec2{1.0f, 1.0f}));
    LI_CHECK(NearVec(ortho.TransformPoint({0.0f, 0.0f}), Vec2{-1.0f, -1.0f}));
    LI_CHECK(NearVec(ortho.TransformPoint({400.0f, 300.0f}), Vec2{0.0f, 0.0f}));
}

LI_TEST(Mat4, OrthoMapsNearAndFarToZeroAndOne) {
    const Mat4 ortho = Mat4::Ortho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 11.0f);
    LI_CHECK(Near(ortho.TransformPoint3D({0.0f, 0.0f, -1.0f}).z, 0.0f));
    LI_CHECK(Near(ortho.TransformPoint3D({0.0f, 0.0f, -11.0f}).z, 1.0f));
}

LI_TEST(Mat4, PerspectiveMapsNearAndFarToZeroAndOne) {
    const Mat4 projection = Mat4::Perspective(HalfPi, 2.0f, 1.0f, 100.0f);
    LI_CHECK(Near(projection.TransformPoint3D({0.0f, 0.0f, -1.0f}).z, 0.0f));
    LI_CHECK(Near(projection.TransformPoint3D({0.0f, 0.0f, -100.0f}).z, 1.0f));
    const float middle = projection.TransformPoint3D({0.0f, 0.0f, -10.0f}).z;
    LI_CHECK(middle > 0.0f && middle < 1.0f);
}

LI_TEST(Mat4, PerspectiveMapsTheFrustumEdgesToClipSpace) {
    const Mat4 projection = Mat4::Perspective(HalfPi, 2.0f, 1.0f, 100.0f);
    // A 90 degree vertical field of view reaches y = distance, and x = distance * aspect
    LI_CHECK(Near(projection.TransformPoint3D({0.0f, 5.0f, -5.0f}).y, 1.0f));
    LI_CHECK(Near(projection.TransformPoint3D({0.0f, -5.0f, -5.0f}).y, -1.0f));
    LI_CHECK(Near(projection.TransformPoint3D({10.0f, 0.0f, -5.0f}).x, 1.0f));
}

LI_TEST(Mat4, LookAtDownNegativeZIsATranslation) {
    const Mat4 view = Mat4::LookAt({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    LI_CHECK(NearMat(view, Mat4::Translation({0.0f, 0.0f, -5.0f})));
}

LI_TEST(Mat4, LookAtPlacesTheEyeAndTarget) {
    const Vec3 eye{3.0f, 4.0f, -2.0f};
    const Vec3 target{-1.0f, 0.5f, 6.0f};
    const Mat4 view = Mat4::LookAt(eye, target, {0.0f, 1.0f, 0.0f});
    LI_CHECK(NearVec(view.TransformPoint3D(eye), Vec3{0.0f, 0.0f, 0.0f}));
    LI_CHECK(NearVec(view.TransformPoint3D(target), Vec3{0.0f, 0.0f, -eye.Distance(target)}));
}

LI_TEST(Mat4, LookAtKeepsUpAndIsRigid) {
    const Mat4 view = Mat4::LookAt({3.0f, 4.0f, -2.0f}, {-1.0f, 0.5f, 6.0f}, {0.0f, 1.0f, 0.0f});
    const Vec3 up = view.TransformDirection({0.0f, 1.0f, 0.0f});
    LI_CHECK(up.y > 0.0f);
    LI_CHECK(Near(up.x, 0.0f));
    LI_CHECK(Near(up.Length(), 1.0f));
}

LI_TEST(Mat4, InverseOfAffineAndGeneralMatrices) {
    const Mat4 affine = Mat4::Transform2D({3.0f, -2.0f}, 0.7f, {2.0f, 0.5f}) * Mat4::Ortho(0.0f, 800.0f, 600.0f, 0.0f);
    LI_CHECK(NearMat(affine * affine.Inverse(), Mat4::Identity()));

    const Mat4 general = MakeGeneral();
    LI_CHECK(NearMat(general * general.Inverse(), Mat4::Identity()));
    LI_CHECK(NearMat(general.Inverse() * general, Mat4::Identity()));
}

LI_TEST(Mat4, InverseOfASingularMatrixIsNotFinite) {
    const Mat4 singular = Mat4::Scaling({0.0f, 1.0f, 1.0f});
    LI_CHECK(!std::isfinite(singular.Inverse().columns[0][0]));
}

LI_TEST(Mat4, ModelViewProjectionRoundTrip) {
    const Mat4 model = Mat4::Translation({1.0f, 0.0f, 0.0f}) * Mat4::Rotation({0.0f, 1.0f, 0.0f}, 0.5f) * Mat4::Scaling({2.0f, 2.0f, 2.0f});
    const Mat4 view = Mat4::LookAt({0.0f, 2.0f, 5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    const Mat4 mvp = Mat4::Perspective(1.0f, 1.5f, 1.0f, 50.0f) * view * model;
    LI_CHECK(NearMat(mvp * mvp.Inverse(), Mat4::Identity(), 1e-3f));

    const Vec3 point{0.25f, -0.5f, 0.5f};
    const Vec3 clip = mvp.TransformPoint3D(point);
    LI_CHECK(NearVec(mvp.Inverse().TransformPoint3D(clip), point, 1e-3f));
}

LI_TEST(Mat4, Transpose) {
    const Mat4 general = MakeGeneral();
    const Mat4 transposed = general.Transpose();
    for(int column = 0; column < 4; ++column) {
        for(int row = 0; row < 4; ++row) {
            LI_CHECK(transposed.columns[column][row] == general.columns[row][column]);
        }
    }
    LI_CHECK(NearMat(transposed.Transpose(), general));
}

LI_TEST(Mat4, NormalsStayPerpendicularUnderNonUniformScale) {
    const Mat4 squash = Mat4::Scaling({1.0f, 4.0f, 1.0f});
    const Vec3 tangent = squash.TransformDirection({1.0f, -1.0f, 0.0f});
    const Vec3 normal = squash.Inverse().Transpose().TransformDirection({1.0f, 1.0f, 0.0f});
    LI_CHECK(Near(tangent.Dot(normal), 0.0f));
}

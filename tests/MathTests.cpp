#include "LiLib/Math/Color.h"
#include "LiLib/Math/MathUtil.h"
#include "LiLib/Math/Random.h"
#include "LiLib/Math/Rect.h"
#include "LiLib/Math/Vec2.h"
#include "LiLib/Math/Vec3.h"
#include "TestFramework.h"

using namespace LiLib;
using LiLibTest::Near;

namespace {

constexpr Color CompileTimeColor = Colors::White;

} // namespace

// Math

LI_TEST(Math, Constants) {
    LI_CHECK(Near(Math::TwoPi, Math::Pi * 2.0f));
    LI_CHECK(Near(180.0f * Math::DegToRad, Math::Pi));
    LI_CHECK(Near(Math::Pi * Math::RadToDeg, 180.0f, 1e-3f));
}

LI_TEST(Math, Lerp) {
    LI_CHECK(Math::Lerp(2.0f, 4.0f, 0.0f) == 2.0f);
    LI_CHECK(Math::Lerp(2.0f, 4.0f, 1.0f) == 4.0f);
    LI_CHECK(Near(Math::Lerp(2.0f, 4.0f, 0.5f), 3.0f));
}

LI_TEST(Math, Saturate) {
    LI_CHECK(Math::Saturate(-1.0f) == 0.0f);
    LI_CHECK(Math::Saturate(0.25f) == 0.25f);
    LI_CHECK(Math::Saturate(2.0f) == 1.0f);
}

// Vec2

LI_TEST(Vec2, Arithmetic) {
    const Vec2 a{1.0f, 2.0f};
    const Vec2 b{3.0f, 4.0f};
    LI_CHECK((a + b == Vec2{4.0f, 6.0f}));
    LI_CHECK((b - a == Vec2{2.0f, 2.0f}));
    LI_CHECK((-a == Vec2{-1.0f, -2.0f}));
    LI_CHECK((a * b == Vec2{3.0f, 8.0f}));
    LI_CHECK((a * 2.0f == Vec2{2.0f, 4.0f}));
    LI_CHECK((2.0f * a == Vec2{2.0f, 4.0f}));
    LI_CHECK((b / 2.0f == Vec2{1.5f, 2.0f}));
    LI_CHECK((b / a == Vec2{3.0f, 2.0f}));
}

LI_TEST(Vec2, InPlaceOperators) {
    Vec2 value{1.0f, 2.0f};
    value += Vec2{1.0f, 1.0f};
    LI_CHECK((value == Vec2{2.0f, 3.0f}));
    value -= Vec2{1.0f, 2.0f};
    LI_CHECK((value == Vec2{1.0f, 1.0f}));
    value *= 3.0f;
    LI_CHECK((value == Vec2{3.0f, 3.0f}));
}

LI_TEST(Vec2, Equality) {
    LI_CHECK((Vec2{1.0f, 2.0f} == Vec2{1.0f, 2.0f}));
    LI_CHECK((Vec2{1.0f, 2.0f} != Vec2{1.0f, 3.0f}));
    LI_CHECK((Vec2{1.0f, 2.0f} != Vec2{0.0f, 2.0f}));
}

LI_TEST(Vec2, Measurements) {
    const Vec2 value{3.0f, 4.0f};
    LI_CHECK(value.Length() == 5.0f);
    LI_CHECK(value.LengthSquared() == 25.0f);
    LI_CHECK(value.Distance({0.0f, 0.0f}) == 5.0f);
    LI_CHECK(value.Dot({2.0f, 1.0f}) == 10.0f);
    LI_CHECK((Vec2{1.0f, 0.0f}).Cross({0.0f, 1.0f}) == 1.0f);
    LI_CHECK((Vec2{0.0f, 1.0f}).Cross({1.0f, 0.0f}) == -1.0f);
}

LI_TEST(Vec2, Normalize) {
    const Vec2 unit = Vec2{3.0f, 4.0f}.Normalize();
    LI_CHECK(Near(unit.Length(), 1.0f));
    LI_CHECK(Near(unit.x, 0.6f));
    LI_CHECK((Vec2{0.0f, 0.0f}.Normalize() == Vec2{0.0f, 0.0f}));
    LI_CHECK((Vec2{1e-7f, 0.0f}.Normalize() == Vec2{0.0f, 0.0f}));
}

LI_TEST(Vec2, RotateIsCounterClockwise) {
    const Vec2 rotated = Vec2{1.0f, 0.0f}.Rotate(Math::Pi / 2.0f);
    LI_CHECK(Near(rotated.x, 0.0f));
    LI_CHECK(Near(rotated.y, 1.0f));
}

LI_TEST(Vec2, Lerp) {
    LI_CHECK((Vec2{0.0f, 2.0f}.Lerp({4.0f, 4.0f}, 0.5f) == Vec2{2.0f, 3.0f}));
}

// Vec3

LI_TEST(Vec3, Arithmetic) {
    const Vec3 a{1.0f, 2.0f, 3.0f};
    const Vec3 b{4.0f, 6.0f, 9.0f};
    LI_CHECK((a + b == Vec3{5.0f, 8.0f, 12.0f}));
    LI_CHECK((b - a == Vec3{3.0f, 4.0f, 6.0f}));
    LI_CHECK((-a == Vec3{-1.0f, -2.0f, -3.0f}));
    LI_CHECK((a * b == Vec3{4.0f, 12.0f, 27.0f}));
    LI_CHECK((a * 2.0f == Vec3{2.0f, 4.0f, 6.0f}));
    LI_CHECK((2.0f * a == Vec3{2.0f, 4.0f, 6.0f}));
    LI_CHECK((b / 2.0f == Vec3{2.0f, 3.0f, 4.5f}));
    LI_CHECK((b / a == Vec3{4.0f, 3.0f, 3.0f}));
}

LI_TEST(Vec3, InPlaceOperators) {
    Vec3 value{1.0f, 2.0f, 3.0f};
    value += Vec3{1.0f, 1.0f, 1.0f};
    LI_CHECK((value == Vec3{2.0f, 3.0f, 4.0f}));
    value -= Vec3{1.0f, 2.0f, 3.0f};
    LI_CHECK((value == Vec3{1.0f, 1.0f, 1.0f}));
    value *= 3.0f;
    LI_CHECK((value == Vec3{3.0f, 3.0f, 3.0f}));
}

LI_TEST(Vec3, Equality) {
    LI_CHECK((Vec3{1.0f, 2.0f, 3.0f} == Vec3{1.0f, 2.0f, 3.0f}));
    LI_CHECK((Vec3{1.0f, 2.0f, 3.0f} != Vec3{0.0f, 2.0f, 3.0f}));
    LI_CHECK((Vec3{1.0f, 2.0f, 3.0f} != Vec3{1.0f, 0.0f, 3.0f}));
    LI_CHECK((Vec3{1.0f, 2.0f, 3.0f} != Vec3{1.0f, 2.0f, 0.0f}));
}

LI_TEST(Vec3, Measurements) {
    const Vec3 value{2.0f, 3.0f, 6.0f};
    LI_CHECK(value.Length() == 7.0f);
    LI_CHECK(value.LengthSquared() == 49.0f);
    LI_CHECK(value.Distance({0.0f, 0.0f, 0.0f}) == 7.0f);
    LI_CHECK(value.Dot({1.0f, 2.0f, 3.0f}) == 26.0f);
}

LI_TEST(Vec3, CrossFollowsTheRightHandRule) {
    const Vec3 x{1.0f, 0.0f, 0.0f};
    const Vec3 y{0.0f, 1.0f, 0.0f};
    const Vec3 z{0.0f, 0.0f, 1.0f};
    LI_CHECK((x.Cross(y) == z));
    LI_CHECK((y.Cross(z) == x));
    LI_CHECK((z.Cross(x) == y));
    LI_CHECK((y.Cross(x) == -z));
}

LI_TEST(Vec3, Normalize) {
    const Vec3 unit = Vec3{2.0f, 3.0f, 6.0f}.Normalize();
    LI_CHECK(Near(unit.Length(), 1.0f));
    LI_CHECK((Vec3{0.0f, 0.0f, 0.0f}.Normalize() == Vec3{0.0f, 0.0f, 0.0f}));
}

LI_TEST(Vec3, Lerp) {
    LI_CHECK((Vec3{0.0f, 2.0f, 4.0f}.Lerp({4.0f, 4.0f, 8.0f}, 0.5f) == Vec3{2.0f, 3.0f, 6.0f}));
}

// Rect

LI_TEST(Rect, ContainsIncludesEveryEdge) {
    const Rect rect{1.0f, 2.0f, 3.0f, 4.0f};
    LI_CHECK(rect.Contains({2.0f, 3.0f}));
    LI_CHECK(rect.Contains({1.0f, 2.0f}));
    LI_CHECK(rect.Contains({4.0f, 6.0f}));
    LI_CHECK(!rect.Contains({0.9f, 3.0f}));
    LI_CHECK(!rect.Contains({4.1f, 3.0f}));
    LI_CHECK(!rect.Contains({2.0f, 1.9f}));
    LI_CHECK(!rect.Contains({2.0f, 6.1f}));
}

LI_TEST(Rect, OverlapsExcludesTouchingEdges) {
    const Rect rect{0.0f, 0.0f, 2.0f, 2.0f};
    LI_CHECK(rect.Overlaps({1.0f, 1.0f, 2.0f, 2.0f}));
    LI_CHECK(rect.Overlaps({0.5f, 0.5f, 1.0f, 1.0f}));
    LI_CHECK(rect.Overlaps({-1.0f, -1.0f, 4.0f, 4.0f}));
    LI_CHECK(!rect.Overlaps({2.0f, 0.0f, 1.0f, 1.0f}));
    LI_CHECK(!rect.Overlaps({0.0f, 2.0f, 1.0f, 1.0f}));
    LI_CHECK(!rect.Overlaps({-1.0f, 0.0f, 1.0f, 1.0f}));
    LI_CHECK(!rect.Overlaps({5.0f, 5.0f, 1.0f, 1.0f}));
}

// Color

LI_TEST(Color, FromHexReadsChannelsInOrder) {
    const Color color = Color::FromHex(0xFF800040);
    LI_CHECK(color.r == 1.0f);
    LI_CHECK(Near(color.g, 128.0f / 255.0f));
    LI_CHECK(color.b == 0.0f);
    LI_CHECK(Near(color.a, 64.0f / 255.0f));
}

LI_TEST(Color, HexRoundTrips) {
    for(uint32_t value = 0; value < 256; ++value) {
        const uint32_t rgba = (value << 24) | ((255 - value) << 16) | (value << 8) | (value / 2);
        LI_CHECK(Color::FromHex(rgba).ToHex() == rgba);
    }
}

LI_TEST(Color, ToHexClampsAndRounds) {
    LI_CHECK((Color{2.0f, -1.0f, 0.5f, 1.0f}.ToHex() == 0xFF0080FF));
}

LI_TEST(Color, MultiplyLerpAndEquality) {
    const Color half{0.5f, 0.5f, 0.5f, 0.5f};
    LI_CHECK((Colors::White * half == half));
    LI_CHECK((Colors::Transparent.Lerp(Colors::White, 0.5f) == half));
    LI_CHECK((Colors::Red == Colors::Red));
    LI_CHECK(!(Colors::Red == Colors::Blue));
}

LI_TEST(Color, Constants) {
    LI_CHECK(Colors::White.ToHex() == 0xFFFFFFFF);
    LI_CHECK(Colors::Black.ToHex() == 0x000000FF);
    LI_CHECK(Colors::Transparent.ToHex() == 0x00000000);
    LI_CHECK(Colors::Red.ToHex() == 0xFF0000FF);
    LI_CHECK(Colors::Green.ToHex() == 0x00FF00FF);
    LI_CHECK(Colors::Blue.ToHex() == 0x0000FFFF);
    LI_CHECK(Colors::Yellow.ToHex() == 0xFFFF00FF);
    LI_CHECK(CompileTimeColor == Colors::White);
}

// Random

LI_TEST(Random, SameSeedGivesTheSameSequence) {
    Random a(42);
    Random b(42);
    for(int i = 0; i < 100; ++i) {
        LI_CHECK(a.NextU64() == b.NextU64());
    }
}

LI_TEST(Random, MatchesTheXorshiftReferenceSequence) {
    // Worked out by hand from the 13, 7, 17 shifts, starting at a state of 1
    Random random(1);
    LI_CHECK(random.NextU64() == 0x40822041ULL);
    LI_CHECK(random.NextU64() == 0x100041060C011441ULL);
}

LI_TEST(Random, DifferentSeedsDiffer) {
    Random a(1);
    Random b(2);
    LI_CHECK(a.NextU64() != b.NextU64());
}

LI_TEST(Random, SetSeedRestartsTheSequence) {
    Random random(42);
    const uint64_t first = random.NextU64();
    random.NextU64();
    random.SetSeed(42);
    LI_CHECK(random.NextU64() == first);
}

LI_TEST(Random, ZeroSeedIsReplacedWithOne) {
    Random zero(0);
    Random one(1);
    LI_CHECK(zero.NextU64() == one.NextU64());
    zero.SetSeed(0);
    one.SetSeed(1);
    LI_CHECK(zero.NextU64() == one.NextU64());
}

LI_TEST(Random, FloatsStayInRange) {
    Random random(7);
    for(int i = 0; i < 100000; ++i) {
        const float unit = random.NextFloat();
        LI_CHECK(unit >= 0.0f && unit < 1.0f);
        const float ranged = random.Range(-5.0f, 5.0f);
        LI_CHECK(ranged >= -5.0f && ranged <= 5.0f);
    }
}

LI_TEST(Random, FloatsCoverTheRangeEvenly) {
    Random random(7);
    int buckets[10] = {};
    constexpr int Samples = 100000;
    for(int i = 0; i < Samples; ++i) {
        ++buckets[static_cast<int>(random.NextFloat() * 10.0f)];
    }
    for(const int bucket : buckets) {
        LI_CHECK(bucket > Samples / 10 * 9 / 10);
        LI_CHECK(bucket < Samples / 10 * 11 / 10);
    }
}

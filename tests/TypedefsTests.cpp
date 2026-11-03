#include "LiLib/Core/Assert.h"
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"
#include <type_traits>

using namespace LiLib;

namespace {

enum class Mode : int {
    FIRST = 3,
    SECOND = 4
};

class NonCopyable {
public:
    NonCopyable() = default;
    LI_NON_COPYABLE(NonCopyable);
};

static_assert(!std::is_copy_constructible_v<NonCopyable>);
static_assert(!std::is_copy_assignable_v<NonCopyable>);
static_assert(std::is_same_v<byte_t, uint8_t>);
static_assert(std::is_same_v<uuid_t, uint64_t>);
static_assert(Min(1, 2) == 1);
static_assert(Max(1, 2) == 2);
static_assert(Clamp(5, 0, 3) == 3);

} // namespace

LI_TEST(Typedefs, PairSupportsStructuredBindings) {
    const Pair<int, float> pair{1, 2.0f};
    const auto& [first, second] = pair;
    LI_CHECK(first == 1);
    LI_CHECK(second == 2.0f);
}

LI_TEST(Typedefs, MinMaxReturnTheRightValue) {
    LI_CHECK(Min(1, 2) == 1);
    LI_CHECK(Min(2, 1) == 1);
    LI_CHECK(Max(1, 2) == 2);
    LI_CHECK(Max(2, 1) == 2);
}

LI_TEST(Typedefs, MinMaxReturnFirstArgumentOnTies) {
    const int a = 1;
    const int b = 1;
    LI_CHECK(&Min(a, b) == &a);
    LI_CHECK(&Max(a, b) == &a);
}

LI_TEST(Typedefs, ClampCoversAllRanges) {
    LI_CHECK(Clamp(-1, 0, 10) == 0);
    LI_CHECK(Clamp(5, 0, 10) == 5);
    LI_CHECK(Clamp(11, 0, 10) == 10);
    LI_CHECK(Clamp(0, 0, 10) == 0);
    LI_CHECK(Clamp(10, 0, 10) == 10);
}

LI_TEST(Typedefs, SwapExchangesValues) {
    int a = 1;
    int b = 2;
    Swap(a, b);
    LI_CHECK(a == 2 && b == 1);

    String longText = "a string long enough that it has to live on the heap";
    String shortText = "short";
    Swap(longText, shortText);
    LI_CHECK(longText == "short");
    LI_CHECK(shortText == "a string long enough that it has to live on the heap");
}

LI_TEST(Typedefs, HashBytesMatchesFnv1aReferenceValues) {
    LI_CHECK(HashBytes("", 0) == 14695981039346656037ULL);
    LI_CHECK(HashBytes("a", 1) == 0xaf63dc4c8601ec8cULL);
    LI_CHECK(HashBytes("foobar", 6) == 0x85944171f73967e8ULL);
}

LI_TEST(Typedefs, HashIntegerSpreadsValues) {
    LI_CHECK(HashInteger(1) != HashInteger(2));
    LI_CHECK(HashInteger(1) == HashInteger(1));
    // Neighboring keys must not land in neighboring slots of a small table
    int sameSlot = 0;
    for(uint64_t i = 0; i < 1000; ++i) {
        if((HashInteger(i) & 15) == (HashInteger(i + 1) & 15)) {
            ++sameSlot;
        }
    }
    LI_CHECK(sameSlot < 200);
}

LI_TEST(Typedefs, HasherCoversIntegersEnumsAndPointers) {
    LI_CHECK(Hasher<int>::Hash(5) == HashInteger(5));
    LI_CHECK(Hasher<Mode>::Hash(Mode::SECOND) == HashInteger(4));

    int a = 0;
    int b = 0;
    LI_CHECK(Hasher<int*>::Hash(&a) == Hasher<int*>::Hash(&a));
    LI_CHECK(Hasher<int*>::Hash(&a) != Hasher<int*>::Hash(&b));
}

LI_TEST(Assert, PassingChecksContinue) {
    LI_ASSERT(true, "Must not fire");
    LI_VERIFY(true, "Must not fire");
}

LI_TEST(Assert, VerifyAlwaysEvaluatesItsExpressionOnce) {
    int evaluations = 0;
    LI_VERIFY(++evaluations > 0, "Must not fire");
    LI_CHECK(evaluations == 1);
}

LI_TEST(Assert, AssertEvaluatesOnlyWithoutNdebug) {
    int evaluations = 0;
    LI_ASSERT(++evaluations > 0, "Must not fire");
#ifdef NDEBUG
    LI_CHECK(evaluations == 0);
#else
    LI_CHECK(evaluations == 1);
#endif
}

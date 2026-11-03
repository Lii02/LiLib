#pragma once
#include <cmath>

// Alignment tests pad their types on purpose, which MSVC warns about at /W4
#ifdef _MSC_VER
#pragma warning(disable : 4324)
#endif

/*
 * A small unit test framework with no dependencies. Tests register themselves, and the runner checks
 * after each one that every allocation made through Memory was freed.
 *
 *     LI_TEST(Array, PushGrows) {
 *         Array<int> values;
 *         values.Push(1);
 *         LI_REQUIRE(values.Size() == 1); // Stops this test on failure, for checks later lines depend on
 *         LI_CHECK(values[0] == 1);       // Records the failure and keeps going
 *     }
 *
 * LI_TEST_DISABLED keeps a test compiling and listed in every run without running it. Use it for a
 * known bug whose test would crash the run, and give the reason as the third argument.
 */

namespace LiLibTest {

/*
 * @brief One registered test. Instances are created at static initialization by LI_TEST.
 */
class TestCase {
public:
    TestCase(const char* suite, const char* name, void (*run)(), const char* disabledReason = nullptr);

    /*
     * @brief Runs the tests whose "Suite.Name" contains the filter, or all of them if it's null.
     * @param includeDisabled Also runs tests declared with LI_TEST_DISABLED.
     * @returns The number of tests that failed.
     */
    static int RunAll(const char* filter, bool includeDisabled);
    static void ListAll();

    /*
     * @brief Records a failed check against the test that's currently running.
     */
    static void ReportFailure(const char* file, int line, const char* expression);

private:
    static TestCase* Head;
    static int CurrentFailures;

    const char* suite;
    const char* name;
    void (*run)();
    const char* disabledReason;
    TestCase* next = nullptr;
};

/*
 * @brief Counts constructions and destructions, to test that containers manage element lifetimes correctly.
 */
class LifetimeCounter {
public:
    LifetimeCounter() { ++Live; }
    explicit LifetimeCounter(int value) : value(value) { ++Live; }
    LifetimeCounter(const LifetimeCounter& other) : value(other.value) {
        ++Live;
        ++Copies;
    }
    LifetimeCounter(LifetimeCounter&& other) noexcept : value(other.value) {
        other.value = MovedFrom;
        ++Live;
        ++Moves;
    }
    ~LifetimeCounter() { --Live; }

    LifetimeCounter& operator=(const LifetimeCounter& other) {
        value = other.value;
        ++Copies;
        return *this;
    }
    LifetimeCounter& operator=(LifetimeCounter&& other) noexcept {
        if(this != &other) {
            value = other.value;
            other.value = MovedFrom;
        }
        ++Moves;
        return *this;
    }

    bool operator==(const LifetimeCounter& other) const { return value == other.value; }
    bool operator<(const LifetimeCounter& other) const { return value < other.value; }

    static void ResetCounts() {
        Live = 0;
        Copies = 0;
        Moves = 0;
    }

    static constexpr int MovedFrom = -999;
    inline static int Live = 0;
    inline static int Copies = 0;
    inline static int Moves = 0;

    int value = 0;
};

inline bool Near(float a, float b, float tolerance = 1e-4f) {
    return std::fabs(a - b) < tolerance;
}

} // namespace LiLibTest

#define LI_TEST(Suite, Name)                                                                                                               \
    static void Suite##Name##Test();                                                                                                       \
    static const ::LiLibTest::TestCase Suite##Name##Case(#Suite, #Name, Suite##Name##Test);                                                \
    static void Suite##Name##Test()

#define LI_TEST_DISABLED(Suite, Name, reason)                                                                                              \
    static void Suite##Name##Test();                                                                                                       \
    static const ::LiLibTest::TestCase Suite##Name##Case(#Suite, #Name, Suite##Name##Test, reason);                                        \
    static void Suite##Name##Test()

#define LI_CHECK(expression)                                                                                                               \
    do {                                                                                                                                   \
        if(!(expression)) {                                                                                                                \
            ::LiLibTest::TestCase::ReportFailure(__FILE__, __LINE__, #expression);                                                         \
        }                                                                                                                                  \
    } while(0)

#define LI_REQUIRE(expression)                                                                                                             \
    do {                                                                                                                                   \
        if(!(expression)) {                                                                                                                \
            ::LiLibTest::TestCase::ReportFailure(__FILE__, __LINE__, #expression);                                                         \
            return;                                                                                                                        \
        }                                                                                                                                  \
    } while(0)

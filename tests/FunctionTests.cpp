#include "LiLib/Core/Function.h"
#include "LiLib/Memory/Memory.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

int AddOne(int value) {
    return value + 1;
}

} // namespace

LI_TEST(Function, DefaultAndNullAreEmpty) {
    const Function<int()> defaulted;
    const Function<int()> fromNull = nullptr;
    LI_CHECK(!defaulted);
    LI_CHECK(!fromNull);
}

LI_TEST(Function, CallsLambdasAndFunctionPointers) {
    const int base = AddOne(9);
    const Function<int(int)> lambda = [base](int value) {
        return value + base;
    };
    const Function<int(int)> pointer = AddOne;
    LI_REQUIRE(lambda && pointer);
    LI_CHECK(lambda(5) == 15);
    LI_CHECK(pointer(5) == 6);
}

LI_TEST(Function, SupportsVoidAndReferenceSignatures) {
    int counter = 0;
    const Function<void()> increment = [&counter]() {
        ++counter;
    };
    increment();
    LI_CHECK(counter == 1);

    const Function<void(int&)> byReference = [](int& value) {
        value = 42;
    };
    byReference(counter);
    LI_CHECK(counter == 42);

    const Function<String(String&&)> byMove = [](String&& text) {
        return String(std::move(text));
    };
    String source = "a string long enough that it has to live on the heap";
    const String result = byMove(std::move(source));
    LI_CHECK(result == "a string long enough that it has to live on the heap");
    LI_CHECK(source.IsEmpty());
}

LI_TEST(Function, MutableStatePersistsAcrossCalls) {
    const Function<int()> counter = [count = 0]() mutable {
        return ++count;
    };
    LI_CHECK(counter() == 1);
    LI_CHECK(counter() == 2);
}

LI_TEST(Function, CopiesHaveIndependentState) {
    const Function<int()> original = [count = 0]() mutable {
        return ++count;
    };
    LI_CHECK(original() == 1);
    const Function<int()> copy = original;
    LI_CHECK(copy() == 2);
    LI_CHECK(copy() == 3);
    LI_CHECK(original() == 2);
}

LI_TEST(Function, MoveTransfersWithoutCopyingOrAllocating) {
    Function<int()> source = []() {
        return 7;
    };
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    const Function<int()> moved = std::move(source);
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);
    LI_CHECK(!source);
    LI_REQUIRE(moved);
    LI_CHECK(moved() == 7);
}

LI_TEST(Function, AssignmentReplacesTheCallable) {
    Function<int()> function = []() {
        return 1;
    };
    const Function<int()> other = []() {
        return 2;
    };
    function = other;
    LI_CHECK(function() == 2);
    function = []() {
        return 3;
    };
    LI_CHECK(function() == 3);
    function = nullptr;
    LI_CHECK(!function);
}

LI_TEST(Function, SelfAssignmentKeepsTheCallable) {
    Function<int()> function = []() {
        return 1;
    };
    Function<int()>& alias = function;
    function = alias;
    LI_REQUIRE(function);
    LI_CHECK(function() == 1);
    function = std::move(alias);
    LI_REQUIRE(function);
    LI_CHECK(function() == 1);
}

LI_TEST(Function, DestroysCapturesExactlyOnce) {
    LifetimeCounter::ResetCounts();
    {
        const Function<int()> function = [counter = LifetimeCounter(5)]() {
            return counter.value;
        };
        const Function<int()> copy = function;
        LI_CHECK(LifetimeCounter::Live == 2);
        LI_CHECK(copy() == 5);
    }
    LI_CHECK(LifetimeCounter::Live == 0);
}

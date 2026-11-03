#include "LiLib/Containers/Array.h"
#include "LiLib/Memory/UniquePtr.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"
#include <cstdint>
#include <type_traits>

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

static_assert(std::is_nothrow_move_constructible_v<Array<int>>);
static_assert(std::is_nothrow_move_assignable_v<Array<int>>);

const char* const LongText = "a string long enough that it has to live on the heap";

bool Equals(const Array<int>& values, std::initializer_list<int> expected) {
    if(values.Size() != expected.size()) {
        return false;
    }
    size_t i = 0;
    for(const int value : expected) {
        if(values[i++] != value) {
            return false;
        }
    }
    return true;
}

// Fills the array exactly to capacity, so the next addition has to reallocate
void FillToCapacity(Array<String>& values) {
    values.Push(LongText);
    while(values.Size() < values.Capacity()) {
        values.Push(LongText);
    }
}

} // namespace

LI_TEST(Array, StartsEmpty) {
    const Array<int> values;
    LI_CHECK(values.IsEmpty());
    LI_CHECK(values.Size() == 0);
    LI_CHECK(values.Capacity() == 0);
    LI_CHECK(values.Data() == nullptr);
    LI_CHECK(values.begin() == values.end());
}

LI_TEST(Array, InitializerList) {
    const Array<int> values{1, 2, 3};
    LI_CHECK(Equals(values, {1, 2, 3}));
    const Array<int> empty{};
    LI_CHECK(empty.IsEmpty());
}

LI_TEST(Array, PushGrowsAndKeepsOrder) {
    Array<int> values;
    for(int i = 0; i < 1000; ++i) {
        values.Push(i);
    }
    LI_REQUIRE(values.Size() == 1000);
    LI_CHECK(values.Capacity() >= 1000);
    for(int i = 0; i < 1000; ++i) {
        LI_CHECK(values[static_cast<size_t>(i)] == i);
    }
}

LI_TEST(Array, PushReturnsTheNewElement) {
    Array<int> values;
    int& pushed = values.Push(5);
    pushed = 6;
    LI_CHECK(values[0] == 6);
    const int& emplaced = values.Emplace(7);
    LI_CHECK(&emplaced == &values.Back());
}

LI_TEST(Array, PushOwnElementSurvivesReallocation) {
    Array<String> values;
    FillToCapacity(values);
    const size_t capacity = values.Capacity();
    values.Push(values[0]);
    LI_CHECK(values.Capacity() > capacity);
    LI_CHECK(values.Back() == LongText);
    LI_CHECK(values[0] == LongText);
}

LI_TEST(Array, PushMovedOwnElementSurvivesReallocation) {
    Array<String> values;
    FillToCapacity(values);
    values.Push(std::move(values[0]));
    LI_CHECK(values.Back() == LongText);
}

LI_TEST(Array, EmplaceConstructsInPlace) {
    LifetimeCounter::ResetCounts();
    Array<LifetimeCounter> values;
    values.Reserve(1);
    values.Emplace(5);
    LI_CHECK(values[0].value == 5);
    LI_CHECK(LifetimeCounter::Copies == 0);
    LI_CHECK(LifetimeCounter::Moves == 0);
}

LI_TEST(Array, EmplaceOwnElementSurvivesReallocation) {
    Array<String> values;
    FillToCapacity(values);
    values.Emplace(values[0]);
    LI_CHECK(values.Back() == LongText);
}

LI_TEST(Array, HoldsMoveOnlyTypes) {
    Array<UniquePtr<int>> values;
    for(int i = 0; i < 100; ++i) {
        values.Push(MakeUnique<int>(i));
    }
    values.Insert(0, MakeUnique<int>(-1));
    values.RemoveAt(1);
    values.RemoveSwap(0);
    LI_REQUIRE(values.Size() == 99);
    LI_CHECK(*values[0] == 99);
    LI_CHECK(*values[1] == 1);
}

LI_TEST(Array, FrontAndBack) {
    Array<int> values{1, 2, 3};
    LI_CHECK(values.Front() == 1);
    LI_CHECK(values.Back() == 3);
    values.Back() = 9;
    LI_CHECK(values[2] == 9);
    const Array<int>& constant = values;
    LI_CHECK(constant.Front() == 1);
    LI_CHECK(constant.Back() == 9);
}

LI_TEST(Array, Reserve) {
    Array<int> values{1, 2};
    values.Reserve(100);
    LI_CHECK(values.Capacity() == 100);
    LI_CHECK(Equals(values, {1, 2}));
    const int* data = values.Data();
    values.Reserve(10);
    LI_CHECK(values.Capacity() == 100);
    LI_CHECK(values.Data() == data);
}

LI_TEST(Array, Resize) {
    Array<int> values{1, 2, 3};
    values.Resize(1);
    LI_CHECK(Equals(values, {1}));
    values.Resize(3);
    LI_CHECK(Equals(values, {1, 0, 0}));
    values.Resize(4, 9);
    LI_CHECK(Equals(values, {1, 0, 0, 9}));
    values.Resize(2, 9);
    LI_CHECK(Equals(values, {1, 0}));
}

LI_TEST(Array, ResizeFillWithOwnElementSurvivesReallocation) {
    Array<String> values;
    FillToCapacity(values);
    const size_t newSize = values.Capacity() + 4;
    values.Resize(newSize, values[0]);
    LI_CHECK(values.Back() == LongText);
}

LI_TEST(Array, Pop) {
    LifetimeCounter::ResetCounts();
    Array<LifetimeCounter> values;
    values.Emplace(1);
    values.Emplace(2);
    values.Pop();
    LI_CHECK(values.Size() == 1);
    LI_CHECK(LifetimeCounter::Live == 1);
}

LI_TEST(Array, ClearKeepsCapacity) {
    LifetimeCounter::ResetCounts();
    Array<LifetimeCounter> values;
    values.Emplace(1);
    const size_t capacity = values.Capacity();
    values.Clear();
    LI_CHECK(values.IsEmpty());
    LI_CHECK(values.Capacity() == capacity);
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(Array, InsertAtEveryPosition) {
    Array<int> values{1, 3};
    values.Insert(1, 2);
    LI_CHECK(Equals(values, {1, 2, 3}));
    values.Insert(0, 0);
    LI_CHECK(Equals(values, {0, 1, 2, 3}));
    values.Insert(4, 4);
    LI_CHECK(Equals(values, {0, 1, 2, 3, 4}));
    Array<int> empty;
    empty.Insert(0, 1);
    LI_CHECK(Equals(empty, {1}));
}

LI_TEST(Array, InsertOwnElement) {
    Array<String> values;
    FillToCapacity(values);
    values.Insert(1, values[0]);
    LI_CHECK(values[1] == LongText);
    LI_CHECK(values[0] == LongText);
}

LI_TEST(Array, InsertMovedOwnElementSurvivesReallocation) {
    Array<String> values;
    FillToCapacity(values);
    values.Insert(1, std::move(values[0]));
    LI_CHECK(values[1] == LongText);
}

LI_TEST(Array, InsertOwnElementWithoutReallocation) {
    Array<String> values{"a", "b", "c"};
    values.Reserve(16);
    values.Insert(0, values[2]);
    LI_CHECK(values[0] == "c");
    LI_CHECK(values[3] == "c");
}

LI_TEST(Array, RemoveAtKeepsOrder) {
    Array<int> values{1, 2, 3, 4};
    values.RemoveAt(1);
    LI_CHECK(Equals(values, {1, 3, 4}));
    values.RemoveAt(2);
    LI_CHECK(Equals(values, {1, 3}));
    values.RemoveAt(0);
    LI_CHECK(Equals(values, {3}));
}

LI_TEST(Array, RemoveSwapMovesTheLastElement) {
    Array<int> values{1, 2, 3, 4};
    values.RemoveSwap(0);
    LI_CHECK(Equals(values, {4, 2, 3}));
    values.RemoveSwap(2);
    LI_CHECK(Equals(values, {4, 2}));
    Array<String> single{"only"};
    single.RemoveSwap(0);
    LI_CHECK(single.IsEmpty());
}

LI_TEST(Array, Search) {
    Array<int> values{5, 6, 5};
    LI_CHECK(values.IndexOf(5) == 0);
    LI_CHECK(values.IndexOf(6) == 1);
    LI_CHECK(values.IndexOf(7) == Array<int>::NotFound);
    LI_CHECK(values.Contains(6));
    LI_CHECK(!values.Contains(7));

    int* found = values.FindIf([](int value) { return value > 5; });
    LI_CHECK(found == values.Data() + 1);
    LI_CHECK(values.FindIf([](int value) { return value > 9; }) == nullptr);
}

LI_TEST(Array, RemoveTakesOnlyTheFirstMatch) {
    Array<int> values{5, 6, 5};
    LI_CHECK(values.Remove(5));
    LI_CHECK(Equals(values, {6, 5}));
    LI_CHECK(!values.Remove(7));
    LI_CHECK(Equals(values, {6, 5}));
}

LI_TEST(Array, RemoveIf) {
    Array<int> values{1, 2, 3, 4, 5, 6};
    const size_t removed = values.RemoveIf([](int value) { return value % 2 == 0; });
    LI_CHECK(removed == 3);
    LI_CHECK(Equals(values, {1, 3, 5}));
    LI_CHECK(values.RemoveIf([](int) { return false; }) == 0);
    LI_CHECK(values.RemoveIf([](int) { return true; }) == 3);
    LI_CHECK(values.IsEmpty());
}

LI_TEST(Array, CopyIsIndependent) {
    Array<String> original{"one", LongText};
    Array<String> copy = original;
    copy[0] = "changed";
    LI_CHECK(original[0] == "one");
    LI_CHECK(copy[1] == LongText);

    Array<String> assigned{"old", "old", "old"};
    assigned = original;
    LI_CHECK(assigned.Size() == 2);
    LI_CHECK(assigned[1] == LongText);
    original.Clear();
    LI_CHECK(assigned[0] == "one");
}

LI_TEST(Array, MoveTransfersStorage) {
    Array<int> source{1, 2, 3};
    const int* data = source.Data();
    Array<int> moved = std::move(source);
    LI_CHECK(moved.Data() == data);
    LI_CHECK(source.IsEmpty());
    LI_CHECK(source.Capacity() == 0);

    Array<int> assigned{9};
    assigned = std::move(moved);
    LI_CHECK(assigned.Data() == data);
    LI_CHECK(moved.IsEmpty());

    moved.Push(1);
    LI_CHECK(moved[0] == 1);
}

LI_TEST(Array, SelfAssignmentKeepsContents) {
    Array<String> values{"one", LongText};
    Array<String>& alias = values;
    values = alias;
    LI_REQUIRE(values.Size() == 2);
    LI_CHECK(values[1] == LongText);
    values = std::move(alias);
    LI_REQUIRE(values.Size() == 2);
    LI_CHECK(values[1] == LongText);
}

LI_TEST(Array, ElementsAreDestroyedExactlyOnce) {
    LifetimeCounter::ResetCounts();
    {
        Array<LifetimeCounter> values;
        for(int i = 0; i < 100; ++i) {
            values.Emplace(i);
        }
        values.Insert(50, LifetimeCounter(-1));
        values.RemoveAt(10);
        values.RemoveSwap(5);
        values.RemoveIf([](const LifetimeCounter& value) { return value.value % 2 == 0; });
        const Array<LifetimeCounter> copy = values;
        LI_CHECK(LifetimeCounter::Live == static_cast<int>(values.Size() * 2));
    }
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(Array, GrowingMovesInsteadOfCopying) {
    LifetimeCounter::ResetCounts();
    Array<LifetimeCounter> values;
    for(int i = 0; i < 100; ++i) {
        values.Emplace(i);
    }
    LI_CHECK(LifetimeCounter::Copies == 0);
}

LI_TEST(Array, PushAddsNoExtraMovesWhenGrowing) {
    LifetimeCounter::ResetCounts();
    Array<LifetimeCounter> values;
    const LifetimeCounter item(1);
    values.Push(item);
    LI_CHECK(LifetimeCounter::Copies == 1);
    LI_CHECK(LifetimeCounter::Moves == 0);
}

LI_TEST(Array, RespectsOverAlignedTypes) {
    struct alignas(128) OverAligned {
        int value = 0;
    };
    Array<OverAligned> values;
    OverAligned item;
    item.value = 7;
    for(int i = 0; i < 20; ++i) {
        values.Emplace();
        values.Push(item);
        values.Insert(0, item);
        values.Insert(0, OverAligned{});
        LI_CHECK(reinterpret_cast<uintptr_t>(values.Data()) % 128 == 0);
    }
    LI_CHECK(values.Back().value == 7);
}

LI_TEST(Array, RangeForVisitsEverything) {
    Array<int> values{1, 2, 3};
    for(int& value : values) {
        value *= 2;
    }
    int total = 0;
    const Array<int>& constant = values;
    for(const int value : constant) {
        total += value;
    }
    LI_CHECK(total == 12);
}

#include "LiLib/Algorithms/Sort.h"
#include "LiLib/Math/Random.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

struct Keyed {
    int key;
    int order;
};

bool SortsAndKeepsOrderOfEqualKeys(size_t size, uint64_t seed, int keyRange) {
    Random random(seed);
    Array<Keyed> values;
    int keySum = 0;
    for(size_t i = 0; i < size; ++i) {
        const int key = static_cast<int>(random.NextU64() % static_cast<uint64_t>(keyRange));
        keySum += key;
        values.Push({key, static_cast<int>(i)});
    }

    StableSort(values, [](const Keyed& a, const Keyed& b) { return a.key < b.key; });

    if(values.Size() != size) {
        return false;
    }
    int sortedKeySum = 0;
    for(size_t i = 0; i < size; ++i) {
        sortedKeySum += values[i].key;
        if(i > 0) {
            const bool inOrder =
                values[i - 1].key < values[i].key || (values[i - 1].key == values[i].key && values[i - 1].order < values[i].order);
            if(!inOrder) {
                return false;
            }
        }
    }
    return sortedKeySum == keySum;
}

} // namespace

LI_TEST(StableSort, EmptyAndSingleElement) {
    Array<int> empty;
    StableSort(empty);
    LI_CHECK(empty.IsEmpty());
    Array<int> single{1};
    StableSort(single);
    LI_CHECK(single[0] == 1);
}

LI_TEST(StableSort, UsesLessThanByDefault) {
    Array<int> values{3, 1, 2};
    StableSort(values);
    LI_CHECK(values[0] == 1 && values[1] == 2 && values[2] == 3);
}

LI_TEST(StableSort, UsesACustomOrdering) {
    Array<int> values{1, 3, 2};
    StableSort(values, [](int a, int b) { return a > b; });
    LI_CHECK(values[0] == 3 && values[1] == 2 && values[2] == 1);
}

// Covers every merge pass count, including sizes either side of each power-of-two run boundary
LI_TEST(StableSort, EverySizeUpTo300) {
    for(size_t size = 0; size <= 300; ++size) {
        LI_CHECK(SortsAndKeepsOrderOfEqualKeys(size, size + 1, 10));
    }
}

LI_TEST(StableSort, LargeArrays) {
    LI_CHECK(SortsAndKeepsOrderOfEqualKeys(20000, 7, 50));
    LI_CHECK(SortsAndKeepsOrderOfEqualKeys(20000, 8, 1000000));
}

LI_TEST(StableSort, AlreadySortedReversedAndAllEqual) {
    constexpr int Count = 1000;
    Array<int> ascending;
    Array<int> descending;
    Array<int> equal;
    for(int i = 0; i < Count; ++i) {
        ascending.Push(i);
        descending.Push(Count - 1 - i);
        equal.Push(5);
    }
    StableSort(ascending);
    StableSort(descending);
    StableSort(equal);
    for(int i = 0; i < Count; ++i) {
        const size_t index = static_cast<size_t>(i);
        LI_CHECK(ascending[index] == i);
        LI_CHECK(descending[index] == i);
        LI_CHECK(equal[index] == 5);
    }
}

LI_TEST(StableSort, SortsStrings) {
    Array<String> values;
    for(int i = 99; i >= 0; --i) {
        values.Push(String::Format("a-string-long-enough-for-the-heap-%02d", i));
    }
    StableSort(values);
    for(int i = 0; i < 100; ++i) {
        LI_CHECK(values[static_cast<size_t>(i)] == String::Format("a-string-long-enough-for-the-heap-%02d", i));
    }
}

LI_TEST(StableSort, ElementsAreDestroyedExactlyOnce) {
    LifetimeCounter::ResetCounts();
    {
        Array<LifetimeCounter> values;
        for(int i = 100; i > 0; --i) {
            values.Emplace(i);
        }
        StableSort(values);
        LI_CHECK(values[0].value == 1);
        LI_CHECK(values[99].value == 100);
        LI_CHECK(LifetimeCounter::Live == 100);
    }
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(StableSort, ShortArraysDoNotAllocate) {
    Array<int> values;
    for(int i = 16; i > 0; --i) {
        values.Push(i);
    }
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    StableSort(values);
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);
    LI_CHECK(values[0] == 1);
}

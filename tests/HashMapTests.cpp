#include "LiLib/Containers/Array.h"
#include "LiLib/Containers/HashMap.h"
#include "LiLib/Math/Random.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"
#include <type_traits>

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

static_assert(std::is_nothrow_move_constructible_v<HashMap<int, int>>);
static_assert(std::is_nothrow_move_assignable_v<HashMap<int, int>>);

enum class Color : int {
    RED,
    BLUE
};

// Every key lands near the end of a small table, which forces long probe chains that wrap around
struct CollidingKey {
    int value;
};

bool operator==(const CollidingKey& a, const CollidingKey& b) {
    return a.value == b.value;
}

} // namespace

namespace LiLib {

template<>
class Hasher<CollidingKey> {
public:
    static uint64_t Hash(const CollidingKey& key) { return 14 + static_cast<uint64_t>(key.value % 3); }
};

} // namespace LiLib

namespace {

// Applies random operations to a map and a simple array model, and checks they always agree
template<typename TKey, typename TMakeKey>
bool MatchesModelUnderRandomOperations(uint64_t seed, int keyRange, int operations, TMakeKey makeKey) {
    Random random(seed);
    HashMap<TKey, int> map;
    Array<Pair<int, int>> model;

    const auto findInModel = [&model](int key) -> int64_t {
        for(size_t i = 0; i < model.Size(); ++i) {
            if(model[i].first == key) {
                return static_cast<int64_t>(i);
            }
        }
        return -1;
    };

    for(int operation = 0; operation < operations; ++operation) {
        const int key = static_cast<int>(random.NextU64() % static_cast<uint64_t>(keyRange));
        const int value = static_cast<int>(random.NextU64() % 1000);
        const int64_t index = findInModel(key);
        if(random.NextU64() % 2 == 0) {
            map.Insert(makeKey(key), value);
            if(index >= 0) {
                model[static_cast<size_t>(index)].second = value;
            } else {
                model.Push({key, value});
            }
        } else {
            const bool removed = map.Remove(makeKey(key));
            if(removed != (index >= 0)) {
                return false;
            }
            if(index >= 0) {
                model.RemoveSwap(static_cast<size_t>(index));
            }
        }

        if(map.Size() != model.Size()) {
            return false;
        }
    }

    // Every model entry must be findable, and nothing else may be present
    for(int key = 0; key < keyRange; ++key) {
        const int64_t index = findInModel(key);
        const int* found = map.Find(makeKey(key));
        if((found != nullptr) != (index >= 0)) {
            return false;
        }
        if(found != nullptr && *found != model[static_cast<size_t>(index)].second) {
            return false;
        }
    }
    size_t iterated = 0;
    for(const Pair<TKey, int>& entry : map) {
        LI_UNUSED(entry);
        ++iterated;
    }
    return iterated == model.Size();
}

} // namespace

LI_TEST(HashMap, StartsEmpty) {
    HashMap<int, int> map;
    LI_CHECK(map.IsEmpty());
    LI_CHECK(map.Size() == 0);
    LI_CHECK(map.Find(1) == nullptr);
    LI_CHECK(!map.Contains(1));
    LI_CHECK(!map.Remove(1));
    LI_CHECK(!(map.begin() != map.end()));
    map.Clear();
}

LI_TEST(HashMap, InsertAndFind) {
    HashMap<int, int> map;
    LI_CHECK(map.Insert(1, 10) == 10);
    map.Insert(2, 20);
    LI_CHECK(map.Size() == 2);
    LI_REQUIRE(map.Find(1) != nullptr);
    LI_CHECK(*map.Find(1) == 10);
    LI_CHECK(map.Contains(2));
    LI_CHECK(map.Find(3) == nullptr);

    *map.Find(1) = 11;
    const HashMap<int, int>& constant = map;
    LI_REQUIRE(constant.Find(1) != nullptr);
    LI_CHECK(*constant.Find(1) == 11);
}

LI_TEST(HashMap, InsertReplacesAnExistingValue) {
    HashMap<int, int> map;
    map.Insert(1, 10);
    LI_CHECK(map.Insert(1, 20) == 20);
    LI_CHECK(map.Size() == 1);
    LI_CHECK(*map.Find(1) == 20);
}

LI_TEST(HashMap, SubscriptInsertsADefaultValue) {
    HashMap<int, int> map;
    LI_CHECK(map[1] == 0);
    LI_CHECK(map.Size() == 1);
    map[1] = 5;
    LI_CHECK(map[1] == 5);
    LI_CHECK(map.Size() == 1);
}

LI_TEST(HashMap, GrowsAndKeepsEverything) {
    HashMap<int, int> map;
    for(int i = 0; i < 10000; ++i) {
        map.Insert(i, i * 2);
    }
    LI_REQUIRE(map.Size() == 10000);
    for(int i = 0; i < 10000; ++i) {
        const int* value = map.Find(i);
        LI_REQUIRE(value != nullptr);
        LI_CHECK(*value == i * 2);
    }
}

LI_TEST(HashMap, Remove) {
    HashMap<int, int> map;
    map.Insert(1, 10);
    map.Insert(2, 20);
    LI_CHECK(map.Remove(1));
    LI_CHECK(!map.Remove(1));
    LI_CHECK(map.Size() == 1);
    LI_CHECK(map.Find(1) == nullptr);
    LI_CHECK(map.Contains(2));
}

LI_TEST(HashMap, RemoveKeepsCollidingKeysFindable) {
    HashMap<CollidingKey, int> map;
    for(int i = 0; i < 9; ++i) {
        map.Insert(CollidingKey{i}, i);
    }
    for(int i = 0; i < 9; i += 2) {
        LI_CHECK(map.Remove(CollidingKey{i}));
    }
    for(int i = 0; i < 9; ++i) {
        const int* value = map.Find(CollidingKey{i});
        if(i % 2 == 0) {
            LI_CHECK(value == nullptr);
        } else {
            LI_REQUIRE(value != nullptr);
            LI_CHECK(*value == i);
        }
    }
}

LI_TEST(HashMap, MatchesModelWithIntegerKeys) {
    for(uint64_t seed = 1; seed <= 5; ++seed) {
        LI_CHECK(MatchesModelUnderRandomOperations<int>(seed, 200, 5000, [](int key) { return key; }));
    }
}

LI_TEST(HashMap, MatchesModelWithCollidingKeys) {
    for(uint64_t seed = 1; seed <= 20; ++seed) {
        LI_CHECK(MatchesModelUnderRandomOperations<CollidingKey>(seed, 11, 2000, [](int key) { return CollidingKey{key}; }));
    }
}

LI_TEST(HashMap, MatchesModelWithStringKeys) {
    for(uint64_t seed = 1; seed <= 3; ++seed) {
        LI_CHECK(MatchesModelUnderRandomOperations<String>(
            seed, 100, 2000, [](int key) { return String::Format("key-number-%d-long-enough-for-heap", key); }));
    }
}

LI_TEST(HashMap, StringKeysAcceptViewsAndCStrings) {
    HashMap<String, int> map;
    map.Insert("alpha", 1);
    map["beta"] = 2;

    const String owned = "alpha";
    const char* cString = "alpha";
    LI_CHECK(map.Contains(owned));
    LI_CHECK(map.Contains(StringView("alpha")));
    LI_CHECK(map.Contains(cString));
    LI_REQUIRE(map.Find(StringView("beta")) != nullptr);
    LI_CHECK(*map.Find(StringView("beta")) == 2);
    LI_CHECK(!map.Contains(StringView("gamma")));
    LI_CHECK(map.Remove(StringView("beta")));
    LI_CHECK(map.Size() == 1);
}

LI_TEST(HashMap, LookupWithAViewDoesNotAllocate) {
    HashMap<String, int> map;
    map.Insert("a key long enough that a temporary string would need the heap", 1);
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    LI_CHECK(map.Contains(StringView("a key long enough that a temporary string would need the heap")));
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);
}

LI_TEST(HashMap, EnumAndPointerKeys) {
    HashMap<Color, int> colors;
    colors.Insert(Color::BLUE, 1);
    LI_CHECK(colors.Contains(Color::BLUE));
    LI_CHECK(!colors.Contains(Color::RED));

    int a = 0;
    int b = 0;
    HashMap<int*, int> pointers;
    pointers.Insert(&a, 1);
    LI_CHECK(pointers.Contains(&a));
    LI_CHECK(!pointers.Contains(&b));
}

LI_TEST(HashMap, IterationVisitsEveryEntryOnce) {
    HashMap<int, int> map;
    for(int i = 0; i < 100; ++i) {
        map.Insert(i, 1);
    }
    for(auto& [key, value] : map) {
        value += key;
    }

    int visited = 0;
    int keySum = 0;
    int valueSum = 0;
    const HashMap<int, int>& constant = map;
    for(const auto& [key, value] : constant) {
        ++visited;
        keySum += key;
        valueSum += value;
    }
    LI_CHECK(visited == 100);
    LI_CHECK(keySum == 4950);
    LI_CHECK(valueSum == 5050);
}

LI_TEST(HashMap, CopyIsIndependent) {
    HashMap<String, int> original;
    original.Insert("one", 1);
    HashMap<String, int> copy = original;
    copy.Insert("two", 2);
    *copy.Find(StringView("one")) = 9;
    LI_CHECK(original.Size() == 1);
    LI_CHECK(*original.Find(StringView("one")) == 1);

    HashMap<String, int> assigned;
    assigned.Insert("old", 0);
    assigned = copy;
    LI_CHECK(assigned.Size() == 2);
    LI_CHECK(!assigned.Contains(StringView("old")));
    LI_CHECK(*assigned.Find(StringView("one")) == 9);
}

LI_TEST(HashMap, CopyOfEmptyIsUsable) {
    const HashMap<int, int> empty;
    HashMap<int, int> copy = empty;
    LI_CHECK(copy.IsEmpty());
    copy.Insert(1, 1);
    LI_CHECK(copy.Contains(1));
}

LI_TEST(HashMap, MoveTransfersEntries) {
    HashMap<String, int> source;
    source.Insert("one", 1);
    HashMap<String, int> moved = std::move(source);
    LI_CHECK(source.IsEmpty());
    LI_CHECK(moved.Contains(StringView("one")));

    HashMap<String, int> assigned;
    assigned.Insert("old", 0);
    assigned = std::move(moved);
    LI_CHECK(moved.IsEmpty());
    LI_CHECK(assigned.Size() == 1);
    LI_CHECK(assigned.Contains(StringView("one")));

    source.Insert("reused", 2);
    LI_CHECK(source.Contains(StringView("reused")));
}

LI_TEST(HashMap, SelfAssignmentKeepsEntries) {
    HashMap<int, int> map;
    map.Insert(1, 10);
    HashMap<int, int>& alias = map;
    map = alias;
    LI_CHECK(map.Size() == 1);
    map = std::move(alias);
    LI_REQUIRE(map.Size() == 1);
    LI_CHECK(*map.Find(1) == 10);
}

LI_TEST(HashMap, ClearRemovesEverythingAndStaysUsable) {
    HashMap<int, int> map;
    for(int i = 0; i < 100; ++i) {
        map.Insert(i, i);
    }
    map.Clear();
    LI_CHECK(map.IsEmpty());
    LI_CHECK(!map.Contains(1));
    LI_CHECK(!(map.begin() != map.end()));
    map.Insert(1, 1);
    LI_CHECK(map.Contains(1));
}

LI_TEST(HashMap, ReservePreventsReallocation) {
    HashMap<int, int> map;
    map.Reserve(1000);
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    for(int i = 0; i < 1000; ++i) {
        map.Insert(i, i);
    }
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);
    LI_CHECK(map.Size() == 1000);
    map.Reserve(1);
    LI_CHECK(map.Contains(999));
}

LI_TEST(HashMap, EntriesAreDestroyedExactlyOnce) {
    LifetimeCounter::ResetCounts();
    {
        HashMap<int, LifetimeCounter> map;
        for(int i = 0; i < 100; ++i) {
            map.Insert(i, LifetimeCounter(i));
        }
        LI_CHECK(LifetimeCounter::Live == 100);
        map.Insert(0, LifetimeCounter(-1));
        LI_CHECK(LifetimeCounter::Live == 100);
        for(int i = 0; i < 50; ++i) {
            map.Remove(i);
        }
        LI_CHECK(LifetimeCounter::Live == 50);
        const HashMap<int, LifetimeCounter> copy = map;
        LI_CHECK(LifetimeCounter::Live == 100);
    }
    LI_CHECK(LifetimeCounter::Live == 0);
}

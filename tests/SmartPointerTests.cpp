#include "LiLib/Memory/SharedPtr.h"
#include "LiLib/Memory/UniquePtr.h"
#include "LiLib/Threading/Thread.h"
#include "TestFramework.h"
#include <atomic>
#include <type_traits>

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

static_assert(!std::is_copy_constructible_v<UniquePtr<int>>);
static_assert(std::is_nothrow_move_constructible_v<UniquePtr<int>>);
static_assert(std::is_nothrow_move_constructible_v<SharedPtr<int>>);
static_assert(std::is_nothrow_move_assignable_v<SharedPtr<int>>);
static_assert(std::is_convertible_v<SharedPtr<int>, SharedPtr<const int>>);
static_assert(!std::is_convertible_v<SharedPtr<const int>, SharedPtr<int>>);

struct IAnimal {
    virtual ~IAnimal() = default;
    virtual int Legs() const = 0;
};

class Dog final : public IAnimal {
public:
    explicit Dog(int& destroyed) : destroyed(destroyed) {}
    ~Dog() override { ++destroyed; }
    int Legs() const override { return 4; }

private:
    int& destroyed;
};

struct Node {
    int value = 0;
    SharedPtr<Node> next;
};

} // namespace

// UniquePtr

LI_TEST(UniquePtr, DefaultAndNullAreEmpty) {
    const UniquePtr<int> defaulted;
    const UniquePtr<int> fromNull = nullptr;
    LI_CHECK(!defaulted);
    LI_CHECK(!fromNull);
    LI_CHECK(defaulted.Get() == nullptr);
    LI_CHECK(defaulted == nullptr);
}

LI_TEST(UniquePtr, OwnsAndDestroysItsObject) {
    LifetimeCounter::ResetCounts();
    {
        const UniquePtr<LifetimeCounter> pointer = MakeUnique<LifetimeCounter>(3);
        LI_REQUIRE(pointer);
        LI_CHECK(pointer->value == 3);
        LI_CHECK((*pointer).value == 3);
        LI_CHECK(pointer != nullptr);
        LI_CHECK(LifetimeCounter::Live == 1);
    }
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(UniquePtr, MoveTransfersOwnership) {
    UniquePtr<int> source = MakeUnique<int>(5);
    const int* raw = source.Get();
    UniquePtr<int> moved = std::move(source);
    LI_CHECK(!source);
    LI_CHECK(moved.Get() == raw);

    UniquePtr<int> assigned;
    assigned = std::move(moved);
    LI_CHECK(!moved);
    LI_CHECK(assigned.Get() == raw);
}

LI_TEST(UniquePtr, ConvertsToBase) {
    int destroyed = 0;
    {
        const UniquePtr<IAnimal> animal = MakeUnique<Dog>(destroyed);
        LI_REQUIRE(animal);
        LI_CHECK(animal->Legs() == 4);
    }
    LI_CHECK(destroyed == 1);
}

LI_TEST(UniquePtr, ReleaseGivesUpOwnership) {
    LifetimeCounter::ResetCounts();
    UniquePtr<LifetimeCounter> pointer = MakeUnique<LifetimeCounter>(1);
    LifetimeCounter* raw = pointer.Release();
    LI_CHECK(!pointer);
    LI_CHECK(LifetimeCounter::Live == 1);
    Memory::Delete(raw);
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(UniquePtr, ResetDestroysTheOldObject) {
    LifetimeCounter::ResetCounts();
    UniquePtr<LifetimeCounter> pointer = MakeUnique<LifetimeCounter>(1);
    pointer.Reset(Memory::New<LifetimeCounter>(2));
    LI_CHECK(LifetimeCounter::Live == 1);
    LI_CHECK(pointer->value == 2);
    pointer.Reset();
    LI_CHECK(!pointer);
    LI_CHECK(LifetimeCounter::Live == 0);
    pointer = MakeUnique<LifetimeCounter>(3);
    pointer = nullptr;
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(UniquePtr, SelfMoveKeepsTheObject) {
    UniquePtr<int> pointer = MakeUnique<int>(5);
    UniquePtr<int>& alias = pointer;
    pointer = std::move(alias);
    LI_REQUIRE(pointer);
    LI_CHECK(*pointer == 5);
}

LI_TEST(UniquePtr, ComparesByAddress) {
    const UniquePtr<int> a = MakeUnique<int>(1);
    const UniquePtr<int> b = MakeUnique<int>(1);
    LI_CHECK(a == a);
    LI_CHECK(!(a == b));
}

// SharedPtr

LI_TEST(SharedPtr, DefaultAndNullAreEmpty) {
    const SharedPtr<int> defaulted;
    const SharedPtr<int> fromNull = nullptr;
    LI_CHECK(!defaulted);
    LI_CHECK(!fromNull);
    LI_CHECK(defaulted == nullptr);
    LI_CHECK(defaulted.UseCount() == 0);
}

LI_TEST(SharedPtr, CountsOwnersAndDestroysWithTheLast) {
    LifetimeCounter::ResetCounts();
    SharedPtr<LifetimeCounter> first = MakeShared<LifetimeCounter>(9);
    LI_CHECK(first.UseCount() == 1);
    {
        const SharedPtr<LifetimeCounter> second = first;
        LI_CHECK(first.UseCount() == 2);
        LI_CHECK(second->value == 9);
        LI_CHECK(second == first);
    }
    LI_CHECK(first.UseCount() == 1);
    LI_CHECK(LifetimeCounter::Live == 1);
    first.Reset();
    LI_CHECK(!first);
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(SharedPtr, MoveLeavesTheCountUnchanged) {
    SharedPtr<int> source = MakeShared<int>(1);
    const SharedPtr<int> moved = std::move(source);
    LI_CHECK(!source);
    LI_CHECK(moved.UseCount() == 1);
}

LI_TEST(SharedPtr, ConvertsToBaseAndDestroysTheRealType) {
    int destroyed = 0;
    {
        SharedPtr<IAnimal> animal;
        {
            const SharedPtr<Dog> dog = MakeShared<Dog>(destroyed);
            animal = dog;
            LI_CHECK(animal.UseCount() == 2);
        }
        LI_CHECK(destroyed == 0);
        LI_CHECK(animal->Legs() == 4);
    }
    LI_CHECK(destroyed == 1);
}

LI_TEST(SharedPtr, ConvertsToConst) {
    const SharedPtr<int> value = MakeShared<int>(5);
    const SharedPtr<const int> constant = value;
    LI_CHECK(*constant == 5);
    LI_CHECK(value.UseCount() == 2);
    LI_CHECK(constant == value);
}

LI_TEST(SharedPtr, AdoptTakesOwnership) {
    LifetimeCounter::ResetCounts();
    {
        const SharedPtr<LifetimeCounter> adopted = SharedPtr<LifetimeCounter>::Adopt(Memory::New<LifetimeCounter>(4));
        LI_CHECK(adopted->value == 4);
        LI_CHECK(adopted.UseCount() == 1);
    }
    LI_CHECK(LifetimeCounter::Live == 0);
    LI_CHECK(!SharedPtr<int>::Adopt(nullptr));
}

LI_TEST(SharedPtr, AssignmentReleasesTheOldObject) {
    LifetimeCounter::ResetCounts();
    SharedPtr<LifetimeCounter> a = MakeShared<LifetimeCounter>(1);
    const SharedPtr<LifetimeCounter> b = MakeShared<LifetimeCounter>(2);
    a = b;
    LI_CHECK(LifetimeCounter::Live == 1);
    LI_CHECK(a->value == 2);
    LI_CHECK(b.UseCount() == 2);
    a = nullptr;
    LI_CHECK(!a);
    LI_CHECK(b.UseCount() == 1);
}

LI_TEST(SharedPtr, SelfAssignmentKeepsTheObject) {
    SharedPtr<int> pointer = MakeShared<int>(5);
    SharedPtr<int>& alias = pointer;
    pointer = alias;
    LI_CHECK(pointer.UseCount() == 1);
    pointer = std::move(alias);
    LI_REQUIRE(pointer);
    LI_CHECK(pointer.UseCount() == 1);
    LI_CHECK(*pointer == 5);
}

LI_TEST(SharedPtr, AssignmentBetweenCopiesOfOneObject) {
    SharedPtr<int> a = MakeShared<int>(1);
    SharedPtr<int> b = a;
    a = b;
    LI_CHECK(a.UseCount() == 2);
    a = std::move(b);
    LI_CHECK(a.UseCount() == 1);
    LI_CHECK(!b);
}

// These read freed memory if the old reference is released first; the AddressSanitizer build detects that
LI_TEST(SharedPtr, CopyAssignFromInsideTheReleasedObject) {
    SharedPtr<Node> head = MakeShared<Node>();
    head->next = MakeShared<Node>();
    head->next->value = 2;
    head = head->next;
    LI_CHECK(head->value == 2);
    LI_CHECK(head.UseCount() == 1);
}

LI_TEST(SharedPtr, MoveAssignFromInsideTheReleasedObject) {
    SharedPtr<Node> head = MakeShared<Node>();
    head->next = MakeShared<Node>();
    head->next->value = 2;
    head = std::move(head->next);
    LI_CHECK(head->value == 2);
    LI_CHECK(head.UseCount() == 1);
}

LI_TEST(SharedPtr, WalkingAList) {
    SharedPtr<Node> head = MakeShared<Node>();
    SharedPtr<Node> tail = head;
    for(int i = 1; i < 100; ++i) {
        tail->next = MakeShared<Node>();
        tail->next->value = i;
        tail = tail->next;
    }
    tail.Reset();

    int visited = 0;
    while(head) {
        LI_CHECK(head->value == visited);
        head = head->next;
        ++visited;
    }
    LI_CHECK(visited == 100);
}

LI_TEST(SharedPtr, ResetWhenTheObjectOwnsThePointer) {
    Node* raw = nullptr;
    {
        const SharedPtr<Node> outer = MakeShared<Node>();
        raw = outer.Get();
        raw->next = outer;
    }
    raw->next.Reset();
}

LI_TEST(SharedPtr, AssignNullWhenTheObjectOwnsThePointer) {
    Node* raw = nullptr;
    {
        const SharedPtr<Node> outer = MakeShared<Node>();
        raw = outer.Get();
        raw->next = outer;
    }
    raw->next = nullptr;
}

// Every thread copies one object, so a non-atomic count loses updates
LI_TEST(SharedPtr, CountStaysExactUnderContention) {
    constexpr int ThreadCount = 8;
    int destroyed = 0;
    SharedPtr<IAnimal> source = MakeShared<Dog>(destroyed);
    {
        Thread threads[ThreadCount];
        for(Thread& thread : threads) {
            const SharedPtr<IAnimal> local = source;
            LI_REQUIRE(thread.Start([local]() {
                int legs = 0;
                for(int i = 0; i < 300000; ++i) {
                    const SharedPtr<IAnimal> copy = local;
                    legs += copy->Legs();
                }
                return legs == 300000 * 4 ? 0 : 1;
            }));
        }
        for(Thread& thread : threads) {
            int exitCode = -1;
            LI_CHECK(thread.Join(&exitCode));
            LI_CHECK(exitCode == 0);
        }
    }
    LI_CHECK(source.UseCount() == 1);
    LI_CHECK(destroyed == 0);
    source.Reset();
    LI_CHECK(destroyed == 1);
}

// Owners let go at the same moment, so exactly one of them must destroy the object
LI_TEST(SharedPtr, LastOwnerRaceDestroysOnce) {
    constexpr int ThreadCount = 8;
    static constexpr int Rounds = 5000;

    class CountedAtomically {
    public:
        explicit CountedAtomically(std::atomic_int& destroyed) : destroyed(destroyed) {}
        ~CountedAtomically() { destroyed.fetch_add(1, std::memory_order_relaxed); }

    private:
        std::atomic_int& destroyed;
    };

    std::atomic_int destroyed{0};
    std::atomic_int round{-1};
    std::atomic_int finished{0};
    SharedPtr<CountedAtomically> slots[ThreadCount];
    Thread threads[ThreadCount];
    for(int t = 0; t < ThreadCount; ++t) {
        SharedPtr<CountedAtomically>* slot = &slots[t];
        LI_REQUIRE(threads[t].Start([slot, &round, &finished]() {
            for(int r = 0; r < Rounds; ++r) {
                while(round.load(std::memory_order_acquire) < r) {
                }
                slot->Reset();
                finished.fetch_add(1, std::memory_order_acq_rel);
            }
            return 0;
        }));
    }
    for(int r = 0; r < Rounds; ++r) {
        SharedPtr<CountedAtomically> owner = MakeShared<CountedAtomically>(destroyed);
        for(SharedPtr<CountedAtomically>& slot : slots) {
            slot = owner;
        }
        owner.Reset();
        finished.store(0, std::memory_order_release);
        round.store(r, std::memory_order_release);
        while(finished.load(std::memory_order_acquire) < ThreadCount) {
        }
    }
    for(Thread& thread : threads) {
        LI_CHECK(thread.Join());
    }
    LI_CHECK(destroyed.load() == Rounds);
}

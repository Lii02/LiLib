#include "LiLib/Memory/Memory.h"
#include "TestFramework.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>

using namespace LiLib;
using LiLibTest::LifetimeCounter;

namespace {

class ThrowsOnConstruction {
public:
    ThrowsOnConstruction() { throw std::runtime_error("construction failed"); }
};

class FirstBase {
public:
    virtual ~FirstBase() = default;
    int first = 1;
};

class SecondBase {
public:
    virtual ~SecondBase() = default;
    int second = 2;
};

// The second base sits at an offset inside the object, so deleting through it needs the real start address
class Derived final : public FirstBase, public SecondBase {
public:
    explicit Derived(int& destroyed) : destroyed(destroyed) {}
    ~Derived() override { ++destroyed; }

private:
    int& destroyed;
};

} // namespace

LI_TEST(Memory, AllocateReturnsWritableMemory) {
    void* block = Memory::Allocate(128);
    LI_REQUIRE(block != nullptr);
    std::memset(block, 0xAB, 128);
    LI_CHECK(Memory::GetSize(block) == 128);
    Memory::Free(block);
}

LI_TEST(Memory, AllocateZeroBytesReturnsAFreeableBlock) {
    void* block = Memory::Allocate(0);
    LI_CHECK(block != nullptr);
    LI_CHECK(Memory::GetSize(block) == 0);
    Memory::Free(block);
}

LI_TEST(Memory, AllocateRespectsAlignment) {
    for(size_t alignment = 1; alignment <= 4096; alignment *= 2) {
        void* block = Memory::Allocate(33, alignment);
        LI_REQUIRE(block != nullptr);
        LI_CHECK(reinterpret_cast<uintptr_t>(block) % alignment == 0);
        LI_CHECK(Memory::GetSize(block) == 33);
        std::memset(block, 0xCD, 33);
        Memory::Free(block);
    }
}

LI_TEST(Memory, DefaultAlignmentFitsAnyType) {
    void* block = Memory::Allocate(1);
    LI_REQUIRE(block != nullptr);
    LI_CHECK(reinterpret_cast<uintptr_t>(block) % alignof(std::max_align_t) == 0);
    Memory::Free(block);
}

LI_TEST(Memory, AllocateReturnsNullWhenTheSizeWouldOverflow) {
    const uint64_t before = Memory::GetStats().currentAllocations;
    LI_CHECK(Memory::Allocate(SIZE_MAX) == nullptr);
    LI_CHECK(Memory::Allocate(SIZE_MAX, 4096) == nullptr);
    LI_CHECK(Memory::GetStats().currentAllocations == before);
}

LI_TEST(Memory, NullIsIgnored) {
    Memory::Free(nullptr);
    Memory::Delete(static_cast<int*>(nullptr));
    LI_CHECK(Memory::GetSize(nullptr) == 0);
}

LI_TEST(Memory, StatsTrackAllocations) {
    const MemoryStats start = Memory::GetStats();
    void* block = Memory::Allocate(1000);
    const MemoryStats during = Memory::GetStats();
    LI_CHECK(during.currentBytes == start.currentBytes + 1000);
    LI_CHECK(during.totalBytes == start.totalBytes + 1000);
    LI_CHECK(during.currentAllocations == start.currentAllocations + 1);
    LI_CHECK(during.totalAllocations == start.totalAllocations + 1);
    LI_CHECK(during.peakBytes >= during.currentBytes);

    Memory::Free(block);
    const MemoryStats end = Memory::GetStats();
    LI_CHECK(end.currentBytes == start.currentBytes);
    LI_CHECK(end.currentAllocations == start.currentAllocations);
    // Totals and the peak never go down
    LI_CHECK(end.totalBytes == during.totalBytes);
    LI_CHECK(end.totalAllocations == during.totalAllocations);
    LI_CHECK(end.peakBytes >= during.currentBytes);
}

LI_TEST(Memory, NewForwardsArgumentsAndDeleteDestroys) {
    LifetimeCounter::ResetCounts();
    LifetimeCounter* counter = Memory::New<LifetimeCounter>(7);
    LI_REQUIRE(counter != nullptr);
    LI_CHECK(counter->value == 7);
    LI_CHECK(LifetimeCounter::Live == 1);
    Memory::Delete(counter);
    LI_CHECK(LifetimeCounter::Live == 0);
}

LI_TEST(Memory, NewRespectsOverAlignedTypes) {
    struct alignas(256) OverAligned {
        int value = 3;
    };
    OverAligned* object = Memory::New<OverAligned>();
    LI_CHECK(reinterpret_cast<uintptr_t>(object) % 256 == 0);
    LI_CHECK(object->value == 3);
    Memory::Delete(object);
}

LI_TEST(Memory, NewFreesTheBlockWhenTheConstructorThrows) {
    const uint64_t before = Memory::GetStats().currentAllocations;
    bool caught = false;
    try {
        Memory::New<ThrowsOnConstruction>();
    } catch(const std::runtime_error&) {
        caught = true;
    }
    LI_CHECK(caught);
    LI_CHECK(Memory::GetStats().currentAllocations == before);
}

LI_TEST(Memory, DeleteThroughABaseAtAnOffsetFreesTheWholeObject) {
    int destroyed = 0;
    Derived* derived = Memory::New<Derived>(destroyed);
    SecondBase* base = derived;
    // Guards the premise of this test: the base pointer must really be offset
    LI_CHECK(static_cast<void*>(base) != static_cast<void*>(derived));
    Memory::Delete(base);
    LI_CHECK(destroyed == 1);
}

LI_TEST(Memory, DeleteAcceptsPointerToConst) {
    const int* value = Memory::New<int>(5);
    LI_CHECK(*value == 5);
    Memory::Delete(value);
}

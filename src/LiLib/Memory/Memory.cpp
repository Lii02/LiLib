#include "LiLib/Memory/Memory.h"
#include "LiLib/Core/Assert.h"
#include <cstdint>
#include <cstdlib>

namespace LiLib {

namespace {

/*
 * @brief Stored immediately before every block handed out by Memory::Allocate.
 */
struct alignas(std::max_align_t) AllocationHeader {
    size_t size;
    // Distance from the start of the underlying malloc block to the user pointer
    size_t offset;
};

AllocationHeader* GetHeader(const void* ptr) {
    return reinterpret_cast<AllocationHeader*>(const_cast<void*>(ptr)) - 1;
}

} // namespace

// Constant-initialized, so they are usable by operator new before any dynamic initialization runs
std::atomic_uint64_t Memory::CurrentBytes{0};
std::atomic_uint64_t Memory::PeakBytes{0};
std::atomic_uint64_t Memory::TotalBytes{0};
std::atomic_uint64_t Memory::CurrentAllocations{0};
std::atomic_uint64_t Memory::TotalAllocations{0};

void* Memory::Allocate(size_t size, size_t alignment) {
    LI_ASSERT(alignment != 0 && (alignment & (alignment - 1)) == 0, "Memory alignment must be a power of two");
    if(alignment < DefaultAlignment) {
        alignment = DefaultAlignment;
    }

    // malloc already returns DefaultAlignment, so only stricter alignments need room to shift the block
    const size_t extra = sizeof(AllocationHeader) + (alignment - DefaultAlignment);
    if(size > SIZE_MAX - extra) {
        return nullptr;
    }
    void* base = std::malloc(size + extra);
    if(base == nullptr) {
        return nullptr;
    }

    const uintptr_t baseAddress = reinterpret_cast<uintptr_t>(base);
    const uintptr_t userAddress = (baseAddress + sizeof(AllocationHeader) + alignment - 1) & ~(static_cast<uintptr_t>(alignment) - 1);
    void* user = reinterpret_cast<void*>(userAddress);
    AllocationHeader* header = GetHeader(user);
    header->size = size;
    header->offset = static_cast<size_t>(userAddress - baseAddress);
    _RecordAllocation(size);
    return user;
}

void Memory::Free(void* ptr) {
    if(ptr == nullptr) {
        return;
    }
    const AllocationHeader* header = GetHeader(ptr);
    _RecordFree(header->size);
    std::free(static_cast<byte_t*>(ptr) - header->offset);
}

size_t Memory::GetSize(const void* ptr) {
    return ptr == nullptr ? 0 : GetHeader(ptr)->size;
}

MemoryStats Memory::GetStats() {
    MemoryStats stats;
    stats.currentBytes = CurrentBytes.load(std::memory_order_relaxed);
    stats.peakBytes = PeakBytes.load(std::memory_order_relaxed);
    stats.totalBytes = TotalBytes.load(std::memory_order_relaxed);
    stats.currentAllocations = CurrentAllocations.load(std::memory_order_relaxed);
    stats.totalAllocations = TotalAllocations.load(std::memory_order_relaxed);
    return stats;
}

void Memory::_RecordAllocation(size_t size) {
    const uint64_t current = CurrentBytes.fetch_add(size, std::memory_order_relaxed) + size;
    TotalBytes.fetch_add(size, std::memory_order_relaxed);
    CurrentAllocations.fetch_add(1, std::memory_order_relaxed);
    TotalAllocations.fetch_add(1, std::memory_order_relaxed);

    uint64_t peak = PeakBytes.load(std::memory_order_relaxed);
    while(current > peak && !PeakBytes.compare_exchange_weak(peak, current, std::memory_order_relaxed)) {
    }
}

void Memory::_RecordFree(size_t size) {
    CurrentBytes.fetch_sub(size, std::memory_order_relaxed);
    CurrentAllocations.fetch_sub(1, std::memory_order_relaxed);
}

} // namespace LiLib

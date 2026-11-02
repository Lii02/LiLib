#pragma once
#include "LiLib/Core/Assert.h"
#include "LiLib/Core/Typedefs.h"
#include <atomic>
#include <cstddef>
#include <new>

namespace LiLib {

/*
 * @brief Snapshot of the tracked heap. Counts every allocation made through Memory.
 */
struct MemoryStats {
    uint64_t currentBytes = 0;
    uint64_t peakBytes = 0;
    uint64_t totalBytes = 0;
    uint64_t currentAllocations = 0;
    uint64_t totalAllocations = 0;
};

/*
 * @brief Tracked heap allocator. Each block carries a small header recording its size so frees
 *        can be accounted without the caller passing the size back. Thread-safe.
 */
class Memory {
public:
    static constexpr size_t DefaultAlignment = alignof(std::max_align_t);

    /*
     * @brief Allocates size bytes aligned to alignment (a power of two).
     * @returns nullptr when the allocation fails.
     */
    static void* Allocate(size_t size, size_t alignment = DefaultAlignment);

    /*
     * @brief Frees a block returned by Allocate. Null is ignored.
     */
    static void Free(void* ptr);

    /*
     * @brief Requested size of a block returned by Allocate.
     */
    static size_t GetSize(const void* ptr);

    /*
     * @brief Allocates and constructs a T. Pair with Delete.
     */
    template<typename T, typename... TArgs>
    static T* New(TArgs&&... args) {
        void* memory = Allocate(sizeof(T), alignof(T));
        LI_VERIFY(memory != nullptr, "Memory::New allocation failed");
        try {
            return new(memory) T(std::forward<TArgs>(args)...);
        } catch(...) {
            Free(memory);
            throw;
        }
    }

    /*
     * @brief Destroys and frees an object created by New. ptr may point to a base class of the
     *        allocated object, as long as the destructor is virtual.
     */
    template<typename T>
    static void Delete(T* ptr) {
        if(ptr == nullptr) {
            return;
        }
        // A base-class pointer can sit at an offset inside the allocation; free from the most-derived object
        void* memory = nullptr;
        if constexpr(std::is_polymorphic_v<T>) {
            memory = dynamic_cast<void*>(ptr);
        } else {
            memory = const_cast<void*>(static_cast<const void*>(ptr));
        }
        ptr->~T();
        Free(memory);
    }

    static MemoryStats GetStats();

private:
    static void _RecordAllocation(size_t size);
    static void _RecordFree(size_t size);

    static std::atomic_uint64_t CurrentBytes;
    static std::atomic_uint64_t PeakBytes;
    static std::atomic_uint64_t TotalBytes;
    static std::atomic_uint64_t CurrentAllocations;
    static std::atomic_uint64_t TotalAllocations;
};

} // namespace LiLib

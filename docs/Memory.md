# Memory

## Memory allocator

`#include "LiLib/Memory/Memory.h"`

All allocations in the library go through `Memory`, which wraps `malloc` and tracks usage. It's thread-safe.

| Function | Description |
|---|---|
| `Memory::Allocate(size, alignment)` | Allocates raw memory. Alignment is optional and must be a power of two. Returns `nullptr` on failure |
| `Memory::Free(ptr)` | Frees a block from `Allocate`. Null is ignored |
| `Memory::GetSize(ptr)` | Requested size of a block, or 0 for null |
| `Memory::New<T>(args...)` | Allocates and constructs a `T`. Aborts through `LI_VERIFY` if the allocation fails |
| `Memory::Delete(ptr)` | Destroys and frees an object from `New`. Null is ignored |
| `Memory::GetStats()` | Returns a `MemoryStats` snapshot |

`Memory::Delete` accepts a pointer to a base class, as long as the destructor is virtual. Prefer `MakeUnique` and `MakeShared` over calling `New` and `Delete` directly.

### Tracking usage

`MemoryStats` counts every allocation made through `Memory`. Comparing snapshots is an easy way to look for leaks:

```cpp
#include "LiLib/Memory/Memory.h"
#include "LiLib/Strings/String.h"
#include <cstdio>

const MemoryStats before = Memory::GetStats();
{
    const String text = "a string long enough to need a heap allocation";
}
const MemoryStats after = Memory::GetStats();
if(after.currentAllocations != before.currentAllocations) {
    std::printf("Leaked %llu allocations\n", static_cast<unsigned long long>(after.currentAllocations - before.currentAllocations));
}
```

| Field | Meaning |
|---|---|
| `currentBytes` | Bytes allocated right now |
| `peakBytes` | Highest value of `currentBytes` so far |
| `totalBytes` | Bytes ever allocated |
| `currentAllocations` | Live allocations right now |
| `totalAllocations` | Allocations ever made |

Sizes are what callers requested, so they don't include allocator overhead. Memory from plain `new` or `malloc` isn't counted.

## UniquePtr

`#include "LiLib/Memory/UniquePtr.h"`

A single-owner smart pointer. It can be moved but not copied, and it deletes the object when it goes out of scope.

```cpp
#include "LiLib/Memory/UniquePtr.h"

class IShape {
public:
    virtual ~IShape() = default;
    virtual int Sides() const = 0;
};

class Square : public IShape {
public:
    int Sides() const override { return 4; }
};

int main() {
    UniquePtr<IShape> shape = MakeUnique<Square>();
    const int sides = shape->Sides();

    UniquePtr<IShape> moved = std::move(shape);
    moved.Reset();
    return sides == 4 && !moved ? 0 : 1;
}
```

| Member | Description |
|---|---|
| `Get()` | Returns the raw pointer without giving up ownership |
| `Release()` | Gives up ownership and returns the raw pointer without deleting |
| `Reset(newPtr)` | Deletes the current object and optionally takes a new one |

Pointers passed to the constructor or `Reset` must come from `Memory::New`.

## SharedPtr

`#include "LiLib/Memory/SharedPtr.h"`

A reference-counted smart pointer. The object is deleted when the last copy goes away.

```cpp
#include "LiLib/Memory/SharedPtr.h"
#include "LiLib/Strings/String.h"

SharedPtr<String> first = MakeShared<String>("shared");
SharedPtr<String> second = first;
const int32_t owners = first.UseCount(); // 2
second.Reset();
LI_UNUSED(owners);
```

| Member | Description |
|---|---|
| `Get()` | Returns the raw pointer |
| `UseCount()` | Number of owners, or 0 when empty. Approximate while other threads hold copies |
| `Reset()` | Gives up this copy's ownership |
| `SharedPtr<T>::Adopt(ptr)` | Takes ownership of an object from `Memory::New`. Prefer `MakeShared` |

There's no `WeakPtr`, so reference cycles leak.

### Thread safety

The reference count is atomic, with the same guarantees as `std::shared_ptr`:

| Situation | Safe? |
|---|---|
| Different `SharedPtr` instances on different threads, even if they share an object | Yes |
| Several threads reading one instance, including copying from it | Yes |
| One thread modifying an instance (assigning, moving, `Reset`) while another uses that same instance | No, this needs a lock |
| Several threads using the object it points to | Only if the object is thread-safe itself |

To share an object with a thread, give the thread its own copy:

```cpp
#include "LiLib/Memory/SharedPtr.h"
#include "LiLib/Threading/Thread.h"

SharedPtr<int> shared = MakeShared<int>(42);

Thread worker;
worker.Start([copy = shared]() { return *copy; });

int exitCode = 0;
worker.Join(&exitCode);
```

## Known limitations

These don't compile at the moment:

- `UniquePtr<const T>` where `T` is polymorphic
- `MakeShared<const T>()`. Converting afterward works: `SharedPtr<const int> b = a;`

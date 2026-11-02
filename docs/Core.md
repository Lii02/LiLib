# Core

Foundational utilities used by the rest of the library.

## Assertions

`#include "LiLib/Core/Assert.h"`

| Macro | Debug builds | With `NDEBUG` defined |
|---|---|---|
| `LI_ASSERT(expression, message)` | Checks the expression | Removed; the expression isn't evaluated |
| `LI_VERIFY(expression, message)` | Checks the expression | Checks the expression |

When a check fails, the expression, message, file and line are printed to `stderr`. The program then triggers a debugger breakpoint and aborts.

```cpp
#include "LiLib/Core/Assert.h"

const int count = 3;
LI_ASSERT(count > 0, "Count must be positive");
LI_VERIFY(count < 10, "Count must be small");
```

Use `LI_ASSERT` for programmer errors, and `LI_VERIFY` for conditions that must hold in release builds too. Don't put side effects inside `LI_ASSERT`, because they disappear when `NDEBUG` is defined.

## Function

`#include "LiLib/Core/Function.h"`

A type-erased, copyable callable that replaces `std::function`.

```cpp
#include "LiLib/Core/Function.h"

const int base = 10;
Function<int(int)> add = [base](int value) { return value + base; };
const int result = add(5); // 15

Function<void()> empty;
if(!empty) {
    empty = [] {};
}
```

- The stored callable must be copyable.
- Every non-empty `Function` makes a heap allocation through `Memory`, and so does each copy.
- Calling an empty `Function` is caught by `LI_ASSERT` in debug builds and is undefined with `NDEBUG`.

## Types and utilities

`#include "LiLib/Core/Typedefs.h"`

| Name | Description |
|---|---|
| `byte_t` | `uint8_t` |
| `uuid_t` | `uint64_t` |
| `Pair<TFirst, TSecond>` | Two values, `first` and `second`. Supports structured bindings |
| `Min(a, b)`, `Max(a, b)` | Smaller or larger of two values of the same type |
| `Clamp(value, low, high)` | `value` limited to the range |
| `Swap(a, b)` | Swaps two values using moves |
| `LI_UNUSED(x)` | Marks a variable or parameter as intentionally unused |
| `LI_NON_COPYABLE(Type)` | Deletes the copy constructor and copy assignment |

```cpp
#include "LiLib/Core/Typedefs.h"

const Pair<int, float> pair{1, 2.0f};
const auto& [id, weight] = pair;
const int limited = Clamp(id + 41, 0, 10); // 10
LI_UNUSED(weight);
LI_UNUSED(limited);
```

`Min`, `Max` and `Clamp` deduce one type from all their arguments, so `Min(1, 2.0f)` doesn't compile. They also return references, so don't keep a reference to a result that came from a temporary.

## Hashing

`#include "LiLib/Core/Typedefs.h"`

| Name | Description |
|---|---|
| `HashBytes(data, size)` | 64-bit FNV-1a hash of a byte range |
| `HashInteger(value)` | SplitMix64 finalizer, which spreads integer values |
| `Hasher<T>::Hash(value)` | The hash function object used by `HashMap` |

`Hasher` is defined for integers, enums and pointers (by address). `String`, `StringView` and `const char*` hash by content. To use your own key type, see [Custom key types](Containers.md#custom-key-types).

These hashes aren't cryptographic and aren't protected against deliberately colliding keys.

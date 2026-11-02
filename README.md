# liLib

A small custom C++17 standard library: containers, strings, smart pointers, threading and math, built around a tracked heap allocator.

```cpp
#include "LiLib/Containers/HashMap.h"
#include "LiLib/Memory/UniquePtr.h"
#include "LiLib/Strings/String.h"
#include <cstdio>

using namespace LiLib;

int main() {
    HashMap<String, int> scores;
    scores.Insert("ada", 10);
    scores["grace"] = 20;

    for(const auto& [name, score] : scores) {
        const String line = String::Format("%s: %d", name.CStr(), score);
        std::printf("%s\n", line.CStr());
    }

    UniquePtr<String> owned = MakeUnique<String>("heap allocated");
    std::printf("%s\n", owned->CStr());
    return 0;
}
```

## What's included

| Module | Provides | Docs |
|---|---|---|
| `Core` | Assertions, `Function`, `Pair`, `Min`/`Max`/`Clamp`/`Swap`, hashing | [Core](docs/Core.md) |
| `Memory` | Tracked allocator, `UniquePtr`, `SharedPtr` | [Memory](docs/Memory.md) |
| `Containers` | `Array`, `HashMap` | [Containers](docs/Containers.md) |
| `Algorithms` | `StableSort` | [Algorithms](docs/Algorithms.md) |
| `Strings` | `String`, `StringView`, `Base64` | [Strings](docs/Strings.md) |
| `Threading` | `Thread`, `Mutex`, `LockGuard`, `ConditionVariable` | [Threading](docs/Threading.md) |
| `Math` | `Vec2`, `Vec3`, `Mat4`, `Rect`, `Color`, `Random`, `Math` | [Math](docs/Math.md) |

Everything is in the `LiLib` namespace, and macros are prefixed `LI_`.

The library replaces the standard containers, strings and smart pointers, but it isn't free of the C++ standard library. It uses `<type_traits>` and `<utility>`, and the threading module wraps `std::thread`, `std::recursive_mutex` and `std::condition_variable_any`.

## Requirements

- A C++17 compiler
- CMake 4.0 or newer
- Exceptions enabled, because `Memory::New` uses `try`/`catch`
- RTTI enabled, because `Memory::Delete` uses `dynamic_cast`. On Windows, a build with RTTI turned off (`/GR-` or `-fno-rtti`) still compiles, but aborted at runtime when tested

Built and tested on Windows with clang 23 and MSVC 19.51. Linux and macOS builds haven't been tested yet.

## Using it in a project

See [Getting Started](docs/GettingStarted.md) for details. In short, add the library as a subdirectory and link to it:

```cmake
add_subdirectory(path/to/liLib)
target_link_libraries(MyApp PRIVATE LiLib::LiLib)
```

Headers are then included by their full path, such as `#include "LiLib/Strings/String.h"`.

## Building

```sh
cmake -S . -B build
cmake --build build
```

This builds the static library. The repo doesn't have a test suite yet.

## Contributing

Code follows [CPP_CONVENTIONS.md](CPP_CONVENTIONS.md), which `.clang-format` and `.clang-tidy` enforce:

```sh
clang-format --dry-run --Werror src/LiLib/Math/Mat4.cpp   # Check formatting of a file
clang-tidy src/LiLib/Math/Mat4.cpp -p build               # Lint, using build/compile_commands.json
```

New `.cpp` and `.h` files anywhere under `src/` are picked up by the build automatically. Keep headers inside `src/LiLib/`, not directly in `src/`. On case-insensitive filesystems, a `src/String.h` would be picked up in place of the system `<string.h>`.

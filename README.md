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
| `Math` | `Vec2`, `Vec3`, `Vec4`, `Mat4`, `Quat`, `AABB`, `Frustum`, `Rect`, `Color`, `Random`, `Math` | [Math](docs/Math.md) |
| `IO` | `File` | [IO](docs/IO.md) |
| `Reflection` | Runtime type information, properties, methods, globals and variants | [Reflection](docs/Reflection.md) |

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

This builds the static library and, when liLib is the top-level project, the unit tests. To leave the tests out, configure with `-DLILIB_BUILD_TESTS=OFF`. They're off by default when liLib is added to another project as a subdirectory.

## Testing

```sh
ctest --test-dir build -C Debug --output-on-failure
```

The tests are in `tests/` and use a small built-in framework, described in `tests/TestFramework.h`. After each test, it also checks that everything allocated through `Memory` was freed. To run the test program directly, or to run only some of the tests:

```sh
build/tests/liLibTests                 # Run everything (Visual Studio builds are in build/tests/Debug/)
build/tests/liLibTests Array.          # Run tests whose "Suite.Name" contains the text
build/tests/liLibTests --list          # List all tests
```

Run the tests under AddressSanitizer as well when changing anything that manages memory, because some bugs only show up there:

```sh
cmake -S . -B build/asan -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=-fsanitize=address "-DCMAKE_CXX_FLAGS_RELWITHDEBINFO=-O0 -g"
cmake --build build/asan
ctest --test-dir build/asan --output-on-failure
```

- **Keep it at `-O0`:** with clang on Windows, AddressSanitizer at `-O1` or higher crashes on `throw;`, even in a program that doesn't use liLib.
- **Windows `PATH`:** the folder containing `clang_rt.asan_dynamic-x86_64.dll` must be on your `PATH`. It's in clang's `lib/clang/<version>/lib/windows` folder.

## Contributing

Code follows [CPP_CONVENTIONS.md](CPP_CONVENTIONS.md). `.clang-format` enforces the layout rules:

```sh
clang-format --dry-run --Werror src/LiLib/Math/Mat4.cpp   # Check formatting of a file
```

New `.cpp` and `.h` files anywhere under `src/` are picked up by the build automatically. Keep headers inside `src/LiLib/`, not directly in `src/`. On case-insensitive filesystems, a `src/String.h` would be picked up in place of the system `<string.h>`.

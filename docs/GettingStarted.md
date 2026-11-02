# Getting Started

## Add the library to your project

liLib builds as a static library. Add it to your CMake project as a subdirectory:

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp LANGUAGES CXX)

add_subdirectory(path/to/liLib)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE LiLib::LiLib)
```

Linking `LiLib::LiLib` also gives your target:

- The include path, so headers are found as `"LiLib/Module/Header.h"`
- A minimum language standard of C++17
- The platform's thread library

liLib's own sources always build as exactly C++17. Your target can use a newer standard, but the headers have only been tested under C++17.

## Include headers by full path

```cpp
#include "LiLib/Containers/Array.h"
#include "LiLib/Strings/String.h"

int main() {
    LiLib::Array<LiLib::String> names{"ada", "grace"};
    names.Push("linus");
    return 0;
}
```

Don't add `src/LiLib` or its subfolders as include paths. On case-insensitive filesystems, system headers such as `<string.h>` and `<memory.h>` could then resolve to liLib's `String.h` and `Memory.h`.

## Examples in these docs

Code examples assume `using namespace LiLib;` and leave out `main()` where it isn't needed.

## Where to go next

- [Core](Core.md): assertions, `Function`, `Pair`, utilities and hashing
- [Memory](Memory.md): the tracked allocator and smart pointers
- [Containers](Containers.md): `Array` and `HashMap`
- [Algorithms](Algorithms.md): `StableSort`
- [Strings](Strings.md): `String`, `StringView` and `Base64`
- [Threading](Threading.md): threads, mutexes and condition variables
- [Math](Math.md): vectors, matrices, colors and random numbers

# C++ Coding Conventions

These are my go-to conventions for C++ projects. The goal is readability and consistency, built on modern C++17.

Most of these rules are enforced automatically:

* [`.clang-format`](.clang-format) handles layout: indentation, braces, spacing, include order.
* [`.clang-tidy`](.clang-tidy) handles naming and common mistakes.

Rules marked **(review)** can't be checked by tools and are enforced in code review.

---

## 🛠️ Tooling

Drop both files in the root of a project. Most editors pick them up automatically (Visual Studio, VS Code with the C/C++ extension, CLion, Rider).

```sh
clang-format -i src/*.cpp src/*.h             # Format files in place
clang-format --dry-run --Werror src/*.cpp     # Check formatting (for CI)
clang-tidy src/*.cpp -p build                 # Lint, using build/compile_commands.json
```

* `.clang-format` requires **clang-format 16 or newer**.
* Naming violations are errors in clang-tidy. Everything else is a warning.

---

## 📁 Files

* **Extensions:** `.h` for headers, `.cpp` for source files.
* **File names** **(review)**: `PascalCase`, matching the main class they contain (`Quad` lives in `Quad.h` / `Quad.cpp`). Directories follow the same rule (`Engine/Core.h`).
* **One class per file pair.** Small helper types used only by that class may share its files.
* **Headers** declare interfaces and classes. **Source files** contain the implementations. Short one-line methods inside a class body are fine in headers.
* **Header guards:** Always use `#pragma once`.

### Includes

Includes form a single block with no blank lines. They're sorted alphabetically in this order:

1. The matching header (`Quad.cpp` includes `Quad.h` first)
2. Project headers in quotes
3. Third-party headers in angle brackets (`<glm/glm.h>`)
4. Standard library headers (`<vector>`)

```cpp
#include "Quad.h"
#include "Engine/Core.h"
#include <glm/glm.h>
#include <vector>
```

---

## 📐 Formatting

| Rule | Setting |
|-|-|
| Indentation | 4 spaces, no tabs |
| Line length | 140 columns |
| Braces | K&R: opening brace on the same line, always |
| Single-line blocks | Always use braces |
| Control keywords | No space before the parenthesis: `if(x)`, `for(...)`, `while(x)` |
| Pointers / references | Bind to the type: `int* p`, `const Quad& quad` |
| Namespaces | Contents are **not** indented; closing brace gets a `// namespace Name` comment |
| Access modifiers | Flush with the `class` keyword |
| Templates | `template<typename T>` on its own line |

### Braces

Curly braces share the line of the statement or declaration. No line is wasted.

```cpp
if(whatever) {
    // Code
} else {
    // Other code
}
```

Statements with a single line of code still use braces.

```cpp
if(whatever) {
    DoThing();
}
```

### Spacing

Binary operators, constants and identifiers are always separated by spaces.

```cpp
int total = count * factor; // Good
int total=count*factor;     // Bad
```

### Short Functions

One-line functions are allowed **only inside a class body**. Free functions and out-of-class definitions are always expanded.

```cpp
class Quad {
public:
    int GetId() const { return id; }
};

int Add(int a, int b) {
    return a + b;
}
```

### Namespaces

```cpp
namespace Render {

class Quad;

} // namespace Render
```

---

## 💬 Comments

Single-line comments have a space after the slashes.

```cpp
// This is a comment
```

Multi-line `/* */` comments are used only for documentation.

```cpp
/*
 * @brief Draws the quad to the active framebuffer.
 */
void Draw();
```

**(review)** Commented-out code is not allowed. Delete it, and version control will remember it.

```cpp
/* printf("Hello");
printf("World"); */ // Not allowed
```

---

## 🏷️ Naming

| Element | Convention | Example |
|-|-|-|
| Classes | `PascalCase` | `Quad`, `SystemManager` |
| Interfaces | `I` + `PascalCase`, declared as `struct` | `IWorker`, `IRenderable` |
| Structs (plain data) | `PascalCase` | `Vec2`, `VertexData` |
| Enums | `PascalCase`, using `enum class` | `RenderMode` |
| Enum values | `SCREAMING_SNAKE_CASE` | `FORWARD`, `DEFERRED` |
| Typedefs / `using` aliases | `camelCase_t` | `byte_t`, `index_t` |
| Template parameters | `T` or `PascalCase` | `T`, `TValue` |
| Namespaces | `PascalCase` | `Render`, `Engine` |
| Functions / methods | `PascalCase` | `GetUserName()`, `Draw()` |
| Private methods | `_` + `PascalCase` | `_RebuildBuffers()` |
| Local / member variables | `camelCase` | `vertexCount` |
| Parameters | `camelCase` | `newId` |
| Static member variables | `PascalCase` | `static int InstanceCount;` |
| `constexpr` / namespace or class-scope constants | `PascalCase` | `constexpr int MaxLights = 8;` |
| Local `const` variables | `camelCase` | `const int doubled = value * 2;` |
| Macros (`#define`) | `SCREAMING_SNAKE_CASE` | `#define TEST_DEF 0x17` |
| Function macros | `SCREAMING_SNAKE_CASE` or `camelCase` | `#define LOG_ERROR(x)`, `#define testMacro(x)` |

### Notes

* **Getters** may start with `Get`. **Setters** must start with `Set`.
* **Member variables** have no prefix. Use `this->` when a parameter shadows a member.
* **Private methods:** names starting with `_` and a capital letter are technically reserved by the C++ standard. This is a deliberate choice; it works on MSVC, GCC and Clang. (clang-tidy's `bugprone-reserved-identifier` is disabled for this reason.)
* **Interfaces:** clang-tidy treats any abstract type (one with a pure virtual method) as an interface and requires the `I` prefix.

---

## 🏗️ Language Elements

### Classes

The `public` section comes first, followed by `protected`, then `private`.

```cpp
class Quad {
public:
    Quad();
    ~Quad();

    int GetId() const { return id; }
    void SetId(int newId);

    static int InstanceCount;

private:
    void _RebuildBuffers();

    int id = 0;
};
```

### Interfaces

Interfaces are `struct`s that start with `I` and contain only pure virtual methods and a virtual destructor.

```cpp
struct IWorker {
    virtual ~IWorker() = default;
    virtual void Run() = 0;
};
```

### Structs

Structs are lightweight data bundles with **no methods** **(review)**.

```cpp
struct Vec2 {
    int x;
    int y;
};
```

### Enums

Always use `enum class`. Specifying the underlying type and values is optional. If a plain `enum` is unavoidable (for example, a C API), its values must be unique across the program.

```cpp
enum class RenderMode : int {
    FORWARD = 0,
    DEFERRED // Automatically increments from the previous value
};
```

### Typedefs

`typedef` and `using` are both fine. Names are `camelCase` with a `_t` suffix.

```cpp
typedef unsigned char byte_t;
using index_t = unsigned int;
```

### Const

* **Encouraged:** Mark anything that doesn't change as `const`, including variables, parameters passed by reference, and methods.
* **Const methods:** Methods that don't modify object state must be `const`. If a method needs to modify state, write a separate non-const method, or mark the member `mutable` when it's an implementation detail such as a cache.
* **Prefer `constexpr` over `#define`** for constant values.

```cpp
constexpr int MaxLights = 8; // Preferred
#define MAX_LIGHTS 8         // Avoid for plain values
```

### Casting **(review)**

Use the C++ named casts. C-style casts `(int)x` and functional casts `int(x)` are **not allowed**: they hide which conversion is happening and are hard to search for.

* **`static_cast`** for ordinary conversions (numeric types, enums, up/down casts with a known type). This should cover almost every cast.
* **`dynamic_cast`** only when the runtime type is genuinely unknown. Prefer virtual methods where possible.
* **`const_cast`** and **`reinterpret_cast`** need a comment explaining why, and a justification in the code review.

```cpp
int index = static_cast<int>(value);               // Good
int mode = static_cast<int>(RenderMode::DEFERRED); // Good
int index = (int)value;                            // Not allowed
int index = int(value);                            // Not allowed
```

### Macros

Use macros only when something can't be done with `constexpr`, templates or inline functions, such as logging with `__FILE__` and `__LINE__`, or platform switches.

```cpp
#define TEST_DEF 0x17
#define testMacro(x) Func(x)
#define TEST_MACRO2(x) Func(x)
```

---

## 🧠 Memory

* **No raw owning pointers.** Ownership is expressed with smart pointers, either the standard ones (`std::unique_ptr`, `std::shared_ptr`) or the project's own smart pointer types.
* **No manual `new` / `delete` or `malloc` / `free`.** Use `std::make_unique` / `std::make_shared`, or the project's equivalent factory.
* Raw pointers and references are fine when they **don't own** the object.

```cpp
std::unique_ptr<Quad> quad = std::make_unique<Quad>(); // Owns
void Draw(const Quad& quad);                           // Borrows
Quad* FindQuad(int id);                                // Borrows, may be null
```

---

## 🚫 Forbidden

### `auto` **(review)**

`auto` is **forbidden**, with these exceptions:

* **Lambdas**, whose type can't be written out.
* **Iterators**, where the full type is long and adds nothing.
* **Structured bindings**, which require `auto`.

```cpp
auto onClick = [this](int button) { _HandleClick(button); }; // OK
for(auto it = quads.begin(); it != quads.end(); ++it) {      // OK
}
for(const auto& [id, quad] : quadsById) {                     // OK
}

auto count = quads.size(); // Not allowed: write size_t
```

Any other use needs a justification in the code review.

### Global Variables

Mutable global variables are **not allowed** (clang-tidy warns). Use singletons for true global services, static class members, or pass dependencies in.

```cpp
class System {
public:
    static int Whatever;
};
```

Global `constexpr` / `const` values are fine.

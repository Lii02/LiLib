# Containers

## Array

`#include "LiLib/Containers/Array.h"`

A growable contiguous array that replaces `std::vector`.

```cpp
#include "LiLib/Containers/Array.h"

Array<int> values{3, 1, 2};
values.Push(4);
values.Insert(0, 9);       // 9 3 1 2 4
values.RemoveAt(1);        // 9 1 2 4
values.RemoveSwap(0);      // 4 1 2

int total = 0;
for(const int value : values) {
    total += value;
}

values.RemoveIf([](int value) { return value > 3; }); // 1 2
```

### Size and storage

| Member | Description |
|---|---|
| `Size()`, `Capacity()`, `IsEmpty()` | Current state |
| `Data()` | Pointer to the first element |
| `Reserve(capacity)` | Grows storage without changing the size |
| `Resize(size)`, `Resize(size, fill)` | Changes the size, default-constructing or copying new elements |
| `Clear()` | Destroys all elements and keeps the storage |

### Access

| Member | Description |
|---|---|
| `operator[]` | Element by index. Checked by `LI_ASSERT` only |
| `Front()`, `Back()` | First or last element |
| `IndexOf(value)` | Index of the first match, or `Array<T>::NotFound` (-1). Needs `==` |
| `Contains(value)` | Whether a match exists. Needs `==` |
| `FindIf(predicate)` | Pointer to the first match, or `nullptr`. Non-const arrays only |

### Adding and removing

| Member | Description |
|---|---|
| `Push(value)` | Appends a copy or moved value |
| `Emplace(args...)` | Constructs a new element in place at the end |
| `Insert(index, value)` | Inserts at an index, shifting later elements. O(n) |
| `Pop()` | Removes the last element |
| `RemoveAt(index)` | Removes an element and keeps the order. O(n) |
| `RemoveSwap(index)` | Moves the last element into the gap. O(1), but changes the order |
| `Remove(value)` | Removes the first match and returns whether it found one. Needs `==` |
| `RemoveIf(predicate)` | Removes all matches and returns how many |

Anything that grows the array can reallocate it, which invalidates pointers, references and iterators to its elements. Passing one of the array's own elements to `Push` is safe.

## HashMap

`#include "LiLib/Containers/HashMap.h"`

An open-addressing hash map that replaces `std::unordered_map`. Iteration order is unspecified.

```cpp
#include "LiLib/Containers/HashMap.h"
#include "LiLib/Strings/String.h"

HashMap<String, int> ages;
ages.Insert("ada", 36);
ages["grace"] = 85;

// Find returns a pointer, or nullptr when the key is missing
if(int* age = ages.Find(StringView("ada"))) {
    *age += 1;
}

for(const auto& [name, age] : ages) {
    LI_UNUSED(name);
    LI_UNUSED(age);
}

ages.Remove(StringView("grace"));
```

| Member | Description |
|---|---|
| `Size()`, `IsEmpty()` | Current state |
| `Insert(key, value)` | Inserts, or replaces the value if the key exists. Returns a reference to the value |
| `operator[](key)` | Returns the value, default-constructing it if the key is missing |
| `Find(key)` | Pointer to the value, or `nullptr` |
| `Contains(key)` | Whether the key exists |
| `Remove(key)` | Removes the entry and returns whether it existed |
| `Reserve(count)` | Prepares for `count` entries, avoiding repeated rehashing |
| `Clear()` | Removes all entries and keeps the storage |

`Find`, `Contains` and `Remove` accept any type that hashes the same as the key and compares equal to it. This lets a `HashMap<String, T>` be searched with a `StringView` without allocating a `String`.

Any `Insert` or `operator[]` call that adds a key can rehash the map, and `Remove` shifts entries. Both invalidate pointers, references and iterators into the map, so don't add or remove entries while iterating.

### Custom key types

Specialize `Hasher` inside the `LiLib` namespace, and give the key an `operator==`:

```cpp
#include "LiLib/Containers/HashMap.h"

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

namespace LiLib {

template<>
class Hasher<Point> {
public:
    static uint64_t Hash(const Point& point) { return HashBytes(&point, sizeof(point)); }
};

} // namespace LiLib

int main() {
    LiLib::HashMap<Point, int> tiles;
    tiles.Insert(Point{1, 2}, 5);
    return *tiles.Find(Point{1, 2}) == 5 ? 0 : 1;
}
```

`HashBytes` on a whole struct is only safe when the type has no padding bytes. Otherwise, combine hashes of the individual fields.

### Known limitations

- **String literals don't compile in `Find`, `Contains` and `Remove`.** `map.Find("key")` fails because there's no `Hasher` for `char[N]`. Wrap the literal as `StringView("key")`, as in the example above. `Insert` and `operator[]` accept literals directly.
- **Non-const `char*` lookups hash by address.** Only `const char*` hashes by content, so a `char*` won't find a matching `String` key. Convert it to `const char*` or `StringView` first.

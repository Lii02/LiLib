# Algorithms

## StableSort

`#include "LiLib/Algorithms/Sort.h"`

Sorts an `Array` in place. Elements that compare equal keep their original order.

```cpp
#include "LiLib/Algorithms/Sort.h"

Array<int> values{3, 1, 2};
StableSort(values); // 1 2 3

StableSort(values, [](int a, int b) { return a > b; }); // 3 2 1
```

| Overload | Ordering |
|---|---|
| `StableSort(values)` | Uses `operator<` |
| `StableSort(values, less)` | `less(a, b)` returns true when `a` must come before `b` |

The comparison must be a strict ordering, so it returns false for equal elements. Using `<=` would break the stable ordering.

It uses insertion sort on runs of 16 elements, then merges them, which takes O(n log n) time. Arrays with more than 16 elements allocate a temporary buffer of the same size.

Elements must be copyable as well as movable, so an `Array<UniquePtr<T>>` can't be sorted directly.

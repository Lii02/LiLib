# Strings

## StringView

`#include "LiLib/Strings/StringView.h"`

A non-owning view over a run of characters. Copying one is cheap, so pass it by value. The text it points to must outlive the view, and it isn't necessarily null-terminated, so build a `String` when you need a C string.

```cpp
#include "LiLib/Strings/StringView.h"

const StringView path = "  assets/textures/grass.png  ";
const StringView trimmed = path.Trimmed();
const size_t slash = trimmed.FindLast('/');
const StringView file = trimmed.Substr(slash + 1); // grass.png

if(file.EndsWith(".png") && file.EqualsIgnoreCase("GRASS.PNG")) {
    LI_UNUSED(file);
}
```

| Member | Description |
|---|---|
| `Data()`, `Length()`, `IsEmpty()` | Current state |
| `operator[]` | Character by index. Unchecked |
| `Substr(start, count)` | Part of the view. Out-of-range values are limited to the view |
| `Find(c, start)`, `Find(text, start)` | First position, or `StringView::NPos` |
| `FindLast(c)` | Last position, or `StringView::NPos` |
| `StartsWith(text)`, `EndsWith(text)` | Prefix and suffix checks |
| `Equals(other)`, `EqualsIgnoreCase(other)` | Comparison. Ignoring case only covers ASCII |
| `Trimmed()` | View without leading or trailing spaces, tabs and line breaks |

## String

`#include "LiLib/Strings/String.h"`

An owning, null-terminated, growable string. Strings of up to 23 characters are stored inline without a heap allocation.

```cpp
#include "LiLib/Strings/String.h"

String text = "Hello";
text += ", ";
text.Append("World");
text.AppendFormat(" %d", 42);

const String lower = text.ToLower();              // hello, world 42
const String replaced = text.Replace("World", "liLib"); // Hello, liLib 42
const Array<String> parts = text.Split(' ');      // "Hello," "World" "42"

const int64_t number = parts[2].ToInt();          // 42
const String formatted = String::Format("%s has %d parts", "text", 3);
```

All the read-only `StringView` searches are also on `String`: `Find`, `FindLast`, `StartsWith`, `EndsWith` and `Contains`.

| Member | Description |
|---|---|
| `CStr()` | Null-terminated C string |
| `Data()`, `Length()`, `IsEmpty()` | Current state |
| `View()` | A `StringView` of the string. `String` also converts to `StringView` automatically |
| `operator[]` | Character by index. Unchecked |
| `Append(text)`, `Append(c)`, `+=` | Adds to the end. Appending a view of the string itself is safe |
| `AppendFormat(format, ...)` | Adds `printf`-style formatted text |
| `Clear()`, `Reserve(capacity)`, `Resize(length, fill)` | Storage control |
| `Substr(start, count)`, `Trimmed()`, `ToLower()`, `Replace(from, to)` | Return a new string |
| `Split(separator, keepEmpty)` | Splits into an `Array<String>`. Empty parts are dropped unless `keepEmpty` is true |
| `ToInt(fallback)`, `ToDouble(fallback)` | Parses a number |
| `String::Format(format, ...)`, `String::FormatV(format, args)` | `printf`-style construction |

- **Parsing:** `ToInt` and `ToDouble` read the number at the start of the string and ignore what follows, so `"12abc"` gives 12. They return the fallback only if no number is found at the start.
- **Case:** `ToLower` only changes ASCII letters.
- **Formatting:** format strings aren't checked at compile time, so arguments must match the format exactly.

## Base64

`#include "LiLib/Strings/Base64.h"`

Standard base64 (RFC 4648).

```cpp
#include "LiLib/Strings/Base64.h"

const char message[] = "any carnal pleas";
const String encoded = Base64::Encode(message, sizeof(message) - 1); // YW55IGNhcm5hbCBwbGVhcw==

Array<byte_t> decoded;
if(!Base64::Decode(encoded, decoded)) {
    LI_VERIFY(false, "Invalid base64");
}
```

`Decode` skips whitespace and doesn't require padding. It returns false and leaves the output empty if the text contains invalid characters or has an invalid length.

## Known limitations

`view == string` doesn't compile because the overloads are ambiguous. Write `string == view` or `view == string.View()` instead.

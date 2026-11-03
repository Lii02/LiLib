# IO

## File

`#include "LiLib/IO/File.h"`

Whole-file reads and writes, and simple file queries. Paths are UTF-8 on every platform.

```cpp
#include "LiLib/IO/File.h"

String config;
if(File::ReadText("settings.txt", config)) {
    config.Append("\nvolume = 3");
    File::WriteText("settings.txt", config);
}

File::CreateDirectories("saves/slot1");
```

| Function | Description |
|---|---|
| `ReadBytes(path, outBytes)` | Reads the whole file into an `Array<byte_t>` |
| `ReadText(path, outText)` | Reads the whole file into a `String`, with no line-ending conversion |
| `WriteBytes(path, data, size)` | Replaces the file's contents, creating it if needed |
| `WriteText(path, text)` | Same, from a `StringView` |
| `Exists(path)` | Whether a file or folder exists |
| `GetModifiedTime(path, outTime)` | Last-modified time in seconds since an unspecified, fixed point |
| `CreateDirectories(path)` | Creates a folder and any missing parents. Succeeds if it already exists |

Every function returns `false` on failure. A failed read leaves its output untouched.

- **Writing:** the parent folder must already exist.
- **Trailing slashes:** Windows reports a path with a trailing slash as missing, so leave it off when calling `Exists`.

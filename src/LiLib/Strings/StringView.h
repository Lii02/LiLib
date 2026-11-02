#pragma once
#include "LiLib/Core/Typedefs.h"

namespace LiLib {

/*
 * @brief Non-owning view over a run of characters. Not necessarily null-terminated.
 */
class StringView {
public:
    static constexpr size_t NPos = static_cast<size_t>(-1);

    constexpr StringView() = default;
    StringView(const char* str) : data(str), length(str ? std::strlen(str) : 0) {}
    constexpr StringView(const char* str, size_t length) : data(str), length(length) {}

    const char* Data() const { return data; }
    size_t Length() const { return length; }
    bool IsEmpty() const { return length == 0; }
    char operator[](size_t index) const { return data[index]; }

    StringView Substr(size_t start, size_t count = NPos) const;
    size_t Find(char c, size_t start = 0) const;
    size_t Find(StringView needle, size_t start = 0) const;
    size_t FindLast(char c) const;
    bool StartsWith(StringView prefix) const;
    bool EndsWith(StringView suffix) const;
    bool Equals(StringView other) const;
    bool EqualsIgnoreCase(StringView other) const;
    StringView Trimmed() const;

    // Lowercase begin/end are required by range-based for loops
    const char* begin() const { return data; }        // NOLINT(readability-identifier-naming)
    const char* end() const { return data + length; } // NOLINT(readability-identifier-naming)

private:
    const char* data = nullptr;
    size_t length = 0;
};

bool operator==(StringView a, StringView b);
bool operator!=(StringView a, StringView b);
bool operator<(StringView a, StringView b);
// Exact-match overloads so comparisons with literals don't go through String's converting constructor
bool operator==(StringView a, const char* b);
bool operator!=(StringView a, const char* b);

template<>
class Hasher<StringView> {
public:
    static uint64_t Hash(StringView value) { return HashBytes(value.Data(), value.Length()); }
};

// C strings hash by content so they can look up String keys
template<>
class Hasher<const char*> {
public:
    static uint64_t Hash(const char* value) { return HashBytes(value, std::strlen(value)); }
};

} // namespace LiLib

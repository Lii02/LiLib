#pragma once
#include "LiLib/Containers/Array.h"
#include "LiLib/Strings/StringView.h"
#include <cstdarg>

namespace LiLib {

/*
 * @brief Owning, null-terminated, growable string with small-string optimization.
 */
class String {
public:
    static constexpr size_t NPos = StringView::NPos;

    String();
    String(const char* str);
    String(const char* str, size_t length);
    String(StringView view);
    String(const String& other);
    String(String&& other) noexcept;
    ~String();

    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;
    String& operator=(StringView view);
    String& operator=(const char* str);

    /*
     * @brief printf-style formatting.
     */
    static String Format(const char* format, ...);
    static String FormatV(const char* format, va_list args);

    const char* CStr() const { return data; }
    char* Data() { return data; }
    size_t Length() const { return length; }
    bool IsEmpty() const { return length == 0; }
    StringView View() const { return StringView(data, length); }
    operator StringView() const { return View(); }
    char operator[](size_t index) const { return data[index]; }
    char& operator[](size_t index) { return data[index]; }

    void Append(StringView text);
    void Append(char c);
    void AppendFormat(const char* format, ...);
    String& operator+=(StringView text);
    String& operator+=(char c);

    void Clear();
    void Reserve(size_t newCapacity);
    void Resize(size_t newLength, char fill = '\0');

    String Substr(size_t start, size_t count = NPos) const { return String(View().Substr(start, count)); }
    size_t Find(char c, size_t start = 0) const { return View().Find(c, start); }
    size_t Find(StringView needle, size_t start = 0) const { return View().Find(needle, start); }
    size_t FindLast(char c) const { return View().FindLast(c); }
    bool StartsWith(StringView prefix) const { return View().StartsWith(prefix); }
    bool EndsWith(StringView suffix) const { return View().EndsWith(suffix); }
    bool Contains(StringView needle) const { return Find(needle) != NPos; }

    String ToLower() const;
    String Trimmed() const { return String(View().Trimmed()); }
    String Replace(StringView from, StringView to) const;
    Array<String> Split(char separator, bool keepEmpty = false) const;

    int64_t ToInt(int64_t fallback = 0) const;
    double ToDouble(double fallback = 0.0) const;

    // Lowercase begin/end are required by range-based for loops
    const char* begin() const { return data; }        // NOLINT(readability-identifier-naming)
    const char* end() const { return data + length; } // NOLINT(readability-identifier-naming)

private:
    static constexpr size_t InlineCapacity = 23;

    bool _IsInline() const { return data == inlineBuffer; }
    void _Assign(const char* str, size_t newLength);

    char* data;
    size_t length = 0;
    size_t capacity = InlineCapacity;
    char inlineBuffer[InlineCapacity + 1];
};

String operator+(StringView a, StringView b);
bool operator==(const String& a, const String& b);
bool operator!=(const String& a, const String& b);
bool operator==(const String& a, StringView b);
bool operator!=(const String& a, StringView b);
bool operator==(const String& a, const char* b);
bool operator!=(const String& a, const char* b);
bool operator<(const String& a, const String& b);

template<>
class Hasher<String> {
public:
    static uint64_t Hash(const String& value) { return HashBytes(value.CStr(), value.Length()); }
};

} // namespace LiLib

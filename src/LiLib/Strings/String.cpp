#include "LiLib/Strings/String.h"
#include "LiLib/Memory/Memory.h"
#include <cstdio>
#include <cstdlib>

namespace LiLib {

String::String() : data(inlineBuffer) {
    inlineBuffer[0] = '\0';
}

String::String(const char* str) : String() {
    if(str) {
        _Assign(str, std::strlen(str));
    }
}

String::String(const char* str, size_t length) : String() {
    _Assign(str, length);
}

String::String(StringView view) : String() {
    _Assign(view.Data(), view.Length());
}

String::String(const String& other) : String() {
    _Assign(other.data, other.length);
}

String::String(String&& other) noexcept : String() {
    *this = std::move(other);
}

String::~String() {
    if(!_IsInline()) {
        Memory::Free(data);
    }
}

String& String::operator=(const String& other) {
    if(this != &other) {
        _Assign(other.data, other.length);
    }
    return *this;
}

String& String::operator=(String&& other) noexcept {
    if(this == &other) {
        return *this;
    }
    if(other._IsInline()) {
        _Assign(other.data, other.length);
    } else {
        if(!_IsInline()) {
            Memory::Free(data);
        }
        data = other.data;
        length = other.length;
        capacity = other.capacity;
        other.data = other.inlineBuffer;
        other.capacity = InlineCapacity;
    }
    other.length = 0;
    other.data[0] = '\0';
    return *this;
}

String& String::operator=(StringView view) {
    if(view.Data() >= data && view.Data() <= data + length) {
        // Assigning a view of ourselves; copy first so the source survives reallocation
        const String copy(view);
        _Assign(copy.data, copy.length);
    } else {
        _Assign(view.Data(), view.Length());
    }
    return *this;
}

String& String::operator=(const char* str) {
    return *this = StringView(str);
}

String String::Format(const char* format, ...) {
    va_list args;
    va_start(args, format);
    String result = FormatV(format, args);
    va_end(args);
    return result;
}

String String::FormatV(const char* format, va_list args) {
    va_list copy;
    va_copy(copy, args);
    const int needed = std::vsnprintf(nullptr, 0, format, copy);
    va_end(copy);

    String result;
    if(needed > 0) {
        result.Resize(static_cast<size_t>(needed));
        std::vsnprintf(result.data, static_cast<size_t>(needed) + 1, format, args);
    }
    return result;
}

void String::Append(StringView text) {
    if(text.IsEmpty()) {
        return;
    }
    if(text.Data() >= data && text.Data() <= data + length) {
        const String copy(text);
        Append(copy.View());
        return;
    }
    const size_t oldLength = length;
    Resize(length + text.Length());
    std::memcpy(data + oldLength, text.Data(), text.Length());
}

void String::Append(char c) {
    Resize(length + 1, c);
}

void String::AppendFormat(const char* format, ...) {
    va_list args;
    va_start(args, format);
    const String formatted = FormatV(format, args);
    va_end(args);
    Append(formatted.View());
}

String& String::operator+=(StringView text) {
    Append(text);
    return *this;
}

String& String::operator+=(char c) {
    Append(c);
    return *this;
}

void String::Clear() {
    length = 0;
    data[0] = '\0';
}

void String::Reserve(size_t newCapacity) {
    if(newCapacity <= capacity) {
        return;
    }
    size_t grown = capacity * 2;
    if(grown < newCapacity) {
        grown = newCapacity;
    }
    char* newData = static_cast<char*>(Memory::Allocate(grown + 1));
    LI_VERIFY(newData != nullptr, "String allocation failed");
    std::memcpy(newData, data, length + 1);
    if(!_IsInline()) {
        Memory::Free(data);
    }
    data = newData;
    capacity = grown;
}

void String::Resize(size_t newLength, char fill) {
    Reserve(newLength);
    if(newLength > length) {
        std::memset(data + length, fill, newLength - length);
    }
    length = newLength;
    data[length] = '\0';
}

String String::ToLower() const {
    String result(*this);
    for(size_t i = 0; i < result.length; ++i) {
        const char c = result.data[i];
        if(c >= 'A' && c <= 'Z') {
            result.data[i] = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}

String String::Replace(StringView from, StringView to) const {
    if(from.IsEmpty()) {
        return *this;
    }
    String result;
    const StringView view = View();
    size_t start = 0;
    while(true) {
        const size_t found = view.Find(from, start);
        if(found == NPos) {
            result.Append(view.Substr(start));
            break;
        }
        result.Append(view.Substr(start, found - start));
        result.Append(to);
        start = found + from.Length();
    }
    return result;
}

Array<String> String::Split(char separator, bool keepEmpty) const {
    Array<String> parts;
    size_t start = 0;
    for(size_t i = 0; i <= length; ++i) {
        if(i == length || data[i] == separator) {
            if(keepEmpty || i > start) {
                parts.Push(String(data + start, i - start));
            }
            start = i + 1;
        }
    }
    return parts;
}

int64_t String::ToInt(int64_t fallback) const {
    if(length == 0) {
        return fallback;
    }
    char* endPtr = nullptr;
    const long long value = std::strtoll(data, &endPtr, 10);
    return endPtr == data ? fallback : static_cast<int64_t>(value);
}

double String::ToDouble(double fallback) const {
    if(length == 0) {
        return fallback;
    }
    char* endPtr = nullptr;
    const double value = std::strtod(data, &endPtr);
    return endPtr == data ? fallback : value;
}

void String::_Assign(const char* str, size_t newLength) {
    Reserve(newLength);
    if(newLength > 0) {
        std::memmove(data, str, newLength);
    }
    length = newLength;
    data[length] = '\0';
}

String operator+(StringView a, StringView b) {
    String result;
    result.Reserve(a.Length() + b.Length());
    result.Append(a);
    result.Append(b);
    return result;
}

bool operator==(const String& a, const String& b) {
    return a.View() == b.View();
}

bool operator!=(const String& a, const String& b) {
    return a.View() != b.View();
}

bool operator==(const String& a, StringView b) {
    return a.View() == b;
}

bool operator!=(const String& a, StringView b) {
    return a.View() != b;
}

bool operator==(const String& a, const char* b) {
    return a.View() == StringView(b);
}

bool operator!=(const String& a, const char* b) {
    return a.View() != StringView(b);
}

bool operator<(const String& a, const String& b) {
    return a.View() < b.View();
}

} // namespace LiLib

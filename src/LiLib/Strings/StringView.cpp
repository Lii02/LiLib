#include "LiLib/Strings/StringView.h"

namespace LiLib {

namespace {

char ToLowerAscii(char c) {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool IsSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace

StringView StringView::Substr(size_t start, size_t count) const {
    if(start >= length) {
        return StringView(data + length, 0);
    }
    const size_t available = length - start;
    return StringView(data + start, count > available ? available : count);
}

size_t StringView::Find(char c, size_t start) const {
    for(size_t i = start; i < length; ++i) {
        if(data[i] == c) {
            return i;
        }
    }
    return NPos;
}

size_t StringView::Find(StringView needle, size_t start) const {
    if(needle.length == 0) {
        return start <= length ? start : NPos;
    }
    if(needle.length > length) {
        return NPos;
    }
    for(size_t i = start; i + needle.length <= length; ++i) {
        if(std::memcmp(data + i, needle.data, needle.length) == 0) {
            return i;
        }
    }
    return NPos;
}

size_t StringView::FindLast(char c) const {
    for(size_t i = length; i > 0; --i) {
        if(data[i - 1] == c) {
            return i - 1;
        }
    }
    return NPos;
}

bool StringView::StartsWith(StringView prefix) const {
    return prefix.length <= length && std::memcmp(data, prefix.data, prefix.length) == 0;
}

bool StringView::EndsWith(StringView suffix) const {
    return suffix.length <= length && std::memcmp(data + length - suffix.length, suffix.data, suffix.length) == 0;
}

bool StringView::Equals(StringView other) const {
    return length == other.length && (length == 0 || std::memcmp(data, other.data, length) == 0);
}

bool StringView::EqualsIgnoreCase(StringView other) const {
    if(length != other.length) {
        return false;
    }
    for(size_t i = 0; i < length; ++i) {
        if(ToLowerAscii(data[i]) != ToLowerAscii(other.data[i])) {
            return false;
        }
    }
    return true;
}

StringView StringView::Trimmed() const {
    size_t start = 0;
    size_t stop = length;
    while(start < stop && IsSpace(data[start])) {
        ++start;
    }
    while(stop > start && IsSpace(data[stop - 1])) {
        --stop;
    }
    return StringView(data + start, stop - start);
}

bool operator==(StringView a, StringView b) {
    return a.Equals(b);
}

bool operator!=(StringView a, StringView b) {
    return !a.Equals(b);
}

bool operator==(StringView a, const char* b) {
    return a.Equals(StringView(b));
}

bool operator!=(StringView a, const char* b) {
    return !a.Equals(StringView(b));
}

bool operator<(StringView a, StringView b) {
    const size_t common = Min(a.Length(), b.Length());
    const int result = common > 0 ? std::memcmp(a.Data(), b.Data(), common) : 0;
    if(result != 0) {
        return result < 0;
    }
    return a.Length() < b.Length();
}

} // namespace LiLib

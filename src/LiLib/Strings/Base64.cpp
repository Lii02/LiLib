#include "LiLib/Strings/Base64.h"

namespace LiLib {

namespace {

constexpr char Base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Returns the 6-bit value of a base64 character, or -1 if it isn't one
int Base64Value(char c) {
    if(c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if(c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if(c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if(c == '+') {
        return 62;
    }
    if(c == '/') {
        return 63;
    }
    return -1;
}

bool IsBase64Whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // namespace

String Base64::Encode(const void* data, size_t size) {
    const byte_t* bytes = static_cast<const byte_t*>(data);
    String result;
    result.Reserve((size + 2) / 3 * 4);

    size_t i = 0;
    for(; i + 2 < size; i += 3) {
        uint32_t chunk = (bytes[i] << 16) | (bytes[i + 1] << 8) | bytes[i + 2];
        result.Append(Base64Alphabet[(chunk >> 18) & 0x3f]);
        result.Append(Base64Alphabet[(chunk >> 12) & 0x3f]);
        result.Append(Base64Alphabet[(chunk >> 6) & 0x3f]);
        result.Append(Base64Alphabet[chunk & 0x3f]);
    }

    size_t remaining = size - i;
    if(remaining == 1) {
        uint32_t chunk = bytes[i] << 16;
        result.Append(Base64Alphabet[(chunk >> 18) & 0x3f]);
        result.Append(Base64Alphabet[(chunk >> 12) & 0x3f]);
        result.Append("==");
    } else if(remaining == 2) {
        uint32_t chunk = (bytes[i] << 16) | (bytes[i + 1] << 8);
        result.Append(Base64Alphabet[(chunk >> 18) & 0x3f]);
        result.Append(Base64Alphabet[(chunk >> 12) & 0x3f]);
        result.Append(Base64Alphabet[(chunk >> 6) & 0x3f]);
        result.Append('=');
    }
    return result;
}

bool Base64::Decode(StringView text, Array<byte_t>& out) {
    out.Clear();
    out.Reserve(text.Length() / 4 * 3 + 3);

    uint32_t chunk = 0;
    int bits = 0;
    size_t padding = 0;
    size_t symbols = 0;
    for(char c : text) {
        if(IsBase64Whitespace(c)) {
            continue;
        }
        if(c == '=') {
            ++padding;
            continue;
        }
        // Data after padding is malformed
        int value = Base64Value(c);
        if(value < 0 || padding > 0) {
            out.Clear();
            return false;
        }
        ++symbols;
        chunk = (chunk << 6) | static_cast<uint32_t>(value);
        bits += 6;
        if(bits >= 8) {
            bits -= 8;
            out.Push(static_cast<byte_t>((chunk >> bits) & 0xff));
        }
    }

    // A lone trailing symbol can't encode a whole byte; padding can't exceed what fills the last quad
    size_t tail = symbols % 4;
    if(tail == 1 || padding > 2 || (padding > 0 && (tail + padding) % 4 != 0)) {
        out.Clear();
        return false;
    }
    return true;
}

} // namespace LiLib

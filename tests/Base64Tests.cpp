#include "LiLib/Strings/Base64.h"
#include "TestFramework.h"
#include <cstring>

using namespace LiLib;

namespace {

bool DecodesTo(StringView encoded, StringView expected) {
    Array<byte_t> decoded;
    if(!Base64::Decode(encoded, decoded) || decoded.Size() != expected.Length()) {
        return false;
    }
    return expected.IsEmpty() || std::memcmp(decoded.Data(), expected.Data(), expected.Length()) == 0;
}

bool IsRejected(StringView encoded) {
    Array<byte_t> decoded;
    decoded.Push(1);
    return !Base64::Decode(encoded, decoded) && decoded.IsEmpty();
}

} // namespace

// The test vectors from RFC 4648, section 10
LI_TEST(Base64, EncodesRfc4648Vectors) {
    LI_CHECK(Base64::Encode("", 0) == "");
    LI_CHECK(Base64::Encode("f", 1) == "Zg==");
    LI_CHECK(Base64::Encode("fo", 2) == "Zm8=");
    LI_CHECK(Base64::Encode("foo", 3) == "Zm9v");
    LI_CHECK(Base64::Encode("foob", 4) == "Zm9vYg==");
    LI_CHECK(Base64::Encode("fooba", 5) == "Zm9vYmE=");
    LI_CHECK(Base64::Encode("foobar", 6) == "Zm9vYmFy");
}

LI_TEST(Base64, DecodesRfc4648Vectors) {
    LI_CHECK(DecodesTo("", ""));
    LI_CHECK(DecodesTo("Zg==", "f"));
    LI_CHECK(DecodesTo("Zm8=", "fo"));
    LI_CHECK(DecodesTo("Zm9v", "foo"));
    LI_CHECK(DecodesTo("Zm9vYg==", "foob"));
    LI_CHECK(DecodesTo("Zm9vYmE=", "fooba"));
    LI_CHECK(DecodesTo("Zm9vYmFy", "foobar"));
}

LI_TEST(Base64, UsesTheStandardAlphabet) {
    const byte_t bytes[] = {0xFB, 0xFF, 0xFE};
    LI_CHECK(Base64::Encode(bytes, 3) == "+//+");
    Array<byte_t> decoded;
    LI_REQUIRE(Base64::Decode("+//+", decoded));
    LI_REQUIRE(decoded.Size() == 3);
    LI_CHECK(decoded[0] == 0xFB && decoded[1] == 0xFF && decoded[2] == 0xFE);
}

LI_TEST(Base64, RoundTripsEveryByteValueAtEveryLength) {
    byte_t bytes[256];
    for(int i = 0; i < 256; ++i) {
        bytes[i] = static_cast<byte_t>(i);
    }
    for(size_t length = 0; length <= 256; ++length) {
        const String encoded = Base64::Encode(bytes, length);
        LI_CHECK(encoded.Length() == (length + 2) / 3 * 4);
        Array<byte_t> decoded;
        LI_REQUIRE(Base64::Decode(encoded, decoded));
        LI_REQUIRE(decoded.Size() == length);
        LI_CHECK(length == 0 || std::memcmp(decoded.Data(), bytes, length) == 0);
    }
}

LI_TEST(Base64, PaddingIsOptional) {
    LI_CHECK(DecodesTo("Zg", "f"));
    LI_CHECK(DecodesTo("Zm8", "fo"));
    LI_CHECK(DecodesTo("Zm9vYg", "foob"));
}

LI_TEST(Base64, SkipsWhitespace) {
    LI_CHECK(DecodesTo(" Zm\t9v\r\nYm Fy ", "foobar"));
    LI_CHECK(DecodesTo("Zg\n==", "f"));
    LI_CHECK(DecodesTo("   ", ""));
}

LI_TEST(Base64, RejectsInvalidCharacters) {
    LI_CHECK(IsRejected("Zm9$"));
    LI_CHECK(IsRejected("Zm-_"));
    LI_CHECK(IsRejected("\xFF\xFF\xFF\xFF"));
}

LI_TEST(Base64, RejectsInvalidLengths) {
    LI_CHECK(IsRejected("Z"));
    LI_CHECK(IsRejected("Zm9vY"));
}

LI_TEST(Base64, RejectsInvalidPadding) {
    LI_CHECK(IsRejected("="));
    LI_CHECK(IsRejected("===="));
    LI_CHECK(IsRejected("Zg="));
    LI_CHECK(IsRejected("Zg==="));
    LI_CHECK(IsRejected("Zm9v="));
    LI_CHECK(IsRejected("Zm9v=="));
    LI_CHECK(IsRejected("Z=="));
}

LI_TEST(Base64, RejectsDataAfterPadding) {
    LI_CHECK(IsRejected("Zg==Zg=="));
    LI_CHECK(IsRejected("Zm=9"));
}

LI_TEST(Base64, DecodeReplacesPreviousOutput) {
    Array<byte_t> decoded;
    decoded.Push(1);
    decoded.Push(2);
    LI_REQUIRE(Base64::Decode("Zg==", decoded));
    LI_REQUIRE(decoded.Size() == 1);
    LI_CHECK(decoded[0] == 'f');
}

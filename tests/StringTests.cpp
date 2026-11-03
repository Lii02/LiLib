#include "LiLib/Memory/Memory.h"
#include "LiLib/Strings/String.h"
#include "TestFramework.h"
#include <cstring>
#include <type_traits>

using namespace LiLib;

namespace {

static_assert(std::is_nothrow_move_constructible_v<String>);
static_assert(std::is_nothrow_move_assignable_v<String>);

const char* const LongText = "a string long enough that it has to live on the heap";

bool IsNullTerminated(const String& text) {
    return std::strlen(text.CStr()) == text.Length();
}

} // namespace

LI_TEST(String, Construction) {
    const String defaulted;
    LI_CHECK(defaulted.IsEmpty());
    LI_CHECK(std::strcmp(defaulted.CStr(), "") == 0);

    const char* null = nullptr;
    const String fromNull(null);
    LI_CHECK(fromNull.IsEmpty());

    const String literal = "hello";
    LI_CHECK(literal.Length() == 5);
    LI_CHECK(std::strcmp(literal.CStr(), "hello") == 0);

    const String partial("hello", 2);
    LI_CHECK(partial == "he");
    LI_CHECK(IsNullTerminated(partial));

    const String fromView(StringView("hello").Substr(1, 3));
    LI_CHECK(fromView == "ell");
    LI_CHECK(IsNullTerminated(fromView));
}

LI_TEST(String, ShortStringsStayInline) {
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    const String twentyThree = "12345678901234567890123";
    const String copy = twentyThree;
    LI_CHECK(copy.Length() == 23);
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);

    const String twentyFour = "123456789012345678901234";
    LI_CHECK(twentyFour.Length() == 24);
    LI_CHECK(Memory::GetStats().totalAllocations == allocations + 1);
}

LI_TEST(String, CopyIsIndependent) {
    String original = LongText;
    String copy = original;
    copy[0] = 'X';
    LI_CHECK(original[0] == 'a');
    LI_CHECK(copy[0] == 'X');

    String assigned = "short";
    assigned = original;
    LI_CHECK(assigned == LongText);
    original = "changed";
    LI_CHECK(assigned == LongText);
}

LI_TEST(String, MoveFromHeapStealsTheBuffer) {
    String source = LongText;
    const char* buffer = source.CStr();
    const String moved = std::move(source);
    LI_CHECK(moved.CStr() == buffer);
    LI_CHECK(moved == LongText);
    LI_CHECK(source.IsEmpty());
    LI_CHECK(std::strcmp(source.CStr(), "") == 0);

    source.Append("still usable");
    LI_CHECK(source == "still usable");
}

LI_TEST(String, MoveFromInlineCopiesTheCharacters) {
    String source = "short";
    const String moved = std::move(source);
    LI_CHECK(moved == "short");
    LI_CHECK(source.IsEmpty());
    LI_CHECK(IsNullTerminated(source));
}

LI_TEST(String, MoveAssignmentCoversEveryCombination) {
    String heapToHeap = "another string long enough to live on the heap";
    String source = LongText;
    heapToHeap = std::move(source);
    LI_CHECK(heapToHeap == LongText);
    LI_CHECK(source.IsEmpty());

    String heapToInline = "short";
    source = LongText;
    heapToInline = std::move(source);
    LI_CHECK(heapToInline == LongText);

    String inlineToHeap = LongText;
    source = "short";
    inlineToHeap = std::move(source);
    LI_CHECK(inlineToHeap == "short");
    LI_CHECK(source.IsEmpty());
}

LI_TEST(String, SelfAssignmentKeepsContents) {
    String text = LongText;
    String& alias = text;
    text = alias;
    LI_CHECK(text == LongText);
    text = std::move(alias);
    LI_CHECK(text == LongText);
}

LI_TEST(String, AssigningAViewOfItself) {
    String text = LongText;
    text = text.View().Substr(2, 6);
    LI_CHECK(text == "string");
    LI_CHECK(IsNullTerminated(text));

    String small = "hello";
    small = small.View().Substr(1, 3);
    LI_CHECK(small == "ell");
}

LI_TEST(String, AssignNullAndLiteral) {
    String text = LongText;
    text = "short";
    LI_CHECK(text == "short");
    const char* null = nullptr;
    text = null;
    LI_CHECK(text.IsEmpty());
    LI_CHECK(IsNullTerminated(text));
}

LI_TEST(String, Format) {
    LI_CHECK(String::Format("%d-%s", 5, "x") == "5-x");
    LI_CHECK(String::Format("").IsEmpty());
    LI_CHECK(String::Format("%s", "").IsEmpty());

    const String longResult = String::Format("%s %s", LongText, LongText);
    LI_CHECK(longResult.Length() == std::strlen(LongText) * 2 + 1);
    LI_CHECK(longResult.StartsWith(LongText));
    LI_CHECK(longResult.EndsWith(LongText));
    LI_CHECK(IsNullTerminated(longResult));
}

LI_TEST(String, Append) {
    String text;
    text.Append("ab");
    text.Append('c');
    text += "de";
    text += 'f';
    LI_CHECK(text == "abcdef");
    text.Append("");
    LI_CHECK(text == "abcdef");
    text.Append(StringView());
    LI_CHECK(text == "abcdef");
    text.AppendFormat("%d", 12);
    LI_CHECK(text == "abcdef12");
    LI_CHECK(IsNullTerminated(text));
}

LI_TEST(String, AppendGrowsPastTheInlineBuffer) {
    String text;
    for(int i = 0; i < 1000; ++i) {
        text.Append('x');
    }
    LI_CHECK(text.Length() == 1000);
    LI_CHECK(IsNullTerminated(text));
    for(const char c : text) {
        LI_CHECK(c == 'x');
    }
}

LI_TEST(String, AppendingItselfSurvivesReallocation) {
    String small = "abc";
    small.Append(small.View());
    LI_CHECK(small == "abcabc");

    String large = LongText;
    const size_t length = large.Length();
    large.Append(large.View());
    LI_CHECK(large.Length() == length * 2);
    LI_CHECK(large.StartsWith(LongText));
    LI_CHECK(large.EndsWith(LongText));

    String part = LongText;
    part.Append(part.View().Substr(0, 8));
    LI_CHECK(part.EndsWith("a string"));

    String operatorForm = "abc";
    operatorForm += operatorForm.View();
    LI_CHECK(operatorForm == "abcabc");
}

LI_TEST(String, ClearKeepsTheBuffer) {
    String text = LongText;
    const char* buffer = text.CStr();
    text.Clear();
    LI_CHECK(text.IsEmpty());
    LI_CHECK(IsNullTerminated(text));
    LI_CHECK(text.CStr() == buffer);
}

LI_TEST(String, ReserveKeepsContents) {
    String text = "abc";
    text.Reserve(1000);
    LI_CHECK(text == "abc");
    const uint64_t allocations = Memory::GetStats().totalAllocations;
    for(int i = 0; i < 900; ++i) {
        text.Append('x');
    }
    LI_CHECK(Memory::GetStats().totalAllocations == allocations);
    text.Reserve(1);
    LI_CHECK(text.Length() == 903);
}

LI_TEST(String, Resize) {
    String text = "abc";
    text.Resize(5, 'x');
    LI_CHECK(text == "abcxx");
    text.Resize(2);
    LI_CHECK(text == "ab");
    LI_CHECK(IsNullTerminated(text));
    text.Resize(40, 'y');
    LI_CHECK(text.Length() == 40);
    LI_CHECK(text[39] == 'y');
    LI_CHECK(IsNullTerminated(text));
}

LI_TEST(String, CharacterAccess) {
    String text = "abc";
    LI_CHECK(text[1] == 'b');
    text[1] = 'x';
    LI_CHECK(text == "axc");
    text.Data()[0] = 'y';
    LI_CHECK(text == "yxc");
}

LI_TEST(String, SearchMatchesStringView) {
    const String text = "abcabc";
    LI_CHECK(text.Find('b') == 1);
    LI_CHECK(text.Find('b', 2) == 4);
    LI_CHECK(text.Find("ca") == 2);
    LI_CHECK(text.Find("zz") == String::NPos);
    LI_CHECK(text.FindLast('a') == 3);
    LI_CHECK(text.StartsWith("abc"));
    LI_CHECK(text.EndsWith("abc"));
    LI_CHECK(text.Contains("cab"));
    LI_CHECK(!text.Contains("zz"));
}

LI_TEST(String, Substr) {
    const String text = "hello";
    LI_CHECK(text.Substr(1, 3) == "ell");
    LI_CHECK(text.Substr(1) == "ello");
    LI_CHECK(text.Substr(99).IsEmpty());
    LI_CHECK(IsNullTerminated(text.Substr(1, 3)));
}

LI_TEST(String, ToLowerOnlyChangesAsciiLetters) {
    LI_CHECK(String("HeLLo 123 @[").ToLower() == "hello 123 @[");
    LI_CHECK(String("\xC3\x89").ToLower() == "\xC3\x89");
    LI_CHECK(String("").ToLower().IsEmpty());
}

LI_TEST(String, Trimmed) {
    LI_CHECK(String("  a b \t\r\n").Trimmed() == "a b");
    LI_CHECK(String("   ").Trimmed().IsEmpty());
}

LI_TEST(String, Replace) {
    LI_CHECK(String("a-b-c").Replace("-", "+") == "a+b+c");
    LI_CHECK(String("abc").Replace("z", "y") == "abc");
    LI_CHECK(String("abc").Replace("", "y") == "abc");
    LI_CHECK(String("aaa").Replace("aa", "x") == "xa");
    LI_CHECK(String("abc").Replace("b", "") == "ac");
    LI_CHECK(String("abc").Replace("abc", "") == "");
    LI_CHECK(String("").Replace("a", "b") == "");
    LI_CHECK(String("a").Replace("a", LongText) == LongText);
}

LI_TEST(String, ReplaceWithViewsOfItself) {
    const String text = "abcabc";
    LI_CHECK(text.Replace(text.View().Substr(0, 1), text.View().Substr(1, 2)) == "bcbcbcbc");
}

LI_TEST(String, Split) {
    const Array<String> parts = String("a,b,c").Split(',');
    LI_REQUIRE(parts.Size() == 3);
    LI_CHECK(parts[0] == "a" && parts[1] == "b" && parts[2] == "c");

    LI_CHECK(String("").Split(',').IsEmpty());
    LI_CHECK(String(",,").Split(',').IsEmpty());
    LI_CHECK(String("a,,b,").Split(',').Size() == 2);
    LI_CHECK(String("abc").Split(',').Size() == 1);
}

LI_TEST(String, SplitKeepingEmptyParts) {
    const Array<String> parts = String("a,,b,").Split(',', true);
    LI_REQUIRE(parts.Size() == 4);
    LI_CHECK(parts[0] == "a" && parts[1].IsEmpty() && parts[2] == "b" && parts[3].IsEmpty());
    LI_CHECK(String("").Split(',', true).Size() == 1);
}

LI_TEST(String, ToInt) {
    LI_CHECK(String("42").ToInt() == 42);
    LI_CHECK(String("-7").ToInt() == -7);
    LI_CHECK(String("  8").ToInt() == 8);
    LI_CHECK(String("12abc").ToInt() == 12);
    LI_CHECK(String("0").ToInt(9) == 0);
    LI_CHECK(String("abc").ToInt(9) == 9);
    LI_CHECK(String("").ToInt(9) == 9);
    LI_CHECK(String("9000000000").ToInt() == 9000000000LL);
}

LI_TEST(String, ToDouble) {
    LI_CHECK(String("1.5").ToDouble() == 1.5);
    LI_CHECK(String("-2").ToDouble() == -2.0);
    LI_CHECK(String("1.5x").ToDouble() == 1.5);
    LI_CHECK(String("0").ToDouble(9.0) == 0.0);
    LI_CHECK(String("abc").ToDouble(9.0) == 9.0);
    LI_CHECK(String("").ToDouble(9.0) == 9.0);
}

LI_TEST(String, ComparisonAndConcatenation) {
    const String text = "abc";
    const StringView view = "abc";
    LI_CHECK(text == String("abc"));
    LI_CHECK(text != String("abd"));
    LI_CHECK(text == view);
    LI_CHECK(text != StringView("abd"));
    LI_CHECK(text == "abc");
    LI_CHECK(text != "abd");
    LI_CHECK(view == text.View());
    LI_CHECK(String("a") < String("b"));
    LI_CHECK(!(String("b") < String("a")));
    LI_CHECK(!(String("a") < String("a")));

    LI_CHECK(String("ab") + "cd" == "abcd");
    LI_CHECK(String("ab") + String("cd") == "abcd");
    LI_CHECK(StringView("ab") + StringView("cd") == "abcd");
}

LI_TEST(String, ConvertsToStringView) {
    const String text = "abc";
    const StringView view = text;
    LI_CHECK(view.Data() == text.CStr());
    LI_CHECK(view.Length() == 3);
}

LI_TEST(String, HashMatchesStringView) {
    LI_CHECK(Hasher<String>::Hash(String("same")) == Hasher<StringView>::Hash("same"));
    LI_CHECK(Hasher<String>::Hash(String("same")) != Hasher<String>::Hash(String("different")));
}

LI_TEST(String, RangeFor) {
    int count = 0;
    for(const char c : String("abc")) {
        count += c == 'b' ? 10 : 1;
    }
    LI_CHECK(count == 12);
}

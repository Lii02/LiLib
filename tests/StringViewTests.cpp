#include "LiLib/Strings/StringView.h"
#include "TestFramework.h"

using namespace LiLib;

LI_TEST(StringView, Construction) {
    const StringView defaulted;
    LI_CHECK(defaulted.IsEmpty());
    LI_CHECK(defaulted.Data() == nullptr);

    const char* null = nullptr;
    const StringView fromNull(null);
    LI_CHECK(fromNull.IsEmpty());

    const StringView literal = "hello";
    LI_CHECK(literal.Length() == 5);
    LI_CHECK(literal[1] == 'e');

    const StringView partial("hello", 2);
    LI_CHECK(partial == "he");
}

LI_TEST(StringView, DoesNotNeedNullTermination) {
    const char raw[] = {'a', 'b', 'c'};
    const StringView view(raw, 3);
    LI_CHECK(view == "abc");
    LI_CHECK(view.Find('c') == 2);
    LI_CHECK(view.EndsWith("bc"));
}

LI_TEST(StringView, KeepsEmbeddedNulls) {
    const StringView view("a\0b", 3);
    LI_CHECK(view.Length() == 3);
    LI_CHECK(view.Find('b') == 2);
    LI_CHECK(view != "a");
}

LI_TEST(StringView, Substr) {
    const StringView view = "hello";
    LI_CHECK(view.Substr(1) == "ello");
    LI_CHECK(view.Substr(1, 2) == "el");
    LI_CHECK(view.Substr(1, 99) == "ello");
    LI_CHECK(view.Substr(5).IsEmpty());
    LI_CHECK(view.Substr(99).IsEmpty());
    LI_CHECK(StringView().Substr(0).IsEmpty());
}

LI_TEST(StringView, FindCharacter) {
    const StringView view = "abcabc";
    LI_CHECK(view.Find('b') == 1);
    LI_CHECK(view.Find('b', 2) == 4);
    LI_CHECK(view.Find('z') == StringView::NPos);
    LI_CHECK(view.Find('a', 99) == StringView::NPos);
    LI_CHECK(view.FindLast('a') == 3);
    LI_CHECK(view.FindLast('z') == StringView::NPos);
    LI_CHECK(StringView().FindLast('a') == StringView::NPos);
}

LI_TEST(StringView, FindText) {
    const StringView view = "abcabc";
    LI_CHECK(view.Find("bc") == 1);
    LI_CHECK(view.Find("bc", 2) == 4);
    LI_CHECK(view.Find("zz") == StringView::NPos);
    LI_CHECK(view.Find("abcabcabc") == StringView::NPos);
    LI_CHECK(view.Find("abcabc") == 0);
    LI_CHECK(view.Find("bc", 99) == StringView::NPos);
}

LI_TEST(StringView, FindEmptyTextMatchesAtTheStart) {
    const StringView view = "abc";
    LI_CHECK(view.Find("") == 0);
    LI_CHECK(view.Find("", 2) == 2);
    LI_CHECK(view.Find("", 3) == 3);
    LI_CHECK(view.Find("", 4) == StringView::NPos);
}

LI_TEST(StringView, StartsAndEndsWith) {
    const StringView view = "hello";
    LI_CHECK(view.StartsWith("he"));
    LI_CHECK(view.StartsWith("hello"));
    LI_CHECK(view.StartsWith(""));
    LI_CHECK(!view.StartsWith("lo"));
    LI_CHECK(!view.StartsWith("hello!"));
    LI_CHECK(view.EndsWith("lo"));
    LI_CHECK(view.EndsWith("hello"));
    LI_CHECK(view.EndsWith(""));
    LI_CHECK(!view.EndsWith("he"));
    LI_CHECK(!view.EndsWith("!hello"));
}

LI_TEST(StringView, Equality) {
    const StringView view = "abc";
    LI_CHECK(view.Equals("abc"));
    LI_CHECK(!view.Equals("abd"));
    LI_CHECK(!view.Equals("ab"));
    LI_CHECK(view == StringView("abc"));
    LI_CHECK(view != StringView("abd"));
    LI_CHECK(view == "abc");
    LI_CHECK(view != "abd");
    LI_CHECK(StringView() == "");
    LI_CHECK(StringView() == StringView());
}

LI_TEST(StringView, EqualsIgnoreCase) {
    LI_CHECK(StringView("HeLLo").EqualsIgnoreCase("hEllO"));
    LI_CHECK(!StringView("hello").EqualsIgnoreCase("hellp"));
    LI_CHECK(!StringView("hello").EqualsIgnoreCase("hell"));
    // These differ by the ASCII case bit but aren't letters, so they must not match
    LI_CHECK(!StringView("@").EqualsIgnoreCase("`"));
    LI_CHECK(!StringView("[").EqualsIgnoreCase("{"));
}

LI_TEST(StringView, Ordering) {
    LI_CHECK(StringView("a") < StringView("b"));
    LI_CHECK(!(StringView("b") < StringView("a")));
    LI_CHECK(!(StringView("a") < StringView("a")));
    LI_CHECK(StringView("ab") < StringView("abc"));
    LI_CHECK(!(StringView("abc") < StringView("ab")));
    LI_CHECK(StringView() < StringView("a"));
    LI_CHECK(!(StringView() < StringView()));
}

LI_TEST(StringView, Trimmed) {
    LI_CHECK(StringView("  a b \t\r\n").Trimmed() == "a b");
    LI_CHECK(StringView("abc").Trimmed() == "abc");
    LI_CHECK(StringView(" \t\r\n").Trimmed().IsEmpty());
    LI_CHECK(StringView("").Trimmed().IsEmpty());
}

LI_TEST(StringView, RangeFor) {
    int count = 0;
    for(const char c : StringView("abc")) {
        count += c == 'b' ? 10 : 1;
    }
    LI_CHECK(count == 12);
}

LI_TEST(StringView, HashesByContent) {
    const char first[] = "same";
    const char second[] = "same";
    LI_CHECK(Hasher<StringView>::Hash(first) == Hasher<StringView>::Hash(second));
    const char* firstPointer = first;
    LI_CHECK(Hasher<const char*>::Hash(firstPointer) == Hasher<StringView>::Hash(second));
    LI_CHECK(Hasher<StringView>::Hash("same") != Hasher<StringView>::Hash("different"));
}

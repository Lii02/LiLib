#include "LiLib/IO/File.h"
#include "TestFramework.h"
#include <cstdio>
#ifdef _WIN32
#include <direct.h>
#endif

using namespace LiLib;

namespace {

constexpr const char* TestPath = "lilib_file_test.tmp";

// Deletes the test file when the test ends, even after a failed check
class ScopedTestFile {
public:
    ~ScopedTestFile() { std::remove(TestPath); }
};

} // namespace

LI_TEST(File, MissingFile) {
    const char* missing = "lilib_this_file_does_not_exist.tmp";
    Array<byte_t> bytes;
    bytes.Push(7);
    String text = "unchanged";
    int64_t time = 42;
    LI_CHECK(!File::ReadBytes(missing, bytes));
    LI_CHECK(!File::ReadText(missing, text));
    LI_CHECK(!File::Exists(missing));
    LI_CHECK(!File::GetModifiedTime(missing, time));
    LI_CHECK(bytes.Size() == 1);
    LI_CHECK(text == "unchanged");
    LI_CHECK(time == 42);
}

LI_TEST(File, BytesRoundTrip) {
    const ScopedTestFile cleanup;
    const byte_t data[] = {0, 1, 2, 255, 13, 10, 0};
    LI_CHECK(File::WriteBytes(TestPath, data, sizeof(data)));
    Array<byte_t> bytes;
    LI_CHECK(File::ReadBytes(TestPath, bytes));
    LI_CHECK(bytes.Size() == sizeof(data));
    for(size_t i = 0; i < sizeof(data) && i < bytes.Size(); ++i) {
        LI_CHECK(bytes[i] == data[i]);
    }
}

LI_TEST(File, TextRoundTripIsNullTerminated) {
    const ScopedTestFile cleanup;
    LI_CHECK(File::WriteText(TestPath, "hello\nworld"));
    String text;
    LI_CHECK(File::ReadText(TestPath, text));
    LI_CHECK(text == "hello\nworld");
    LI_CHECK(text.CStr()[text.Length()] == '\0');
}

LI_TEST(File, EmptyFile) {
    const ScopedTestFile cleanup;
    LI_CHECK(File::WriteBytes(TestPath, nullptr, 0));
    LI_CHECK(File::Exists(TestPath));
    Array<byte_t> bytes;
    bytes.Push(1);
    LI_CHECK(File::ReadBytes(TestPath, bytes));
    LI_CHECK(bytes.IsEmpty());
}

LI_TEST(File, WriteReplacesContents) {
    const ScopedTestFile cleanup;
    LI_CHECK(File::WriteText(TestPath, "a long first version"));
    LI_CHECK(File::WriteText(TestPath, "short"));
    String text;
    LI_CHECK(File::ReadText(TestPath, text));
    LI_CHECK(text == "short");
}

LI_TEST(File, ModifiedTimeOfExistingFile) {
    const ScopedTestFile cleanup;
    LI_CHECK(File::WriteText(TestPath, "x"));
    int64_t time = 0;
    LI_CHECK(File::GetModifiedTime(TestPath, time));
    LI_CHECK(time > 0);
}

LI_TEST(File, WriteFailsWithoutParentFolder) {
    LI_CHECK(!File::WriteText("lilib_missing_folder/file.tmp", "x"));
}

LI_TEST(File, CreateDirectoriesMakesParentsAndAllowsExisting) {
    const char* nested = "lilib_dir_test/a/b";
    LI_CHECK(File::CreateDirectories(nested));
    LI_CHECK(File::CreateDirectories(nested));
    LI_CHECK(File::Exists(nested));
    LI_CHECK(File::WriteText("lilib_dir_test/a/b/file.tmp", "x"));
    std::remove("lilib_dir_test/a/b/file.tmp");
#ifdef _WIN32
    const int removedB = _rmdir("lilib_dir_test/a/b");
    const int removedA = _rmdir("lilib_dir_test/a");
    const int removedRoot = _rmdir("lilib_dir_test");
#else
    const int removedB = std::remove("lilib_dir_test/a/b");
    const int removedA = std::remove("lilib_dir_test/a");
    const int removedRoot = std::remove("lilib_dir_test");
#endif
    LI_CHECK(removedB == 0 && removedA == 0 && removedRoot == 0);
}

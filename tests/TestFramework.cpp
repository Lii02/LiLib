#include "TestFramework.h"
#include "LiLib/Memory/Memory.h"
#include <cstdio>
#include <cstring>

namespace LiLibTest {

namespace {

bool Matches(const char* suite, const char* name, const char* filter) {
    if(filter == nullptr) {
        return true;
    }
    char fullName[256];
    std::snprintf(fullName, sizeof(fullName), "%s.%s", suite, name);
    return std::strstr(fullName, filter) != nullptr;
}

} // namespace

TestCase* TestCase::Head = nullptr;
int TestCase::CurrentFailures = 0;

TestCase::TestCase(const char* suite, const char* name, void (*run)(), const char* disabledReason)
    : suite(suite), name(name), run(run), disabledReason(disabledReason) {
    // Keep the list sorted so the run order doesn't depend on link order. This runs during static
    // initialization and stays in a singly linked list, so the framework never relies on the code under test.
    TestCase** link = &Head;
    while(*link != nullptr) {
        const int bySuite = std::strcmp((*link)->suite, suite);
        if(bySuite > 0 || (bySuite == 0 && std::strcmp((*link)->name, name) > 0)) {
            break;
        }
        link = &(*link)->next;
    }
    next = *link;
    *link = this;
}

int TestCase::RunAll(const char* filter, bool includeDisabled) {
    int ran = 0;
    int failed = 0;
    int skipped = 0;
    for(TestCase* test = Head; test != nullptr; test = test->next) {
        if(!Matches(test->suite, test->name, filter)) {
            continue;
        }
        if(test->disabledReason != nullptr && !includeDisabled) {
            std::printf("[SKIP] %s.%s: %s\n", test->suite, test->name, test->disabledReason);
            ++skipped;
            continue;
        }

        // Printed and flushed first, so a crash can be traced to the test that caused it
        std::printf("[RUN] %s.%s\n", test->suite, test->name);
        std::fflush(stdout);

        CurrentFailures = 0;
        const LiLib::MemoryStats before = LiLib::Memory::GetStats();
        test->run();
        const LiLib::MemoryStats after = LiLib::Memory::GetStats();
        if(after.currentAllocations != before.currentAllocations) {
            const long long difference =
                static_cast<long long>(after.currentAllocations) - static_cast<long long>(before.currentAllocations);
            std::printf("  Memory: live allocations changed by %lld\n", difference);
            ++CurrentFailures;
        }

        ++ran;
        if(CurrentFailures > 0) {
            ++failed;
            std::printf("[FAIL] %s.%s\n", test->suite, test->name);
        }
    }

    std::printf("\n%d tests ran, %d failed, %d skipped\n", ran, failed, skipped);
    if(ran == 0) {
        std::printf("No tests matched\n");
        return 1;
    }
    return failed;
}

void TestCase::ListAll() {
    for(const TestCase* test = Head; test != nullptr; test = test->next) {
        std::printf("%s.%s\n", test->suite, test->name);
    }
}

void TestCase::ReportFailure(const char* file, int line, const char* expression) {
    std::printf("  %s:%d: check failed: %s\n", file, line, expression);
    ++CurrentFailures;
}

} // namespace LiLibTest

/*
 * Usage: liLibTests [--list] [--include-disabled] [filter]
 * A filter runs the tests whose "Suite.Name" contains it, such as "Array." or "Mat4.Inverse".
 */
int main(int argc, char** argv) {
    const char* filter = nullptr;
    bool includeDisabled = false;
    for(int i = 1; i < argc; ++i) {
        if(std::strcmp(argv[i], "--list") == 0) {
            LiLibTest::TestCase::ListAll();
            return 0;
        }
        if(std::strcmp(argv[i], "--include-disabled") == 0) {
            includeDisabled = true;
        } else {
            filter = argv[i];
        }
    }
    return LiLibTest::TestCase::RunAll(filter, includeDisabled) == 0 ? 0 : 1;
}

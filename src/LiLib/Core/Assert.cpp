#include "LiLib/Core/Assert.h"
#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace LiLib {

void AssertFailed(const char* expression, const char* message, const char* file, int line) {
    std::fprintf(stderr, "Assertion failed: %s\n  %s\n  at %s:%d\n", expression, message, file, line);
#if defined(__clang__)
    __builtin_debugtrap();
#elif defined(_MSC_VER)
    __debugbreak();
#endif
    std::abort();
}

} // namespace LiLib

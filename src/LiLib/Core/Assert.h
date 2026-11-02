#pragma once

namespace LiLib {

/*
 * @brief Reports a failed assertion and breaks into the debugger (or aborts).
 */
[[noreturn]] void AssertFailed(const char* expression, const char* message, const char* file, int line);

} // namespace LiLib

#ifdef NDEBUG
#define LI_ASSERT(expression, message) static_cast<void>(0)
#else
#define LI_ASSERT(expression, message)                                                                                                     \
    do {                                                                                                                                   \
        if(!(expression)) {                                                                                                                \
            ::LiLib::AssertFailed(#expression, message, __FILE__, __LINE__);                                                               \
        }                                                                                                                                  \
    } while(0)
#endif

#define LI_VERIFY(expression, message)                                                                                                     \
    do {                                                                                                                                   \
        if(!(expression)) {                                                                                                                \
            ::LiLib::AssertFailed(#expression, message, __FILE__, __LINE__);                                                               \
        }                                                                                                                                  \
    } while(0)

#pragma once
#include "LiLib/Core/Typedefs.h"
#include <condition_variable>
#include <cstdint>

namespace LiLib {

class Mutex;

/*
 * @brief Condition variable backed by a std::condition_variable_any, so it works with the recursive Mutex.
 */
class ConditionVariable {
public:
    ConditionVariable() = default;
    LI_NON_COPYABLE(ConditionVariable);

    void Signal() { handle.notify_one(); }
    void Broadcast() { handle.notify_all(); }

    /*
     * @brief Waits atomically while releasing and reacquiring an already-locked mutex.
     */
    void Wait(Mutex& mutex);

    /*
     * @brief Waits for a signal for at most timeoutMS milliseconds; -1 waits indefinitely.
     * @returns true when signaled, false when the timeout expires.
     */
    bool WaitFor(Mutex& mutex, int32_t timeoutMS);

private:
    std::condition_variable_any handle;
};

} // namespace LiLib

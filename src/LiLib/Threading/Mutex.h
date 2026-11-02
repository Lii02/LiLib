#pragma once
#include "LiLib/Core/Typedefs.h"
#include <mutex>

namespace LiLib {

class ConditionVariable;

/*
 * @brief Recursive mutex backed by a std::recursive_mutex.
 */
class Mutex {
public:
    Mutex() = default;
    LI_NON_COPYABLE(Mutex);

    void Lock() { handle.lock(); }
    bool TryLock() { return handle.try_lock(); }
    void Unlock() { handle.unlock(); }

private:
    friend class ConditionVariable;

    std::recursive_mutex handle;
};

/*
 * @brief Locks a mutex for the lifetime of this guard.
 */
class LockGuard {
public:
    explicit LockGuard(Mutex& mutex) : mutex(mutex) { this->mutex.Lock(); }
    LI_NON_COPYABLE(LockGuard);
    ~LockGuard() { mutex.Unlock(); }

private:
    Mutex& mutex;
};

} // namespace LiLib

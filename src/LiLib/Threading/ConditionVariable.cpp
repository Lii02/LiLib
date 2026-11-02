#include "LiLib/Threading/ConditionVariable.h"
#include "LiLib/Core/Assert.h"
#include "LiLib/Threading/Mutex.h"
#include <chrono>

namespace LiLib {

void ConditionVariable::Wait(Mutex& mutex) {
    handle.wait(mutex.handle);
}

bool ConditionVariable::WaitFor(Mutex& mutex, int32_t timeoutMS) {
    LI_VERIFY(timeoutMS >= -1, "Condition variable timeout must be -1 or nonnegative");
    if(timeoutMS == -1) {
        handle.wait(mutex.handle);
        return true;
    }
    return handle.wait_for(mutex.handle, std::chrono::milliseconds(timeoutMS)) == std::cv_status::no_timeout;
}

} // namespace LiLib

# Threading

## Thread

`#include "LiLib/Threading/Thread.h"`

Owns a `std::thread` and joins it on destruction unless it was detached. The worker function returns an `int` exit code.

```cpp
#include "LiLib/Threading/Thread.h"

Thread worker;
const bool started = worker.Start([]() { return 7; });

int exitCode = 0;
if(started && worker.Join(&exitCode)) {
    LI_VERIFY(exitCode == 7, "Unexpected exit code");
}
```

| Member | Description |
|---|---|
| `Start(function)` | Starts the thread. Returns false if it's already joinable, the function is empty, or the thread can't be created |
| `Join(exitCode)` | Waits for the thread and optionally reads its exit code. Returns false if it isn't joinable, or if a thread tries to join itself |
| `Detach()` | Lets the thread run on its own. Returns false if it isn't joinable |
| `IsJoinable()` | Whether there's a thread to join or detach |
| `IsRunning()` | Whether the worker function is still executing |
| `GetId()` | The thread's ID |
| `Thread::GetCurrentId()` | The calling thread's ID |

`Thread` can be moved but not copied. Errors are printed to `stderr`.

An exception that escapes the worker function is caught and printed, and the exit code stays at -1.

## Mutex and LockGuard

`#include "LiLib/Threading/Mutex.h"`

`Mutex` is a recursive mutex, so a thread that already holds it can lock it again. Each `Lock` needs a matching `Unlock`. `LockGuard` locks for the length of a scope, and is the safest way to use it.

```cpp
#include "LiLib/Threading/Mutex.h"

Mutex mutex;
int counter = 0;
{
    LockGuard guard(mutex);
    ++counter;
}

if(mutex.TryLock()) {
    ++counter;
    mutex.Unlock();
}
```

## ConditionVariable

`#include "LiLib/Threading/ConditionVariable.h"`

Lets a thread sleep until another thread signals it.

```cpp
#include "LiLib/Threading/ConditionVariable.h"
#include "LiLib/Threading/Mutex.h"
#include "LiLib/Threading/Thread.h"

Mutex mutex;
ConditionVariable ready;
bool hasWork = false;

Thread producer;
producer.Start([&]() {
    LockGuard guard(mutex);
    hasWork = true;
    ready.Signal();
    return 0;
});

{
    LockGuard guard(mutex);
    // Waits can wake without a signal, so always check the condition in a loop
    while(!hasWork) {
        ready.Wait(mutex);
    }
}
```

| Member | Description |
|---|---|
| `Signal()` | Wakes one waiting thread |
| `Broadcast()` | Wakes all waiting threads |
| `Wait(mutex)` | Releases the mutex, sleeps, and locks it again before returning |
| `WaitFor(mutex, timeoutMS)` | Same, for at most `timeoutMS` milliseconds. Returns false on timeout. -1 waits indefinitely |

The mutex must already be locked when you call `Wait` or `WaitFor`.

**Hold the mutex only once while waiting.** A wait releases just one level of a recursive lock. If the thread has locked the mutex twice, it stays locked during the wait, so the thread that should send the signal can't acquire it and the program can deadlock.

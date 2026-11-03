#include "LiLib/Memory/Memory.h"
#include "LiLib/Threading/ConditionVariable.h"
#include "LiLib/Threading/Mutex.h"
#include "LiLib/Threading/Thread.h"
#include "TestFramework.h"
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <type_traits>

using namespace LiLib;

namespace {

static_assert(!std::is_copy_constructible_v<Thread>);
static_assert(std::is_nothrow_move_constructible_v<Thread>);
static_assert(!std::is_copy_constructible_v<Mutex>);
static_assert(!std::is_copy_constructible_v<ConditionVariable>);

// Waits for a condition without depending on how fast this machine schedules threads
template<typename TCondition>
bool WaitUntil(TCondition condition) {
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while(!condition()) {
        if(std::chrono::steady_clock::now() > deadline) {
            return false;
        }
        std::this_thread::yield();
    }
    return true;
}

} // namespace

// Thread

LI_TEST(Thread, DefaultIsNotJoinable) {
    Thread thread;
    LI_CHECK(!thread.IsJoinable());
    LI_CHECK(!thread.IsRunning());
    LI_CHECK(!thread.Join());
    LI_CHECK(!thread.Detach());
}

LI_TEST(Thread, RunsAndReturnsItsExitCode) {
    int value = 0;
    Thread thread;
    LI_REQUIRE(thread.Start([&value]() {
        value = 5;
        return 7;
    }));
    LI_CHECK(thread.IsJoinable());
    int exitCode = 0;
    LI_REQUIRE(thread.Join(&exitCode));
    LI_CHECK(exitCode == 7);
    LI_CHECK(value == 5);
    LI_CHECK(!thread.IsJoinable());
    LI_CHECK(!thread.Join());
}

LI_TEST(Thread, JoinWithoutAnExitCode) {
    Thread thread;
    LI_REQUIRE(thread.Start([]() { return 0; }));
    LI_CHECK(thread.Join());
}

LI_TEST(Thread, RejectsAnEmptyFunction) {
    Thread thread;
    LI_CHECK(!thread.Start(Function<int()>()));
    LI_CHECK(!thread.IsJoinable());
}

LI_TEST(Thread, RejectsStartingTwice) {
    Thread thread;
    LI_REQUIRE(thread.Start([]() { return 1; }));
    LI_CHECK(!thread.Start([]() { return 2; }));
    int exitCode = 0;
    LI_REQUIRE(thread.Join(&exitCode));
    LI_CHECK(exitCode == 1);
}

LI_TEST(Thread, CanRestartAfterJoining) {
    Thread thread;
    LI_REQUIRE(thread.Start([]() { return 1; }));
    LI_REQUIRE(thread.Join());
    LI_REQUIRE(thread.Start([]() { return 2; }));
    int exitCode = 0;
    LI_REQUIRE(thread.Join(&exitCode));
    LI_CHECK(exitCode == 2);
}

LI_TEST(Thread, IsRunningFollowsTheWorker) {
    std::atomic_bool release{false};
    Thread thread;
    LI_REQUIRE(thread.Start([&release]() {
        while(!release.load()) {
            std::this_thread::yield();
        }
        return 0;
    }));
    LI_CHECK(thread.IsRunning());
    release.store(true);
    LI_CHECK(WaitUntil([&thread]() { return !thread.IsRunning(); }));
    LI_CHECK(thread.IsJoinable());
    LI_CHECK(thread.Join());
}

LI_TEST(Thread, DestructorJoins) {
    std::atomic_bool finished{false};
    {
        Thread thread;
        LI_REQUIRE(thread.Start([&finished]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            finished.store(true);
            return 0;
        }));
    }
    LI_CHECK(finished.load());
}

LI_TEST(Thread, MoveTransfersOwnership) {
    Thread source;
    LI_REQUIRE(source.Start([]() { return 3; }));
    const threadId_t id = source.GetId();

    Thread moved = std::move(source);
    LI_CHECK(!source.IsJoinable());
    LI_CHECK(moved.GetId() == id);

    Thread assigned;
    assigned = std::move(moved);
    LI_CHECK(!moved.IsJoinable());
    int exitCode = 0;
    LI_REQUIRE(assigned.Join(&exitCode));
    LI_CHECK(exitCode == 3);
}

LI_TEST(Thread, MoveAssignmentJoinsTheOldThread) {
    std::atomic_bool finished{false};
    Thread target;
    LI_REQUIRE(target.Start([&finished]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        finished.store(true);
        return 0;
    }));
    Thread other;
    target = std::move(other);
    LI_CHECK(finished.load());
    LI_CHECK(!target.IsJoinable());
}

LI_TEST(Thread, ExceptionsAreCaughtAndLeaveTheDefaultExitCode) {
    Thread standard;
    LI_REQUIRE(standard.Start([]() -> int { throw std::runtime_error("expected by this test"); }));
    int exitCode = 0;
    LI_REQUIRE(standard.Join(&exitCode));
    LI_CHECK(exitCode == -1);

    Thread unknown;
    LI_REQUIRE(unknown.Start([]() -> int { throw 5; }));
    LI_REQUIRE(unknown.Join(&exitCode));
    LI_CHECK(exitCode == -1);
}

LI_TEST(Thread, DetachedThreadKeepsRunningAfterTheOwnerIsGone) {
    const uint64_t allocations = Memory::GetStats().currentAllocations;
    std::atomic_bool release{false};
    std::atomic_bool finished{false};
    {
        Thread thread;
        LI_REQUIRE(thread.Start([&release, &finished]() {
            while(!release.load()) {
                std::this_thread::yield();
            }
            finished.store(true);
            return 0;
        }));
        LI_CHECK(thread.Detach());
        LI_CHECK(!thread.IsJoinable());
        LI_CHECK(!thread.IsRunning());
    }
    release.store(true);
    LI_CHECK(WaitUntil([&finished]() { return finished.load(); }));
    // The worker frees its shared state just after finishing, so give it time before the leak check
    LI_CHECK(WaitUntil([allocations]() { return Memory::GetStats().currentAllocations == allocations; }));
}

LI_TEST(Thread, IdentifiesThreads) {
    const threadId_t mainId = Thread::GetCurrentId();
    std::atomic<threadId_t> workerId{};
    Thread thread;
    LI_REQUIRE(thread.Start([&workerId]() {
        workerId.store(Thread::GetCurrentId());
        return 0;
    }));
    const threadId_t reportedId = thread.GetId();
    LI_REQUIRE(thread.Join());
    LI_CHECK(workerId.load() == reportedId);
    LI_CHECK(workerId.load() != mainId);
}

LI_TEST(Thread, CannotJoinItself) {
    Thread thread;
    std::atomic_bool ready{false};
    std::atomic_bool joinResult{true};
    LI_REQUIRE(thread.Start([&]() {
        while(!ready.load()) {
            std::this_thread::yield();
        }
        joinResult.store(thread.Join());
        return 0;
    }));
    ready.store(true);
    LI_REQUIRE(WaitUntil([&thread]() { return !thread.IsRunning(); }));
    LI_CHECK(!joinResult.load());
    LI_CHECK(thread.Join());
}

// Mutex

LI_TEST(Mutex, ProtectsSharedData) {
    Mutex mutex;
    int counter = 0;
    constexpr int ThreadCount = 8;
    static constexpr int Increments = 100000;
    {
        Thread threads[ThreadCount];
        for(Thread& thread : threads) {
            LI_REQUIRE(thread.Start([&mutex, &counter]() {
                for(int i = 0; i < Increments; ++i) {
                    const LockGuard guard(mutex);
                    ++counter;
                }
                return 0;
            }));
        }
    }
    LI_CHECK(counter == ThreadCount * Increments);
}

LI_TEST(Mutex, IsRecursive) {
    Mutex mutex;
    mutex.Lock();
    mutex.Lock();
    LI_CHECK(mutex.TryLock());
    mutex.Unlock();
    mutex.Unlock();
    mutex.Unlock();
}

LI_TEST(Mutex, TryLockFailsWhileAnotherThreadHoldsIt) {
    Mutex mutex;
    mutex.Lock();
    std::atomic_bool acquired{true};
    std::thread other([&]() { acquired.store(mutex.TryLock()); });
    other.join();
    LI_CHECK(!acquired.load());
    mutex.Unlock();

    std::thread afterUnlock([&]() {
        acquired.store(mutex.TryLock());
        if(acquired.load()) {
            mutex.Unlock();
        }
    });
    afterUnlock.join();
    LI_CHECK(acquired.load());
}

LI_TEST(Mutex, LockGuardHoldsForItsScope) {
    Mutex mutex;
    std::atomic_bool acquired{true};
    const auto tryFromOtherThread = [&]() {
        std::thread other([&]() {
            acquired.store(mutex.TryLock());
            if(acquired.load()) {
                mutex.Unlock();
            }
        });
        other.join();
        return acquired.load();
    };
    {
        const LockGuard guard(mutex);
        LI_CHECK(!tryFromOtherThread());
    }
    LI_CHECK(tryFromOtherThread());
}

// ConditionVariable

LI_TEST(ConditionVariable, SignalWakesAWaiter) {
    Mutex mutex;
    ConditionVariable condition;
    bool ready = false;
    Thread thread;
    LI_REQUIRE(thread.Start([&]() {
        const LockGuard guard(mutex);
        ready = true;
        condition.Signal();
        return 0;
    }));
    {
        const LockGuard guard(mutex);
        while(!ready) {
            condition.Wait(mutex);
        }
    }
    LI_CHECK(ready);
}

LI_TEST(ConditionVariable, BroadcastWakesEveryWaiter) {
    constexpr int ThreadCount = 8;
    Mutex mutex;
    ConditionVariable condition;
    bool ready = false;
    int woken = 0;
    {
        Thread threads[ThreadCount];
        for(Thread& thread : threads) {
            LI_REQUIRE(thread.Start([&]() {
                const LockGuard guard(mutex);
                while(!ready) {
                    condition.Wait(mutex);
                }
                ++woken;
                return 0;
            }));
        }
        const LockGuard guard(mutex);
        ready = true;
        condition.Broadcast();
    }
    LI_CHECK(woken == ThreadCount);
}

LI_TEST(ConditionVariable, WaitForTimesOutWithoutASignal) {
    Mutex mutex;
    ConditionVariable condition;
    const LockGuard guard(mutex);
    // A wait is allowed to wake early without a signal, so a true result alone isn't a failure
    bool timedOut = false;
    for(int attempt = 0; attempt < 1000 && !timedOut; ++attempt) {
        timedOut = !condition.WaitFor(mutex, 1);
    }
    LI_CHECK(timedOut);
}

LI_TEST(ConditionVariable, WaitForReturnsTrueWhenSignaled) {
    Mutex mutex;
    ConditionVariable condition;
    bool ready = false;
    bool signaled = false;
    Thread thread;
    {
        const LockGuard guard(mutex);
        LI_REQUIRE(thread.Start([&]() {
            const LockGuard workerGuard(mutex);
            ready = true;
            condition.Signal();
            return 0;
        }));
        while(!ready) {
            signaled = condition.WaitFor(mutex, 30000);
            LI_REQUIRE(signaled);
        }
    }
    LI_CHECK(signaled);
}

LI_TEST(ConditionVariable, WaitForMinusOneWaitsUntilSignaled) {
    Mutex mutex;
    ConditionVariable condition;
    bool ready = false;
    Thread thread;
    const LockGuard guard(mutex);
    LI_REQUIRE(thread.Start([&]() {
        const LockGuard workerGuard(mutex);
        ready = true;
        condition.Signal();
        return 0;
    }));
    while(!ready) {
        LI_CHECK(condition.WaitFor(mutex, -1));
    }
}

LI_TEST(ConditionVariable, WaitReleasesAndReacquiresTheMutex) {
    Mutex mutex;
    ConditionVariable condition;
    std::atomic_bool acquired{false};
    mutex.Lock();
    std::thread other([&]() {
        const bool success = WaitUntil([&]() {
            if(mutex.TryLock()) {
                mutex.Unlock();
                return true;
            }
            return false;
        });
        acquired.store(success);
    });
    while(!acquired.load()) {
        condition.WaitFor(mutex, 1);
    }
    other.join();
    // Still held by this thread once the wait returns, so another thread must not get it
    std::atomic_bool afterWait{true};
    std::thread check([&]() { afterWait.store(mutex.TryLock()); });
    check.join();
    LI_CHECK(!afterWait.load());
    mutex.Unlock();
}

// Documents the limitation in docs/Threading.md: a wait releases only one level of a recursive lock
LI_TEST(ConditionVariable, WaitKeepsARecursiveLockHeld) {
    Mutex mutex;
    ConditionVariable condition;
    std::atomic_bool done{false};
    std::atomic_bool acquired{false};
    mutex.Lock();
    mutex.Lock();
    std::thread other([&]() {
        while(!done.load()) {
            if(mutex.TryLock()) {
                acquired.store(true);
                mutex.Unlock();
            }
            std::this_thread::yield();
        }
    });
    condition.WaitFor(mutex, 50);
    done.store(true);
    other.join();
    LI_CHECK(!acquired.load());
    mutex.Unlock();
    mutex.Unlock();
}

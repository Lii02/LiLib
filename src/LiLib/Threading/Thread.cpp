#include "LiLib/Threading/Thread.h"
#include "LiLib/Memory/Memory.h"
#include <atomic>
#include <cstdio>
#include <exception>
#include <system_error>
#include <utility>

namespace LiLib {

struct Thread::State {
    explicit State(Function<int()>&& function) : function(std::move(function)) {}

    Function<int()> function;
    // One reference for the owning Thread, one for the worker
    std::atomic_int32_t refCount{2};
    std::atomic_bool running{true};
    int exitCode = -1;
};

Thread::Thread(Thread&& other) noexcept : worker(std::move(other.worker)), state(other.state) {
    other.state = nullptr;
}

Thread& Thread::operator=(Thread&& other) noexcept {
    if(this != &other) {
        _Cleanup();
        worker = std::move(other.worker);
        state = other.state;
        other.state = nullptr;
    }
    return *this;
}

Thread::~Thread() {
    _Cleanup();
}

bool Thread::Start(Function<int()> function) {
    if(worker.joinable()) {
        std::fprintf(stderr, "Thread: cannot start a thread that is already joinable\n");
        return false;
    }
    if(!function) {
        std::fprintf(stderr, "Thread: cannot start with an empty function\n");
        return false;
    }

    State* newState = Memory::New<State>(std::move(function));
    try {
        worker = std::thread(_Run, newState);
    } catch(const std::system_error& error) {
        std::fprintf(stderr, "Thread: failed to create thread: %s\n", error.what());
        Memory::Delete(newState);
        return false;
    }
    state = newState;
    return true;
}

bool Thread::Join(int* exitCode) {
    if(!worker.joinable()) {
        return false;
    }
    if(worker.get_id() == GetCurrentId()) {
        std::fprintf(stderr, "Thread: a thread cannot join itself\n");
        return false;
    }

    worker.join();
    if(exitCode != nullptr) {
        *exitCode = state->exitCode;
    }
    _ReleaseState(state);
    state = nullptr;
    return true;
}

bool Thread::Detach() {
    if(!worker.joinable()) {
        return false;
    }
    worker.detach();
    _ReleaseState(state);
    state = nullptr;
    return true;
}

bool Thread::IsRunning() const {
    return worker.joinable() && state->running.load(std::memory_order_acquire);
}

void Thread::_Run(State* state) {
    try {
        state->exitCode = state->function();
    } catch(const std::exception& exception) {
        std::fprintf(stderr, "Thread: worker function threw an exception: %s\n", exception.what());
    } catch(...) {
        std::fprintf(stderr, "Thread: worker function threw an unknown exception\n");
    }
    state->running.store(false, std::memory_order_release);
    _ReleaseState(state);
}

void Thread::_ReleaseState(State* state) {
    if(state->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        Memory::Delete(state);
    }
}

void Thread::_Cleanup() {
    if(!worker.joinable()) {
        return;
    }

    if(worker.get_id() == GetCurrentId()) {
        worker.detach();
    } else {
        worker.join();
    }
    _ReleaseState(state);
    state = nullptr;
}

} // namespace LiLib

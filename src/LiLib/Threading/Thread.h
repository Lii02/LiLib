#pragma once
#include "LiLib/Core/Function.h"
#include <thread>

namespace LiLib {

using threadId_t = std::thread::id;

/*
 * @brief Owns a std::thread and joins it on destruction unless detached.
 */
class Thread {
public:
    Thread() = default;
    LI_NON_COPYABLE(Thread);
    Thread(Thread&& other) noexcept;
    Thread& operator=(Thread&& other) noexcept;
    ~Thread();

    bool Start(Function<int()> function);
    bool Join(int* exitCode = nullptr);
    bool Detach();
    bool IsJoinable() const { return worker.joinable(); }
    bool IsRunning() const;
    threadId_t GetId() const { return worker.get_id(); }
    static threadId_t GetCurrentId() { return std::this_thread::get_id(); }

private:
    struct State;
    static void _Run(State* state);
    static void _ReleaseState(State* state);
    void _Cleanup();

    std::thread worker;
    // Reference counted between this and the worker so a detached thread never touches a destroyed Thread
    State* state = nullptr;
};

} // namespace LiLib

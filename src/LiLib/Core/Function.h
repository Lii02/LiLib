#pragma once
#include "LiLib/Core/Assert.h"
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Memory/Memory.h"

namespace LiLib {

template<typename TSignature>
class Function;

/*
 * @brief Type-erased, copyable callable. Replaces std::function.
 */
template<typename TResult, typename... TArgs>
class Function<TResult(TArgs...)> {
public:
    Function() = default;
    Function(std::nullptr_t) {}

    template<typename TCallable, typename = std::enable_if_t<!std::is_same_v<std::decay_t<TCallable>, Function>>>
    Function(TCallable&& callable) : callable(Memory::New<Callable<std::decay_t<TCallable>>>(std::forward<TCallable>(callable))) {}

    Function(const Function& other) : callable(other.callable ? other.callable->Clone() : nullptr) {}
    Function(Function&& other) noexcept : callable(other.callable) { other.callable = nullptr; }

    ~Function() { Memory::Delete(callable); }

    Function& operator=(const Function& other) {
        if(this != &other) {
            Memory::Delete(callable);
            callable = other.callable ? other.callable->Clone() : nullptr;
        }
        return *this;
    }

    Function& operator=(Function&& other) noexcept {
        if(this != &other) {
            Memory::Delete(callable);
            callable = other.callable;
            other.callable = nullptr;
        }
        return *this;
    }

    explicit operator bool() const { return callable != nullptr; }

    TResult operator()(TArgs... args) const {
        LI_ASSERT(callable != nullptr, "Calling an empty Function");
        return callable->Invoke(std::forward<TArgs>(args)...);
    }

private:
    struct ICallable {
        virtual ~ICallable() = default;
        virtual TResult Invoke(TArgs... args) = 0;
        virtual ICallable* Clone() const = 0;
    };

    template<typename TCallable>
    class Callable final : public ICallable {
    public:
        template<typename TInit>
        explicit Callable(TInit&& init) : target(std::forward<TInit>(init)) {}

        TResult Invoke(TArgs... args) override { return target(std::forward<TArgs>(args)...); }
        ICallable* Clone() const override { return Memory::New<Callable>(target); }

    private:
        TCallable target;
    };

    // Owned; Function manages its lifetime manually because it is part of the standard library layer
    ICallable* callable = nullptr;
};

} // namespace LiLib

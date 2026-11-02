#pragma once
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Memory/Memory.h"

namespace LiLib {

/*
 * @brief Single-owner smart pointer. Deletes the object when it goes out of scope.
 */
template<typename T>
class UniquePtr {
public:
    UniquePtr() = default;
    UniquePtr(std::nullptr_t) {}
    // Takes ownership of an object created with Memory::New. Prefer MakeUnique.
    explicit UniquePtr(T* ptr) : ptr(ptr) {}
    UniquePtr(UniquePtr&& other) noexcept : ptr(other.Release()) {}

    template<typename TOther, typename = std::enable_if_t<std::is_convertible_v<TOther*, T*>>>
    UniquePtr(UniquePtr<TOther>&& other) noexcept : ptr(other.Release()) {}

    LI_NON_COPYABLE(UniquePtr);

    ~UniquePtr() { Reset(); }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    T* Get() const { return ptr; }
    T* operator->() const { return ptr; }
    T& operator*() const { return *ptr; }
    explicit operator bool() const { return ptr != nullptr; }

    /*
     * @brief Gives up ownership without deleting the object.
     */
    T* Release() {
        T* released = ptr;
        ptr = nullptr;
        return released;
    }

    void Reset(T* newPtr = nullptr) {
        T* old = ptr;
        ptr = newPtr;
        Memory::Delete(old);
    }

private:
    T* ptr = nullptr;
};

template<typename T, typename... TArgs>
UniquePtr<T> MakeUnique(TArgs&&... args) {
    return UniquePtr<T>(Memory::New<T>(std::forward<TArgs>(args)...));
}

template<typename T, typename TOther>
bool operator==(const UniquePtr<T>& a, const UniquePtr<TOther>& b) {
    return a.Get() == b.Get();
}

template<typename T>
bool operator==(const UniquePtr<T>& a, std::nullptr_t) {
    return a.Get() == nullptr;
}

template<typename T>
bool operator!=(const UniquePtr<T>& a, std::nullptr_t) {
    return a.Get() != nullptr;
}

} // namespace LiLib

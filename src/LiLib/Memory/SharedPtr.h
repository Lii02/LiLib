#pragma once
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Memory/Memory.h"
#include <atomic>

namespace LiLib {

/*
 * @brief Reference count shared between all SharedPtr copies of one object.
 */
struct SharedControl {
    std::atomic_int32_t refCount{1};
    void* object = nullptr;
    void (*destroy)(void* object) = nullptr;
};

/*
 * @brief Reference-counted smart pointer. The object is deleted when the last copy goes away.
 *
 *        Different SharedPtr instances can be copied, moved and destroyed on different threads, even when they
 *        share an object. A single instance can't be modified by one thread while another thread uses it, and
 *        the object itself isn't protected, so access to it needs its own synchronization.
 */
template<typename T>
class SharedPtr {
public:
    SharedPtr() = default;
    SharedPtr(std::nullptr_t) {}

    SharedPtr(const SharedPtr& other) : ptr(other.ptr), control(other.control) { _AddRef(); }
    SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), control(other.control) {
        other.ptr = nullptr;
        other.control = nullptr;
    }

    template<typename TOther, typename = std::enable_if_t<std::is_convertible_v<TOther*, T*>>>
    SharedPtr(const SharedPtr<TOther>& other) : ptr(other.Get()), control(other._GetControl()) {
        _AddRef();
    }

    ~SharedPtr() { _Release(); }

    // Assignments take the new reference before releasing the old one, because other may live
    // inside the object being released (as in node = node->next)
    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr copy(other);
        _Swap(copy);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        SharedPtr moved(std::move(other));
        _Swap(moved);
        return *this;
    }

    SharedPtr& operator=(std::nullptr_t) {
        _Release();
        return *this;
    }

    T* Get() const { return ptr; }
    T* operator->() const { return ptr; }
    T& operator*() const { return *ptr; }
    explicit operator bool() const { return ptr != nullptr; }

    /*
     * @brief Number of owners. Other threads can change it at any time, so treat it as approximate when sharing across threads.
     */
    int32_t UseCount() const { return control ? control->refCount.load(std::memory_order_relaxed) : 0; }

    void Reset() { _Release(); }

    /*
     * @brief Takes ownership of an object created with Memory::New. Prefer MakeShared.
     */
    static SharedPtr Adopt(T* object) {
        SharedPtr result;
        if(object) {
            result.ptr = object;
            result.control = Memory::New<SharedControl>();
            result.control->object = object;
            result.control->destroy = [](void* raw) {
                Memory::Delete(static_cast<T*>(raw));
            };
        }
        return result;
    }

    // Internal: used by the converting constructor of other SharedPtr instantiations
    SharedControl* _GetControl() const { return control; }

private:
    void _AddRef() {
        if(control) {
            // Relaxed is enough: the caller already holds a reference, so the count can't reach zero here
            control->refCount.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void _Swap(SharedPtr& other) {
        Swap(ptr, other.ptr);
        Swap(control, other.control);
    }

    void _Release() {
        // Clear the members first: destroying the object can destroy this pointer too, if the object owns it
        SharedControl* released = control;
        ptr = nullptr;
        control = nullptr;

        // The thread that destroys the object must see every write other owners made to it, hence acq_rel
        if(released && released->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            released->destroy(released->object);
            Memory::Delete(released);
        }
    }

    T* ptr = nullptr;
    SharedControl* control = nullptr;
};

template<typename T, typename... TArgs>
SharedPtr<T> MakeShared(TArgs&&... args) {
    return SharedPtr<T>::Adopt(Memory::New<T>(std::forward<TArgs>(args)...));
}

template<typename T, typename TOther>
bool operator==(const SharedPtr<T>& a, const SharedPtr<TOther>& b) {
    return a.Get() == b.Get();
}

template<typename T, typename TOther>
bool operator!=(const SharedPtr<T>& a, const SharedPtr<TOther>& b) {
    return a.Get() != b.Get();
}

template<typename T>
bool operator==(const SharedPtr<T>& a, std::nullptr_t) {
    return a.Get() == nullptr;
}

template<typename T>
bool operator!=(const SharedPtr<T>& a, std::nullptr_t) {
    return a.Get() != nullptr;
}

} // namespace LiLib

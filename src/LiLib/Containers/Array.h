#pragma once
#include "LiLib/Core/Assert.h"
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Memory/Memory.h"
#include <initializer_list>
#include <new>
#include <utility>

namespace LiLib {

/*
 * @brief Growable contiguous array. Replaces std::vector.
 */
template<typename T>
class Array {
public:
    static constexpr int64_t NotFound = -1;

    Array() = default;

    Array(std::initializer_list<T> values) { _AppendRange(values.begin(), values.size()); }
    Array(const Array& other) { _AppendRange(other.data, other.size); }
    Array(Array&& other) noexcept { _TakeStorage(other); }

    ~Array() {
        Clear();
        Memory::Free(data);
    }

    Array& operator=(const Array& other) {
        if(this != &other) {
            Clear();
            _AppendRange(other.data, other.size);
        }
        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        if(this != &other) {
            Clear();
            Memory::Free(data);
            _TakeStorage(other);
        }
        return *this;
    }

    size_t Size() const { return size; }
    size_t Capacity() const { return capacity; }
    bool IsEmpty() const { return size == 0; }
    T* Data() { return data; }
    const T* Data() const { return data; }

    T& operator[](size_t index) {
        LI_ASSERT(index < size, "Array index out of range");
        return data[index];
    }

    const T& operator[](size_t index) const {
        LI_ASSERT(index < size, "Array index out of range");
        return data[index];
    }

    T& Front() { return (*this)[0]; }
    const T& Front() const { return (*this)[0]; }
    T& Back() { return (*this)[size - 1]; }
    const T& Back() const { return (*this)[size - 1]; }

    void Reserve(size_t newCapacity) {
        if(newCapacity <= capacity) {
            return;
        }
        _AdoptStorage(_AllocateStorage(newCapacity), newCapacity);
    }

    void Resize(size_t newSize) {
        Reserve(newSize);
        _ShrinkTo(newSize);
        while(size < newSize) {
            new(data + size) T();
            ++size;
        }
    }

    void Resize(size_t newSize, const T& fill) {
        if(newSize > capacity) {
            // fill may live inside this array, so copy it before reallocating
            const T copy(fill);
            Reserve(newSize);
            _AppendCopies(newSize, copy);
            return;
        }
        _ShrinkTo(newSize);
        _AppendCopies(newSize, fill);
    }

    void Clear() {
        for(size_t i = 0; i < size; ++i) {
            data[i].~T();
        }
        size = 0;
    }

    T& Push(const T& value) { return Emplace(value); }
    T& Push(T&& value) { return Emplace(std::move(value)); }

    template<typename... TArgs>
    T& Emplace(TArgs&&... args) {
        if(size == capacity) {
            // args may refer to this array's elements, so construct before relocating them
            const size_t newCapacity = _NextCapacity();
            T* newData = _AllocateStorage(newCapacity);
            new(newData + size) T(std::forward<TArgs>(args)...);
            _AdoptStorage(newData, newCapacity);
        } else {
            new(data + size) T(std::forward<TArgs>(args)...);
        }
        return data[size++];
    }

    void Pop() {
        LI_ASSERT(size > 0, "Pop on empty array");
        --size;
        data[size].~T();
    }

    void Insert(size_t index, const T& value) {
        T copy(value);
        _InsertOwned(index, std::move(copy));
    }

    void Insert(size_t index, T&& value) { _InsertOwned(index, std::move(value)); }

    /*
     * @brief Removes the element at index, keeping the order of the remaining elements.
     */
    void RemoveAt(size_t index) {
        LI_ASSERT(index < size, "RemoveAt index out of range");
        for(size_t i = index; i + 1 < size; ++i) {
            data[i] = std::move(data[i + 1]);
        }
        Pop();
    }

    /*
     * @brief Removes the element at index by moving the last element into its place. O(1).
     */
    void RemoveSwap(size_t index) {
        LI_ASSERT(index < size, "RemoveSwap index out of range");
        if(index != size - 1) {
            data[index] = std::move(data[size - 1]);
        }
        Pop();
    }

    int64_t IndexOf(const T& value) const {
        for(size_t i = 0; i < size; ++i) {
            if(data[i] == value) {
                return static_cast<int64_t>(i);
            }
        }
        return NotFound;
    }

    bool Contains(const T& value) const { return IndexOf(value) != NotFound; }

    bool Remove(const T& value) {
        const int64_t index = IndexOf(value);
        if(index == NotFound) {
            return false;
        }
        RemoveAt(static_cast<size_t>(index));
        return true;
    }

    template<typename TPredicate>
    size_t RemoveIf(TPredicate predicate) {
        size_t write = 0;
        for(size_t read = 0; read < size; ++read) {
            if(!predicate(data[read])) {
                if(write != read) {
                    data[write] = std::move(data[read]);
                }
                ++write;
            }
        }
        const size_t removed = size - write;
        _ShrinkTo(write);
        return removed;
    }

    template<typename TPredicate>
    T* FindIf(TPredicate predicate) {
        for(size_t i = 0; i < size; ++i) {
            if(predicate(data[i])) {
                return data + i;
            }
        }
        return nullptr;
    }

    // Lowercase begin/end are required by range-based for loops
    T* begin() { return data; }
    T* end() { return data + size; }
    const T* begin() const { return data; }
    const T* end() const { return data + size; }

private:
    static constexpr size_t MinCapacity = 8;

    size_t _NextCapacity() const { return capacity < MinCapacity ? MinCapacity : capacity * 2; }

    static T* _AllocateStorage(size_t newCapacity) {
        T* storage = static_cast<T*>(Memory::Allocate(newCapacity * sizeof(T), alignof(T)));
        LI_VERIFY(storage != nullptr, "Array allocation failed");
        return storage;
    }

    void _AdoptStorage(T* newData, size_t newCapacity) {
        for(size_t i = 0; i < size; ++i) {
            new(newData + i) T(std::move(data[i]));
            data[i].~T();
        }
        Memory::Free(data);
        data = newData;
        capacity = newCapacity;
    }

    void _TakeStorage(Array& other) {
        data = other.data;
        size = other.size;
        capacity = other.capacity;
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    void _AppendRange(const T* values, size_t count) {
        Reserve(size + count);
        for(size_t i = 0; i < count; ++i) {
            new(data + size) T(values[i]);
            ++size;
        }
    }

    void _AppendCopies(size_t newSize, const T& fill) {
        while(size < newSize) {
            new(data + size) T(fill);
            ++size;
        }
    }

    void _ShrinkTo(size_t newSize) {
        while(size > newSize) {
            Pop();
        }
    }

    void _InsertOwned(size_t index, T&& value) {
        LI_ASSERT(index <= size, "Insert index out of range");
        // value may live inside this array, so take it out before shifting or reallocating
        T owned(std::move(value));
        if(size == capacity) {
            Reserve(_NextCapacity());
        }
        if(index == size) {
            new(data + size) T(std::move(owned));
        } else {
            new(data + size) T(std::move(data[size - 1]));
            for(size_t i = size - 1; i > index; --i) {
                data[i] = std::move(data[i - 1]);
            }
            data[index] = std::move(owned);
        }
        ++size;
    }

    T* data = nullptr;
    size_t size = 0;
    size_t capacity = 0;
};

} // namespace LiLib

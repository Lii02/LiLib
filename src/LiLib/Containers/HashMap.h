#pragma once
#include "LiLib/Core/Assert.h"
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Memory/Memory.h"
#include <new>

namespace LiLib {

/*
 * @brief Open-addressing hash map with linear probing and backward-shift deletion.
 *        Replaces std::unordered_map. Iteration order is unspecified.
 *
 *        Lookups accept any key type that hashes identically and compares equal to TKey,
 *        so a HashMap<String, T> can be searched with a StringView without allocating.
 */
template<typename TKey, typename TValue>
class HashMap {
public:
    using entry_t = Pair<TKey, TValue>;

    template<typename TEntry, typename TMap>
    class Iterator {
    public:
        Iterator(TMap* map, size_t index) : map(map), index(index) { _SkipEmpty(); }

        TEntry& operator*() const { return map->entries[index]; }
        TEntry* operator->() const { return &map->entries[index]; }

        Iterator& operator++() {
            ++index;
            _SkipEmpty();
            return *this;
        }

        bool operator!=(const Iterator& other) const { return index != other.index; }
        bool operator==(const Iterator& other) const { return index == other.index; }

    private:
        void _SkipEmpty() {
            while(index < map->capacity && !map->occupied[index]) {
                ++index;
            }
        }

        TMap* map;
        size_t index;
    };

    using iterator_t = Iterator<entry_t, HashMap>;
    using constIterator_t = Iterator<const entry_t, const HashMap>;

    HashMap() = default;

    HashMap(const HashMap& other) {
        _Allocate(other.capacity);
        for(const entry_t& entry : other) {
            Insert(entry.first, entry.second);
        }
    }

    HashMap(HashMap&& other) noexcept : entries(other.entries), occupied(other.occupied), size(other.size), capacity(other.capacity) {
        other.entries = nullptr;
        other.occupied = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    ~HashMap() {
        Clear();
        _Free();
    }

    HashMap& operator=(const HashMap& other) {
        if(this != &other) {
            Clear();
            for(const entry_t& entry : other) {
                Insert(entry.first, entry.second);
            }
        }
        return *this;
    }

    HashMap& operator=(HashMap&& other) noexcept {
        if(this != &other) {
            Clear();
            _Free();
            entries = other.entries;
            occupied = other.occupied;
            size = other.size;
            capacity = other.capacity;
            other.entries = nullptr;
            other.occupied = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    size_t Size() const { return size; }
    bool IsEmpty() const { return size == 0; }

    template<typename TLookup>
    TValue* Find(const TLookup& key) {
        const int64_t index = _IndexOf(key);
        return index < 0 ? nullptr : &entries[index].second;
    }

    template<typename TLookup>
    const TValue* Find(const TLookup& key) const {
        const int64_t index = _IndexOf(key);
        return index < 0 ? nullptr : &entries[index].second;
    }

    template<typename TLookup>
    bool Contains(const TLookup& key) const {
        return _IndexOf(key) >= 0;
    }

    /*
     * @brief Inserts the value, or replaces it if the key already exists.
     */
    TValue& Insert(const TKey& key, TValue value) {
        const int64_t existing = _IndexOf(key);
        if(existing >= 0) {
            entries[existing].second = std::move(value);
            return entries[existing].second;
        }
        TKey keyCopy(key);
        return _InsertNew(std::move(keyCopy), std::move(value));
    }

    /*
     * @brief Returns the value for key, default-constructing it if missing.
     */
    TValue& operator[](const TKey& key) {
        const int64_t existing = _IndexOf(key);
        if(existing >= 0) {
            return entries[existing].second;
        }
        TKey keyCopy(key);
        return _InsertNew(std::move(keyCopy), TValue());
    }

    template<typename TLookup>
    bool Remove(const TLookup& key) {
        const int64_t found = _IndexOf(key);
        if(found < 0) {
            return false;
        }

        const size_t mask = capacity - 1;
        size_t hole = static_cast<size_t>(found);
        entries[hole].~entry_t();
        occupied[hole] = false;
        --size;

        // Backward shift: pull later entries of the same probe chain into the hole
        size_t next = (hole + 1) & mask;
        while(occupied[next]) {
            const size_t home = Hasher<TKey>::Hash(entries[next].first) & mask;
            const bool canMove = (next > hole) ? (home <= hole || home > next) : (home <= hole && home > next);
            if(canMove) {
                new(&entries[hole]) entry_t(std::move(entries[next]));
                entries[next].~entry_t();
                occupied[hole] = true;
                occupied[next] = false;
                hole = next;
            }
            next = (next + 1) & mask;
        }
        return true;
    }

    void Clear() {
        for(size_t i = 0; i < capacity; ++i) {
            if(occupied[i]) {
                entries[i].~entry_t();
                occupied[i] = false;
            }
        }
        size = 0;
    }

    void Reserve(size_t count) {
        size_t needed = 16;
        while(needed * 3 < count * 4) {
            needed *= 2;
        }
        if(needed > capacity) {
            _Rehash(needed);
        }
    }

    // Lowercase begin/end are required by range-based for loops
    iterator_t begin() { return iterator_t(this, 0); }
    iterator_t end() { return iterator_t(this, capacity); }
    constIterator_t begin() const { return constIterator_t(this, 0); }
    constIterator_t end() const { return constIterator_t(this, capacity); }

private:
    template<typename TLookup>
    int64_t _IndexOf(const TLookup& key) const {
        if(size == 0) {
            return -1;
        }
        const size_t mask = capacity - 1;
        size_t index = Hasher<TLookup>::Hash(key) & mask;
        while(occupied[index]) {
            if(entries[index].first == key) {
                return static_cast<int64_t>(index);
            }
            index = (index + 1) & mask;
        }
        return -1;
    }

    TValue& _InsertNew(TKey&& key, TValue&& value) {
        if((size + 1) * 4 > capacity * 3) {
            _Rehash(capacity == 0 ? 16 : capacity * 2);
        }
        const size_t mask = capacity - 1;
        size_t index = Hasher<TKey>::Hash(key) & mask;
        while(occupied[index]) {
            index = (index + 1) & mask;
        }
        new(&entries[index]) entry_t{std::move(key), std::move(value)};
        occupied[index] = true;
        ++size;
        return entries[index].second;
    }

    void _Rehash(size_t newCapacity) {
        entry_t* oldEntries = entries;
        bool* oldOccupied = occupied;
        const size_t oldCapacity = capacity;

        _Allocate(newCapacity);
        size = 0;
        for(size_t i = 0; i < oldCapacity; ++i) {
            if(oldOccupied[i]) {
                _InsertNew(std::move(oldEntries[i].first), std::move(oldEntries[i].second));
                oldEntries[i].~entry_t();
            }
        }
        Memory::Free(oldEntries);
        Memory::Free(oldOccupied);
    }

    void _Allocate(size_t newCapacity) {
        LI_ASSERT((newCapacity & (newCapacity - 1)) == 0, "HashMap capacity must be a power of two");
        entries = static_cast<entry_t*>(Memory::Allocate(newCapacity * sizeof(entry_t), alignof(entry_t)));
        occupied = static_cast<bool*>(Memory::Allocate(newCapacity * sizeof(bool)));
        LI_VERIFY(entries != nullptr && occupied != nullptr, "HashMap allocation failed");
        std::memset(occupied, 0, newCapacity * sizeof(bool));
        capacity = newCapacity;
    }

    void _Free() {
        Memory::Free(entries);
        Memory::Free(occupied);
        entries = nullptr;
        occupied = nullptr;
        capacity = 0;
    }

    entry_t* entries = nullptr;
    bool* occupied = nullptr;
    size_t size = 0;
    size_t capacity = 0;
};

} // namespace LiLib

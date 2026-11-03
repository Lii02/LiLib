#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>

/*
 * @brief Marks a variable or parameter as intentionally unused.
 */
#define LI_UNUSED(x) static_cast<void>(x)

/*
 * @brief Deletes the copy constructor and copy assignment. Moves stay as the class declares them.
 */
#define LI_NON_COPYABLE(Type)                                                                                                              \
    Type(const Type&) = delete;                                                                                                            \
    Type& operator=(const Type&) = delete

namespace LiLib {

using byte_t = uint8_t;
using uuid_t = uint64_t;

/*
 * @brief Two values bundled together. Supports structured bindings.
 */
template<typename TFirst, typename TSecond>
struct Pair {
    TFirst first;
    TSecond second;
};

template<typename T>
constexpr T Min(const T& a, const T& b) {
    return b < a ? b : a;
}

template<typename T>
constexpr T Max(const T& a, const T& b) {
    return a < b ? b : a;
}

template<typename T>
constexpr T Clamp(const T& value, const T& low, const T& high) {
    return value < low ? low : (high < value ? high : value);
}

template<typename T>
void Swap(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

/*
 * @brief 64-bit FNV-1a hash of a byte range.
 */
inline uint64_t HashBytes(const void* data, size_t size) {
    const byte_t* bytes = static_cast<const byte_t*>(data);
    uint64_t hash = 14695981039346656037ULL;
    for(size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

/*
 * @brief Finalizer from SplitMix64; spreads integer keys across the hash table.
 */
inline uint64_t HashInteger(uint64_t value) {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ULL;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebULL;
    value ^= value >> 31;
    return value;
}

/*
 * @brief Hash function object used by HashMap. Specialize Hash() for custom key types.
 */
template<typename T, typename TEnable = void>
class Hasher;

template<typename T>
class Hasher<T, std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>> {
public:
    static uint64_t Hash(T value) { return HashInteger(static_cast<uint64_t>(value)); }
};

template<typename T>
class Hasher<T*> {
public:
    // Pointer identity hashing; reinterpret_cast is the only way to read an address as an integer
    static uint64_t Hash(const T* value) { return HashInteger(reinterpret_cast<uintptr_t>(value)); }
};

} // namespace LiLib

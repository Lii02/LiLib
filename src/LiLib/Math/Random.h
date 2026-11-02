#pragma once
#include "LiLib/Core/Typedefs.h"

namespace LiLib {

/*
 * @brief Deterministic xorshift random number generator.
 */
class Random {
public:
    explicit Random(uint64_t seed = 0x9E3779B97F4A7C15ULL) { SetSeed(seed); }

    uint64_t NextU64() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return state;
    }

    // Returns a float in [0, 1)
    float NextFloat() { return static_cast<float>(NextU64() >> 40) / static_cast<float>(1ULL << 24); }
    float Range(float low, float high) { return low + (high - low) * NextFloat(); }

    // Zero is a fixed point of xorshift, so it is replaced with a nonzero seed
    void SetSeed(uint64_t seed) { state = seed != 0 ? seed : 1; }

private:
    uint64_t state = 1;
};

} // namespace LiLib

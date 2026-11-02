#pragma once
#include "LiLib/Core/Typedefs.h"
#include "LiLib/Math/MathUtil.h"

namespace LiLib {

/*
 * @brief Linear RGBA color with float channels in [0, 1] (values above 1 are allowed for HDR light).
 */
class Color {
public:
    Color operator*(Color other) const { return {r * other.r, g * other.g, b * other.b, a * other.a}; }
    bool operator==(Color other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }

    Color Lerp(Color to, float t) const { return {r + (to.r - r) * t, g + (to.g - g) * t, b + (to.b - b) * t, a + (to.a - a) * t}; }

    /*
     * @brief Builds a color from 0xRRGGBBAA.
     */
    static Color FromHex(uint32_t rgba) {
        return {static_cast<float>((rgba >> 24) & 0xFF) / 255.0f, static_cast<float>((rgba >> 16) & 0xFF) / 255.0f,
                static_cast<float>((rgba >> 8) & 0xFF) / 255.0f, static_cast<float>(rgba & 0xFF) / 255.0f};
    }

    /*
     * @brief Packs the color into 0xRRGGBBAA. Channels are clamped to [0, 1] and rounded to the nearest byte.
     */
    uint32_t ToHex() const {
        auto toByte = [](float channel) -> uint32_t {
            return static_cast<uint32_t>(Math::Saturate(channel) * 255.0f + 0.5f);
        };
        return (toByte(r) << 24) | (toByte(g) << 16) | (toByte(b) << 8) | toByte(a);
    }

    float r;
    float g;
    float b;
    float a;
};

namespace Colors {

constexpr Color White = {1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color Black = {0.0f, 0.0f, 0.0f, 1.0f};
constexpr Color Transparent = {0.0f, 0.0f, 0.0f, 0.0f};
constexpr Color Red = {1.0f, 0.0f, 0.0f, 1.0f};
constexpr Color Green = {0.0f, 1.0f, 0.0f, 1.0f};
constexpr Color Blue = {0.0f, 0.0f, 1.0f, 1.0f};
constexpr Color Yellow = {1.0f, 1.0f, 0.0f, 1.0f};

} // namespace Colors

} // namespace LiLib

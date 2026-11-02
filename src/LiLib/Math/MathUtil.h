#pragma once
#include "LiLib/Core/Typedefs.h"

namespace LiLib {

class Math {
public:
    static constexpr float Pi = 3.14159265358979323846f;
    static constexpr float TwoPi = Pi * 2.0f;
    static constexpr float DegToRad = Pi / 180.0f;
    static constexpr float RadToDeg = 180.0f / Pi;

    static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    static float Saturate(float value) { return Clamp(value, 0.0f, 1.0f); }
};

} // namespace LiLib

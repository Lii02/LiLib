#pragma once
#include "LiLib/Math/Vec2.h"

namespace LiLib {

/*
 * @brief Axis-aligned rectangle given by its minimum corner and size.
 */
class Rect {
public:
    bool Contains(Vec2 point) const { return point.x >= x && point.y >= y && point.x <= x + width && point.y <= y + height; }
    bool Overlaps(const Rect& other) const {
        return x < other.x + other.width && other.x < x + width && y < other.y + other.height && other.y < y + height;
    }

    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

} // namespace LiLib

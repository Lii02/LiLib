#pragma once
#include "LiLib/Containers/Array.h"
#include "LiLib/Math/Color.h"
#include "LiLib/Math/Vec2.h"
#include "LiLib/Math/Vec3.h"
#include "LiLib/Strings/String.h"

namespace LiLib {

class TypeInfo;

enum class VariantType : uint8_t {
    NONE,
    BOOL,
    INT,
    UINT,
    FLOAT,
    VEC2,
    VEC3,
    COLOR,
    STRING,
    INT_ARRAY,
    OBJECT
};

/*
 * @brief Non-owning reference to a reflected object. type is null for objects of unregistered types.
 */
struct ObjectRef {
    void* instance = nullptr;
    const TypeInfo* type = nullptr;
};

/*
 * @brief Type-erased value that crosses the reflection boundary: property values, method arguments and
 *        return values, global variables. Numeric getters convert between bool, int, uint and float, so
 *        a script can pass 1 where a float is expected.
 */
class Variant {
public:
    Variant() = default;

    static Variant FromBool(bool value);
    static Variant FromInt(int64_t value);
    static Variant FromUInt(uint64_t value);
    static Variant FromFloat(double value);
    static Variant FromVec2(Vec2 value);
    static Variant FromVec3(Vec3 value);
    static Variant FromColor(Color value);
    static Variant FromString(StringView value);
    static Variant FromIntArray(const Array<int>& value);
    static Variant FromObject(ObjectRef value);

    VariantType GetType() const { return type; }
    bool IsValid() const { return type != VariantType::NONE; }
    bool IsNumeric() const {
        return type == VariantType::BOOL || type == VariantType::INT || type == VariantType::UINT || type == VariantType::FLOAT;
    }

    bool AsBool(bool fallback = false) const;
    int64_t AsInt(int64_t fallback = 0) const;
    uint64_t AsUInt(uint64_t fallback = 0) const;
    double AsFloat(double fallback = 0.0) const;
    Vec2 AsVec2(Vec2 fallback = {0.0f, 0.0f}) const;
    Vec3 AsVec3(Vec3 fallback = {0.0f, 0.0f, 0.0f}) const;
    Color AsColor(Color fallback = Colors::White) const;
    StringView AsString(StringView fallback = StringView()) const;
    const Array<int>& AsIntArray() const { return intArray; }
    ObjectRef AsObject() const { return type == VariantType::OBJECT ? object : ObjectRef{}; }

    /*
     * @brief Whether the value can be read as target. Numeric types convert freely between each other.
     */
    bool CanConvertTo(VariantType target) const;

private:
    VariantType type = VariantType::NONE;
    union {
        bool boolean;
        int64_t sint;
        uint64_t uint;
        double real;
    } scalar = {};
    Vec2 vec2 = {0.0f, 0.0f};
    Vec3 vec3 = {0.0f, 0.0f, 0.0f};
    Color color = Colors::White;
    String text;
    Array<int> intArray;
    ObjectRef object;
};

} // namespace LiLib

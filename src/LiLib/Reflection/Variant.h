#pragma once
#include "LiLib/Containers/Array.h"
#include "LiLib/Math/AABB.h"
#include "LiLib/Math/Color.h"
#include "LiLib/Math/Frustum.h"
#include "LiLib/Math/Quat.h"
#include "LiLib/Math/Vec2.h"
#include "LiLib/Math/Vec3.h"
#include "LiLib/Math/Vec4.h"
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
    VEC4,
    QUAT,
    AABB,
    FRUSTUM,
    COLOR,
    STRING,
    ARRAY,
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
    Variant(const Variant& other);
    Variant(Variant&& other) noexcept;
    Variant& operator=(const Variant& other);
    Variant& operator=(Variant&& other) noexcept;
    ~Variant();

    static Variant FromBool(bool value);
    static Variant FromInt(int64_t value);
    static Variant FromUInt(uint64_t value);
    static Variant FromFloat(double value);
    static Variant FromVec2(Vec2 value);
    static Variant FromVec3(Vec3 value);
    static Variant FromVec4(Vec4 value);
    static Variant FromQuat(Quat value);
    static Variant FromAABB(AABB value);
    static Variant FromFrustum(Frustum value);
    static Variant FromColor(Color value);
    static Variant FromString(StringView value);
    static Variant FromArray(const Array<Variant>& value);
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
    Vec4 AsVec4(Vec4 fallback = {0.0f, 0.0f, 0.0f, 0.0f}) const;
    Quat AsQuat(Quat fallback = Quat::Identity()) const;
    AABB AsAABB(AABB fallback = {}) const;
    Frustum AsFrustum(Frustum fallback = {}) const;
    Color AsColor(Color fallback = Colors::White) const;
    StringView AsString(StringView fallback = StringView()) const;
    const Array<Variant>& AsArray() const {
        if(type == VariantType::ARRAY) {
            return data.array;
        }
        static const Array<Variant> empty;
        return empty;
    }
    ObjectRef AsObject() const { return type == VariantType::OBJECT ? data.object : ObjectRef{}; }

    /*
     * @brief Whether the value can be read as target. Numeric types convert freely between each other.
     */
    bool CanConvertTo(VariantType target) const;

private:
    union Storage {
        Storage() {}
        ~Storage() {}

        bool boolean;
        int64_t integer;
        uint64_t uinteger;
        double number;
        Vec2 vec2;
        Vec3 vec3;
        Vec4 vec4;
        Quat quat;
        AABB aabb;
        Frustum frustum;
        Color color;
        String string;
        Array<Variant> array;
        ObjectRef object;
    };

    void _Destroy();
    void _CopyFrom(const Variant& other);
    void _MoveFrom(Variant& other);

    VariantType type = VariantType::NONE;
    Storage data;
};

} // namespace LiLib

#pragma once
#include "LiLib/Reflection/TypeInfo.h"

namespace LiLib {

template<typename T>
const TypeInfo& GetReflectedType();

/*
 * @brief Maps a C++ type to a Variant representation. A type without a specialization cannot cross the
 *        reflection boundary. To(value) wraps a value; From(variant, out) reads it back and returns false
 *        when the variant holds something incompatible.
 */
template<typename T, typename TEnable = void>
class VariantTraits;

template<>
class VariantTraits<bool> {
public:
    static constexpr VariantType Type = VariantType::BOOL;
    static Variant To(bool value) { return Variant::FromBool(value); }
    static bool From(const Variant& variant, bool& out) {
        out = variant.AsBool();
        return variant.CanConvertTo(Type);
    }
};

template<typename T>
class VariantTraits<T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>> {
public:
    static constexpr VariantType Type = std::is_signed_v<T> ? VariantType::INT : VariantType::UINT;
    static Variant To(T value) {
        if constexpr(std::is_signed_v<T>) {
            return Variant::FromInt(static_cast<int64_t>(value));
        } else {
            return Variant::FromUInt(static_cast<uint64_t>(value));
        }
    }
    static bool From(const Variant& variant, T& out) {
        if constexpr(std::is_signed_v<T>) {
            out = static_cast<T>(variant.AsInt());
        } else {
            out = static_cast<T>(variant.AsUInt());
        }
        return variant.CanConvertTo(Type);
    }
};

template<typename T>
class VariantTraits<T, std::enable_if_t<std::is_floating_point_v<T>>> {
public:
    static constexpr VariantType Type = VariantType::FLOAT;
    static Variant To(T value) { return Variant::FromFloat(static_cast<double>(value)); }
    static bool From(const Variant& variant, T& out) {
        out = static_cast<T>(variant.AsFloat());
        return variant.CanConvertTo(Type);
    }
};

// Enums travel as integers
template<typename T>
class VariantTraits<T, std::enable_if_t<std::is_enum_v<T>>> {
public:
    static constexpr VariantType Type = VariantType::INT;
    static Variant To(T value) { return Variant::FromInt(static_cast<int64_t>(value)); }
    static bool From(const Variant& variant, T& out) {
        out = static_cast<T>(variant.AsInt());
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Vec2> {
public:
    static constexpr VariantType Type = VariantType::VEC2;
    static Variant To(Vec2 value) { return Variant::FromVec2(value); }
    static bool From(const Variant& variant, Vec2& out) {
        out = variant.AsVec2();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Vec3> {
public:
    static constexpr VariantType Type = VariantType::VEC3;
    static Variant To(Vec3 value) { return Variant::FromVec3(value); }
    static bool From(const Variant& variant, Vec3& out) {
        out = variant.AsVec3();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Vec4> {
public:
    static constexpr VariantType Type = VariantType::VEC4;
    static Variant To(Vec4 value) { return Variant::FromVec4(value); }
    static bool From(const Variant& variant, Vec4& out) {
        out = variant.AsVec4();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Quat> {
public:
    static constexpr VariantType Type = VariantType::QUAT;
    static Variant To(Quat value) { return Variant::FromQuat(value); }
    static bool From(const Variant& variant, Quat& out) {
        out = variant.AsQuat();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<AABB> {
public:
    static constexpr VariantType Type = VariantType::AABB;
    static Variant To(AABB value) { return Variant::FromAABB(value); }
    static bool From(const Variant& variant, AABB& out) {
        out = variant.AsAABB();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Frustum> {
public:
    static constexpr VariantType Type = VariantType::FRUSTUM;
    static Variant To(Frustum value) { return Variant::FromFrustum(value); }
    static bool From(const Variant& variant, Frustum& out) {
        out = variant.AsFrustum();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Color> {
public:
    static constexpr VariantType Type = VariantType::COLOR;
    static Variant To(Color value) { return Variant::FromColor(value); }
    static bool From(const Variant& variant, Color& out) {
        out = variant.AsColor();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<String> {
public:
    static constexpr VariantType Type = VariantType::STRING;
    static Variant To(const String& value) { return Variant::FromString(value.View()); }
    static bool From(const Variant& variant, String& out) {
        out = variant.AsString();
        return variant.CanConvertTo(Type);
    }
};

// A StringView argument points into the Variant, so it is only valid while the Variant is alive
template<>
class VariantTraits<StringView> {
public:
    static constexpr VariantType Type = VariantType::STRING;
    static Variant To(StringView value) { return Variant::FromString(value); }
    static bool From(const Variant& variant, StringView& out) {
        out = variant.AsString();
        return variant.CanConvertTo(Type);
    }
};

template<>
class VariantTraits<Array<Variant>> {
public:
    static constexpr VariantType Type = VariantType::ARRAY;
    static Variant To(const Array<Variant>& value) { return Variant::FromArray(value); }
    static bool From(const Variant& variant, Array<Variant>& out) {
        out = variant.AsArray();
        return variant.CanConvertTo(Type);
    }
};

/*
 * @brief Pointers to reflected types. A null pointer stays null; an object of an unrelated type reads back as
 *        null and reports failure.
 */
template<typename T>
class VariantTraits<T*, std::enable_if_t<std::is_class_v<T>>> {
public:
    static constexpr VariantType Type = VariantType::OBJECT;
    static Variant To(T* value) {
        // ObjectRef stores a non-const void*; constness is enforced by the reflected method signatures
        return Variant::FromObject(
            ObjectRef{const_cast<std::remove_const_t<T>*>(value), value ? &GetReflectedType<std::remove_const_t<T>>() : nullptr});
    }
    static bool From(const Variant& variant, T*& out) {
        out = nullptr;
        if(variant.GetType() != Type) {
            return false;
        }
        const ObjectRef object = variant.AsObject();
        if(!object.instance) {
            return true;
        }
        void* adjusted = object.type ? object.type->Upcast(object.instance, GetReflectedType<std::remove_const_t<T>>()) : nullptr;
        out = static_cast<T*>(adjusted);
        return adjusted != nullptr;
    }
};

} // namespace LiLib

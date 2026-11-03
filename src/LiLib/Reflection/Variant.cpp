#include "LiLib/Reflection/Variant.h"
#include <new>
#include <utility>

namespace LiLib {

Variant::Variant(const Variant& other) {
    _CopyFrom(other);
}

Variant::Variant(Variant&& other) noexcept {
    _MoveFrom(other);
}

Variant& Variant::operator=(const Variant& other) {
    if(this != &other) {
        _Destroy();
        _CopyFrom(other);
    }
    return *this;
}

Variant& Variant::operator=(Variant&& other) noexcept {
    if(this != &other) {
        _Destroy();
        _MoveFrom(other);
    }
    return *this;
}

Variant::~Variant() {
    _Destroy();
}

void Variant::_Destroy() {
    switch(type) {
        case VariantType::STRING:
            data.string.~String();
            break;
        case VariantType::ARRAY:
            data.array.~Array<Variant>();
            break;
        default:
            break;
    }
    type = VariantType::NONE;
}

void Variant::_CopyFrom(const Variant& other) {
    switch(other.type) {
        case VariantType::NONE:
            break;
        case VariantType::BOOL:
            new(&data.boolean) bool(other.data.boolean);
            break;
        case VariantType::INT:
            new(&data.integer) int64_t(other.data.integer);
            break;
        case VariantType::UINT:
            new(&data.uinteger) uint64_t(other.data.uinteger);
            break;
        case VariantType::FLOAT:
            new(&data.number) double(other.data.number);
            break;
        case VariantType::VEC2:
            new(&data.vec2) Vec2(other.data.vec2);
            break;
        case VariantType::VEC3:
            new(&data.vec3) Vec3(other.data.vec3);
            break;
        case VariantType::VEC4:
            new(&data.vec4) Vec4(other.data.vec4);
            break;
        case VariantType::QUAT:
            new(&data.quat) Quat(other.data.quat);
            break;
        case VariantType::AABB:
            new(&data.aabb) AABB(other.data.aabb);
            break;
        case VariantType::FRUSTUM:
            new(&data.frustum) Frustum(other.data.frustum);
            break;
        case VariantType::COLOR:
            new(&data.color) Color(other.data.color);
            break;
        case VariantType::STRING:
            new(&data.string) String(other.data.string);
            break;
        case VariantType::ARRAY:
            new(&data.array) Array<Variant>(other.data.array);
            break;
        case VariantType::OBJECT:
            new(&data.object) ObjectRef(other.data.object);
            break;
    }
    type = other.type;
}

void Variant::_MoveFrom(Variant& other) {
    switch(other.type) {
        case VariantType::STRING:
            new(&data.string) String(std::move(other.data.string));
            type = other.type;
            break;
        case VariantType::ARRAY:
            new(&data.array) Array<Variant>(std::move(other.data.array));
            type = other.type;
            break;
        default:
            _CopyFrom(other);
            break;
    }
    other._Destroy();
}

Variant Variant::FromBool(bool value) {
    Variant result;
    new(&result.data.boolean) bool(value);
    result.type = VariantType::BOOL;
    return result;
}

Variant Variant::FromInt(int64_t value) {
    Variant result;
    new(&result.data.integer) int64_t(value);
    result.type = VariantType::INT;
    return result;
}

Variant Variant::FromUInt(uint64_t value) {
    Variant result;
    new(&result.data.uinteger) uint64_t(value);
    result.type = VariantType::UINT;
    return result;
}

Variant Variant::FromFloat(double value) {
    Variant result;
    new(&result.data.number) double(value);
    result.type = VariantType::FLOAT;
    return result;
}

Variant Variant::FromVec2(Vec2 value) {
    Variant result;
    new(&result.data.vec2) Vec2(value);
    result.type = VariantType::VEC2;
    return result;
}

Variant Variant::FromVec3(Vec3 value) {
    Variant result;
    new(&result.data.vec3) Vec3(value);
    result.type = VariantType::VEC3;
    return result;
}

Variant Variant::FromVec4(Vec4 value) {
    Variant result;
    new(&result.data.vec4) Vec4(value);
    result.type = VariantType::VEC4;
    return result;
}

Variant Variant::FromQuat(Quat value) {
    Variant result;
    new(&result.data.quat) Quat(value);
    result.type = VariantType::QUAT;
    return result;
}

Variant Variant::FromAABB(AABB value) {
    Variant result;
    new(&result.data.aabb) AABB(value);
    result.type = VariantType::AABB;
    return result;
}

Variant Variant::FromFrustum(Frustum value) {
    Variant result;
    new(&result.data.frustum) Frustum(value);
    result.type = VariantType::FRUSTUM;
    return result;
}

Variant Variant::FromColor(Color value) {
    Variant result;
    new(&result.data.color) Color(value);
    result.type = VariantType::COLOR;
    return result;
}

Variant Variant::FromString(StringView value) {
    Variant result;
    new(&result.data.string) String(value);
    result.type = VariantType::STRING;
    return result;
}

Variant Variant::FromArray(const Array<Variant>& value) {
    Variant result;
    new(&result.data.array) Array<Variant>(value);
    result.type = VariantType::ARRAY;
    return result;
}

Variant Variant::FromObject(ObjectRef value) {
    Variant result;
    new(&result.data.object) ObjectRef(value);
    result.type = VariantType::OBJECT;
    return result;
}

bool Variant::AsBool(bool fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return data.boolean;
        case VariantType::INT:
            return data.integer != 0;
        case VariantType::UINT:
            return data.uinteger != 0;
        case VariantType::FLOAT:
            return data.number != 0.0;
        default:
            return fallback;
    }
}

int64_t Variant::AsInt(int64_t fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return data.boolean ? 1 : 0;
        case VariantType::INT:
            return data.integer;
        case VariantType::UINT:
            return static_cast<int64_t>(data.uinteger);
        case VariantType::FLOAT:
            return static_cast<int64_t>(data.number);
        default:
            return fallback;
    }
}

uint64_t Variant::AsUInt(uint64_t fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return data.boolean ? 1 : 0;
        case VariantType::INT:
            return static_cast<uint64_t>(data.integer);
        case VariantType::UINT:
            return data.uinteger;
        case VariantType::FLOAT:
            return static_cast<uint64_t>(data.number);
        default:
            return fallback;
    }
}

double Variant::AsFloat(double fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return data.boolean ? 1.0 : 0.0;
        case VariantType::INT:
            return static_cast<double>(data.integer);
        case VariantType::UINT:
            return static_cast<double>(data.uinteger);
        case VariantType::FLOAT:
            return data.number;
        default:
            return fallback;
    }
}

Vec2 Variant::AsVec2(Vec2 fallback) const {
    return type == VariantType::VEC2 ? data.vec2 : fallback;
}

Vec3 Variant::AsVec3(Vec3 fallback) const {
    return type == VariantType::VEC3 ? data.vec3 : fallback;
}

Vec4 Variant::AsVec4(Vec4 fallback) const {
    return type == VariantType::VEC4 ? data.vec4 : fallback;
}

Quat Variant::AsQuat(Quat fallback) const {
    return type == VariantType::QUAT ? data.quat : fallback;
}

AABB Variant::AsAABB(AABB fallback) const {
    return type == VariantType::AABB ? data.aabb : fallback;
}

Frustum Variant::AsFrustum(Frustum fallback) const {
    return type == VariantType::FRUSTUM ? data.frustum : fallback;
}

Color Variant::AsColor(Color fallback) const {
    return type == VariantType::COLOR ? data.color : fallback;
}

StringView Variant::AsString(StringView fallback) const {
    return type == VariantType::STRING ? data.string.View() : fallback;
}

bool Variant::CanConvertTo(VariantType target) const {
    if(type == target) {
        return true;
    }
    const bool numericTarget =
        target == VariantType::BOOL || target == VariantType::INT || target == VariantType::UINT || target == VariantType::FLOAT;
    return numericTarget && IsNumeric();
}

} // namespace LiLib

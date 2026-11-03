#include "LiLib/Reflection/Variant.h"

namespace LiLib {

Variant Variant::FromBool(bool value) {
    Variant result;
    result.type = VariantType::BOOL;
    result.scalar.boolean = value;
    return result;
}

Variant Variant::FromInt(int64_t value) {
    Variant result;
    result.type = VariantType::INT;
    result.scalar.sint = value;
    return result;
}

Variant Variant::FromUInt(uint64_t value) {
    Variant result;
    result.type = VariantType::UINT;
    result.scalar.uint = value;
    return result;
}

Variant Variant::FromFloat(double value) {
    Variant result;
    result.type = VariantType::FLOAT;
    result.scalar.real = value;
    return result;
}

Variant Variant::FromVec2(Vec2 value) {
    Variant result;
    result.type = VariantType::VEC2;
    result.vec2 = value;
    return result;
}

Variant Variant::FromVec3(Vec3 value) {
    Variant result;
    result.type = VariantType::VEC3;
    result.vec3 = value;
    return result;
}

Variant Variant::FromColor(Color value) {
    Variant result;
    result.type = VariantType::COLOR;
    result.color = value;
    return result;
}

Variant Variant::FromString(StringView value) {
    Variant result;
    result.type = VariantType::STRING;
    result.text = value;
    return result;
}

Variant Variant::FromIntArray(const Array<int>& value) {
    Variant result;
    result.type = VariantType::INT_ARRAY;
    result.intArray = value;
    return result;
}

Variant Variant::FromObject(ObjectRef value) {
    Variant result;
    result.type = VariantType::OBJECT;
    result.object = value;
    return result;
}

bool Variant::AsBool(bool fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return scalar.boolean;
        case VariantType::INT:
            return scalar.sint != 0;
        case VariantType::UINT:
            return scalar.uint != 0;
        case VariantType::FLOAT:
            return scalar.real != 0.0;
        default:
            return fallback;
    }
}

int64_t Variant::AsInt(int64_t fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return scalar.boolean ? 1 : 0;
        case VariantType::INT:
            return scalar.sint;
        case VariantType::UINT:
            return static_cast<int64_t>(scalar.uint);
        case VariantType::FLOAT:
            return static_cast<int64_t>(scalar.real);
        default:
            return fallback;
    }
}

uint64_t Variant::AsUInt(uint64_t fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return scalar.boolean ? 1 : 0;
        case VariantType::INT:
            return static_cast<uint64_t>(scalar.sint);
        case VariantType::UINT:
            return scalar.uint;
        case VariantType::FLOAT:
            return static_cast<uint64_t>(scalar.real);
        default:
            return fallback;
    }
}

double Variant::AsFloat(double fallback) const {
    switch(type) {
        case VariantType::BOOL:
            return scalar.boolean ? 1.0 : 0.0;
        case VariantType::INT:
            return static_cast<double>(scalar.sint);
        case VariantType::UINT:
            return static_cast<double>(scalar.uint);
        case VariantType::FLOAT:
            return scalar.real;
        default:
            return fallback;
    }
}

Vec2 Variant::AsVec2(Vec2 fallback) const {
    return type == VariantType::VEC2 ? vec2 : fallback;
}

Vec3 Variant::AsVec3(Vec3 fallback) const {
    return type == VariantType::VEC3 ? vec3 : fallback;
}

Color Variant::AsColor(Color fallback) const {
    return type == VariantType::COLOR ? color : fallback;
}

StringView Variant::AsString(StringView fallback) const {
    return type == VariantType::STRING ? text.View() : fallback;
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

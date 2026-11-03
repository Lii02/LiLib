#pragma once
#include "LiLib/Core/Assert.h"
#include "LiLib/Memory/Memory.h"
#include "LiLib/Reflection/VariantTraits.h"
#include <initializer_list>

namespace LiLib {

namespace Detail {

template<typename T>
std::decay_t<T> ReadArgument(const Variant& variant) {
    std::decay_t<T> value{};
    VariantTraits<std::decay_t<T>>::From(variant, value);
    return value;
}

template<typename TReturn, typename... TArgs, typename TCall, size_t... TIndices>
Variant CallWithArguments(TCall&& call, const Variant* args, std::index_sequence<TIndices...>) {
    LI_UNUSED(args);
    if constexpr(std::is_void_v<TReturn>) {
        call(ReadArgument<TArgs>(args[TIndices])...);
        return Variant();
    } else {
        return VariantTraits<std::decay_t<TReturn>>::To(call(ReadArgument<TArgs>(args[TIndices])...));
    }
}

template<typename TReturn, typename... TArgs>
void DescribeSignature(MethodInfo& method) {
    (method.paramTypes.Push(VariantTraits<std::decay_t<TArgs>>::Type), ...);
    if constexpr(std::is_void_v<TReturn>) {
        method.returnType = VariantType::NONE;
    } else {
        method.returnType = VariantTraits<std::decay_t<TReturn>>::Type;
    }
}

/*
 * @brief Describes a free or static function.
 */
template<typename TReturn, typename... TArgs>
MethodInfo MakeFunctionInfo(const char* name, TReturn (*function)(TArgs...)) {
    MethodInfo method;
    method.name = name;
    method.isStatic = true;
    DescribeSignature<TReturn, TArgs...>(method);
    method.invoke = [function](void*, const Variant* args) {
        return CallWithArguments<TReturn, TArgs...>(
            [function](auto&&... values) -> TReturn { return function(std::forward<decltype(values)>(values)...); }, args,
            std::index_sequence_for<TArgs...>());
    };
    return method;
}

template<typename TValue>
constexpr PropertyKind KindOf() {
    if constexpr(std::is_same_v<TValue, bool>) {
        return PropertyKind::BOOL;
    } else if constexpr(std::is_integral_v<TValue> && !std::is_same_v<TValue, bool>) {
        return PropertyKind::INT;
    } else if constexpr(std::is_floating_point_v<TValue>) {
        return PropertyKind::FLOAT;
    } else if constexpr(std::is_same_v<TValue, Vec2>) {
        return PropertyKind::VECTOR2;
    } else if constexpr(std::is_same_v<TValue, Vec3>) {
        return PropertyKind::VECTOR3;
    } else if constexpr(std::is_same_v<TValue, Vec4>) {
        return PropertyKind::VECTOR4;
    } else if constexpr(std::is_same_v<TValue, Quat>) {
        return PropertyKind::QUAT;
    } else if constexpr(std::is_same_v<TValue, AABB>) {
        return PropertyKind::AABB;
    } else if constexpr(std::is_same_v<TValue, Frustum>) {
        return PropertyKind::FRUSTUM;
    } else if constexpr(std::is_same_v<TValue, Color>) {
        return PropertyKind::COLOR;
    } else if constexpr(std::is_same_v<TValue, String>) {
        return PropertyKind::TEXT;
    } else if constexpr(std::is_same_v<TValue, Array<Variant>>) {
        return PropertyKind::ARRAY;
    } else {
        static_assert(sizeof(TValue) == 0, "No property kind for this type; specialize VariantTraits or use Enum()");
        return PropertyKind::INT;
    }
}

} // namespace Detail

/*
 * @brief Describes a reflected type. Used from a type's static Reflect function:
 *
 *        void Light::Reflect(TypeBuilder<Light>& type) {
 *            type.Property("color", &Light::color)
 *                .Property("intensity", &Light::intensity)
 *                .Method("Toggle", &Light::Toggle);
 *        }
 *
 *        Properties are saved, edited and scriptable by default. Serialized, Editable and Scriptable change that
 *        for the property added last.
 */
template<typename T>
class TypeBuilder {
public:
    explicit TypeBuilder(TypeInfo& info) : info(info) {
        if constexpr(std::is_default_constructible_v<T> && !std::is_abstract_v<T>) {
            info.construct = []() -> void* {
                return Memory::New<T>();
            };
            info.destroy = [](void* object) {
                Memory::Delete(static_cast<T*>(object));
            };
        }
    }

    /*
     * @brief Declares the base type, whose properties and methods are inherited.
     */
    template<typename TBase>
    TypeBuilder& Base() {
        static_assert(std::is_base_of_v<TBase, T>, "Base must be a base class of the reflected type");
        info.base = &GetReflectedType<TBase>();
        info.upcastToBase = [](void* object) {
            return static_cast<void*>(static_cast<TBase*>(static_cast<T*>(object)));
        };
        return *this;
    }

    /*
     * @brief Section title in the inspector.
     */
    TypeBuilder& Header(const char* name) {
        _AddProperty(name, PropertyKind::HEADER, PropertyHint::NONE, VariantType::NONE);
        PropertyInfo& property = info.properties.Back();
        property.serialized = false;
        property.scriptable = false;
        return *this;
    }

    /*
     * @brief A data member. The kind follows the member's type: bool, integer, float, Vec2, Vec3, Vec4, Quat,
     *        AABB, Frustum, Color, String or Array<Variant>.
     */
    template<typename TValue, typename TClass>
    TypeBuilder& Property(const char* name, TValue TClass::* member, PropertyHint hint = PropertyHint::NONE) {
        using Traits = VariantTraits<TValue>;
        PropertyInfo& property = _AddProperty(name, Detail::KindOf<TValue>(), hint, Traits::Type);
        property.get = [member](const void* object) {
            return Traits::To(static_cast<const T*>(object)->*member);
        };
        property.set = [member](void* object, const Variant& value) {
            TValue converted{};
            if(!Traits::From(value, converted)) {
                return false;
            }
            static_cast<T*>(object)->*member = std::move(converted);
            return true;
        };
        return *this;
    }

    /*
     * @brief An enum or int member shown as a dropdown and saved by option name. The value is the option index.
     */
    template<typename TValue, typename TClass>
    TypeBuilder& Enum(const char* name, TValue TClass::* member, std::initializer_list<const char*> options) {
        using Traits = VariantTraits<TValue>;
        PropertyInfo& property = _AddProperty(name, PropertyKind::ENUM, PropertyHint::NONE, VariantType::INT);
        for(const char* option : options) {
            property.enumOptions.Push(String(option));
        }
        const int64_t optionCount = static_cast<int64_t>(options.size());
        property.get = [member](const void* object) {
            return Traits::To(static_cast<const T*>(object)->*member);
        };
        property.set = [member, optionCount](void* object, const Variant& value) {
            const int64_t index = value.AsInt(-1);
            if(!value.IsNumeric() || index < 0 || index >= optionCount) {
                return false;
            }
            static_cast<T*>(object)->*member = static_cast<TValue>(index);
            return true;
        };
        return *this;
    }

    /*
     * @brief A property backed by functions instead of a member: getter(const T&) and setter(T&, TValue).
     *        Use it when the stored form differs from the exposed one, or when setting has side effects.
     */
    template<typename TGetter, typename TSetter>
    TypeBuilder& Accessor(const char* name, TGetter getter, TSetter setter, PropertyHint hint = PropertyHint::NONE) {
        using TValue = std::decay_t<std::invoke_result_t<TGetter, const T&>>;
        using Traits = VariantTraits<TValue>;
        PropertyInfo& property = _AddProperty(name, Detail::KindOf<TValue>(), hint, Traits::Type);
        property.get = [getter](const void* object) {
            return Traits::To(getter(*static_cast<const T*>(object)));
        };
        property.set = [setter](void* object, const Variant& value) {
            TValue converted{};
            if(!Traits::From(value, converted)) {
                return false;
            }
            setter(*static_cast<T*>(object), std::move(converted));
            return true;
        };
        return *this;
    }

    /*
     * @brief A computed value with no setter. Visible to scripts only.
     */
    template<typename TGetter>
    TypeBuilder& ReadOnly(const char* name, TGetter getter) {
        using TValue = std::decay_t<std::invoke_result_t<TGetter, const T&>>;
        using Traits = VariantTraits<TValue>;
        PropertyInfo& property = _AddProperty(name, Detail::KindOf<TValue>(), PropertyHint::NONE, Traits::Type);
        property.serialized = false;
        property.editable = false;
        property.get = [getter](const void* object) {
            return Traits::To(getter(*static_cast<const T*>(object)));
        };
        return *this;
    }

    /*
     * @brief Whether the property of the last Property/Enum/... call is written to data files.
     */
    TypeBuilder& Serialized(bool value) {
        _Last().serialized = value;
        return *this;
    }

    TypeBuilder& Editable(bool value) {
        _Last().editable = value;
        return *this;
    }

    TypeBuilder& Scriptable(bool value) {
        _Last().scriptable = value;
        return *this;
    }

    template<typename TReturn, typename TClass, typename... TArgs>
    TypeBuilder& Method(const char* name, TReturn (TClass::*function)(TArgs...)) {
        MethodInfo method = _DescribeMember<TReturn, TArgs...>(name, false);
        method.invoke = [function](void* object, const Variant* args) {
            T* self = static_cast<T*>(object);
            return Detail::CallWithArguments<TReturn, TArgs...>(
                [self, function](auto&&... values) -> TReturn { return (self->*function)(std::forward<decltype(values)>(values)...); },
                args, std::index_sequence_for<TArgs...>());
        };
        info.methods.Push(std::move(method));
        return *this;
    }

    template<typename TReturn, typename TClass, typename... TArgs>
    TypeBuilder& Method(const char* name, TReturn (TClass::*function)(TArgs...) const) {
        MethodInfo method = _DescribeMember<TReturn, TArgs...>(name, true);
        method.invoke = [function](void* object, const Variant* args) {
            const T* self = static_cast<const T*>(object);
            return Detail::CallWithArguments<TReturn, TArgs...>(
                [self, function](auto&&... values) -> TReturn { return (self->*function)(std::forward<decltype(values)>(values)...); },
                args, std::index_sequence_for<TArgs...>());
        };
        info.methods.Push(std::move(method));
        return *this;
    }

    /*
     * @brief A static member function or free function exposed as a method of the type.
     */
    template<typename TReturn, typename... TArgs>
    TypeBuilder& Method(const char* name, TReturn (*function)(TArgs...)) {
        MethodInfo method = Detail::MakeFunctionInfo(name, function);
        method.owner = &info;
        info.methods.Push(std::move(method));
        return *this;
    }

private:
    PropertyInfo& _AddProperty(const char* name, PropertyKind kind, PropertyHint hint, VariantType valueType) {
        PropertyInfo& property = info.properties.Emplace();
        property.name = name;
        property.kind = kind;
        property.hint = hint;
        property.valueType = valueType;
        property.owner = &info;
        return property;
    }

    PropertyInfo& _Last() {
        LI_ASSERT(!info.properties.IsEmpty(), "No property to modify");
        return info.properties.Back();
    }

    template<typename TReturn, typename... TArgs>
    MethodInfo _DescribeMember(const char* name, bool isConst) {
        MethodInfo method;
        method.name = name;
        method.isConst = isConst;
        method.owner = &info;
        Detail::DescribeSignature<TReturn, TArgs...>(method);
        return method;
    }

    TypeInfo& info;
};

} // namespace LiLib

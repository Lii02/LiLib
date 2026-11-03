#include "LiLib/Reflection/TypeInfo.h"

namespace LiLib {

void TypeInfo::GetAllProperties(Array<const PropertyInfo*>& outProperties) const {
    for(const TypeInfo* type = this; type; type = type->base) {
        for(const PropertyInfo& property : type->properties) {
            outProperties.Push(&property);
        }
    }
}

const PropertyInfo* TypeInfo::FindProperty(StringView propertyName) const {
    for(const TypeInfo* type = this; type; type = type->base) {
        for(const PropertyInfo& property : type->properties) {
            if(property.kind != PropertyKind::HEADER && property.name == propertyName) {
                return &property;
            }
        }
    }
    return nullptr;
}

const MethodInfo* TypeInfo::FindMethod(StringView methodName, int argCount) const {
    for(const TypeInfo* type = this; type; type = type->base) {
        for(const MethodInfo& method : type->methods) {
            if(method.name == methodName && (argCount < 0 || static_cast<int>(method.paramTypes.Size()) == argCount)) {
                return &method;
            }
        }
    }
    return nullptr;
}

bool TypeInfo::IsA(const TypeInfo& other) const {
    for(const TypeInfo* type = this; type; type = type->base) {
        if(type == &other) {
            return true;
        }
    }
    return false;
}

void* TypeInfo::Upcast(void* object, const TypeInfo& target) const {
    const TypeInfo* type = this;
    while(type != &target) {
        // The root of a hierarchy has no upcast function, so stop there rather than calling an empty one
        if(type->base == nullptr) {
            return nullptr;
        }
        object = type->upcastToBase(object);
        type = type->base;
    }
    return object;
}

Variant TypeInfo::GetValue(const void* object, const PropertyInfo& property) const {
    // Upcast takes a mutable pointer because it is shared with SetValue; the getter only reads
    void* adjusted = Upcast(const_cast<void*>(object), *property.owner);
    return adjusted && property.get ? property.get(adjusted) : Variant();
}

bool TypeInfo::SetValue(void* object, const PropertyInfo& property, const Variant& value) const {
    void* adjusted = Upcast(object, *property.owner);
    return adjusted && property.set && property.set(adjusted, value);
}

bool TypeInfo::Invoke(void* object, const MethodInfo& method, const Variant* args, size_t argCount, Variant* outResult) const {
    void* adjusted = method.isStatic ? nullptr : Upcast(object, *method.owner);
    if(!method.isStatic && !adjusted) {
        return false;
    }
    return InvokeMethod(method, adjusted, args, argCount, outResult);
}

bool InvokeMethod(const MethodInfo& method, void* object, const Variant* args, size_t argCount, Variant* outResult) {
    if(argCount != method.paramTypes.Size() || !method.invoke) {
        return false;
    }
    for(size_t i = 0; i < argCount; ++i) {
        if(!args[i].CanConvertTo(method.paramTypes[i])) {
            return false;
        }
    }
    Variant result = method.invoke(object, args);
    if(outResult) {
        *outResult = std::move(result);
    }
    return true;
}

} // namespace LiLib

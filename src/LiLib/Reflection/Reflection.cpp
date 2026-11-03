#include "LiLib/Reflection/Reflection.h"

namespace LiLib {

namespace {

struct Registry {
    Array<UniquePtr<TypeInfo>> types;
    Array<const TypeInfo*> typeViews;
    Array<UniquePtr<MethodInfo>> functions;
    Array<UniquePtr<SingletonInfo>> singletons;
    Array<UniquePtr<GlobalInfo>> globals;
};

Registry& GetRegistry() {
    static Registry registry;
    return registry;
}

template<typename T>
const T* FindByName(const Array<UniquePtr<T>>& items, StringView name) {
    for(size_t i = 0; i < items.Size(); i++) {
        if(items[i]->name.View() == name) {
            return items[i].Get();
        }
    }
    return nullptr;
}

} // namespace

const TypeInfo* Reflection::FindType(StringView name) {
    const Registry& registry = GetRegistry();
    for(size_t i = 0; i < registry.types.Size(); i++) {
        if(registry.types[i]->GetName() == name) {
            return registry.types[i].Get();
        }
    }
    return nullptr;
}

const Array<const TypeInfo*>& Reflection::GetTypes() {
    return GetRegistry().typeViews;
}

const MethodInfo* Reflection::FindFunction(StringView name) {
    return FindByName(GetRegistry().functions, name);
}

const Array<UniquePtr<MethodInfo>>& Reflection::GetFunctions() {
    return GetRegistry().functions;
}

bool Reflection::CallFunction(StringView name, const Variant* args, size_t argCount, Variant* outResult) {
    const MethodInfo* function = FindFunction(name);
    if(function == nullptr) {
        return false;
    }
    return InvokeMethod(*function, nullptr, args, argCount, outResult);
}

const SingletonInfo* Reflection::FindSingleton(StringView name) {
    return FindByName(GetRegistry().singletons, name);
}

const Array<UniquePtr<SingletonInfo>>& Reflection::GetSingletons() {
    return GetRegistry().singletons;
}

ObjectRef Reflection::GetSingleton(StringView name) {
    const SingletonInfo* singleton = FindSingleton(name);
    if(singleton == nullptr) {
        return ObjectRef{nullptr, nullptr};
    }
    return ObjectRef{singleton->get(), singleton->type};
}

const GlobalInfo* Reflection::FindGlobal(StringView name) {
    return FindByName(GetRegistry().globals, name);
}

const Array<UniquePtr<GlobalInfo>>& Reflection::GetGlobals() {
    return GetRegistry().globals;
}

Variant Reflection::GetGlobal(StringView name) {
    const GlobalInfo* global = FindGlobal(name);
    return global != nullptr ? global->get() : Variant();
}

bool Reflection::SetGlobal(StringView name, const Variant& value) {
    const GlobalInfo* global = FindGlobal(name);
    if(global == nullptr || !global->writable) {
        return false;
    }
    return global->set(value);
}

void Reflection::_AddType(UniquePtr<TypeInfo> info) {
    Registry& registry = GetRegistry();
    registry.typeViews.Push(info.Get());
    registry.types.Push(std::move(info));
}

void Reflection::_AddFunction(MethodInfo function) {
    GetRegistry().functions.Push(MakeUnique<MethodInfo>(std::move(function)));
}

void Reflection::_AddSingleton(SingletonInfo singleton) {
    GetRegistry().singletons.Push(MakeUnique<SingletonInfo>(std::move(singleton)));
}

void Reflection::_AddGlobal(GlobalInfo global) {
    GetRegistry().globals.Push(MakeUnique<GlobalInfo>(std::move(global)));
}

} // namespace LiLib

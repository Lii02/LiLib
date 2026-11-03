#pragma once
#include "LiLib/Core/Function.h"
#include "LiLib/Memory/UniquePtr.h"
#include "LiLib/Reflection/TypeBuilder.h"

namespace LiLib {

/*
 * @brief A named global value, backed by a variable or by getter/setter functions.
 */
struct GlobalInfo {
    String name;
    VariantType valueType = VariantType::NONE;
    bool writable = false;
    Function<Variant()> get;
    Function<bool(const Variant&)> set;
};

/*
 * @brief A named application service with a single instance. get returns null while the service does not exist.
 */
struct SingletonInfo {
    String name;
    const TypeInfo* type = nullptr;
    Function<void*()> get;
};

/*
 * @brief Registry of everything reflected: types, global functions, singletons and global values.
 *
 *        A type is reflected by giving it a static TypeName and a static Reflect(TypeBuilder<T>&) function. It is
 *        registered the first time GetType<T>() is called. Registration is not thread safe and is expected to happen at startup.
 *
 *        Lookups return null (or an empty value) for unknown names rather than failing.
 */
class Reflection {
public:
    template<typename T>
    static const TypeInfo& GetType() {
        static const TypeInfo* info = _Build<T>();
        return *info;
    }

    static const TypeInfo* FindType(StringView name);
    static const Array<const TypeInfo*>& GetTypes();

    // Global functions

    template<typename TReturn, typename... TArgs>
    static void RegisterFunction(const char* name, TReturn (*function)(TArgs...)) {
        _AddFunction(Detail::MakeFunctionInfo(name, function));
    }

    static const MethodInfo* FindFunction(StringView name);
    static const Array<UniquePtr<MethodInfo>>& GetFunctions();

    /*
     * @brief Calls a global function. Returns false for an unknown name or arguments that do not match.
     */
    static bool CallFunction(StringView name, const Variant* args, size_t argCount, Variant* outResult = nullptr);

    // Singletons

    template<typename T>
    static void RegisterSingleton(const char* name, T* (*getter)()) {
        SingletonInfo singleton;
        singleton.name = name;
        singleton.type = &GetType<T>();
        singleton.get = [getter]() -> void* {
            return getter();
        };
        _AddSingleton(std::move(singleton));
    }

    static const SingletonInfo* FindSingleton(StringView name);
    static const Array<UniquePtr<SingletonInfo>>& GetSingletons();

    /*
     * @brief The live instance of a singleton; instance is null when unknown or not created yet.
     */
    static ObjectRef GetSingleton(StringView name);

    // Globals

    /*
     * @brief Exposes a variable with static storage. A const variable is read-only.
     */
    template<typename T>
    static void RegisterGlobalVariable(const char* name, T* variable) {
        using TValue = std::remove_const_t<T>;
        using Traits = VariantTraits<TValue>;
        GlobalInfo global;
        global.name = name;
        global.valueType = Traits::Type;
        global.get = [variable]() {
            return Traits::To(*variable);
        };
        if constexpr(!std::is_const_v<T>) {
            global.writable = true;
            global.set = [variable](const Variant& value) {
                TValue converted{};
                if(!Traits::From(value, converted)) {
                    return false;
                }
                *variable = std::move(converted);
                return true;
            };
        }
        _AddGlobal(std::move(global));
    }

    /*
     * @brief Exposes a value read through a getter function, and optionally written through a setter.
     */
    template<typename TGetter>
    static void RegisterGlobal(const char* name, TGetter getter) {
        using Traits = VariantTraits<std::decay_t<std::invoke_result_t<TGetter>>>;
        GlobalInfo global;
        global.name = name;
        global.valueType = Traits::Type;
        global.get = [getter]() {
            return Traits::To(getter());
        };
        _AddGlobal(std::move(global));
    }

    template<typename TGetter, typename TSetter>
    static void RegisterGlobal(const char* name, TGetter getter, TSetter setter) {
        using TValue = std::decay_t<std::invoke_result_t<TGetter>>;
        using Traits = VariantTraits<TValue>;
        GlobalInfo global;
        global.name = name;
        global.valueType = Traits::Type;
        global.writable = true;
        global.get = [getter]() {
            return Traits::To(getter());
        };
        global.set = [setter](const Variant& value) {
            TValue converted{};
            if(!Traits::From(value, converted)) {
                return false;
            }
            setter(std::move(converted));
            return true;
        };
        _AddGlobal(std::move(global));
    }

    static const GlobalInfo* FindGlobal(StringView name);
    static const Array<UniquePtr<GlobalInfo>>& GetGlobals();

    /*
     * @brief Reads a global; an invalid Variant when the name is unknown.
     */
    static Variant GetGlobal(StringView name);

    /*
     * @brief Writes a global. False when it is unknown, read-only, or the value has the wrong type.
     */
    static bool SetGlobal(StringView name, const Variant& value);

private:
    template<typename T>
    static const TypeInfo* _Build() {
        UniquePtr<TypeInfo> info = MakeUnique<TypeInfo>(StringView(T::TypeName));
        TypeInfo* raw = info.Get();
        TypeBuilder<T> builder(*raw);
        T::Reflect(builder);
        _AddType(std::move(info));
        return raw;
    }

    static void _AddType(UniquePtr<TypeInfo> info);
    static void _AddFunction(MethodInfo function);
    static void _AddSingleton(SingletonInfo singleton);
    static void _AddGlobal(GlobalInfo global);
};

template<typename T>
const TypeInfo& GetReflectedType() {
    return Reflection::GetType<T>();
}

} // namespace LiLib

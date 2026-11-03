#include "LiLib/Reflection/Reflection.h"
#include "TestFramework.h"

using namespace LiLib;

namespace {

struct ReflectBase {
    static constexpr const char* TypeName = "Reflection.Base";

    int baseValue = 1;

    int AddToBase(int amount) {
        baseValue += amount;
        return baseValue;
    }

    static void Reflect(TypeBuilder<ReflectBase>& type) {
        type.Property("baseValue", &ReflectBase::baseValue).Method("Add", &ReflectBase::AddToBase);
    }
};

struct ReflectDerived : ReflectBase {
    static constexpr const char* TypeName = "Reflection.Derived";

    int value = 7;
    Vec3 direction = {1.0f, 2.0f, 3.0f};
    String label = "default";

    int Doubled() const { return value * 2; }
    void SetValue(int newValue) { value = newValue; }

    static void Reflect(TypeBuilder<ReflectDerived>& type) {
        type.Base<ReflectBase>()
            .Property("value", &ReflectDerived::value)
            .Property("direction", &ReflectDerived::direction)
            .Property("label", &ReflectDerived::label)
            .ReadOnly("doubled", [](const ReflectDerived& object) { return object.Doubled(); })
            .Method("SetValue", &ReflectDerived::SetValue);
    }
};

int reflectedGlobal = 11;
const int reflectedReadOnlyGlobal = 23;

[[maybe_unused]] const bool reflectionFixturesRegistered = [] {
    Reflection::GetType<ReflectDerived>();
    Reflection::RegisterFunction("Reflection.AddGlobal", +[](int left, int right) { return left + right; });
    Reflection::RegisterGlobalVariable("Reflection.Global", &reflectedGlobal);
    Reflection::RegisterGlobalVariable("Reflection.ReadOnly", &reflectedReadOnlyGlobal);
    return true;
}();

} // namespace

LI_TEST(Reflection, RegistersPropertiesAndInheritance) {
    ReflectDerived object;
    const TypeInfo& type = Reflection::GetType<ReflectDerived>();
    LI_CHECK(type.GetName() == "Reflection.Derived");
    LI_CHECK(type.IsA(Reflection::GetType<ReflectBase>()));
    LI_CHECK(Reflection::FindType("Reflection.Derived") == &type);

    const PropertyInfo* value = type.FindProperty("value");
    LI_REQUIRE(value != nullptr);
    LI_CHECK(type.GetValue(&object, *value).AsInt() == 7);
    LI_CHECK(type.SetValue(&object, *value, Variant::FromFloat(12.0)));
    LI_CHECK(object.value == 12);

    const PropertyInfo* inherited = type.FindProperty("baseValue");
    LI_REQUIRE(inherited != nullptr);
    LI_CHECK(type.SetValue(&object, *inherited, Variant::FromInt(20)));
    LI_CHECK(object.baseValue == 20);

    const PropertyInfo* doubled = type.FindProperty("doubled");
    LI_REQUIRE(doubled != nullptr);
    LI_CHECK(type.GetValue(&object, *doubled).AsInt() == 24);
    LI_CHECK(!type.SetValue(&object, *doubled, Variant::FromInt(0)));
}

LI_TEST(Reflection, InvokesMethodsAndConvertsObjectPointers) {
    ReflectDerived object;
    const TypeInfo& type = Reflection::GetType<ReflectDerived>();
    const Variant setArgs[] = {Variant::FromInt(15)};
    const MethodInfo* setValue = type.FindMethod("SetValue", 1);
    LI_REQUIRE(setValue != nullptr);
    LI_CHECK(type.Invoke(&object, *setValue, setArgs, 1));
    LI_CHECK(object.value == 15);

    const Variant addArgs[] = {Variant::FromInt(3)};
    const MethodInfo* add = type.FindMethod("Add", 1);
    LI_REQUIRE(add != nullptr);
    Variant result;
    LI_CHECK(type.Invoke(&object, *add, addArgs, 1, &result));
    LI_CHECK(result.AsInt() == 4);

    Variant derived = VariantTraits<ReflectDerived*>::To(&object);
    ReflectBase* base = nullptr;
    LI_CHECK(VariantTraits<ReflectBase*>::From(derived, base));
    LI_CHECK(base == static_cast<ReflectBase*>(&object));
}

LI_TEST(Reflection, ConstructsAndDestroysReflectedTypes) {
    const TypeInfo& type = Reflection::GetType<ReflectDerived>();
    LI_CHECK(type.CanConstruct());
    void* object = type.Construct();
    LI_REQUIRE(object != nullptr);
    LI_CHECK(static_cast<ReflectDerived*>(object)->value == 7);
    type.Destroy(object);
}

LI_TEST(Reflection, RegistersGlobalFunctionsAndValues) {
    const Variant args[] = {Variant::FromInt(5), Variant::FromFloat(2.0)};
    Variant result;
    LI_CHECK(Reflection::CallFunction("Reflection.AddGlobal", args, 2, &result));
    LI_CHECK(result.AsInt() == 7);
    LI_CHECK(!Reflection::CallFunction("Reflection.AddGlobal", args, 1));

    LI_CHECK(Reflection::GetGlobal("Reflection.Global").AsInt() == 11);
    LI_CHECK(Reflection::SetGlobal("Reflection.Global", Variant::FromInt(19)));
    LI_CHECK(reflectedGlobal == 19);
    LI_CHECK(!Reflection::SetGlobal("Reflection.Global", Variant::FromString("wrong type")));

    LI_CHECK(Reflection::GetGlobal("Reflection.ReadOnly").AsInt() == 23);
    LI_CHECK(!Reflection::SetGlobal("Reflection.ReadOnly", Variant::FromInt(1)));
}

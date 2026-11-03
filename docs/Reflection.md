# Reflection

LiLib's reflection module provides runtime type information, inherited properties and methods, type-erased values, object construction, and registries for functions, singletons, and global values. It has no application- or engine-specific registration; each project registers the types and values it needs.

## Reflecting a type

Give a type a stable `TypeName` and a static `Reflect` function. Use `TypeBuilder` to declare the data and methods that should be visible:

```cpp
#include "LiLib/Reflection/Reflection.h"

using namespace LiLib;

struct Actor {
    static constexpr const char* TypeName = "Actor";

    String name = "Player";
    float speed = 4.0f;

    void Move(float distance) { speed += distance; }

    static void Reflect(TypeBuilder<Actor>& type) {
        type.Property("name", &Actor::name)
            .Property("speed", &Actor::speed)
            .Method("Move", &Actor::Move);
    }
};

int main() {
    const TypeInfo& actorType = Reflection::GetType<Actor>();
    Actor actor;
    const PropertyInfo* speed = actorType.FindProperty("speed");
    actorType.SetValue(&actor, *speed, Variant::FromFloat(6.0));
}
```

`GetType<T>()` registers the type on first use. Call `FindType()` to look up a type by its declared name. Register base classes with `type.Base<Base>()`; their properties and methods are then available through the derived type. Default-constructible, non-abstract types can be created with `TypeInfo::Construct()` and released with `TypeInfo::Destroy()`.

Properties support `bool`, integral and floating-point values, `Vec2`, `Vec3`, `Vec4`, `Quat`, `AABB`, `Frustum`, `Color`, `String`, and `Array<Variant>`. Use `Enum()` for a value represented by named options, `Accessor()` for getter/setter-backed properties, and `ReadOnly()` for computed values. `Serialized()`, `Editable()`, and `Scriptable()` set metadata on the most recently added property. Specialize `VariantTraits<T>` to expose additional types in properties, methods, and globals.

## Variant

`Variant` is the type-erased value used for property values, method arguments, return values, and globals. It holds one of: none, `bool`, signed or unsigned integer, `double`, `Vec2`, `Vec3`, `Vec4`, `Quat`, `AABB`, `Frustum`, `Color`, `String`, an array of `Variant`s (which can nest), or an `ObjectRef` (a non-owning pointer plus its `TypeInfo`). Create one with `Variant::FromX()`, check it with `GetType()`, and read it with `AsX()`, which returns a fallback when the type doesn't match. Use `AsArray()` to read an array.

## Registering application values

Register global functions, singleton getters, and global values during application startup:

```cpp
Reflection::RegisterFunction("Math.Add", +[](int a, int b) { return a + b; });
Reflection::RegisterGlobalVariable("App.Version", &version);
Reflection::RegisterGlobal("App.IsPaused", [] { return isPaused; }, [](bool value) { isPaused = value; });
```

`CallFunction()`, `GetGlobal()`, and `SetGlobal()` access the registrations by name. `RegisterGlobalVariable()` can also take a pointer to a const variable to expose a read-only value. Numeric `Variant` values convert between boolean, signed integer, unsigned integer, and floating-point forms; other values must match their registered type.

The registry is process-wide and registration is not thread-safe, so complete registration before concurrent access. Add `#include "LiLib/Reflection/Reflection.h"` and link to `LiLib::LiLib` as described in [Getting Started](GettingStarted.md).

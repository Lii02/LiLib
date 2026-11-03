#pragma once
#include "LiLib/Core/Function.h"
#include "LiLib/Reflection/Variant.h"

namespace LiLib {

/*
 * @brief Extra information about a property for editors.
 */
enum class PropertyHint : uint8_t {
    NONE,
    TEXTURE,
    FONT,
    AUDIO,
    ANIMATION,
    MULTILINE,
    SCENE,
    ANGLE, // Float stored in radians, edited in degrees
    WHOLE, // Float edited and saved as a whole number
    MODEL
};

/*
 * @brief How a property is stored, drawn by the inspector and written to data files.
 */
enum class PropertyKind : uint8_t {
    BOOL,
    INT,
    FLOAT,
    VECTOR2,
    VECTOR3,
    COLOR,
    TEXT,
    ENUM,
    INT_ARRAY,
    HEADER // Not a value: a section title in the inspector
};

/*
 * @brief One reflected field. get/set operate on an instance of the owning type; use
 *        TypeInfo::GetValue / SetValue to call them on an instance of a derived type.
 */
struct PropertyInfo {
    String name;
    PropertyKind kind = PropertyKind::BOOL;
    PropertyHint hint = PropertyHint::NONE;
    VariantType valueType = VariantType::NONE;
    Array<String> enumOptions;
    bool serialized = true;
    bool editable = true;
    bool scriptable = true;
    const TypeInfo* owner = nullptr;
    Function<Variant(const void*)> get;
    Function<bool(void*, const Variant&)> set;
};

/*
 * @brief One reflected function: a member of a type, a static function of a type, or a global function.
 *        invoke receives arguments that were already checked against paramTypes.
 */
struct MethodInfo {
    String name;
    Array<VariantType> paramTypes;
    VariantType returnType = VariantType::NONE;
    bool isStatic = false;
    bool isConst = false;
    const TypeInfo* owner = nullptr;
    Function<Variant(void*, const Variant*)> invoke;
};

/*
 * @brief Runtime description of a type: its properties, methods and base type.
 *        Built once through TypeBuilder and immutable afterwards.
 */
class TypeInfo {
public:
    explicit TypeInfo(StringView name) : name(name) {}

    LI_NON_COPYABLE(TypeInfo);

    StringView GetName() const { return name.View(); }
    const TypeInfo* GetBase() const { return base; }

    /*
     * @brief Properties and methods declared by this type, without inherited ones.
     */
    const Array<PropertyInfo>& GetProperties() const { return properties; }
    const Array<MethodInfo>& GetMethods() const { return methods; }

    /*
     * @brief Every property including inherited ones: this type's first, then each base's.
     */
    void GetAllProperties(Array<const PropertyInfo*>& outProperties) const;

    const PropertyInfo* FindProperty(StringView propertyName) const;

    /*
     * @brief Finds a method here or in a base type. argCount selects among overloads; -1 accepts any.
     */
    const MethodInfo* FindMethod(StringView methodName, int argCount = -1) const;

    bool IsA(const TypeInfo& other) const;

    /*
     * @brief Converts a pointer to this type into a pointer to base type target; null when unrelated.
     */
    void* Upcast(void* object, const TypeInfo& target) const;

    /*
     * @brief Property access on an instance of this type (or a type derived from it).
     */
    Variant GetValue(const void* object, const PropertyInfo& property) const;
    bool SetValue(void* object, const PropertyInfo& property, const Variant& value) const;

    /*
     * @brief Calls a method on an instance. Returns false when the argument count or types do not match.
     *        object may be null for static methods. outResult receives the return value when given.
     */
    bool Invoke(void* object, const MethodInfo& method, const Variant* args, size_t argCount, Variant* outResult = nullptr) const;

    bool CanConstruct() const { return static_cast<bool>(construct); }

    /*
     * @brief Creates a default instance; null when the type is abstract or not default-constructible.
     *        Release it with Destroy.
     */
    void* Construct() const { return construct ? construct() : nullptr; }
    void Destroy(void* object) const {
        if(destroy && object) {
            destroy(object);
        }
    }

private:
    template<typename T>
    friend class TypeBuilder;

    String name;
    const TypeInfo* base = nullptr;
    Function<void*(void*)> upcastToBase;
    Array<PropertyInfo> properties;
    Array<MethodInfo> methods;
    Function<void*()> construct;
    Function<void(void*)> destroy;
};

/*
 * @brief Validates arguments against method and calls it. Shared by type methods and global functions.
 */
bool InvokeMethod(const MethodInfo& method, void* object, const Variant* args, size_t argCount, Variant* outResult);

} // namespace LiLib

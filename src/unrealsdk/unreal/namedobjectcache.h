#ifndef UNREALSDK_UNREAL_NAMEDOBJECTCACHE_H
#define UNREALSDK_UNREAL_NAMEDOBJECTCACHE_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

class UObject;
class UClass;

/**
 * @brief Provides name based lookup of all objects which are an instance of the templated type.
 * @note All objects of the class must have infinite lifetime - UObject is not allowed.
 *
 * @tparam ObjectType The type to cache.
 * @tparam CacheValue The value to hold in the cache. Defaults to a pointer to the object.
 */
template <typename ObjectType,
          typename CacheType = ObjectType*,
          typename = std::enable_if_t<
              std::conjunction_v<std::is_base_of<UObject, ObjectType>,
                                 std::negation<std::is_same<ObjectType, UObject>>>>>
class NamedObjectCache {
   protected:
    UClass* uclass = nullptr;
    std::unordered_map<FName, CacheType> cache{};

    /**
     * @brief Finds the UClass object for the templated type.
     *
     * @return The UClass object for the templated type.
     */
    [[nodiscard]] virtual UClass* find_uclass(void) const { return find_class<ObjectType>(); }

    /**
     * @brief Adds an object to the cache.
     * @note Must be overwritten if using a custom cache type.
     *
     * @param obj The object to cache.
     * @return The cached value.
     */
    virtual CacheType add_to_cache(ObjectType* obj) {
        cache[obj->Name()] = obj;
        return obj;
    }

    /**
     * @brief Initializes the cache if needed, including populating it with all instances found in
     *        gobjects at the time it's run.
     */
    void ensure_initialized(void) {
        if (!this->cache.empty()) {
            return;
        }

        this->uclass = this->find_uclass();

        for (const auto& obj : unrealsdk::gobjects()) {
            if (!obj->is_instance(this->uclass)) {
                continue;
            }

            this->add_to_cache(reinterpret_cast<ObjectType*>(obj));
        }
    }

   public:
    /**
     * @brief Destroys the cache.
     */
    virtual ~NamedObjectCache() = default;

    /**
     * @brief Finds an instance by name.
     * @note If two objects share a name, calling with their FName returns an undefined instance.
     *       Calling with a string performs a fully qualified lookup, which is less efficient, but
     *       properly handles this case.
     *
     * @param name The object's name.
     * @return The object's associated value, or the default constructed value if unable to find.
     */
    [[nodiscard]] CacheType find(const FName& name) {
        this->ensure_initialized();

        if (!this->cache.contains(name)) {
            return {};
        }

        return this->cache[name];
    }
    [[nodiscard]] CacheType find(std::wstring_view name) {
        this->ensure_initialized();

        auto obj = unrealsdk::find_object(this->uclass, name);
        if (obj == nullptr) {
            return {};
        }
        // This should never really happen, but double check
        if (!obj->is_instance(this->uclass)) {
            throw std::invalid_argument(unrealsdk::utils::narrow(
                std::format(L"Found object of unexpected class when searching for '{}': {}", name,
                            obj->get_path_name())));
        }

        return add_to_cache(reinterpret_cast<ObjectType*>(obj));
    }

    /**
     * @brief Checks if the cache has a value for a particular name.
     *
     * @param name The name to query.
     * @return True if the name has a cached value.
     */
    [[nodiscard]] bool has(const FName& name) const {
        this->ensure_initialized();
        return this->cache.contains(name);
    }
};

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_NAMEDOBJECTCACHE_H */

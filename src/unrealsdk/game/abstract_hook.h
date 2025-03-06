#ifndef UNREALSDK_GAME_ABSTRACT_HOOK_H
#define UNREALSDK_GAME_ABSTRACT_HOOK_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/structs/tarray.h"

#ifndef UNREALSDK_IMPORTING

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UEnum;
class UFunction;
class UObject;
struct FFrame;
struct FLazyObjectPtr;
struct FSoftObjectPtr;
struct FText;
struct TemporaryFString;

template <typename KeyType, typename ValueType>
struct TPair;

}  // namespace unrealsdk::unreal

namespace unrealsdk::unreal::offsets {

struct OffsetList;

}  // namespace unrealsdk::unreal::offsets

namespace unrealsdk::game {

#pragma region Hook Classes

/**
 * @brief Base class for hooking a game.
 */
struct AbstractHook {
   public:
    AbstractHook(void) = default;
    virtual ~AbstractHook() = default;

    /**
     * @brief Hooks the active game.
     * @note Always called before any of the other functions on this object.
     * @note On return, all other functions on this object must be set up and valid to call.
     * @note The unrealsdk globals are not yet valid during this call, and may crash if used.
     */
    virtual void hook(void) = 0;

    /**
     * @brief Performs any additional hooking which requires unrealsdk initialization.
     * @note unrealsdk globals *are* valid during this call.
     * @note Typically used to setup hooks, e.g. for console.
     */
    virtual void post_init(void) {};

    // Inner methods accessed by the global wrappers in `unrealsdk.h` - see there for documentation
    [[nodiscard]] virtual bool is_console_ready(void) const = 0;

    [[nodiscard]] virtual const unreal::GObjects& gobjects(void) const = 0;
    [[nodiscard]] virtual const unreal::GNames& gnames(void) const = 0;
    [[nodiscard]] virtual void* u_malloc(size_t len) const = 0;
    [[nodiscard]] virtual void* u_realloc(void* original, size_t len) const = 0;
    virtual void u_free(void* data) const = 0;
    [[nodiscard]] virtual unreal::UObject* construct_object(
        unreal::UClass* cls,
        unreal::UObject* outer,
        const unreal::FName& name,
        uint64_t flags,
        unreal::UObject* template_obj) const = 0;
    [[nodiscard]] virtual unreal::UObject* find_object(unreal::UClass* cls,
                                                       const std::wstring& name) const = 0;
    [[nodiscard]] virtual unreal::UObject* load_package(const std::wstring& name,
                                                        uint32_t flags) const = 0;

    virtual void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const = 0;
    virtual void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const = 0;
    virtual void process_event(unreal::UObject* object,
                               unreal::UFunction* func,
                               void* params) const = 0;
    virtual void uconsole_output_text(const std::wstring& str) const = 0;
    [[nodiscard]] virtual std::wstring uobject_path_name(const unreal::UObject* obj) const = 0;
    virtual void ftext_as_culture_invariant(unreal::FText* text,
                                            unreal::TemporaryFString&& str) const = 0;
    virtual void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr,
                                       const unreal::UObject* obj) const = 0;
    virtual void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr,
                                       const unreal::UObject* obj) const = 0;
    [[nodiscard]] virtual const unreal::offsets::OffsetList& get_offsets(void) const = 0;
    [[nodiscard]] virtual unreal::TArray<unreal::TPair<unreal::FName, uint64_t>> uenum_get_names(
        const unreal::UEnum* uenum) const = 0;
};

#pragma endregion

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_ABSTRACT_HOOK_H */

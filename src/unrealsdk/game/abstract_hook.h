#ifndef UNREALSDK_GAME_ABSTRACT_HOOK_H
#define UNREALSDK_GAME_ABSTRACT_HOOK_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"

#ifndef UNREALSDK_IMPORTING

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
struct FFrame;
struct FText;
struct TemporaryFString;

}  // namespace unrealsdk::unreal

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
     * @brief Hooks the current game and sets up all other functions on this object.
     */
    virtual void hook(void) = 0;

    // Inner methods accessed by the global wrappers in `unrealsdk.h`
    [[nodiscard]] virtual const unreal::GObjects& gobjects(void) const = 0;
    [[nodiscard]] virtual const unreal::GNames& gnames(void) const = 0;
    virtual void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const = 0;
    virtual void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const = 0;
    [[nodiscard]] virtual void* u_malloc(size_t len) const = 0;
    [[nodiscard]] virtual void* u_realloc(void* original, size_t len) const = 0;
    virtual void u_free(void* data) const = 0;
    virtual void process_event(unreal::UObject* object,
                               unreal::UFunction* func,
                               void* params) const = 0;
    [[nodiscard]] virtual unreal::UObject* construct_object(
        unreal::UClass* cls,
        unreal::UObject* outer,
        const unreal::FName& name,
        decltype(unreal::UObject::ObjectFlags) flags,
        unreal::UObject* template_obj) const = 0;
    virtual void uconsole_output_text(const std::wstring& str) const = 0;
    [[nodiscard]] virtual bool is_console_ready(void) const = 0;
    [[nodiscard]] virtual std::wstring uobject_path_name(const unreal::UObject* obj) const = 0;
    [[nodiscard]] virtual unreal::UObject* find_object(unreal::UClass* cls,
                                                       const std::wstring& name) const = 0;
#ifdef UE4
    virtual void ftext_as_culture_invariant(unreal::FText* text,
                                            unreal::TemporaryFString&& str) const = 0;
#endif
};

#pragma endregion

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_ABSTRACT_HOOK_H */

#ifndef GAME_GAME_HOOK_H
#define GAME_GAME_HOOK_H

#include "pch.h"

#include "unreal/classes/uobject.h"
#include "unreal/structs/fname.h"

using namespace unrealsdk::unreal;

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
struct FFrame;

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
    [[nodiscard]] virtual const GObjects& gobjects(void) const = 0;
    [[nodiscard]] virtual const GNames& gnames(void) const = 0;
    virtual void fname_init(FName* name, const std::wstring& str, int32_t number) const = 0;
    virtual void fname_init(FName* name, const wchar_t* str, int32_t number) const = 0;
    virtual void fframe_step(FFrame* frame, UObject* obj, void* param) const = 0;
    [[nodiscard]] virtual void* u_malloc(size_t len) const = 0;
    [[nodiscard]] virtual void* u_realloc(void* original, size_t len) const = 0;
    virtual void u_free(void* data) const = 0;
    virtual void process_event(UObject* object, UFunction* func, void* params) const = 0;
    [[nodiscard]] virtual UObject* construct_object(UClass* cls,
                                                    UObject* outer,
                                                    const FName& name,
                                                    decltype(UObject::ObjectFlags) flags,
                                                    UObject* template_obj) const = 0;
    virtual void uconsole_output_text(const std::wstring& str) const = 0;
    virtual std::wstring uobject_path_name(const UObject* obj) const = 0;
};

#pragma endregion

}  // namespace unrealsdk::game

#endif /* GAME_GAME_HOOK_H */

#ifndef UNREALSDK_GAME_ABSTRACT_HOOK_H
#define UNREALSDK_GAME_ABSTRACT_HOOK_H

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/structs/fname.h"

#ifndef UNREALSDK_IMPORTING

namespace unrealsdk::unreal {

class GNames;
class GObjects;
class UClass;
class UFunction;
class UObject;
struct FField;
struct FFrame;
struct FLazyObjectPtr;
struct FSoftObjectPtr;
struct FText;
struct TemporaryFString;

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
    [[nodiscard]] virtual bool is_console_ready(void) const;

    [[nodiscard]] virtual const unreal::GObjects& gobjects(void) const;
    [[nodiscard]] virtual void* u_malloc(size_t len) const;
    [[nodiscard]] virtual void* u_realloc(void* original, size_t len) const;
    virtual void u_free(void* data) const;
    [[nodiscard]] virtual unreal::UObject* construct_object(unreal::UClass* cls,
                                                            unreal::UObject* outer,
                                                            const unreal::FName& name,
                                                            uint64_t flags,
                                                            unreal::UObject* template_obj) const;
    [[nodiscard]] virtual unreal::UObject* find_object(unreal::UClass* cls,
                                                       const std::wstring& name) const;
    [[nodiscard]] virtual unreal::UObject* load_package(const std::wstring& name,
                                                        uint32_t flags) const;

    virtual void fname_init(unreal::FName* name, const wchar_t* str, uint32_t number) const;
    [[nodiscard]] virtual std::variant<const std::string_view, const std::wstring_view>
    fname_get_str(const unreal::FName& name) const;

    virtual void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const;
    virtual void process_event(unreal::UObject* object,
                               unreal::UFunction* func,
                               void* params) const;
    virtual void uconsole_output_text(const std::wstring& str) const;
    [[nodiscard]] virtual std::wstring uobject_path_name(const unreal::UObject* obj) const;
    [[nodiscard]] virtual std::wstring ffield_path_name(const unreal::FField* field) const;
    virtual void ftext_as_culture_invariant(unreal::FText* text, std::wstring_view str) const;
    virtual void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr,
                                       const unreal::UObject* obj) const;
    virtual void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr,
                                       const unreal::UObject* obj) const;
    [[nodiscard]] virtual const unreal::offsets::OffsetList& get_offsets(void) const;
};

#pragma endregion

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_ABSTRACT_HOOK_H */

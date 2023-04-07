#ifndef UNREALSDK_GAME_BL2_BL2_H
#define UNREALSDK_GAME_BL2_BL2_H

#include "unrealsdk/pch.h"

#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/game/selector.h"

#if defined(UE3) && defined(ARCH_X86)

using namespace unrealsdk::unreal;

namespace unrealsdk::game {

class BL2Hook : public AbstractHook {
   protected:
    /**
     * @brief Hex edits out the `obj dump` array limit message.
     */
    virtual void hexedit_array_limit_message(void) const;

    /**
     * @brief Finds GNames, and sets up such that `gnames` may be called.
     */
    virtual void find_gnames(void) const;

    /**
     * @brief Finds `FName::Init`, and sets up such that `fname_init` may be called.
     */
    void find_fname_init(void);

    // Deliberately storing in a void pointer member, because the type changes in bl2/tps
    void* fname_init_ptr;

    /**
     * @brief Hooks the antidebug functions and disables them.
     */
    static void hook_antidebug(void);

    /**
     * @brief Hex edits out the protection on the set command.
     */
    static void hexedit_set_command(void);

    /**
     * @brief Hex edits out the `obj dump` array limit.
     */
    static void hexedit_array_limit(void);

    /**
     * @brief Hooks `UObject::ProcessEvent` and points it at the hook manager.
     */
    static void hook_process_event(void);

    /**
     * @brief Hooks `UObject::CallFunction` and points it at the hook manager.
     */
    static void hook_call_function(void);

    /**
     * @brief Finds GObjects, and populates the wrapper member.
     */
    static void find_gobjects(void);

    /**
     * @brief Finds `FFrame::Step`, and sets up such that `fframe_step` may be called.
     */
    static void find_fframe_step(void);

    /**
     * @brief Finds `GMalloc`, and sets up such that `malloc`, `realloc`, and `free` may be called.
     */
    static void find_gmalloc(void);

    /**
     * @brief Finds `StaticConstructObject`, and sets up such that `construct_object` may be called.
     */
    static void find_construct_object(void);

    /**
     * @brief Creates a console and sets the bind (if required), and hooks logging onto it.
     */
    static void inject_console(void);

   public:
    void hook(void) override;

    [[nodiscard]] const GObjects& gobjects(void) const override;
    [[nodiscard]] const GNames& gnames(void) const override;
    void fname_init(FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(FFrame* frame, UObject* obj, void* param) const override;
    [[nodiscard]] void* u_malloc(size_t len) const override;
    [[nodiscard]] void* u_realloc(void* original, size_t len) const override;
    void u_free(void* data) const override;
    void process_event(UObject* object, UFunction* func, void* params) const override;
    [[nodiscard]] UObject* construct_object(UClass* cls,
                                            UObject* outer,
                                            const FName& name,
                                            decltype(UObject::ObjectFlags) flags,
                                            UObject* template_obj) const override;
    void uconsole_output_text(const std::wstring& str) const override;
    [[nodiscard]] std::wstring uobject_path_name(const UObject* obj) const override;
    [[nodiscard]] UObject* find_object(UClass* cls, const std::wstring& name) const override;
};

template <>
struct GameTraits<BL2Hook> {
    static constexpr auto NAME = "Borderlands 2";

    static bool matches_executable(const std::string& executable) {
        return executable == "Borderlands2.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_BL2_BL2_H */

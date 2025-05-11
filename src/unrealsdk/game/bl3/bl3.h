#ifndef UNREALSDK_GAME_BL3_BL3_H
#define UNREALSDK_GAME_BL3_BL3_H

#include "unrealsdk/pch.h"

#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/game/selector.h"

#if defined(UE4) && defined(ARCH_X64) && !defined(UNREALSDK_IMPORTING)

namespace unrealsdk::game {

class BL3Hook : public AbstractHook {
   protected:
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
     * @brief Finds GNames, and sets up such that `gnames` may be called.
     */
    static void find_gnames(void);

    /**
     * @brief Finds `FName::Init`, and sets up such that `fname_init` may be called.
     */
    static void find_fname_init(void);

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
     * @brief Finds `UObject::GetPathName`, and sets up such that `uobject_path_name` may be called.
     */
    static void find_get_path_name(void);

    /**
     * @brief Finds `StaticFindObject`, and sets up such that `find_object` may be called.
     */
    static void find_static_find_object(void);

    /**
     * @brief Finds `FText::AsCultureInvariant`, and sets up such that `ftext_as_culture_invariant`
     *        may be called.
     */
    static void find_ftext_as_culture_invariant(void);

    /**
     * @brief Finds `LoadPackage`, and sets up such that `load_package` may be called.
     */
    static void find_load_package(void);

    /**
     * @brief Finds the required pointers such that `fsoftobjectptr_assign` and
     *        `flazyobjectptr_assign` may be called.
     */
    static void find_persistent_obj_ptrs(void);

    /**
     * @brief Creates a console and sets the bind (if required), and hooks logging onto it.
     */
    static void inject_console(void);

   public:
    void hook(void) override;
    void post_init(void) override;

    [[nodiscard]] bool is_console_ready(void) const override;

    [[nodiscard]] const unreal::GObjects& gobjects(void) const override;
    [[nodiscard]] const unreal::GNames& gnames(void) const override;
    [[nodiscard]] void* u_malloc(size_t len) const override;
    [[nodiscard]] void* u_realloc(void* original, size_t len) const override;
    void u_free(void* data) const override;
    [[nodiscard]] unreal::UObject* construct_object(unreal::UClass* cls,
                                                    unreal::UObject* outer,
                                                    const unreal::FName& name,
                                                    uint64_t flags,
                                                    unreal::UObject* template_obj) const override;
    [[nodiscard]] unreal::UObject* find_object(unreal::UClass* cls,
                                               const std::wstring& name) const override;
    [[nodiscard]] unreal::UObject* load_package(const std::wstring& name,
                                                uint32_t flags) const override;

    void fname_init(unreal::FName* name, const wchar_t* str, int32_t number) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
    void process_event(unreal::UObject* object,
                       unreal::UFunction* func,
                       void* params) const override;
    void uconsole_output_text(const std::wstring& str) const override;
    [[nodiscard]] std::wstring uobject_path_name(const unreal::UObject* obj) const override;
    void ftext_as_culture_invariant(unreal::FText* text,
                                    unreal::TemporaryFString&& str) const override;
    void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr,
                               const unreal::UObject* obj) const override;
    void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr,
                               const unreal::UObject* obj) const override;
    [[nodiscard]] const unreal::offsets::OffsetList& get_offsets(void) const override;
};

template <>
struct GameTraits<BL3Hook> {
    static constexpr auto NAME = "Borderlands 3";

    static bool matches_executable(std::string_view executable) {
        return executable == "Borderlands3.exe" || executable == "Wonderlands.exe";
    }
};

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_BL3_BL3_H */

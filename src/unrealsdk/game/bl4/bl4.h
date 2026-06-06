#ifndef UNREALSDK_GAME_BL4_BL4_H
#define UNREALSDK_GAME_BL4_BL4_H

#include "unrealsdk/pch.h"
#include "unrealsdk/exports.h"
#include "unrealsdk/utils.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_OAK2

#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/game/selector.h"
#include "unrealsdk/multi_sigscan.h"

namespace unrealsdk::game {

#ifndef UNREALSDK_IMPORTING
class BL4Hook : public AbstractHook {
   protected:
    static void hook_antidebug(void);
    static void hook_call_function(void);
    static void hook_process_event_and_wait_for_unpack(void);

    static void find_fname_funcs(void);
    static void find_gobjects(void);
    static void find_gmalloc(void);
    static void find_get_path_name(void);

    static void find_construct_object(void);
    static void find_static_find_object(void);
    static void find_load_package(void);
    static void find_fframe_step(void);
    static void find_ftext_as_culture_invariant(void);

    static void inject_console(void);

   public:
    void hook(void) override;
    void post_init(void) override;

    [[nodiscard]] bool is_console_ready(void) const override;

    [[nodiscard]] const unreal::GObjects& gobjects(void) const override;
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
    [[nodiscard]] std::variant<const std::string_view, const std::wstring_view> fname_get_str(
        const unreal::FName& name) const override;
    void fframe_step(unreal::FFrame* frame, unreal::UObject* obj, void* param) const override;
    void process_event(unreal::UObject* object,
                       unreal::UFunction* func,
                       void* params) const override;
    void uconsole_output_text(const std::wstring& str) const override;
    [[nodiscard]] std::wstring uobject_path_name(const unreal::UObject* obj) const override;
    [[nodiscard]] std::wstring ffield_path_name(const unreal::FField* field) const override;

    void ftext_as_culture_invariant(unreal::FText* text, std::wstring_view str) const override;
    /*
    void fsoftobjectptr_assign(unreal::FSoftObjectPtr* ptr,
                               const unreal::UObject* obj) const override;
    void flazyobjectptr_assign(unreal::FLazyObjectPtr* ptr,
                               const unreal::UObject* obj) const override;
    */
    [[nodiscard]] const unreal::offsets::OffsetList& get_offsets(void) const override;
};

template <>
struct GameTraits<BL4Hook> {
    static constexpr auto NAME = "BL4";

    static bool matches_executable(std::string_view executable) {
        return executable == "Borderlands4.exe";
    }
};
#endif

namespace bl4 {

extern constinit memory::MultiPattern gnatives_multi;
extern constinit memory::MultiPattern ftexthistory_vftable_multi;
extern constinit memory::MultiPattern fnamepool_multi;
extern constinit memory::MultiPattern fname_find_or_store_multi;
extern constinit memory::MultiPattern gobjects_multi;
extern constinit memory::MultiPattern call_function_multi;
extern constinit memory::MultiPattern process_event_multi;
extern constinit memory::MultiPattern gmalloc_multi;
extern constinit memory::MultiPattern get_obj_path_name_multi;
extern constinit memory::MultiPattern get_field_path_name_multi;
extern constinit memory::MultiPattern construct_obj_multi;
extern constinit memory::MultiPattern find_obj_multi;
extern constinit memory::MultiPattern load_package_multi;

namespace {

constexpr auto DEFAULT_POLL_INTERVAL = std::chrono::milliseconds{100};
constexpr auto DEFAULT_TIMEOUT = std::chrono::seconds{60};

}  // namespace

/**
 * @brief Wait for an address to become executable, and only then detour the given function.
 * @note This waits for the unpacking done by this game's antidebug, avoiding errors that can occur
 *       when detouring too early.
 *
 * @tparam T The signature of the detour'd function (should be picked up automatically).
 * @param addr The address of the function.
 * @param detour_func The detour function.
 * @param original_func Pointer to store the original function.
 * @param name Name of the detour, to be used in log messages on error.
 * @param poll_interval How long to wait between polls.
 * @param timeout Duration to timeout after.
 */
void detour_once_executable(uintptr_t addr,
                            void* detour_func,
                            void** original_func,
                            std::string&& name,
                            std::chrono::milliseconds poll_interval = DEFAULT_POLL_INTERVAL,
                            std::chrono::milliseconds timeout = DEFAULT_TIMEOUT);
template <typename T>
void detour_once_executable(uintptr_t addr,
                            T* detour_func,
                            T** original_func,
                            std::string&& name,
                            std::chrono::milliseconds poll_interval = DEFAULT_POLL_INTERVAL,
                            std::chrono::milliseconds timeout = DEFAULT_TIMEOUT) {
    detour_once_executable(addr, reinterpret_cast<void*>(detour_func),
                           reinterpret_cast<void**>(original_func), std::move(name), poll_interval,
                           timeout);
}

}  // namespace bl4

}  // namespace unrealsdk::game

#endif

#endif /* UNREALSDK_GAME_BL4_BL4_H */

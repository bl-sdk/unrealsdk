#include "unrealsdk/pch.h"
#include "unrealsdk/game/abstract_hook.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/structs/fframe.h"
#include "unrealsdk/unreal/structs/ftext.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"
#include "unrealsdk/version_error.h"

#ifndef UNREALSDK_IMPORTING

namespace unrealsdk::game {

bool AbstractHook::is_console_ready(void) const {
    throw_version_error("is_console_ready not implemented");
    unreachable();
}
const unreal::GObjects& AbstractHook::gobjects(void) const {
    throw_version_error("gobjects not implemented");
    unreachable();
}
void* AbstractHook::u_malloc(size_t /*len*/) const {
    throw_version_error("u_malloc not implemented");
    unreachable();
}
void* AbstractHook::u_realloc(void* /*original*/, size_t /*len*/) const {
    throw_version_error("u_realloc not implemented");
    unreachable();
}
void AbstractHook::u_free(void* /*data*/) const {
    throw_version_error("u_free not implemented");
    unreachable();
}
unreal::UObject* AbstractHook::construct_object(unreal::UClass* /*cls*/,
                                                unreal::UObject* /*outer*/,
                                                const unreal::FName& /*name*/,
                                                uint64_t /*flags*/,
                                                unreal::UObject* /*template_obj*/) const {
    throw_version_error("construct_object not implemented");
    unreachable();
}
unreal::UObject* AbstractHook::find_object(unreal::UClass* /*cls*/,
                                           const std::wstring& /*name*/) const {
    throw_version_error("find_object not implemented");
    unreachable();
}
unreal::UObject* AbstractHook::load_package(const std::wstring& /*name*/,
                                            uint32_t /*flags*/) const {
    throw_version_error("load_package not implemented");
    unreachable();
}
void AbstractHook::fname_init(unreal::FName* /*name*/,
                              const wchar_t* /*str*/,
                              uint32_t /*number*/) const {
    throw_version_error("fname_init not implemented");
    unreachable();
}
std::variant<const std::string_view, const std::wstring_view> AbstractHook::fname_get_str(
    const unreal::FName& /*name*/) const {
    throw_version_error("fname_get_str not implemented");
    unreachable();
};
void AbstractHook::fframe_step(unreal::FFrame* /*frame*/,
                               unreal::UObject* /*obj*/,
                               void* /*param*/) const {
    throw_version_error("fframe_step not implemented");
    unreachable();
}
void AbstractHook::process_event(unreal::UObject* /*object*/,
                                 unreal::UFunction* /*func*/,
                                 void* /*params*/) const {
    throw_version_error("process_event not implemented");
    unreachable();
}
void AbstractHook::uconsole_output_text(const std::wstring& /*str*/) const {
    // Ok breaking the contract a little here: this one just swallows calls without throwing
    // Otherwise every single log message throws
}
std::wstring AbstractHook::uobject_path_name(const unreal::UObject* /*obj*/) const {
    throw_version_error("uobject_path_name not implemented");
    unreachable();
}
std::wstring AbstractHook::ffield_path_name(const unreal::FField* /*field*/) const {
    throw_version_error("ffield_path_name not implemented");
    unreachable();
}
void AbstractHook::ftext_as_culture_invariant(unreal::FText* /*text*/,
                                              std::wstring_view /*str*/) const {
    throw_version_error("ftext_as_culture_invariant not implemented");
    unreachable();
}
void AbstractHook::fsoftobjectptr_assign(unreal::FSoftObjectPtr* /*ptr*/,
                                         const unreal::UObject* /*obj*/) const {
    throw_version_error("fsoftobjectptr_assign not implemented");
    unreachable();
}
void AbstractHook::flazyobjectptr_assign(unreal::FLazyObjectPtr* /*ptr*/,
                                         const unreal::UObject* /*obj*/) const {
    throw_version_error("flazyobjectptr_assign not implemented");
    unreachable();
}
const unreal::offsets::OffsetList& AbstractHook::get_offsets(void) const {
    throw_version_error("get_offsets not implemented");
    unreachable();
}

}  // namespace unrealsdk::game

#endif

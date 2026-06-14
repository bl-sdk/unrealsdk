#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fgbxinlinestruct.h"
#include "unrealsdk/config.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

UScriptStruct* FGbxInlineStruct::get_type(void) const {
    auto value = this->ptr.obj;
    if (value == nullptr) {
        return nullptr;
    }

    static auto idx = config::get_int("unrealsdk.fgbxinlinestruct_gettype_vf_index").value_or(1);
    auto get_type = (*reinterpret_cast<uintptr_t**>(value))[idx];
    auto type = reinterpret_cast<UObject* (*)(const void*)>(get_type)(value);
    if (type == nullptr) {
        throw std::runtime_error(
            std::format("Failed to get type of existing FGbxInlineStruct at {:p}",
                        reinterpret_cast<const void*>(this)));
    }
    return validate_type<UScriptStruct>(type);
}

}  // namespace unrealsdk::unreal

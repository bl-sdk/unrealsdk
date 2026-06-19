#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fgbxdef.h"
#include "unrealsdk/config.h"
#include "unrealsdk/unreal/cast.h"
#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

namespace unrealsdk::unreal {

UScriptStruct* FGbxDef::get_type(void) const {
    static auto idx = config::get_int("unrealsdk.fgbxdef_gettype_vf_index").value_or(1);

    auto type = reinterpret_cast<UObject* (*)(const void*)>(this->vftable[idx])(this);
    if (type == nullptr) {
        throw std::runtime_error(std::format("Failed to get type of FGbxDef at {:p}",
                                             reinterpret_cast<const void*>(this)));
    }

    return validate_type<UScriptStruct>(type);
}

WrappedStruct FGbxDef::to_struct(const UnrealPointer<void>& parent) {
    return {this->get_type(), this, parent};
}

}  // namespace unrealsdk::unreal

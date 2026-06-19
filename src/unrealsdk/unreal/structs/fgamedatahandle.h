#ifndef UNREALSDK_UNREAL_STRUCTS_FGAMEDATAHANDLE_H
#define UNREALSDK_UNREAL_STRUCTS_FGAMEDATAHANDLE_H

#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/structs/fname.h"

namespace unrealsdk::unreal {

struct FGbxDef;
class UScriptStruct;

UNREALSDK_UNREAL_STRUCT_PADDING_PUSH()

struct FGameDataHandle {
    FName name;
    uint32_t type_handle{};
    uint32_t unknown = 0;
    FGbxDef* instance = nullptr;
};

UNREALSDK_UNREAL_STRUCT_PADDING_POP()

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FGAMEDATAHANDLE_H */

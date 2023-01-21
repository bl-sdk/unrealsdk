#ifndef UNREAL_WRAPPERS_WRAPPED_STRUCT_H
#define UNREAL_WRAPPERS_WRAPPED_STRUCT_H

#include "unreal/structs/fname.h"

namespace unrealsdk::unreal {

class UStruct;
class UProperty;

class WrappedStruct {
   public:
    UStruct* type;
    void* base;
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPED_STRUCT_H */

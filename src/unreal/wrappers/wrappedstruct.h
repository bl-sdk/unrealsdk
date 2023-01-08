#ifndef UNREAL_WRAPPERS_WRAPPEDSTRUCT_H
#define UNREAL_WRAPPERS_WRAPPEDSTRUCT_H

namespace unrealsdk::unreal {

class UStruct;

class WrappedStruct {
   public:
    UStruct* type;
    void* base;
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_WRAPPERS_WRAPPEDSTRUCT_H */

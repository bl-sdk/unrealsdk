#ifndef UNREAL_CLASSES_UFIELD_H
#define UNREAL_CLASSES_UFIELD_H

#include "unreal/classes/uobject.h"

namespace unrealsdk::unreal {

class UField : public UObject {
    // NOLINTNEXTLINE(readability-identifier-naming)
    UField* Next;
};
}  // namespace unrealsdk::unreal

#endif /* UNREAL_CLASSES_UFIELD_H */

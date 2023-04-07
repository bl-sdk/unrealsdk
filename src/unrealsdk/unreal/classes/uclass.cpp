#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"

namespace unrealsdk::unreal {

bool UClass::inherits(const UClass* cls) const {
    // For each class in the inheritance chain
    for (const UStruct* our_cls = this; our_cls != nullptr; our_cls = our_cls->SuperField) {
        // If it matches
        if (our_cls == cls) {
            return true;
        }
    }

    return false;
}

bool UClass::implements(const UClass* iface, FImplementedInterface** impl_out) const {
    // For each class in the inheritance chain
    for (auto cls = this->Class; cls != nullptr; cls = validate_type<UClass>(cls->SuperField)) {
        // For each interface on that class
        for (auto our_iface : cls->Interfaces) {
            // If the interface matches
            if (our_iface.Class == iface) {
                // Output the implementation, if necessary
                if (impl_out != nullptr) {
                    *impl_out = &our_iface;
                }
                return true;
            }
        }
    }

    return false;
}

}  // namespace unrealsdk::unreal

#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"

namespace unrealsdk::unreal {

bool UClass::implements(const UClass* iface, FImplementedInterface** impl_out) const {
    // For each class in the inheritance chain
    for (const UStruct* str = this; str != nullptr; str = str->SuperField) {
        // Make sure it's a class
        if (!str->inherits(find_class<UClass>())) {
            continue;
        }
        auto cls = reinterpret_cast<const UClass*>(str);

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

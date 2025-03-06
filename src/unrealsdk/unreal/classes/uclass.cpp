#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/offset_list.h"
#include "unrealsdk/unreal/offsets.h"
#include "unrealsdk/unrealsdk.h"

namespace unrealsdk::unreal {

UNREALSDK_DEFINE_FIELDS_SOURCE_FILE(UClass, UNREALSDK_UCLASS_FIELDS);

bool UClass::implements(const UClass* iface, FImplementedInterface* impl_out) const {
    // For each class in the inheritance chain
    for (const UObject* superfield : this->superfields()) {
        // Make sure it's a class
        if (!superfield->is_instance(find_class<UClass>())) {
            continue;
        }
        auto cls = reinterpret_cast<const UClass*>(superfield);

        // For each interface on that class
        for (auto our_iface : cls->Interfaces()) {
            // If the interface matches
            if (our_iface.Class == iface) {
                // Output the implementation, if necessary
                if (impl_out != nullptr) {
                    *impl_out = our_iface;
                }
                return true;
            }
        }
    }

    return false;
}

}  // namespace unrealsdk::unreal

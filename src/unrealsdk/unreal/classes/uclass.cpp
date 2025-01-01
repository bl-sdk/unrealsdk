#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/find_class.h"

namespace unrealsdk::unreal {

decltype(UClass::ClassDefaultObject_internal)& UClass::ClassDefaultObject(void) {
    return this->get_field(&UClass::ClassDefaultObject_internal);
}
[[nodiscard]] const decltype(UClass::ClassDefaultObject_internal)& UClass::ClassDefaultObject(
    void) const {
    return this->get_field(&UClass::ClassDefaultObject_internal);
}
decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) {
    return this->get_field(&UClass::Interfaces_internal);
}
[[nodiscard]] const decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) const {
    return this->get_field(&UClass::Interfaces_internal);
}

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

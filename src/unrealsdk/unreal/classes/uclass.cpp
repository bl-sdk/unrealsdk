#include "unrealsdk/pch.h"

#include "unrealsdk/config.h"
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

// To further complicate things, UClass::Interfaces also shifts between BL2 + TPS
#ifdef UE3

// This is awful hacky code to get a working release out sooner, the whole system needs a rework.
// Check if the size of UClass is that we've observed in TPS, and if so use it's hardcoded offset.
namespace {

const constexpr auto UCLASS_SIZE_TPS = 0x18C;
const constexpr auto UCLASS_INTERFACES_OFFSET_TPS = 0x160;

}  // namespace

decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<decltype(UClass::Interfaces_internal)&>(
        const_cast<const UClass*>(this)->Interfaces());
}
[[nodiscard]] const decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) const {
    static const auto use_tps_offset =
        unrealsdk::config::get_bool("unrealsdk.__force_uclass_interfaces_tps_offset")
            .value_or(this->Class->get_struct_size() == UCLASS_SIZE_TPS);

    if (use_tps_offset) {
        return *reinterpret_cast<decltype(UClass::Interfaces_internal)*>(
            reinterpret_cast<uintptr_t>(this) + UCLASS_INTERFACES_OFFSET_TPS);
    }

    return this->get_field(&UClass::Interfaces_internal);
}
#else

decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) {
    return this->get_field(&UClass::Interfaces_internal);
}
[[nodiscard]] const decltype(UClass::Interfaces_internal)& UClass::Interfaces(void) const {
    return this->get_field(&UClass::Interfaces_internal);
}

#endif

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

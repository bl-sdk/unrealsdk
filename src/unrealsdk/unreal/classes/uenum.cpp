#include "unrealsdk/pch.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/classes/uobject_funcs.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unreal/wrappers/bound_function.h"

namespace unrealsdk::unreal {

#ifdef UE4
std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    std::unordered_map<FName, uint64_t> output;
    for (size_t i = 0; i < this->Names.size(); i++) {
        auto pair = this->Names.at(i);
        output.emplace(pair.key, pair.value);
    }
    return output;
}

#else

std::unordered_map<FName, uint64_t> UEnum::get_names(void) const {
    static const auto name_sentinel = L"None"_fn;
    auto func = this->get<UFunction, BoundFunction>(L"GetEnum"_fn);

    std::unordered_map<FName, uint64_t> output;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto this_obj = const_cast<UObject*>(static_cast<const UObject*>(this));

    for (int32_t i = 0; i < std::numeric_limits<int32_t>::max(); i++) {
        auto name = func.call<UNameProperty, UObjectProperty, UIntProperty>(this_obj, i);
        if (name == name_sentinel) {
            break;
        }
        output.emplace(name, i);
    }
    return output;
}

#endif

}  // namespace unrealsdk::unreal

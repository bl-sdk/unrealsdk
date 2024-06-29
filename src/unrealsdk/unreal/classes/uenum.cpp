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
    std::unordered_map<FName, uint64_t> output;
    for (size_t i = 0; i < this->Names.size(); i++) {
        output.emplace(this->Names.at(i), i);
    }
    return output;
}

#endif

}  // namespace unrealsdk::unreal

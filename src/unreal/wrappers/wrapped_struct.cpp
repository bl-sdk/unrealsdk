#include "pch.h"

#include "unreal/cast_prop.h"
#include "unreal/wrappers/wrapped_struct.h"
#include "unrealsdk.h"

namespace unrealsdk::unreal {

WrappedStruct::WrappedStruct(const UStruct* type)
    : type(type),
      base(std::shared_ptr<void>(unrealsdk::u_malloc(type->get_struct_size()), [type](void* data) {
          for (const auto& prop : type->properties()) {
              cast_prop(prop, [data]<typename T>(const T* prop) {
                  destroy_property<T>(prop, 0, reinterpret_cast<uintptr_t>(data));
              });
          }

          unrealsdk::u_free(data);
      })) {}

WrappedStruct::WrappedStruct(const UStruct* type, void* base, const std::shared_ptr<void>& parent)
    : type(type), base(parent, base) {}

}  // namespace unrealsdk::unreal

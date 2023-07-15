#include "unrealsdk/pch.h"

#include "unrealsdk/unreal/class_name.h"

namespace unrealsdk::unreal {

template <>
[[nodiscard]] FName cls_fname<UObject>(void) {
    static auto name = L"Object"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UField>(void) {
    static auto name = L"Field"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UStruct>(void) {
    static auto name = L"Struct"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UClass>(void) {
    static auto name = L"Class"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UFunction>(void) {
    static auto name = L"Function"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UScriptStruct>(void) {
    static auto name = L"ScriptStruct"_fn;
    return name;
}
template <>
[[nodiscard]] FName cls_fname<UProperty>(void) {
    static auto name = L"Property"_fn;
    return name;
}

}  // namespace unrealsdk::unreal

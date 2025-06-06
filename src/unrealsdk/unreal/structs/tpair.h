#ifndef UNREALSDK_UNREAL_STRUCTS_TPAIR_H
#define UNREALSDK_UNREAL_STRUCTS_TPAIR_H

namespace unrealsdk::unreal {

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(push, 0x4)
#endif

template <typename KeyType, typename ValueType>
struct TPair {
    KeyType key;
    ValueType value;
};

#if defined(_MSC_VER) && UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_TPAIR_H */

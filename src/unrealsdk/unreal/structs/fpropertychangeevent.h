#ifndef UNREALSDK_UNREAL_STRUCTS_FPROPERTYCHANGEEVENT_H
#define UNREALSDK_UNREAL_STRUCTS_FPROPERTYCHANGEEVENT_H

#include "unrealsdk/pch.h"

namespace unrealsdk::unreal {

class UProperty;

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(push, 0x4)
#endif

struct FPropertyChangedEvent {
    static constexpr auto CHANGE_TYPE_UNSPECIFIED = 1;

    /**
     * @brief Creates a new property changed event.
     *
     * @param prop The property which was changed.
     */
    FPropertyChangedEvent(UProperty* prop) : Property(prop), ChangeType(CHANGE_TYPE_UNSPECIFIED) {}

   protected:
    // NOLINTBEGIN(readability-identifier-naming)
    UProperty* Property{};
    UProperty* MemberProperty{};
    uint32_t ChangeType{};
    int32_t ObjectIteratorIndex{};
    // NOLINTEND(readability-identifier-naming)
};

struct FEditPropertyChain {
   public:
    /**
     * @brief Constructs a new edit chain pointing at the give properties.
     *
     * @param chain The chain of properties.
     */
    FEditPropertyChain(const std::vector<UProperty*>& chain);

   protected:
    struct Node {
        UProperty* value = nullptr;
        Node* next = nullptr;
        Node* prev = nullptr;
    };
    Node* head = nullptr;
    Node* tail = nullptr;
    uint32_t size = 0;

    // I think there's some real fields here, but a zero-init seems to work well enough
    // Add a bunch of padding before we add our own fields
    // NOLINTNEXTLINE(readability-magic-numbers)
    uint8_t dummy[64] = {0};

    // Store all the nodes as part of the same object, so we don't need to deal with raw pointers
    std::vector<Node> nodes;
};

struct FPropertyChangedChainEvent : public FPropertyChangedEvent {
    // NOLINTNEXTLINE(readability-identifier-naming)
    FEditPropertyChain* PropertyChain{};
};

#if defined(_MSC_VER) && defined(ARCH_X86)
#pragma pack(pop)
#endif

}  // namespace unrealsdk::unreal

#endif /* UNREALSDK_UNREAL_STRUCTS_FPROPERTYCHANGEEVENT_H */

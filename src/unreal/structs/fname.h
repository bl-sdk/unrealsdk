#ifndef UNREAL_STRUCTS_FNAME_H
#define UNREAL_STRUCTS_FNAME_H

namespace unrealsdk::unreal {

struct FName {
   public:
    int index{};
    int number{};

    bool operator==(const FName& other) const;
    bool operator!=(const FName& other) const;

    // TODO: string conversions
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_FNAME_H */

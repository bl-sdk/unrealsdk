#ifndef UNREAL_STRUCTS_FNAME_H
#define UNREAL_STRUCTS_FNAME_H

namespace unrealsdk::unreal {

struct FName {
   private:
    int32_t index;
    int32_t number;

   public:
    /**
     * @brief Construct a new FName object.
     *
     * @param index The name index to use.
     * @param name A string to lookup/create the name index of.
     * @param number The name number to use.
     */
    FName(int32_t index, int32_t number);
    // TODO: FName(const std::string& name, int32_t number = 0);
    FName(const std::wstring& name, int32_t number = 0);

    bool operator==(const FName& other) const;
    bool operator!=(const FName& other) const;

    // TODO: string conversions
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_FNAME_H */

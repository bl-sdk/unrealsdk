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
     * @note Automatically converts utf8 to utf16 if needed.
     *
     * @param index The name index to use.
     * @param name A string to lookup/create the name index of.
     * @param number The name number to use.
     */
    FName(int32_t index, int32_t number);
    FName(const std::string& name, int32_t number = 0);
    FName(const std::wstring& name, int32_t number = 0);

    bool operator==(const FName& other) const;
    bool operator!=(const FName& other) const;

    /**
     * @brief Convert the FName to a string.
     * @note Automatically converts utf8 to utf16 (or vice versa), as needed.
     *
     * @return The converted string.
     */
    operator std::string() const;
    operator std::wstring() const;
};

}  // namespace unrealsdk::unreal

#endif /* UNREAL_STRUCTS_FNAME_H */

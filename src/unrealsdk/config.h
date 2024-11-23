#ifndef UNREALSDK_CONFIG_H
#define UNREALSDK_CONFIG_H

#include "unrealsdk/pch.h"

namespace unrealsdk::config {

struct Config {
    // NOLINTBEGIN(readability-magic-numbers)

    bool external_console = false;
    std::string_view log_file{"unrealsdk.log"};
    std::string_view console_log_level{"INFO"};
    std::string_view exe_override;
    size_t uproperty_size = 0;
    size_t alloc_alignment = 0;
    std::string_view console_key{"Tilde"};
    ptrdiff_t uconsole_console_command_vf_index = -1;
    ptrdiff_t uconsole_output_text_vf_index = -1;
    ptrdiff_t treference_controller_destroy_obj_vf_index = -1;
    ptrdiff_t treference_controller_destructor_vf_index = -1;
    ptrdiff_t ftext_get_display_string_vf_index = -1;
    bool locking_process_event = false;
    std::string_view log_all_calls_file{"unrealsdk.calls.tsv"};

    // NOLINTEND(readability-magic-numbers)
};

#ifndef UNREALSDK_IMPORTING
/**
 * @brief Loads the config file.
 */
void load(void);

#endif

/**
 * @brief Gets a reference to the loaded config.
 *
 * @return A reference to the config.
 */
const Config& get(void);

/**
 * @brief Gets additional values from the merged config file, suitable for extending it.
 * @note Returned reference types (i.e. string view) are owned by a static table in `unrealsdk.dll`.
 *
 * @param path The path to the value to get - e.g. "unrealsdk.outerfield[1].innerfield"
 * @return The returned value, or std::nullopt if not set or the incorrect type.
 */
std::optional<bool> get_bool(std::string_view path);
std::optional<int64_t> get_int(std::string_view path);
std::optional<std::string_view> get_str(std::string_view path);

}  // namespace unrealsdk::config

#endif /* UNREALSDK_CONFIG_H */

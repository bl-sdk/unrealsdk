#include "unrealsdk/pch.h"
#include "unrealsdk/config.h"
#include "unrealsdk/utils.h"

// Choosing to include tomlplusplus here, rather than in the PCH, for compile times.
// We can't try split the declarations/implementation, since the PCH will cache the header with just
// the declarations, so we won't be able to re-include the implementation, meaning we must use it as
// a header only library.
// Including in the PCH would require anything linking against this as a shared library to compile
// it, when it's entirely internal to this file, and isn't exposed anywhere.

#ifndef UNREALSDK_IMPORTING
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TOML_ENABLE_FORMATTERS 0
#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>
// NOLINTEND(cppcoreguidelines-macro-usage)
#endif

namespace unrealsdk::config {

#pragma region Config file loading
#ifndef UNREALSDK_IMPORTING

namespace {

const constexpr auto CONFIG_FILE_ENV_VAR = "UNREALSDK_CONFIG_FILE";
const constexpr auto DEFAULT_CONFIG_FILE_NAME = "unrealsdk.toml";

toml::table merged_raw_config{};
Config config{};

}  // namespace

namespace {

/**
 * @brief Get the path of the base config file.
 *
 * @return The config file path.
 */
std::filesystem::path get_config_file_path(void) {
    std::string filename = DEFAULT_CONFIG_FILE_NAME;

    auto num_chars = GetEnvironmentVariableA(CONFIG_FILE_ENV_VAR, nullptr, 0);
    if (num_chars != 0) {
        // The size returned when the buffer is too small includes the null terminator
        std::string env_var_name(num_chars, '\0');
        // The size returned when the buffer is big enough does *not* include the null terminaator
        if (GetEnvironmentVariableA(CONFIG_FILE_ENV_VAR, env_var_name.data(), num_chars)
            != (num_chars - 1)) {
            // We reserved one extra for the null, get rid of it
            env_var_name.pop_back();

            filename = std::move(env_var_name);
        }
    }

    return utils::get_this_dll().parent_path() / filename;
}

/**
 * @brief Recursively merges two toml tables.
 *
 * @param base The base table. Modified in place.
 * @param overrides The overrides tables.
 */
void recursive_merge_table(toml::table& base, const toml::table& overrides) {
    overrides.for_each([&base](auto& key, auto& value) {
        if constexpr (toml::is_table<decltype(value)>) {
            if (base.contains(key)) {
                auto base_table = base[key].as_table();
                if (base_table != nullptr) {
                    recursive_merge_table(*base_table, value);
                    return;
                }
            }
        }
        base.insert_or_assign(key, value);
    });
}

/**
 * @brief Loads and merges the config files into `merged_raw_config`.
 */
void load_raw_config(void) {
    auto base_config_path = get_config_file_path();

    bool loaded_base_config = false;
    if (std::filesystem::exists(base_config_path)) {
        auto base_config = toml::parse_file(base_config_path.string());
        if (base_config.succeeded()) {
            merged_raw_config = std::move(base_config.table());
            loaded_base_config = true;
        } else {
            LOG(ERROR, "Failed to load {}", base_config_path.string());

            std::stringstream stream;
            stream << base_config.error();
            std::string line;
            while (std::getline(stream, line)) {
                LOG(ERROR, "{}", line);
            }

            // Continue anyway - maybe we can load the user config
        }
    }

    auto user_config_path =
        base_config_path.replace_extension(".user" + base_config_path.extension().string());
    if (!std::filesystem::exists(user_config_path)) {
        return;
    }

    auto user_config = toml::parse_file(user_config_path.string());
    if (user_config.failed()) {
        LOG(ERROR, "Failed to load {}", user_config_path.string());

        std::stringstream stream;
        stream << user_config.error();
        std::string line;
        while (std::getline(stream, line)) {
            LOG(ERROR, "{}", line);
        }

        // Either both failed, in which case we can just keep the default constructed table, or the
        // base config succeeded, but this one failed, in which case there's nothing to merge
        return;
    }

    if (!loaded_base_config) {
        // If only the user config succeeded, there's nothing to merge, just load it directly
        merged_raw_config = std::move(user_config.table());
        return;
    }

    // We managed to parse both config files, merge them
    recursive_merge_table(merged_raw_config, user_config.table());
}

}  // namespace

/**
 * @brief Helper macro to do a basic load into the config struct.
 *
 * @param name The name of the field to load.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASIC_LOAD_CONFIG(name)                                     \
    do {                                                            \
        auto val = unrealsdk[#name].value<decltype(config.name)>(); \
        if (val.has_value()) {                                      \
            config.name = *val;                                     \
        }                                                           \
    } while (0)

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void load(void) {
    load_raw_config();

    auto unrealsdk = merged_raw_config["unrealsdk"];

    BASIC_LOAD_CONFIG(external_console);
    BASIC_LOAD_CONFIG(log_file);
    BASIC_LOAD_CONFIG(console_log_level);
    BASIC_LOAD_CONFIG(exe_override);
    BASIC_LOAD_CONFIG(uproperty_size);
    BASIC_LOAD_CONFIG(alloc_alignment);
    BASIC_LOAD_CONFIG(console_key);
    BASIC_LOAD_CONFIG(uconsole_console_command_vf_index);
    BASIC_LOAD_CONFIG(uconsole_output_text_vf_index);
    BASIC_LOAD_CONFIG(treference_controller_destroy_obj_vf_index);
    BASIC_LOAD_CONFIG(treference_controller_destructor_vf_index);
    BASIC_LOAD_CONFIG(ftext_get_display_string_vf_index);
    BASIC_LOAD_CONFIG(locking_process_event);
    BASIC_LOAD_CONFIG(log_all_calls_file);
}

#undef BASIC_LOAD_CONFIG

#endif
#pragma endregion

// =================================================================================================

#pragma region C API Wrappers

UNREALSDK_CAPI([[nodiscard]] const Config*, config_get) {
    return &config;
}

UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_bool,
               const char* path,
               size_t path_size,
               bool* value) {
    auto config_value = merged_raw_config.at_path({path, path_size}).value<bool>();
    if (!config_value.has_value()) {
        return false;
    }
    *value = *config_value;
    return true;
}

UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_int,
               const char* path,
               size_t path_size,
               int64_t* value) {
    auto config_value = merged_raw_config.at_path({path, path_size}).value<int64_t>();
    if (!config_value.has_value()) {
        return false;
    }
    *value = *config_value;
    return true;
}

UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_str,
               const char* path,
               size_t path_size,
               const char** value,
               size_t* value_size) {
    auto config_value = merged_raw_config.at_path({path, path_size}).value<std::string_view>();
    if (!config_value.has_value()) {
        return false;
    }
    *value = config_value->data();
    *value_size = config_value->size();
    return true;
}

const Config& get(void) {
    return *UNREALSDK_MANGLE(config_get)();
}

std::optional<bool> get_bool(std::string_view path) {
    bool value{};
    if (UNREALSDK_MANGLE(config_get_bool)(path.data(), path.size(), &value)) {
        return {value};
    }
    return std::nullopt;
}
std::optional<int64_t> get_int(std::string_view path) {
    int64_t value{};
    if (UNREALSDK_MANGLE(config_get_int)(path.data(), path.size(), &value)) {
        return {value};
    }
    return std::nullopt;
}
std::optional<std::string_view> get_str(std::string_view path) {
    const char* value_ptr{};
    size_t value_size{};
    if (UNREALSDK_MANGLE(config_get_str)(path.data(), path.size(), &value_ptr, &value_size)) {
        return {{value_ptr, value_size}};
    }
    return std::nullopt;
}

#pragma endregion

}  // namespace unrealsdk::config

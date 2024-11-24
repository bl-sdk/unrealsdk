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

namespace {

const constexpr auto CONFIG_FILE_ENV_VAR = "UNREALSDK_CONFIG_FILE";
const constexpr auto DEFAULT_CONFIG_FILE_NAME = "unrealsdk.toml";

#ifndef UNREALSDK_IMPORTING
toml::table merged_raw_config{};
#endif

}  // namespace

std::filesystem::path get_base_config_file_path(void) {
    static std::optional<std::filesystem::path> path = std::nullopt;
    if (path.has_value()) {
        return *path;
    }

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

    path = utils::get_this_dll().parent_path() / filename;
    return *path;
}

std::filesystem::path get_user_config_file_path(void) {
    static std::optional<std::filesystem::path> path = std::nullopt;
    if (path.has_value()) {
        return *path;
    }

    auto base_path = get_base_config_file_path();
    path = base_path.replace_extension(".user" + base_path.extension().string());
    return *path;
}

#ifndef UNREALSDK_IMPORTING
namespace {

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

}  // namespace

void load(void) {
    auto load_config_file = [](const std::filesystem::path&& path) -> std::optional<toml::table> {
        if (!std::filesystem::exists(path)) {
            return std::nullopt;
        }

        auto config = toml::parse_file(path.string());
        if (config.succeeded()) {
            return config.table();
        }

        LOG(ERROR, "Failed to load {}", path.string());

        std::stringstream stream;
        stream << config.error();
        std::string line;
        while (std::getline(stream, line)) {
            LOG(ERROR, "{}", line);
        }

        return std::nullopt;
    };

    auto base_config = load_config_file(get_base_config_file_path());
    auto user_config = load_config_file(get_user_config_file_path());

    if (!base_config.has_value()) {
        if (user_config.has_value()) {
            // Only user config got loaded
            merged_raw_config = std::move(*user_config);
            return;
        }
        // No config files got loaded, use the default constructed empty dict
        return;
    }
    if (!user_config.has_value()) {
        // Only base config got loaded
        merged_raw_config = std::move(*base_config);
        return;
    }

    // Both configs were loaded, we need to merge them
    merged_raw_config = std::move(*base_config);
    recursive_merge_table(merged_raw_config, *user_config);
}

#endif

#pragma endregion

// =================================================================================================

#pragma region C API Wrappers

#ifdef UNREALSDK_EXPORTING

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

#endif  // defined(UNREALSDK_EXPORTING)

#ifdef UNREALSDK_IMPORTING

UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_bool,
               const char* path,
               size_t path_size,
               bool* value);
UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_int,
               const char* path,
               size_t path_size,
               int64_t* value);
UNREALSDK_CAPI([[nodiscard]] bool,
               config_get_str,
               const char* path,
               size_t path_size,
               const char** value,
               size_t* value_size);

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

#else  // defined(UNREALSDK_IMPORTING)

std::optional<bool> get_bool(std::string_view path) {
    return merged_raw_config.at_path(path).value<bool>();
}
std::optional<int64_t> get_int(std::string_view path) {
    return merged_raw_config.at_path(path).value<int64_t>();
}
std::optional<std::string_view> get_str(std::string_view path) {
    return merged_raw_config.at_path(path).value<std::string_view>();
}

#endif  // defined(UNREALSDK_IMPORTING)

#pragma endregion

}  // namespace unrealsdk::config

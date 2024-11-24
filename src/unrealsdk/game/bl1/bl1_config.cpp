//
// Date       : 24/11/2024
// Project    : unrealsdk
// Author     : -Ry
//

#include "unrealsdk/game/bl1/bl1_config.h"
#include "unrealsdk/config.h"

namespace unrealsdk::game::bl1_cfg {

// ############################################################################//
//  | TOML DEF |
// ############################################################################//

namespace {
using TomlKeyType = std::string_view;

// clang-format off

// USDK
constexpr TomlKeyType CONSOLE_KEY            = "unrealsdk.console_key";
constexpr TomlKeyType LOCKING_PROCESS_EVENT  = "unrealsdk.locking_process_event";

// BL1SDK
constexpr TomlKeyType LOG_LOAD_PACKAGE       = "unrealsdk.bl1.log_load_package";
constexpr TomlKeyType LOG_SAVE_PACKAGE       = "unrealsdk.bl1.log_save_package";
constexpr TomlKeyType LOCKING_CONSOLE_WRITE  = "unrealsdk.bl1.locking_console_write";

constexpr TomlKeyType INIT_FUNC_POLL_RATE_MS = "unrealsdk.bl1.init_func_poll_rate_ms";
constexpr int64_t     DEFAULT_POLL_RATE_MS   = 20;

// clang-format on

}  // namespace

// ############################################################################//
//  | FUNCTION IMPLS |
// ############################################################################//

std::string console_key(void) {
    return std::string{config::get_str("unrealsdk.console_key").value_or("Tilde")};
}

bool is_locking_console_write(void) {
    return config::get_bool(LOCKING_CONSOLE_WRITE).value_or(false);
}

bool is_locking_process_event(void) {
    return config::get_bool(LOCKING_PROCESS_EVENT).value_or(false);
}

bool is_log_load_package(void) {
    return config::get_bool(LOG_LOAD_PACKAGE).value_or(false);
}

bool is_log_save_package(void) {
    return config::get_bool(LOG_LOAD_PACKAGE).value_or(false);
}

int64_t init_func_poll_rate_ms(void) {
    return config::get_int(INIT_FUNC_POLL_RATE_MS).value_or(DEFAULT_POLL_RATE_MS);
}

}  // namespace unrealsdk::game::bl1_cfg
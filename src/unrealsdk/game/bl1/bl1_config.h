//
// Date       : 24/11/2024
// Project    : unrealsdk
// Author     : -Ry
//
#ifndef UNREALSDK_BL1_CONFIG_H
#define UNREALSDK_BL1_CONFIG_H

#include "unrealsdk/pch.h"

#if defined(UE3) && defined(ARCH_X86) && !defined(UNREALSDK_IMPORTING) \
    && defined(UNREALSDK_GAME_BL1)

namespace unrealsdk::game::bl1_cfg {

std::string console_key(void);

bool is_log_load_package(void);
bool is_log_save_package(void);
int64_t init_func_poll_rate_ms(void);

}  // namespace unrealsdk::game::bl1_cfg

#endif

#endif

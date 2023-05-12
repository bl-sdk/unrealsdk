set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_C_COMPILER ${MINGW_TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW_TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${MINGW_TOOLCHAIN_PREFIX}-windres)

add_compile_options("$<$<CONFIG:DEBUG>:-ggdb3>")
add_link_options("$<$<CONFIG:DEBUG>:-ggdb3>")

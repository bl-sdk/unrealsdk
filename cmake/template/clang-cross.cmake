set(CMAKE_SYSTEM_NAME Windows)

set(triple x86_64-pc-windows-msvc)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${CLANG_TRIPLE})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TRIPLE})

# Problem: CMake runs toolchain files multiple times, but can't read cache variables on some runs.
# Workaround: On first run (in which cache variables are always accessible), set an intermediary environment variable.
# https://stackoverflow.com/a/29997033
if(MSVC_WINE_ENV_SCRIPT OR XWIN_DIR)
    set(ENV{_MSVC_WINE_ENV_SCRIPT} "${MSVC_WINE_ENV_SCRIPT}")
    set(ENV{_XWIN_DIR} "${XWIN_DIR}")
else()
    set(MSVC_WINE_ENV_SCRIPT "$ENV{_MSVC_WINE_ENV_SCRIPT}")
    set(XWIN_DIR "$ENV{_XWIN_DIR}")
endif()

if(EXISTS ${MSVC_WINE_ENV_SCRIPT})
    # @brief Extract paths from the env script and pass them to another function
    #
    # @param env_var The environment variable to extract
    # @param prefix A prefix to add to the start of each path (e.g. `-I`)
    # @param output_function The function to call with the list of extracted paths
    function(_extract_from_env env_var prefix output_function)
        execute_process(
            COMMAND bash -c ". ${MSVC_WINE_ENV_SCRIPT} && echo \"\$${env_var}\""
            OUTPUT_VARIABLE env_output
            COMMAND_ERROR_IS_FATAL ANY
        )
        string(REPLACE "z:\\" "${prefix}/" env_output "${env_output}")
        string(REPLACE "\\" "/" env_output "${env_output}")
        string(REGEX MATCHALL "[^;\r\n]+" env_output_list "${env_output}")

        cmake_language(CALL ${output_function} ${env_output_list})
    endfunction()

    _extract_from_env("INCLUDE" "-isystem" add_compile_options)
    _extract_from_env("LIB" "-L" add_link_options)
elseif(EXISTS ${XWIN_DIR})
    add_compile_options(
        "-isystem${XWIN_DIR}/sdk/include/um"
        "-isystem${XWIN_DIR}/sdk/include/ucrt"
        "-isystem${XWIN_DIR}/sdk/include/shared"
        "-isystem${XWIN_DIR}/crt/include"
    )
    add_link_options(
        "-L${XWIN_DIR}/sdk/lib/um/x86_64"
        "-L${XWIN_DIR}/crt/lib/x86_64"
        "-L${XWIN_DIR}/sdk/lib/ucrt/x86_64"
    )
else()
    message(FATAL_ERROR "One of 'MSVC_WINE_ENV_SCRIPT' or 'XWIN_DIR' must be defined, could not find windows headers/libs!")
endif()

add_compile_options(-ffreestanding)

add_compile_options("$<$<CONFIG:DEBUG>:-gdwarf>")
add_link_options("$<$<CONFIG:DEBUG>:-gdwarf>" "$<$<CONFIG:DEBUG>:-Wl,/ignore:longsections>")

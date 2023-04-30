cmake_minimum_required(VERSION 3.24)

project(unrealsdk)

set(UNREALSDK_UE_VERSION "UE4" CACHE STRING "The unreal engine version to build the SDK for. One of 'UE3' or 'UE4'.")
set(UNREALSDK_ARCH "x64" CACHE STRING "The architecture to build the sdk for. One of 'x86' or 'x64'.")
set(UNREALSDK_SHARED False CACHE BOOL "If set, compiles as a shared library instead of as an object.")

add_library(_unrealsdk_interface INTERFACE)

target_compile_features(_unrealsdk_interface INTERFACE cxx_std_20)
set_target_properties(_unrealsdk_interface PROPERTIES
    COMPILE_WARNING_AS_ERROR True
    INTERPROCEDURAL_OPTIMIZATION True
)
if(MSVC)
    target_compile_options(_unrealsdk_interface INTERFACE /W4)
else()
    target_compile_options(_unrealsdk_interface INTERFACE -Wall -Wextra -Wpedantic)
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
try_compile(supports_format
    SOURCE_FROM_CONTENT test.cpp "  \
    #include <version>\n            \
    #ifndef __cpp_lib_format\n      \
        #error\n                    \
    #endif                          \
    "
    CXX_STANDARD 20
)

add_subdirectory(libs/minhook)
if(NOT supports_format)
    add_subdirectory(libs/fmt)
endif()

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/unrealsdk/*.cpp" "src/unrealsdk/*.h")
target_sources(_unrealsdk_interface INTERFACE ${sources})

target_include_directories(_unrealsdk_interface INTERFACE "src")
target_link_libraries(_unrealsdk_interface INTERFACE minhook)
if(NOT ${supports_format})
    target_link_libraries(_unrealsdk_interface INTERFACE fmt)
endif()

target_compile_definitions(_unrealsdk_interface INTERFACE
    "${UNREALSDK_UE_VERSION}"
    "ARCH_$<UPPER_CASE:${UNREALSDK_ARCH}>"
    "$<$<BOOL:${UNREALSDK_SHARED}>:UNREALSDK_SHARED>"
)

target_precompile_headers(_unrealsdk_interface INTERFACE "src/unrealsdk/pch.h")

if(MSVC)
    # Enable Edit and Continue
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    target_compile_options(_unrealsdk_interface INTERFACE "$<$<CONFIG:DEBUG>:/ZI>")
    target_link_options(_unrealsdk_interface INTERFACE "/INCREMENTAL")

    # UTF-8 encoded source files
    target_compile_options(_unrealsdk_interface INTERFACE "/utf-8")
endif()

if(NOT UNREALSDK_SHARED)
    add_library(unrealsdk OBJECT)
    target_link_libraries(unrealsdk PUBLIC _unrealsdk_interface)
    set_target_properties(unrealsdk PROPERTIES
        EXPORT_COMPILE_COMMANDS True
    )
else()
    add_library(unrealsdk SHARED)
    target_link_libraries(unrealsdk PUBLIC _unrealsdk_interface)
    set_target_properties(unrealsdk PROPERTIES
        EXPORT_COMPILE_COMMANDS True
        PREFIX ""
    )

    file(GLOB_RECURSE shared_sources CONFIGURE_DEPENDS "src/shared/*.cpp" "src/shared/*.h")
    target_sources(unrealsdk PRIVATE ${shared_sources})

    # Unconditionally add the exporting flag
    # Add it privately, so it doesn't appear in anything linking against this
    target_compile_definitions(unrealsdk PRIVATE "UNREALSDK_EXPORTING")

    if(MINGW)
        # Link `libc++.dll` and `libunwind.dll` statically
        target_link_options(unrealsdk PRIVATE "-static-libstdc++" "-Wl,-Bstatic" "-lunwind")
    endif()

    # Postbuild
    if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        set(POSTBUILD_SCRIPT "postbuild")
        if(CMAKE_HOST_WIN32)
            set(POSTBUILD_SCRIPT "${POSTBUILD_SCRIPT}.bat")
        endif()
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${POSTBUILD_SCRIPT}")
            add_custom_command(
                TARGET unrealsdk
                POST_BUILD
                COMMAND ${POSTBUILD_SCRIPT} "\"$<SHELL_PATH:$<TARGET_FILE:unrealsdk>>\" \"${UNREALSDK_UE_VERSION}\" \"${UNREALSDK_ARCH}\" \"$<IF:$<CONFIG:DEBUG>,DEBUG,RELEASE>\""
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        endif()
    endif()
endif()

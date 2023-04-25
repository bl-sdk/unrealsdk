cmake_minimum_required(VERSION 3.24)

project(unrealsdk)

set(UNREALSDK_UE_VERSION "UE4" CACHE STRING "The unreal engine version to build the SDK for. One of 'UE3' or 'UE4'.")
set(UNREALSDK_ARCH "x64" CACHE STRING "The architecture to build the sdk for. One of 'x86' or 'x64'.")
set(UNREALSDK_SHARED False CACHE BOOL "If set, compiles as a shared library instead of as an object.")
set(UNREALSDK_EXPORTING False CACHE BOOL "If set, compiles with dll exports. Useful to make a custom shared library with different initialization.")

if (${UNREALSDK_SHARED})
    add_library(unrealsdk SHARED)
else()
    add_library(unrealsdk OBJECT)
endif()

target_compile_features(unrealsdk PUBLIC cxx_std_20)
set_target_properties(unrealsdk PROPERTIES
    EXPORT_COMPILE_COMMANDS True
    COMPILE_WARNING_AS_ERROR True
)
if(MSVC)
    target_compile_options(unrealsdk PRIVATE /W4)
else()
    target_compile_options(unrealsdk PRIVATE -Wall -Wextra -Wpedantic)
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
target_sources(unrealsdk PRIVATE ${sources})

if (UNREALSDK_SHARED)
    file(GLOB_RECURSE shared_sources CONFIGURE_DEPENDS "src/shared/*.cpp" "src/shared/*.h")
    target_sources(unrealsdk PRIVATE ${shared_sources})
endif()

target_include_directories(unrealsdk PUBLIC "src")
target_link_libraries(unrealsdk PUBLIC minhook)
if(NOT ${supports_format})
    target_link_libraries(unrealsdk PUBLIC fmt)
endif()

target_compile_definitions(unrealsdk PUBLIC
    "${UNREALSDK_UE_VERSION}"
    "ARCH_$<UPPER_CASE:${UNREALSDK_ARCH}>"
    "$<$<BOOL:${UNREALSDK_SHARED}>:UNREALSDK_SHARED>"
    "$<$<BOOL:${UNREALSDK_EXPORTING}>:UNREALSDK_EXPORTING>"
)

if(UNREALSDK_SHARED AND (NOT UNREALSDK_EXPORTING))
    # Set as private, so anything linking will import instead
    target_compile_definitions(unrealsdk PRIVATE "UNREALSDK_EXPORTING")
endif()

target_precompile_headers(unrealsdk PUBLIC "src/unrealsdk/pch.h")

if(MSVC)
    # Enable Edit and Continue
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    target_compile_options(unrealsdk PUBLIC "$<$<CONFIG:DEBUG>:/ZI>")
    target_link_options(unrealsdk PUBLIC "/INCREMENTAL")

    # UTF-8 encoded source files
    target_compile_options(unrealsdk PUBLIC "/utf-8")
endif()

if (UNREALSDK_SHARED)
    if (MINGW)
        # Want to link statically into a single file
        target_link_options(unrealsdk PRIVATE "-static")
    endif()

    # Postbuild
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

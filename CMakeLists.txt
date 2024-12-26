cmake_minimum_required(VERSION 3.25)

project(unrealsdk VERSION 1.5.0)

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
    # Under MSVC, enable edit and continue in debug - which conflicts with LTO
    set_target_properties(${target_name} PROPERTIES
        MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug>:EditAndContinue>"
        INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
    )
endif()

if(MSVC)
    target_compile_options(_unrealsdk_interface INTERFACE /W4)
else()
    target_compile_options(_unrealsdk_interface INTERFACE -Wall -Wextra -Wpedantic)
endif()
# CMake doesn't understand warnings as errors for MinGW yet
if(MINGW)
    target_compile_options(_unrealsdk_interface INTERFACE -Werror)
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

set(CONFIGURE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/configure")

configure_file(
    "src/unrealsdk/version.inl.in"
    "${CONFIGURE_FILES_DIR}/unrealsdk/version.inl"
)

set(GIT_PRE_CONFIGURE_FILE "src/unrealsdk/git.inl.in")
set(GIT_POST_CONFIGURE_FILE "${CONFIGURE_FILES_DIR}/unrealsdk/git.inl")
include(common_cmake/git_watcher.cmake)

add_subdirectory(libs/minhook)
add_subdirectory(libs/tomlplusplus)
if(NOT supports_format)
    add_subdirectory(libs/fmt)
endif()

# If using the clang windows cross compilation toolchain
if((NOT CMAKE_HOST_WIN32)
    AND (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    # Disable some intrinsics - MSVC implicitly defines intrinsics, but Clang does not, so we get
    # a linking error otherwise
    target_compile_definitions(minhook PRIVATE MINHOOK_DISABLE_INTRINSICS)
    target_compile_definitions(_unrealsdk_interface INTERFACE TOML_ENABLE_SIMD=0)
endif()

target_include_directories(_unrealsdk_interface INTERFACE "src" ${CONFIGURE_FILES_DIR})
target_link_libraries(_unrealsdk_interface INTERFACE minhook tomlplusplus::tomlplusplus)
if(NOT ${supports_format})
    target_link_libraries(_unrealsdk_interface INTERFACE fmt)
endif()

target_compile_definitions(_unrealsdk_interface INTERFACE
    "${UNREALSDK_UE_VERSION}"
    "ARCH_$<UPPER_CASE:${UNREALSDK_ARCH}>"
    "$<$<BOOL:${UNREALSDK_SHARED}>:UNREALSDK_SHARED>"
)

target_precompile_headers(_unrealsdk_interface INTERFACE "src/unrealsdk/pch.h")

# How we include sources depends on what sort of library we're building as
# Including this in the interface target would force them to be public
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/unrealsdk/*.cpp" "src/unrealsdk/*.h")
list(APPEND sources ${GIT_POST_CONFIGURE_FILE})

if(NOT UNREALSDK_SHARED)
    add_library(unrealsdk OBJECT)
    target_link_libraries(unrealsdk PUBLIC _unrealsdk_interface)
    set_target_properties(unrealsdk PROPERTIES
        EXPORT_COMPILE_COMMANDS True
    )

    # If a static library, we can keep sources private
    target_sources(unrealsdk PRIVATE ${sources})
else()
    add_library(unrealsdk SHARED)
    target_link_libraries(unrealsdk PUBLIC _unrealsdk_interface)
    set_target_properties(unrealsdk PROPERTIES
        EXPORT_COMPILE_COMMANDS True
        PREFIX ""
    )

    # If a shared library, make sources public
    # Because we only expose the bare minimum functionality which relies on an internal state,
    # anything linking against us needs to statically compile the rest - making the sources public
    # is an easy way to accomplish that, without even needing another `unrealsdk_shared` target.
    # This does mean they get compiled twice, but we can't easily avoid that, since the two passes
    # use different values for `UNREALSDK_EXPORTING` anyway.
    target_sources(unrealsdk PUBLIC ${sources})

    file(GLOB_RECURSE shared_sources CONFIGURE_DEPENDS "src/shared/*.cpp" "src/shared/*.h")
    target_sources(unrealsdk PRIVATE
        ${shared_sources}
        "src/shared/versioninfo.rc"
    )

    # Unconditionally add the exporting flag
    # Add it privately, so it doesn't appear in anything linking against this
    target_compile_definitions(unrealsdk PRIVATE "UNREALSDK_EXPORTING")

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
                COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/${POSTBUILD_SCRIPT}"
                ARGS
                    "$<SHELL_PATH:$<TARGET_FILE:unrealsdk>>"
                    "${UNREALSDK_UE_VERSION}"
                    "${UNREALSDK_ARCH}"
                    "$<IF:$<CONFIG:DEBUG>,DEBUG,RELEASE>"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        endif()
    endif()
endif()

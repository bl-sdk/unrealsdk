cmake_minimum_required (VERSION 3.24)

project(unrealsdk)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/user-includes.cmake")
   include("${CMAKE_CURRENT_SOURCE_DIR}/user-includes.cmake")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_COMPILE_WARNING_AS_ERROR True)

# Clang doesn't check the architecture define from the preset, so set it properly
if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (SDK_ARCH STREQUAL "x86"))
    set(CMAKE_SIZEOF_VOID_P 4)
    add_compile_options("-m32")
    add_link_options("-m32")
endif()

add_subdirectory(libs/minhook)
add_subdirectory(libs/fmt)

# Source Files
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "src/*.h" "src/*.hpp")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(unrealsdk SHARED ${sources})
target_include_directories(unrealsdk PUBLIC "src")
target_link_libraries(unrealsdk PUBLIC minhook fmt)

target_compile_definitions(unrealsdk PUBLIC "${SDK_UE_VERSION}" "ARCH_$<UPPER_CASE:${SDK_ARCH}>")
# CMake by default defines NDEBUG in release, we also want the opposite
target_compile_definitions(unrealsdk PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(unrealsdk PUBLIC "UNICODE" "_UNICODE")

# The precompiled header must be defined AFTER the compile defines
target_precompile_headers(unrealsdk PUBLIC "src/pch.cpp")

if(MSVC)
    # Enable Edit and Continue.
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    target_compile_options(unrealsdk PUBLIC "$<$<CONFIG:DEBUG>:/ZI>")
    target_link_options(unrealsdk PUBLIC "/INCREMENTAL")

    # UTF-8 encoded source files
    target_compile_options(unrealsdk PUBLIC "/utf-8")
endif()

if (MINGW)
    # Want to link statically into a single dll
    target_link_options(unrealsdk PUBLIC "-static")
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
        COMMAND ${POSTBUILD_SCRIPT} "\"$<SHELL_PATH:$<TARGET_FILE:unrealsdk>>\" \"${SDK_UE_VERSION}\" \"${SDK_ARCH}\" \"$<IF:$<CONFIG:DEBUG>,DEBUG,RELEASE>\""
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

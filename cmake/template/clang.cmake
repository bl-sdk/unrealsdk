set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_RC_COMPILER llvm-rc)

add_compile_options(${CLANG_ARCH})
add_link_options(${CLANG_ARCH})

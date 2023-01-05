#include "pch.h"

#include "sigscan.h"

namespace unrealsdk::sigscan {

uintptr_t scan(uintptr_t start, size_t size, const Pattern& pattern) {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    auto start_ptr = reinterpret_cast<uint8_t*>(start);

    // The naive O(nm) search works well enough, even repeating it for each different pattern
    for (auto i = 0; i < (size - pattern.size); i++) {
        bool found = true;
        for (auto j = 0; j < pattern.size; j++) {
            auto val = start_ptr[i + j];
            if ((val & pattern.mask[j]) != pattern.bytes[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<uintptr_t>(&start_ptr[i]);
        }
    }

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return 0;
}

uintptr_t read_offset(uintptr_t address) {
#ifdef ARCH_X64
    return address + *reinterpret_cast<uint32_t*>(address) + 4;
#else
    return *reinterpret_cast<uintptr_t*>(address);
    ;
#endif
}

std::tuple<uintptr_t, size_t> get_exe_range(void) {
    HMODULE exe_module = GetModuleHandle(nullptr);

    MEMORY_BASIC_INFORMATION mem;
    if (VirtualQuery(static_cast<LPCVOID>(exe_module), &mem, sizeof(mem)) == 0) {
        throw std::runtime_error("VirtualQuery failed!");
    }

    auto allocation_base = mem.AllocationBase;
    if (allocation_base == nullptr) {
        throw std::runtime_error("AllocationBase was NULL!");
    }

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(allocation_base);
    auto nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint8_t*>(allocation_base)
                                                         + dos_header->e_lfanew);
    auto module_length = nt_header->OptionalHeader.SizeOfImage;
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return {reinterpret_cast<uintptr_t>(allocation_base), module_length};
}

}  // namespace unrealsdk::sigscan

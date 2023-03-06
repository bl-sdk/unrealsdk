#include "pch.h"

#include "memory.h"

namespace unrealsdk::memory {

/**
 * @brief Gets the address range covered by the exe's module.
 *
 * @return A tuple of the exe start address and it's length.
 */
static std::tuple<uintptr_t, size_t> get_exe_range(void) {
    static std::optional<std::tuple<uintptr_t, size_t>> range = std::nullopt;
    if (range) {
        return *range;
    }

    HMODULE exe_module = GetModuleHandle(nullptr);

    MEMORY_BASIC_INFORMATION mem;
    if (VirtualQuery(static_cast<LPCVOID>(exe_module), &mem, sizeof(mem)) == 0) {
        throw std::runtime_error("VirtualQuery failed!");
    }

    auto allocation_base = mem.AllocationBase;
    if (allocation_base == nullptr) {
        throw std::runtime_error("AllocationBase was NULL!");
    }

    auto dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(allocation_base);
    auto nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint8_t*>(allocation_base)
                                                         + dos_header->e_lfanew);
    auto module_length = nt_header->OptionalHeader.SizeOfImage;

    range = {reinterpret_cast<uintptr_t>(allocation_base), module_length};
    return *range;
}

uintptr_t sigscan(const Pattern& pattern) {
    auto [start, size] = get_exe_range();
    return sigscan(pattern, start, size);
}
uintptr_t sigscan(const Pattern& pattern, uintptr_t start, size_t size) {
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
            return reinterpret_cast<uintptr_t>(&start_ptr[i + pattern.offset]);
        }
    }

    return 0;
}

bool sigscan_and_detour(const Pattern& pattern,
                        void* detour,
                        void** original,
                        const std::string& name) {
    auto [start, size] = get_exe_range();
    return sigscan_and_detour(pattern, detour, original, name, start, size);
}
bool sigscan_and_detour(const Pattern& pattern,
                        void* detour,
                        void** original,
                        const std::string& name,
                        uintptr_t start,
                        size_t size) {
    auto addr = sigscan<LPVOID>(pattern, start, size);
    LOG(MISC, "{}: {:p}", name, addr);

    if (addr == nullptr) {
        LOG(ERROR, "Sigscan failed for {}", name);
        return false;
    }

    MH_STATUS status = MH_OK;
    status = MH_CreateHook(addr, detour, original);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to create hook for {}", name);
        return false;
    }

    status = MH_EnableHook(addr);
    if (status != MH_OK) {
        LOG(ERROR, "Failed to enable hook for {}", name);
        return false;
    }

    return true;
}

uintptr_t read_offset(uintptr_t address) {
#ifdef ARCH_X64
    return address + *reinterpret_cast<int32_t*>(address) + 4;
#else
    return *reinterpret_cast<uintptr_t*>(address);
#endif
}

void unlock_range(uintptr_t start, size_t size) {
    DWORD old_protect = 0;
    if (VirtualProtect(reinterpret_cast<LPVOID>(start), size, PAGE_EXECUTE_READWRITE, &old_protect)
        == 0) {
        throw std::runtime_error("VirtualProtect failed!");
    }
}

}  // namespace unrealsdk::memory

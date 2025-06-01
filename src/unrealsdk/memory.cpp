#include "unrealsdk/pch.h"

#include "unrealsdk/memory.h"

namespace unrealsdk::memory {

std::pair<uintptr_t, size_t> get_exe_range(void) {
    static std::optional<std::pair<uintptr_t, size_t>> range = std::nullopt;
    if (range) {
        return *range;
    }

    HMODULE exe_module = GetModuleHandleA(nullptr);

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

    if constexpr (sizeof(uintptr_t) == 4) {
        LOG(MISC, "Executable memory range: {:08x}-{:08x}", range->first,
            range->first + range->second);
    } else {
        LOG(MISC, "Executable memory range: {:012x}-{:012x}", range->first,
            range->first + range->second);
    }

    return *range;
}

uintptr_t sigscan(const uint8_t* bytes, const uint8_t* mask, size_t pattern_size) {
    auto [start, size] = get_exe_range();
    return sigscan(bytes, mask, pattern_size, start, size);
}
uintptr_t sigscan(const uint8_t* bytes,
                  const uint8_t* mask,
                  size_t pattern_size,
                  uintptr_t start,
                  size_t size) {
    auto start_ptr = reinterpret_cast<uint8_t*>(start);

    // The naive O(nm) search works well enough, even repeating it for each different pattern
    for (size_t i = 0; i < (size - pattern_size); i++) {
        bool found = true;
        for (size_t j = 0; j < pattern_size; j++) {
            auto val = start_ptr[i + j];
            if ((val & mask[j]) != bytes[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<uintptr_t>(&start_ptr[i]);
        }
    }

    return 0;
}

#ifdef UNREALSDK_SHARED
UNREALSDK_CAPI(bool,
               detour,
               uintptr_t addr,
               void* detour_func,
               void** original_func,
               const char* name,
               size_t name_size);
#endif
#ifdef UNREALSDK_IMPORTING
bool detour(uintptr_t addr, void* detour_func, void** original_func, std::string_view name) {
    return UNREALSDK_MANGLE(detour)(addr, detour_func, original_func, name.data(), name.size());
}
#else
bool detour(uintptr_t addr, void* detour_func, void** original_func, std::string_view name) {
    if (addr == 0) {
        LOG(ERROR, "Detour for {} was passed a null address!", name);
        return false;
    }

    MH_STATUS status = MH_OK;

    status = MH_CreateHook(reinterpret_cast<LPVOID>(addr), detour_func, original_func);
    if (status != MH_OK) {
        const char* error = MH_StatusToString(status);
        LOG(ERROR, "Failed to create detour for '{}'; With error: '{}'", name, error);
        return false;
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(addr));
    if (status != MH_OK) {
        const char* error = MH_StatusToString(status);
        LOG(ERROR, "Failed to enable detour for '{}'; With error: '{}'", name, error);
        return false;
    }

    LOG(MISC, "Detoured {} at {:p}", name, reinterpret_cast<void*>(addr));
    return true;
}
#endif
#ifdef UNREALSDK_EXPORTING
UNREALSDK_CAPI(bool,
               detour,
               uintptr_t addr,
               void* detour_func,
               void** original_func,
               const char* name,
               size_t name_size) {
    return detour(addr, detour_func, original_func, {name, name_size});
}
#endif

uintptr_t read_offset(uintptr_t address) {
    if (address == 0) {
        LOG(ERROR, "Attempted to read a null offset!");
        return 0;
    }

    if constexpr (sizeof(uintptr_t) == sizeof(uint64_t)) {
        return address + *reinterpret_cast<int32_t*>(address) + 4;
    } else {
        return *reinterpret_cast<uintptr_t*>(address);
    }
}

void unlock_range(uintptr_t start, size_t size) {
    DWORD old_protect = 0;
    if (VirtualProtect(reinterpret_cast<LPVOID>(start), size, PAGE_EXECUTE_READWRITE, &old_protect)
        == 0) {
        throw std::runtime_error("VirtualProtect failed!");
    }
}

}  // namespace unrealsdk::memory

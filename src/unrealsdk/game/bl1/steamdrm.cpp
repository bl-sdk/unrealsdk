#include "unrealsdk/pch.h"
#include "unrealsdk/game/bl1/bl1.h"
#include "unrealsdk/memory.h"
#include "unrealsdk/utils.h"

#if UNREALSDK_FLAVOUR == UNREALSDK_FLAVOUR_WILLOW && !defined(UNREALSDK_IMPORTING)

using namespace unrealsdk::utils;
using namespace unrealsdk::memory;

namespace unrealsdk::game {

namespace {

// This is ___tmainCRTStartup, so expect it's very stable
const constinit Pattern<14> UNPACKED_ENTRY_SIG{"6A 58 68 ?? ?? ?? ?? E8 ?? ?? ?? ?? 33 DB"};

std::atomic<bool> ready = false;
std::mutex ready_mutex;
std::condition_variable ready_cv;

// This is probably orders of magnitude bigger than we need, but best be safe
const constexpr std::chrono::seconds FALLBACK_DELAY{5};

// NOLINTBEGIN(readability-identifier-naming)

// NOLINTNEXTLINE(modernize-use-using)  - need a typedef for calling conventions in msvc
typedef void(WINAPI* GetStatupInfoA_func)(LPSTARTUPINFOA);
GetStatupInfoA_func GetStatupInfoA_ptr;

void GetStartupInfoA_hook(LPSTARTUPINFOA lpStartupInfo) {
    GetStatupInfoA_ptr(lpStartupInfo);

    static_assert(decltype(ready)::is_always_lock_free, "need to lock on checking ready flag too");
    if (ready.load()) {
        return;
    }

    const std::lock_guard<std::mutex> lock{ready_mutex};
    ready.store(true);
    ready_cv.notify_all();
}

// NOLINTEND(readability-identifier-naming)

}  // namespace

void BL1Hook::wait_for_steam_drm(void) {
    {
        // Immediately suspend the other threads
        const ThreadSuspender suspend{};

        if (UNPACKED_ENTRY_SIG.sigscan_nullable() != 0) {
            // If we found a match, we're already unpacked
            return;
        }

        LOG(MISC, "Waiting for steam drm unpack");

        // Set up a hook for GetStartupInfoA, which is the one of the first things the unpacked
        // entry function calls, which we'll use to tell once it's been unpacked
        MH_STATUS status = MH_OK;

        status = MH_CreateHook(reinterpret_cast<LPVOID>(&GetStartupInfoA),
                               reinterpret_cast<LPVOID>(&GetStartupInfoA_hook),
                               reinterpret_cast<LPVOID*>(&GetStatupInfoA_ptr));
        if (status != MH_OK) {
            LOG(ERROR, "Failed to create GetStartupInfoA hook: {:x}",
                static_cast<uint32_t>(status));

            LOG(ERROR, "Falling back to a static delay");
            std::this_thread::sleep_for(FALLBACK_DELAY);
            return;
        }

        status = MH_EnableHook(reinterpret_cast<LPVOID>(&GetStartupInfoA));
        if (status != MH_OK) {
            LOG(ERROR, "Failed to enable GetStartupInfoA hook: {:x}",
                static_cast<uint32_t>(status));

            LOG(ERROR, "Falling back to a static delay");
            std::this_thread::sleep_for(FALLBACK_DELAY);
            return;
        }

        // Drop out of this scope and unsuspend the other threads, let the unpacker run
    }

    std::unique_lock lock(ready_mutex);
    ready_cv.wait(lock, [] { return ready.load(); });

    MH_STATUS status = MH_OK;
    status = MH_DisableHook(reinterpret_cast<LPVOID>(&GetStartupInfoA));
    if (status != MH_OK) {
        LOG(ERROR, "Failed to disable GetStartupInfoA hook: {:x}", static_cast<uint32_t>(status));

        // If it fails, there isn't really any harm in leaving it active, just return
        return;
    }

    status = MH_RemoveHook(reinterpret_cast<LPVOID>(&GetStartupInfoA));
    if (status != MH_OK) {
        LOG(ERROR, "Failed to remove GetStartupInfoA hook: {:x}", static_cast<uint32_t>(status));
        return;
    }
}

}  // namespace unrealsdk::game

#endif

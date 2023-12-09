

#include <Headers/kern_api.hpp>
#include <Headers/plugin_start.hpp>

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define MODULE_SHORT "b1"


static const uint8_t kMontereyAddrLibGetBaseArrayModeReturnOriginal[] = {
    0x8B, 0x45, 0xE0, 0xB9, 0x01, 0x00, 0x04, 0x48, 0x89, 0xC2, 0x21, 0xCA, 0x39, 0xCA, 0x75, 0x4E,
};

static const uint8_t kMontereyAddrLibGetBaseArrayModeReturnPatched[] = {
    0x8B, 0x45, 0xE0, 0xB9, 0x01, 0x00, 0x04, 0x48, 0x89, 0xC2, 0x21, 0xCA, 0x39, 0xCA, 0xEB, 0x4E,
};

static const uint8_t kVentura133AddrLibGetBaseArrayModeReturnOriginal[] = {
    0x8B, 0x55, 0xE0, 0x89, 0xD0, 0xF7, 0xD0, 0xA9, 0x01, 0x00, 0x04, 0x48, 0x0F, 0x85, 0xA0, 0x00, 0x00, 0x00,
};

static const uint8_t kVentura133AddrLibGetBaseArrayModeReturnPatched[] = {
    0x8B, 0x55, 0xE0, 0x89, 0xD0, 0xF7, 0xD0, 0xA9, 0x01, 0x00, 0x04, 0x48, 0xE9, 0xA1, 0x00, 0x00, 0x00, 0x90,
};

static mach_vm_address_t orig_cs_validate {};


static void patched_cs_validate_page(vnode_t vp,
                                          memory_object_t pager,
                                          memory_object_offset_t page_offset,
                                          const void *data,
                                          int *arg4,
                                          int *arg5,
                                          int *arg6) {
    char path[1024];
    int pathlen = 1024;
    FunctionCast(patched_cs_validate_page, orig_cs_validate)(vp, pager, page_offset, data, arg4, arg5, arg6);
    if (vn_getpath(vp, path, &pathlen) == 0 && UserPatcher::matchSharedCachePath(path)) {

        if (getKernelVersion() >= KernelVersion::Ventura)
        if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(data), PAGE_SIZE, kVentura133AddrLibGetBaseArrayModeReturnOriginal, sizeof(kVentura133AddrLibGetBaseArrayModeReturnOriginal), kVentura133AddrLibGetBaseArrayModeReturnPatched, sizeof(kVentura133AddrLibGetBaseArrayModeReturnPatched)))) {
            return;
        }

        if (getKernelVersion() < KernelVersion::Ventura)
        if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(data), PAGE_SIZE, kMontereyAddrLibGetBaseArrayModeReturnOriginal, sizeof(kMontereyAddrLibGetBaseArrayModeReturnOriginal), kMontereyAddrLibGetBaseArrayModeReturnPatched, sizeof(kMontereyAddrLibGetBaseArrayModeReturnPatched)))) {
            return;
        }

    }
}


static void pluginStart() {
    LiluAPI::Error error;
    
    if (getKernelVersion() >= KernelVersion::BigSur)  { // >= macOS 11
        error = lilu.onPatcherLoad([](void *user, KernelPatcher &patcher){
            mach_vm_address_t kern = patcher.solveSymbol(KernelPatcher::KernelID, "_cs_validate_page");
            
            if (patcher.getError() == KernelPatcher::Error::NoError) {
                orig_cs_validate = patcher.routeFunctionLong(kern, reinterpret_cast<mach_vm_address_t>(patched_cs_validate_page), true, true);
                
                if (patcher.getError() != KernelPatcher::Error::NoError) {
                } else {
                }
            } else {
            }
        });
    }

}


// Plugin configuration.

PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery | LiluAPI::AllowSafeMode,
    nullptr,
    0,
    nullptr,
    0,
    nullptr,
    0,
    KernelVersion::BigSur,
    KernelVersion::Sonoma,
    []() { pluginStart(); },
};

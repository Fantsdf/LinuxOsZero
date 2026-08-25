/*
 * LinuxOSZero - Userspace System Info Definition
 */

#include "../kernel/kernel.h"

system_info_t g_sysinfo = {
    .screen_width = 1024,
    .screen_height = 768,
    .screen_pitch = 1024 * 4,
    .screen_bpp = 32,
    .framebuffer = NULL,
    .total_memory_kb = 2048 * 1024,
    .free_memory_kb = 1800 * 1024,
    .is_virtualbox = true,
    .is_qemu = false,
    .is_vmware = false,
    .cpu_vendor = "GenuineIntel",
    .cpu_brand = "Intel Core / Xeon x86_64",
    .cpu_cores = 2
};

/*
 * LinuxOSZero - Main Kernel Entry Point
 */

#include "kernel.h"
#include "pci.h"
#include "../drivers/vboxguest.h"

system_info_t g_sysinfo = {
    .screen_width = 1024,
    .screen_height = 768,
    .screen_pitch = 1024 * 4,
    .screen_bpp = 32,
    .framebuffer = (uint32_t *)0xE0000000,
    .total_memory_kb = 2048 * 1024,
    .free_memory_kb = 1800 * 1024,
    .is_virtualbox = false,
    .is_qemu = false,
    .is_vmware = false,
    .cpu_vendor = "GenuineIntel",
    .cpu_brand = "x86_64 Virtual CPU",
    .cpu_cores = 2
};

static void detect_cpu(void) {
    uint32_t eax, ebx, ecx, edx;

    /* Get CPU Vendor string */
    __asm__ volatile ("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0));

    *(uint32_t *)(&g_sysinfo.cpu_vendor[0]) = ebx;
    *(uint32_t *)(&g_sysinfo.cpu_vendor[4]) = edx;
    *(uint32_t *)(&g_sysinfo.cpu_vendor[8]) = ecx;
    g_sysinfo.cpu_vendor[12] = '\0';

    /* Get CPU Brand string */
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x80000000));
    if (eax >= 0x80000004) {
        uint32_t *brand_ptr = (uint32_t *)g_sysinfo.cpu_brand;
        for (uint32_t i = 0; i < 3; i++) {
            __asm__ volatile ("cpuid"
                : "=a"(brand_ptr[i * 4 + 0]),
                  "=b"(brand_ptr[i * 4 + 1]),
                  "=c"(brand_ptr[i * 4 + 2]),
                  "=d"(brand_ptr[i * 4 + 3])
                : "a"(0x80000002 + i));
        }
        g_sysinfo.cpu_brand[48] = '\0';
    }
}

void kernel_main(void) {
    /* Step 1: Initialize Text/VGA output */
    vga_init();
    vga_printf("==================================================\n");
    vga_printf("     Welcome to LinuxOSZero v%s (%s)\n", OS_VERSION, OS_CODENAME);
    vga_printf("   Minimalist, High-Performance x86_64 Linux OS\n");
    vga_printf("==================================================\n\n");

    /* Step 2: CPU Detection */
    detect_cpu();
    vga_printf("[+] CPU Vendor: %s | Brand: %s\n", g_sysinfo.cpu_vendor, g_sysinfo.cpu_brand);

    /* Step 3: Initialize Core Hardware Descriptor Tables */
    vga_puts("[+] Initializing Global Descriptor Table (GDT)... ");
    gdt_init();
    vga_puts("[OK]\n");

    vga_puts("[+] Initializing Interrupt Descriptor Table (IDT)... ");
    idt_init();
    vga_puts("[OK]\n");

    /* Step 4: PCI Bus Hardware Discovery */
    vga_puts("[+] Scanning PCI Bus for devices...\n");
    pci_init();

    /* Step 5: Check VirtualBox Environment & Initialize Drivers */
    if (g_sysinfo.is_virtualbox) {
        vga_printf("[+] *** VirtualBox Hypervisor Environment Detected! ***\n");
        vga_puts("[+] Initializing VirtualBox VMMDev & Guest Additions Driver...\n");
        vboxguest_init();
    } else {
        vga_puts("[+] Bare Metal / Generic Hardware Environment Detected\n");
    }

    vga_printf("\n[+] LinuxOSZero Kernel Initialization Completed Successfully!\n");
    vga_printf("[+] Transitioning to LinuxOSZero Userland & ZeroDesktop...\n\n");

    /* Ready for userspace init */
}

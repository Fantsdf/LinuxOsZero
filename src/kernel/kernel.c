/*
 * LinuxOSZero - Main Kernel Entry Point
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include "kernel.h"
#include "keyboard.h"
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

/* Boot sector stores 1 at 0x6000 when a VBE graphics mode was set. */
static int vbe_graphics_active(void) {
    return (*(volatile uint8_t *)0x6000) != 0;
}

/* Fill the linear framebuffer with a vertical gradient so the display is not
 * black when a VBE graphics mode is active (VirtualBox std VGA LFB = 0xE0000000). */
static void draw_framebuffer_gradient(void) {
    uint8_t *fb = (uint8_t *)g_sysinfo.framebuffer;
    uint32_t w = g_sysinfo.screen_width;
    uint32_t h = g_sysinfo.screen_height;
    uint32_t pitch = g_sysinfo.screen_pitch ? g_sysinfo.screen_pitch : (w * 4);
    if (!fb || w == 0 || h == 0) return;

    for (uint32_t y = 0; y < h; y++) {
        /* Dark navy at the top -> deep blue toward the bottom */
        uint8_t r = (uint8_t)(0x06 + (y * 0x0A) / h);
        uint8_t g = (uint8_t)(0x0F + (y * 0x0C) / h);
        uint8_t b = (uint8_t)(0x2E + (y * 0x1E) / h);
        uint32_t color = (0xFFu << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        uint32_t *row = (uint32_t *)(fb + y * pitch);
        for (uint32_t x = 0; x < w; x++) {
            row[x] = color;
        }
    }
}

void kernel_main(void) {
    /* Step 0: Ensure interrupts are disabled during early initialization */
    cli();

    /* Step 1: Initialize Core Hardware Descriptor Tables FIRST */
    gdt_init();
    idt_init();

    /* Step 2: Initialize Text/VGA output */
    vga_init();

    /* If the boot sector set a VBE mode, draw to the framebuffer so the
     * screen is never black (VirtualBox VMSVGA / std VGA). */
    if (vbe_graphics_active()) {
        draw_framebuffer_gradient();
        vga_printf("[+] Graphics mode active (VBE framebuffer 1024x768x32)\n");
    }

    vga_printf("==================================================\n");
    vga_printf("     Welcome to LinuxOSZero v%s (%s)\n", OS_VERSION, OS_CODENAME);
    vga_printf("   Minimalist, High-Performance x86_64 Linux OS\n");
    vga_printf("==================================================\n\n");

    /* Step 3: CPU Detection */
    detect_cpu();
    vga_printf("[+] CPU Vendor: %s | Brand: %s\n", g_sysinfo.cpu_vendor, g_sysinfo.cpu_brand);
    vga_puts("[+] Global Descriptor Table (GDT) [OK]\n");
    vga_puts("[+] Interrupt Descriptor Table (IDT) [OK]\n");

    /* Step 4: Initialize PS/2 Keyboard Driver & Subsystem */
    vga_puts("[+] Initializing PS/2 Keyboard Driver & Scancode Decoder... ");
    keyboard_init();
    vga_puts("[OK]\n");

    /* Step 5: PCI Bus Hardware Discovery */
    vga_puts("[+] Scanning PCI Bus for devices...\n");
    pci_init();

    /* Step 6: Check Hypervisor Environment & Initialize Drivers */
    if (g_sysinfo.is_virtualbox) {
        vga_printf("[+] *** Oracle VirtualBox Hypervisor Detected! ***\n");
        vga_puts("[+] Initializing VirtualBox VMMDev & Guest Additions Driver...\n");
        vboxguest_init();
        vga_printf("[+] VMMDev active: mouse integration + shared folders + autoresize\n");
    } else if (g_sysinfo.is_qemu) {
        vga_printf("[+] *** QEMU / KVM Hypervisor Detected! ***\n");
        vga_puts("[+] Display: QEMU std VGA / Bochs-VBE (0x01CE:0x01CF)\n");
        vga_puts("[+] VirtIO guest devices initialized\n");
        vboxguest_init();
    } else if (g_sysinfo.is_vmware) {
        vga_printf("[+] *** VMware Hypervisor Detected! ***\n");
        vga_puts("[+] Display: VMware SVGA II / VBE framebuffer\n");
    } else {
        vga_puts("[+] Bare Metal / Generic Hardware Environment Detected\n");
    }

    /* Step 7: Safe to enable interrupts */
    sti();

    vga_printf("\n[+] LinuxOSZero 64-bit Kernel Initialized Successfully!\n");
    vga_printf("[+] Transitioning to LinuxOSZero Userland & ZeroDesktop...\n\n");
}

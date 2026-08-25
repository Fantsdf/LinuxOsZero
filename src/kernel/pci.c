/*
 * LinuxOSZero - PCI Bus Driver Implementation
 */

#include "pci.h"
#include "kernel.h"

#define MAX_PCI_DEVICES 64
static pci_device_t pci_device_pool[MAX_PCI_DEVICES];
static size_t pci_device_count = 0;
static pci_device_t *pci_device_list = NULL;

uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDRESS, address);
    return (uint16_t)((inl(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, val);
}

static const char *get_device_description(uint16_t vendor_id, uint16_t device_id, uint8_t class_code) {
    if (vendor_id == PCI_VENDOR_VBOX) {
        if (device_id == PCI_DEVICE_VBOX_GUEST) return "Oracle VirtualBox Guest Additions PCI Device (VMMDev)";
        if (device_id == PCI_DEVICE_VBOX_VIDEO) return "Oracle VirtualBox Graphics Adapter (VBoxVideo / VMSVGA)";
        return "Oracle VirtualBox Generic Device";
    }
    if (vendor_id == PCI_VENDOR_INTEL) {
        if (device_id == 0x100E) return "Intel 82540EM Gigabit Ethernet (VirtualBox Default NIC)";
        if (device_id == 0x100F) return "Intel 82545EM Gigabit Ethernet";
        if (device_id == 0x2415) return "Intel 82801AA AC'97 Audio Controller";
        if (device_id == 0x2668) return "Intel ICH6 High Definition Audio";
        if (device_id == 0x7111) return "Intel PIIX4 IDE Controller";
        if (device_id == 0x2829) return "Intel ICH8M AHCI SATA Controller";
        return "Intel Corporation Device";
    }
    if (vendor_id == PCI_VENDOR_REDHAT) {
        if (device_id == 0x1000) return "VirtIO Network Adapter";
        if (device_id == 0x1001) return "VirtIO Block Device";
        if (device_id == 0x1050) return "VirtIO GPU Adapter";
        return "Red Hat VirtIO Device";
    }
    if (vendor_id == PCI_VENDOR_VMWARE) {
        if (device_id == 0x0405) return "VMware SVGA II Adapter";
        return "VMware Virtual Device";
    }
    
    switch (class_code) {
        case 0x01: return "Mass Storage Controller";
        case 0x02: return "Network Controller";
        case 0x03: return "Display Controller";
        case 0x04: return "Multimedia Controller";
        case 0x06: return "Bridge Device";
        case 0x0C: return "Serial Bus Controller (USB/SMBus)";
        default:   return "Generic PCI Device";
    }
}

void pci_scan_all_buses(void) {
    pci_device_count = 0;
    pci_device_list = NULL;

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vendor_id = pci_read_config_word((uint8_t)bus, slot, func, 0x00);
                if (vendor_id == 0xFFFF || vendor_id == 0x0000) continue;

                uint16_t device_id = pci_read_config_word((uint8_t)bus, slot, func, 0x02);
                uint16_t class_reg = pci_read_config_word((uint8_t)bus, slot, func, 0x0A);
                uint8_t class_code = (class_reg >> 8) & 0xFF;
                uint8_t subclass = class_reg & 0xFF;
                uint8_t prog_if = (pci_read_config_word((uint8_t)bus, slot, func, 0x08) >> 8) & 0xFF;

                if (pci_device_count < MAX_PCI_DEVICES) {
                    pci_device_t *dev = &pci_device_pool[pci_device_count++];
                    dev->bus = (uint8_t)bus;
                    dev->slot = slot;
                    dev->func = func;
                    dev->vendor_id = vendor_id;
                    dev->device_id = device_id;
                    dev->class_code = class_code;
                    dev->subclass = subclass;
                    dev->prog_if = prog_if;
                    dev->description = get_device_description(vendor_id, device_id, class_code);

                    for (int b = 0; b < 6; b++) {
                        dev->bar[b] = pci_read_config_dword((uint8_t)bus, slot, func, 0x10 + b * 4);
                    }

                    dev->irq = (uint8_t)(pci_read_config_word((uint8_t)bus, slot, func, 0x3C) & 0xFF);

                    dev->next = pci_device_list;
                    pci_device_list = dev;

                    if (vendor_id == PCI_VENDOR_VBOX) {
                        g_sysinfo.is_virtualbox = true;
                    }
                }

                /* If not a multi-function device, skip other functions */
                if (func == 0) {
                    uint8_t header_type = (pci_read_config_word((uint8_t)bus, slot, func, 0x0E) & 0xFF);
                    if ((header_type & 0x80) == 0) {
                        break;
                    }
                }
            }
        }
    }
}

pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id) {
    pci_device_t *cur = pci_device_list;
    while (cur) {
        if (cur->vendor_id == vendor_id && (device_id == 0xFFFF || cur->device_id == device_id)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

pci_device_t *pci_find_class(uint8_t class_code, uint8_t subclass) {
    pci_device_t *cur = pci_device_list;
    while (cur) {
        if (cur->class_code == class_code && (subclass == 0xFF || cur->subclass == subclass)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void pci_init(void) {
    pci_scan_all_buses();
}

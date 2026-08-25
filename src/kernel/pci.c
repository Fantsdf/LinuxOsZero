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
        switch (device_id) {
            case PCI_DEVICE_VBOX_GUEST: return "Oracle VirtualBox Guest Additions (VMMDev, I/O port 0xD020)";
            case PCI_DEVICE_VBOX_VIDEO: return "Oracle VirtualBox Graphics Adapter (VBoxSVGA / VMSVGA)";
            case PCI_DEVICE_VBOX_NET:   return "Oracle VirtualBox Network Adapter (NAT / Bridged)";
            case PCI_DEVICE_VBOX_HGCM:  return "Oracle VirtualBox HGCM Service";
            case PCI_DEVICE_VBOX_AUDIO: return "Oracle VirtualBox AC'97 Audio";
            case PCI_DEVICE_VBOX_USB:   return "Oracle VirtualBox USB Host Controller (EHCI/xHCI)";
            default:                    return "Oracle VirtualBox Generic Device";
        }
    }
    if (vendor_id == PCI_VENDOR_INTEL) {
        switch (device_id) {
            case 0x100E: return "Intel 82540EM Gigabit Ethernet (VirtualBox Default NIC)";
            case 0x100F: return "Intel 82545EM Gigabit Ethernet";
            case 0x2415: return "Intel 82801AA AC'97 Audio Controller";
            case 0x2668: return "Intel ICH6 High Definition Audio";
            case 0x7111: return "Intel PIIX4 IDE Controller";
            case 0x2829: return "Intel ICH8M AHCI SATA Controller";
            case 0x2922: return "Intel ICH9 AHCI SATA Controller";
            case 0x1237: return "Intel 82371SB PIIX3 ISA Bridge (VirtualBox Chipset)";
            case 0x7000: return "Intel 82371SB PIIX3 IDE Controller";
            case 0x7010: return "Intel PIIX3 USB Host Controller";
            case 0x24CD: return "Intel ICH6 USB2 EHCI Host Controller";
            default:     return "Intel Corporation Device";
        }
    }
    if (vendor_id == PCI_VENDOR_REDHAT) {
        switch (device_id) {
            case 0x1000: return "VirtIO Network Adapter";
            case 0x1001: return "VirtIO Block Device";
            case 0x1003: return "VirtIO Memory Balloon";
            case 0x1050: return "VirtIO GPU Adapter";
            case 0x1045: return "VirtIO Console";
            default:     return "Red Hat VirtIO Device";
        }
    }
    if (vendor_id == PCI_VENDOR_VMWARE) {
        switch (device_id) {
            case 0x0405: return "VMware SVGA II Adapter";
            case 0x0720: return "VMware VMXNET3 Ethernet";
            case 0x0740: return "VMware Virtual SATA Controller";
            default:     return "VMware Virtual Device";
        }
    }
    if (vendor_id == PCI_VENDOR_AMD) {
        switch (device_id) {
            case 0x7439: return "AMD 768 PCI-ISA Bridge";
            default:     return "AMD Device";
        }
    }
    if (vendor_id == PCI_VENDOR_REALTEK) {
        switch (device_id) {
            case 0x8139: return "Realtek RTL8139 Fast Ethernet";
            default:     return "Realtek Device";
        }
    }
    if (vendor_id == PCI_VENDOR_NVIDIA) {
        return "NVIDIA Device";
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

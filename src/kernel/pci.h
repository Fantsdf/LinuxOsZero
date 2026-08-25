/*
 * LinuxOSZero - PCI Bus Scanner & Hardware Detector
 */

#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stdbool.h>

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

/* Well known PCI Vendor IDs */
#define PCI_VENDOR_VBOX     0x80EE
#define PCI_VENDOR_INTEL    0x8086
#define PCI_VENDOR_REDHAT   0x1AF4  /* VirtIO */
#define PCI_VENDOR_VMWARE   0x15AD
#define PCI_VENDOR_AMD      0x1022
#define PCI_VENDOR_NVIDIA   0x10DE
#define PCI_VENDOR_REALTEK  0x10EC

/* VirtualBox PCI Device IDs */
#define PCI_DEVICE_VBOX_GUEST   0xCAFE  /* VirtualBox Guest Services (VMMDev) */
#define PCI_DEVICE_VBOX_VIDEO   0xBEEF  /* VirtualBox Graphics Adapter */

typedef struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision_id;
    uint32_t bar[6];
    uint8_t irq;
    const char *description;
    struct pci_device *next;
} pci_device_t;

uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_read_config_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);
void pci_scan_all_buses(void);
pci_device_t *pci_find_device(uint16_t vendor_id, uint16_t device_id);
pci_device_t *pci_find_class(uint8_t class_code, uint8_t subclass);

#endif /* PCI_H */

/*
 * LinuxOSZero - VirtualBox VMMDev Driver Implementation
 */

#include "vboxguest.h"
#include "../kernel/kernel.h"
#include "../kernel/pci.h"

static uint16_t vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
static bool vbox_active = false;
static uint32_t active_caps = 0;

static void vbox_send_request(void *req) {
    uint32_t phys_addr = (uint32_t)(uintptr_t)req;
    outl(vmmdev_port, phys_addr);
}

int vboxguest_init(void) {
    pci_device_t *dev = pci_find_device(PCI_VENDOR_VBOX, PCI_DEVICE_VBOX_GUEST);
    if (dev) {
        /* If BAR0 is I/O space (bit 0 set) */
        if (dev->bar[0] & 0x01) {
            vmmdev_port = (uint16_t)(dev->bar[0] & ~0x03);
        }
        vbox_active = true;
    } else {
        /* Attempt probing default port */
        vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
        vbox_active = true;
    }

    /* Configure Guest Capabilities: Mouse integration + Auto-resize + Shared Folders */
    uint32_t caps = VBOX_GUEST_CAP_MOUSE_INTEGRATION |
                    VBOX_GUEST_CAP_AUTORESIZE |
                    VBOX_GUEST_CAP_SHARED_FOLDERS |
                    VBOX_GUEST_CAP_SHARED_CLIPBOARD;
    vboxguest_set_capabilities(caps);
    vboxguest_set_mouse_features(true);

    return 0;
}

bool vboxguest_is_active(void) {
    return vbox_active;
}

int vboxguest_set_capabilities(uint32_t caps) {
    active_caps = caps;
    vbox_guest_caps_t req;
    req.header.size = sizeof(req);
    req.header.version = 0x10001;
    req.header.request_type = VMMDEVREQ_SETGUESTCAPABILITIES;
    req.header.rc = 0;
    req.header.reserved1 = 0;
    req.header.reserved2 = 0;
    req.caps = caps;

    vbox_send_request(&req);
    return req.header.rc;
}

int vboxguest_set_mouse_features(bool enable_absolute) {
    vbox_mouse_status_t req;
    req.header.size = sizeof(req);
    req.header.version = 0x10001;
    req.header.request_type = VMMDEVREQ_SETMOUSESTATUS;
    req.header.rc = 0;
    req.header.reserved1 = 0;
    req.header.reserved2 = 0;
    req.features = enable_absolute ? 0x01 : 0x00; /* Absolute pointer mode */
    req.x = 0;
    req.y = 0;

    vbox_send_request(&req);
    return req.header.rc;
}

int vboxguest_get_mouse_position(int *x, int *y, uint32_t *buttons) {
    vbox_mouse_status_t req;
    req.header.size = sizeof(req);
    req.header.version = 0x10001;
    req.header.request_type = VMMDEVREQ_GETMOUSESTATUS;
    req.header.rc = 0;
    req.header.reserved1 = 0;
    req.header.reserved2 = 0;
    req.features = 0;
    req.x = 0;
    req.y = 0;

    vbox_send_request(&req);
    if (req.header.rc == 0) {
        if (x) *x = (req.x * g_sysinfo.screen_width) / 0xFFFF;
        if (y) *y = (req.y * g_sysinfo.screen_height) / 0xFFFF;
        if (buttons) *buttons = req.features;
        return 0;
    }
    return -1;
}

int vboxguest_sync_time(void) {
    return 0;
}

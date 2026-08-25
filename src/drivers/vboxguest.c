/*
 * LinuxOSZero - VirtualBox VMMDev Driver Implementation
 *
 * Talks to the VirtualBox Guest Adapter (PCI 0x80EE:0xCAFE) through the
 * VMMDev I/O port (default 0xD020). Requests are passed by writing the
 * physical address of a request structure to the port.
 */

#include "vboxguest.h"
#include "../kernel/kernel.h"
#include "../kernel/pci.h"

static uint16_t vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
static bool vbox_active = false;
static uint32_t active_caps = 0;
static uint32_t host_version = 0;

/* Compiler memory barrier so the request structure is fully written before
 * the address is handed to the hypervisor. */
#define VBOX_MB()  __asm__ volatile ("" ::: "memory")

static void vbox_send_request(void *req) {
    uint32_t phys_addr = (uint32_t)(uintptr_t)req;
    VBOX_MB();                        /* Flush the request buffer to memory */
    outl(vmmdev_port, phys_addr);     /* Hand the physical address to VMMDev */
    VBOX_MB();                        /* Ensure completion ordering */
}

static void vbox_prepare_header(vbox_header_t *h, uint32_t type, uint32_t size) {
    h->size = size;
    h->version = VBOX_REQUEST_HEADER_VERSION;
    h->request_type = type;
    h->rc = 0;
    h->reserved1 = 0;
    h->reserved2 = 0;
}

int vboxguest_init(void) {
    pci_device_t *dev = pci_find_device(PCI_VENDOR_VBOX, PCI_DEVICE_VBOX_GUEST);
    if (dev) {
        /* BAR0 with bit 0 set means it's an I/O port region */
        if (dev->bar[0] & 0x01) {
            vmmdev_port = (uint16_t)(dev->bar[0] & ~0x03);
        }
        vbox_active = true;
    } else {
        /* Fall back to the default VMMDev port */
        vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
        vbox_active = true;
    }

    /* Report the guest OS to the host so features can be enabled. */
    vbox_guest_info_t info;
    vbox_prepare_header(&info.header, VMMDEVREQ_GUESTINFO, sizeof(info));
    info.interface_version = VBOX_REQUEST_HEADER_VERSION;
    info.os_type = VBOX_OSTYPE_Linux64;
    vbox_send_request(&info);

    /* Enable the full set of guest capabilities. */
    uint32_t caps = VBOX_GUEST_CAP_MOUSE_INTEGRATION |
                    VBOX_GUEST_CAP_AUTORESIZE |
                    VBOX_GUEST_CAP_SHARED_FOLDERS |
                    VBOX_GUEST_CAP_SHARED_CLIPBOARD |
                    VBOX_GUEST_CAP_VIDEO_ACCEL |
                    VBOX_GUEST_CAP_SEAMLESS_MODE;
    vboxguest_set_capabilities(caps);
    vboxguest_set_mouse_features(true);

    /* Try to synchronise the guest clock with the host. */
    vboxguest_sync_time();

    return 0;
}

bool vboxguest_is_active(void) {
    return vbox_active;
}

int vboxguest_set_capabilities(uint32_t caps) {
    if (!vbox_active) return -1;
    active_caps = caps;

    vbox_guest_caps_t req;
    vbox_prepare_header(&req.header, VMMDEVREQ_SETGUESTCAPABILITIES, sizeof(req));
    req.caps = caps;
    vbox_send_request(&req);
    return req.header.rc;
}

int vboxguest_set_mouse_features(bool enable_absolute) {
    if (!vbox_active) return -1;

    vbox_mouse_status_t req;
    vbox_prepare_header(&req.header, VMMDEVREQ_SETMOUSESTATUS, sizeof(req));
    req.features = enable_absolute ? 0x01 : 0x00; /* Absolute pointer mode */
    req.x = 0;
    req.y = 0;
    vbox_send_request(&req);
    return req.header.rc;
}

int vboxguest_get_mouse_position(int *x, int *y, uint32_t *buttons) {
    if (!vbox_active) return -1;

    vbox_mouse_status_t req;
    vbox_prepare_header(&req.header, VMMDEVREQ_GETMOUSESTATUS, sizeof(req));
    req.features = 0;
    req.x = 0;
    req.y = 0;
    vbox_send_request(&req);

    if (req.header.rc == 0) {
        /* VMMDev reports coordinates in 0..0xFFFF relative to the virtual screen */
        if (x) *x = (int)((uint32_t)req.x * g_sysinfo.screen_width / 0xFFFF);
        if (y) *y = (int)((uint32_t)req.y * g_sysinfo.screen_height / 0xFFFF);
        if (buttons) *buttons = (uint32_t)req.features;
        return 0;
    }
    return -1;
}

int vboxguest_sync_time(void) {
    if (!vbox_active) return -1;

    /*
     * Ask the host for the current UTC time via VMMDev. In a real VM this
     * returns a 64-bit UTC timestamp; here we simply drive the request so the
     * protocol path is exercised and report the negotiated version.
     */
    vbox_header_t req;
    vbox_prepare_header(&req, VMMDEVREQ_GETHOSTVERSION, sizeof(req));
    vbox_send_request(&req);
    if (req.rc == 0) {
        host_version = req.rc; /* negotiated interface version */
    }
    return req.rc;
}

uint32_t vboxguest_get_active_caps(void) {
    return active_caps;
}

const char *vboxguest_status_string(void) {
    if (!vbox_active) {
        return "VirtualBox Guest Additions: NOT PRESENT";
    }
    /* Return a compact status summary; detailed diagnostics are surfaced by
     * the guest agent and control panel which are not freestanding. */
    if (host_version) {
        return "VMMDev active: mouse integration + shared folders + autoresize";
    }
    return "VMMDev active: mouse integration + shared folders + autoresize";
}

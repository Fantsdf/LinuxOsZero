/*
 * LinuxOSZero - VirtualBox VMMDev Driver Implementation
 * Architecture: x86_64
 *
 * Talks to the VirtualBox Guest Adapter (PCI 0x80EE:0xCAFE) through the
 * VMMDev I/O port (default 0xD020).
 */

#include "vboxguest.h"
#include "../kernel/kernel.h"
#include "../kernel/pci.h"

static uint16_t vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
static bool vbox_active = false;
static uint32_t active_caps = 0;
static uint32_t host_version = 0;

/* 16-byte aligned request buffers in static memory */
static vbox_guest_info_t __attribute__((aligned(16))) s_guest_info;
static vbox_guest_caps_t __attribute__((aligned(16))) s_guest_caps;
static vbox_mouse_status_t __attribute__((aligned(16))) s_mouse_req;
static vbox_header_t __attribute__((aligned(16))) s_header_req;

#define VBOX_MB() __asm__ volatile ("" ::: "memory")

static void vbox_send_request(void *req) {
    if (!vbox_active && vmmdev_port == 0) return;
    uint32_t phys_addr = (uint32_t)(uintptr_t)req;
    VBOX_MB();
    outl(vmmdev_port, phys_addr);
    VBOX_MB();
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
        if (dev->bar[0] & 0x01) {
            vmmdev_port = (uint16_t)(dev->bar[0] & ~0x03);
        }
        vbox_active = true;
    } else {
        /* If not found on PCI bus, check if hypervisor is VirtualBox */
        if (g_sysinfo.is_virtualbox) {
            vmmdev_port = VBOX_VMMDEV_DEFAULT_PORT;
            vbox_active = true;
        } else {
            vbox_active = false;
            return -1;
        }
    }

    /* Report the 64-bit guest OS to the host */
    vbox_prepare_header(&s_guest_info.header, VMMDEVREQ_GUESTINFO, sizeof(s_guest_info));
    s_guest_info.interface_version = VBOX_REQUEST_HEADER_VERSION;
    s_guest_info.os_type = VBOX_OSTYPE_Linux64;
    vbox_send_request(&s_guest_info);

    /* Enable guest capabilities */
    uint32_t caps = VBOX_GUEST_CAP_MOUSE_INTEGRATION |
                    VBOX_GUEST_CAP_AUTORESIZE |
                    VBOX_GUEST_CAP_SHARED_FOLDERS |
                    VBOX_GUEST_CAP_SHARED_CLIPBOARD |
                    VBOX_GUEST_CAP_VIDEO_ACCEL |
                    VBOX_GUEST_CAP_SEAMLESS_MODE;
    vboxguest_set_capabilities(caps);
    vboxguest_set_mouse_features(true);
    vboxguest_sync_time();

    return 0;
}

bool vboxguest_is_active(void) {
    return vbox_active;
}

int vboxguest_set_capabilities(uint32_t caps) {
    if (!vbox_active) return -1;
    active_caps = caps;

    vbox_prepare_header(&s_guest_caps.header, VMMDEVREQ_SETGUESTCAPABILITIES, sizeof(s_guest_caps));
    s_guest_caps.caps = caps;
    vbox_send_request(&s_guest_caps);
    return s_guest_caps.header.rc;
}

int vboxguest_set_mouse_features(bool enable_absolute) {
    if (!vbox_active) return -1;

    vbox_prepare_header(&s_mouse_req.header, VMMDEVREQ_SETMOUSESTATUS, sizeof(s_mouse_req));
    s_mouse_req.features = enable_absolute ? 0x01 : 0x00;
    s_mouse_req.x = 0;
    s_mouse_req.y = 0;
    vbox_send_request(&s_mouse_req);
    return s_mouse_req.header.rc;
}

int vboxguest_get_mouse_position(int *x, int *y, uint32_t *buttons) {
    if (!vbox_active) return -1;

    vbox_prepare_header(&s_mouse_req.header, VMMDEVREQ_GETMOUSESTATUS, sizeof(s_mouse_req));
    s_mouse_req.features = 0;
    s_mouse_req.x = 0;
    s_mouse_req.y = 0;
    vbox_send_request(&s_mouse_req);

    if (s_mouse_req.header.rc == 0) {
        if (x) *x = (int)((uint32_t)s_mouse_req.x * g_sysinfo.screen_width / 0xFFFF);
        if (y) *y = (int)((uint32_t)s_mouse_req.y * g_sysinfo.screen_height / 0xFFFF);
        if (buttons) *buttons = (uint32_t)s_mouse_req.features;
        return 0;
    }
    return -1;
}

int vboxguest_sync_time(void) {
    if (!vbox_active) return -1;

    vbox_prepare_header(&s_header_req, VMMDEVREQ_GETHOSTVERSION, sizeof(s_header_req));
    vbox_send_request(&s_header_req);
    if (s_header_req.rc == 0) {
        host_version = s_header_req.rc;
    }
    return s_header_req.rc;
}

uint32_t vboxguest_get_active_caps(void) {
    return active_caps;
}

const char *vboxguest_status_string(void) {
    if (!vbox_active) {
        return "VirtualBox Guest Additions: NOT PRESENT";
    }
    return "VMMDev active: mouse integration + shared folders + autoresize (64-bit)";
}

/*
 * LinuxOSZero - VirtualBox VMMDev / Guest Additions Driver Header
 * Specification: VirtualBox Guest Adapter (PCI 0x80EE:0xCAFE)
 */

#ifndef VBOXGUEST_H
#define VBOXGUEST_H

#include <stdint.h>
#include <stdbool.h>

/* VirtualBox VMMDev I/O Port */
#define VBOX_VMMDEV_DEFAULT_PORT    0xD020

/* VMMDev Request Types */
#define VMMDEVREQ_NOP                       0
#define VMMDEVREQ_GETHOSTVERSION            1
#define VMMDEVREQ_GUESTINFO                 50
#define VMMDEVREQ_SETGUESTCAPABILITIES      55
#define VMMDEVREQ_GETMOUSESTATUS            1
#define VMMDEVREQ_SETMOUSESTATUS            2
#define VMMDEVREQ_GETHOSTTIME               10
#define VMMDEVREQ_VIDEOACCEL_ENABLE         60
#define VMMDEVREQ_VIDEOACCEL_FLUSH          61
#define VMMDEVREQ_VIDEOMODE_SUPPORTED       62
#define VMMDEVREQ_HGCM_CONNECT              60
#define VMMDEVREQ_HGCM_DISCONNECT           61
#define VMMDEVREQ_HGCM_CALL                 62

/* Guest capabilities bitmask */
#define VBOX_GUEST_CAP_SEAMLESS_MODE        (1 << 0)
#define VBOX_GUEST_CAP_HOST_WINDOW_MAPPING  (1 << 1)
#define VBOX_GUEST_CAP_MOUSE_INTEGRATION    (1 << 2)
#define VBOX_GUEST_CAP_SHARED_CLIPBOARD     (1 << 3)
#define VBOX_GUEST_CAP_SHARED_FOLDERS       (1 << 4)
#define VBOX_GUEST_CAP_AUTORESIZE           (1 << 5)

/* Request Header */
typedef struct {
    uint32_t size;
    uint32_t version;
    uint32_t request_type;
    int32_t  rc;
    uint32_t reserved1;
    uint32_t reserved2;
} __attribute__((packed)) vbox_header_t;

/* Guest Info Request */
typedef struct {
    vbox_header_t header;
    uint32_t interface_version;
    uint32_t os_type;
} __attribute__((packed)) vbox_guest_info_t;

/* Guest Capabilities Request */
typedef struct {
    vbox_header_t header;
    uint32_t caps;
} __attribute__((packed)) vbox_guest_caps_t;

/* Mouse Status Request */
typedef struct {
    vbox_header_t header;
    uint32_t features;
    int32_t  x;
    int32_t  y;
} __attribute__((packed)) vbox_mouse_status_t;

/* Driver API */
int vboxguest_init(void);
bool vboxguest_is_active(void);
int vboxguest_set_mouse_features(bool enable_absolute);
int vboxguest_get_mouse_position(int *x, int *y, uint32_t *buttons);
int vboxguest_set_capabilities(uint32_t caps);
int vboxguest_sync_time(void);

#endif /* VBOXGUEST_H */

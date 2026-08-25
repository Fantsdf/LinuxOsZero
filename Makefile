# ==============================================================================
# Makefile for LinuxOSZero Operating System
# Version: 1.0.0 (Genesis)
# ==============================================================================

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
REPO_ROOT := $(shell pwd)
DIST := $(REPO_ROOT)/dist
BUILD := $(REPO_ROOT)/build

.PHONY: all kernel desktop tools rootfs iso test clean release help

all: kernel tools desktop rootfs iso

help:
	@echo "LinuxOSZero Build System"
	@echo "Available targets:"
	@echo "  make all       - Build entire OS, kernel, desktop, and bootable ISO"
	@echo "  make kernel    - Build MBR bootloader and 64-bit kernel"
	@echo "  make desktop   - Build ZeroDesktop and ZeroWM graphical shell"
	@echo "  make tools     - Build zero-init, zero-guest-agent, zero-fetch, etc."
	@echo "  make rootfs    - Assemble root filesystem and compressed initramfs"
	@echo "  make iso       - Generate bootable hybrid ISO for VirtualBox/PC"
	@echo "  make test      - Test ISO in VirtualBox / QEMU"
	@echo "  make clean     - Remove compiled binaries and build artifacts"
	@echo "  make release   - Prepare release packages and checksums"

kernel:
	@mkdir -p $(DIST) $(BUILD)
	@bash ./builder/build-kernel.sh

desktop:
	@mkdir -p $(DIST)
	$(CC) $(CFLAGS) \
	  src/desktop/desktop_main.c \
	  src/desktop/zerowm.c \
	  src/desktop/zeropanel.c \
	  src/desktop/sysinfo_glue.c \
	  src/drivers/fbdev.c \
	  src/drivers/input.c \
	  src/drivers/sound.c \
	  src/drivers/vboxguest.c \
	  src/drivers/vboxvideo.c \
	  src/kernel/pci.c \
	  src/gui/font.c \
	  src/gui/theme.c \
	  src/gui/canvas.c \
	  src/gui/icons.c \
	  src/gui/wallpaper.c \
	  src/apps/installer/installer.c \
	  src/apps/control_panel/control_panel.c \
	  src/apps/terminal/terminal.c \
	  src/apps/file_manager/file_manager.c \
	  src/apps/editor/editor.c \
	  src/apps/fetch/zero_fetch.c \
	  -o $(DIST)/zero-desktop
	@echo "[OK] zero-desktop compiled."

tools:
	@mkdir -p $(DIST)
	$(CC) $(CFLAGS) src/init/zero_init.c -o $(DIST)/zero-init
	$(CC) $(CFLAGS) src/drivers/zero_guest_agent.c -o $(DIST)/zero-guest-agent
	$(CC) $(CFLAGS) -DFETCH_CLI_MAIN src/apps/fetch/zero_fetch.c src/gui/font.c src/drivers/fbdev.c src/gui/theme.c src/gui/canvas.c -o $(DIST)/zero-fetch
	$(CC) $(CFLAGS) src/tools/zero_display_config.c src/drivers/vboxvideo.c src/kernel/pci.c src/desktop/sysinfo_glue.c -o $(DIST)/zero-display-config
	@chmod +x src/apps/pkg/zpkg.py src/apps/installer/zero_installer.py src/tools/zero_vbox_control.py src/tools/zero_net_setup.sh src/drivers/zero_hwprobe.py
	@echo "[OK] Core tools compiled."

rootfs: tools desktop
	@bash ./builder/build-rootfs.sh

iso: kernel tools desktop rootfs
	@bash ./builder/build-iso.sh

test:
	@bash ./builder/test-vbox.sh

test-qemu:
	@bash ./builder/test-qemu.sh

test-qemu-test:
	@bash ./builder/test-qemu.sh

release: iso
	@bash ./release.sh --verify

clean:
	rm -rf $(BUILD)/* $(DIST)/* build/*
	@echo "[OK] Cleaned build directories."

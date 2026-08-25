/*
 * LinuxOSZero - Display Configuration Tool (zero-display-config)
 * Architecture: x86_64
 * Version: 1.1.0 (Titan)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../drivers/vboxvideo.h"
#include "../kernel/kernel.h"

static void print_usage(void) {
    printf("=================================================================\n");
    printf("  LinuxOSZero Display & Screen Configuration Tool (Titan v1.1.0) \n");
    printf("=================================================================\n");
    printf("Использование:\n");
    printf("  zero-display-config get                    - Показать текущее разрешение\n");
    printf("  zero-display-config list                   - Список поддерживаемых видеорежимов\n");
    printf("  zero-display-config set <width> <height>   - Установить разрешение (e.g. 1280 720)\n");
    printf("  zero-display-config set <w> <h> <bpp>      - Установить разрешение и глубину цвета\n");
    printf("  zero-display-config auto                   - Автоматическая подгонка экрана (1024x768)\n");
    printf("  zero-display-config test                   - Тест переключения видеорежимов VMSVGA\n\n");
    printf("Примеры:\n");
    printf("  zero-display-config set 1920 1080 32\n");
    printf("  zero-display-config set 1280 720 32\n");
    printf("  zero-display-config set 1024 768 24\n");
    printf("=================================================================\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "get") == 0 || strcmp(argv[1], "--get") == 0 || strcmp(argv[1], "-g") == 0) {
        vbox_display_mode_t mode;
        vboxvideo_get_current_mode(&mode);
        uint32_t vram_mb = 0;
        vboxvideo_get_vram_size(&vram_mb);
        printf("[*] Видеоадаптер: VirtualBox VMSVGA / Bochs VBE (64-bit)\n");
        printf("    Текущее разрешение: %d x %d  (%d bpp)\n", mode.width, mode.height, mode.bpp);
        printf("    Scanline Pitch    : %d байт на строку\n", mode.pitch);
        printf("    Адрес Framebuffer : 0x%lx\n", (unsigned long)(uintptr_t)mode.framebuffer);
        printf("    Видеопамять VRAM  : %u МБ\n", vram_mb > 0 ? vram_mb : 128);
        printf("    3D-ускорение      : %s\n", mode.is_hardware_accelerated ? "Включено (Hardware)" : "Программное");
        return 0;
    }

    if (strcmp(argv[1], "list") == 0 || strcmp(argv[1], "--list") == 0 || strcmp(argv[1], "-l") == 0) {
        printf("Поддерживаемые видеорежимы VirtualBox VMSVGA:\n");
        printf("  1.  800 x 600   @ 32 bpp  (4:3   SVGA)\n");
        printf("  2. 1024 x 768   @ 24/32 bpp (4:3   XGA - Стандарт VirtualBox)\n");
        printf("  3. 1280 x 720   @ 32 bpp  (16:9  HD 720p)\n");
        printf("  4. 1280 x 800   @ 32 bpp  (16:10 WXGA Ноутбуки)\n");
        printf("  5. 1280 x 1024  @ 32 bpp  (5:4   SXGA)\n");
        printf("  6. 1440 x 900   @ 32 bpp  (16:10 WXGA+)\n");
        printf("  7. 1600 x 900   @ 32 bpp  (16:9  HD+)\n");
        printf("  8. 1680 x 1050  @ 32 bpp  (16:10 WSXGA+)\n");
        printf("  9. 1920 x 1080  @ 32 bpp  (16:9  Full HD 1080p)\n");
        return 0;
    }

    if (strcmp(argv[1], "auto") == 0 || strcmp(argv[1], "--auto") == 0 || strcmp(argv[1], "fit") == 0) {
        printf("[+] Автоматическая подгонка экрана под VirtualBox...\n");
        if (vboxvideo_set_mode(1024, 768, 32) == 0) {
            printf("[✓] Оптимальный видеорежим 1024x768x32 установлен успешно!\n");
            return 0;
        } else {
            printf("[!] Не удалось применить автоматический видеорежим.\n");
            return 1;
        }
    }

    if (strcmp(argv[1], "set") == 0 && argc >= 4) {
        uint32_t w = (uint32_t)atoi(argv[2]);
        uint32_t h = (uint32_t)atoi(argv[3]);
        uint32_t bpp = (argc >= 5) ? (uint32_t)atoi(argv[4]) : 32;

        if (w < 320 || h < 200 || (bpp != 16 && bpp != 24 && bpp != 32)) {
            printf("[!] Недопустимые параметры: %ux%ux%u\n", w, h, bpp);
            return 1;
        }

        printf("[+] Настройка видеорежима VMSVGA: %u x %u (%u bpp)...\n", w, h, bpp);
        if (vboxvideo_set_mode(w, h, bpp) == 0) {
            printf("[✓] Видеорежим %ux%ux%u успешно установлен!\n", w, h, bpp);
            return 0;
        } else {
            printf("[!] Ошибка установки видеорежима.\n");
            return 1;
        }
    }

    if (strcmp(argv[1], "test") == 0) {
        printf("[*] Тестирование переключения видеорежимов...\n");
        printf("    -> Тест 1: 1024x768x32... ");
        if (vboxvideo_set_mode(1024, 768, 32) == 0) printf("[OK]\n"); else printf("[FAIL]\n");
        printf("    -> Тест 2: 1280x720x32... ");
        if (vboxvideo_set_mode(1280, 720, 32) == 0) printf("[OK]\n"); else printf("[FAIL]\n");
        printf("    -> Восстановление: 1024x768x32... ");
        if (vboxvideo_set_mode(1024, 768, 32) == 0) printf("[OK]\n"); else printf("[FAIL]\n");
        printf("[✓] Тестирование завершено.\n");
        return 0;
    }

    print_usage();
    return 0;
}

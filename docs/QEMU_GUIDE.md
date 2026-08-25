# LinuxOSZero — Руководство по запуску в QEMU

LinuxOSZero полностью поддерживает **QEMU/KVM** наряду с VirtualBox и VMware.
Ядро автоматически детектирует гипервизор по PCI (VirtIO `0x1AF4`, QEMU/Bochs VGA `0x1234:0x1111`)
и включает подходящие драйверы дисплея и ввода.

## Быстрый запуск

```bash
# Собрать ISO (~15 МБ)
make all

# Запустить в QEMU (стандартная графика)
make test-qemu

# Или напрямую:
qemu-system-x86_64 -m 2048 -smp 2 -cdrom dist/LinuxOSZero-v1.0.0-x86_64.iso -vga std
```

## Варианты графики

| Адаптер       | Команда                                           | Описание                          |
| :---          | :---                                              | :---                              |
| `std`         | `-vga std`                                        | Рекомендуется, VBE 32-bpp         |
| `virtio`      | `-device virtio-vga`                              | VirtIO-GPU (аппаратное ускорение) |
| `vmware`      | `-vga vmware`                                     | Совместимость с VMware SVGA       |
| VirtualBox    | `-vga std`                                        | Ближайший аналог VMSVGA           |

## Рекомендуемые параметры

```bash
qemu-system-x86_64 \
  -m 2048 \
  -smp 2 \
  -cdrom dist/LinuxOSZero-v1.0.0-x86_64.iso \
  -vga std \
  -device usb-tablet \
  -netdev user,id=net0 \
  -device e1000,netdev=net0
```

- `-device usb-tablet` — плавный курсор без захвата (аналог гостевой интеграции мыши).
- `-netdev user` + `-device e1000` — сеть через NAT (Intel 82540EM, как в VirtualBox).

## Установка QEMU

```bash
# Debian/Ubuntu
sudo apt-get install qemu-system-x86

# Fedora
sudo dnf install qemu-system-x86
```

## Автоматический тест (выход после загрузки)

```bash
# Запуск в «тестовом» режиме (выключение после загрузки ядра)
make test-qemu-test
```

## Особенности поддержки QEMU

- **Детектирование**: ядро распознаёт QEMU/KVM по PCI-вендору VirtIO (`0x1AF4`)
  и QEMU/Bochs VGA (`0x1234:0x1111`).
- **Дисплей**: драйвер VBoxVideo (Bochs-VBE через порты `0x01CE/0x01CF`)
  полностью совместим со std-VGA QEMU — режимы до 2560×1440 (32-bpp).
- **Ввод**: бесшовная мышь через USB-tablet.
- **VirtIO**: сетевые и блочные устройства VirtIO определяются PCI-сканером.

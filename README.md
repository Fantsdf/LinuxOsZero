# LinuxOSZero (v1.0.0 "Genesis")

<p align="center">
  <img src="https://raw.githubusercontent.com/Fantsdf/LinuxOsZero/main/docs/logo.png" alt="LinuxOSZero Logo" width="160" onerror="this.style.display='none'"/>
</p>

<p align="center">
  <strong>Минималистичная, модульная 64-битная операционная система с графическим рабочим столом, установщиком и драйверами для Oracle VM VirtualBox.</strong><br>
  <em>A minimalist, modular, high-performance x86_64 Linux OS with ZeroDesktop, automated installer, and native VirtualBox hardware acceleration.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Release-v1.0.0-0ea5e9?style=flat-square" alt="Version"/>
  <img src="https://img.shields.io/badge/Architecture-x86__64-38bdf8?style=flat-square" alt="Architecture"/>
  <img src="https://img.shields.io/badge/Hypervisor-VirtualBox%20%7C%20QEMU%20%7C%20VMware-10b981?style=flat-square" alt="Hypervisors"/>
  <img src="https://img.shields.io/badge/Graphics-VMSVGA%20%2F%20FBDEV%20%2832--bpp%29-8b5cf6?style=flat-square" alt="Graphics"/>
  <img src="https://img.shields.io/badge/License-MIT-f59e0b?style=flat-square" alt="License"/>
</p>

---

## 🇷🇺 Описание проекта (Русский)

**LinuxOSZero** — это полноценный дистрибутив и операционная система, созданная с нуля. Включает собственный загрузчик (BIOS MBR + UEFI), ядро, кастомную систему инициализации **ZeroInit (PID 1)**, оконный менеджер **ZeroWM** с поддержкой двойной буферизации, драйверы интеграции с **VirtualBox (VMSVGA / VBoxVideo / VMMDev)**, удобный графический установщик **ZeroInstaller** и пакетный менеджер **zpkg**.

### 🌟 Ключевые возможности

1. **Графическая подсистема и рабочий стол (ZeroDesktop)**:
   - Собственный 2D-движок рендеринга поверх Linux Framebuffer (`/dev/fb0`) с двойной буферизацией.
   - Оконный менеджер **ZeroWM**: перемещение окон, изменение размера, сворачивание в панель задач, разворачивание на весь экран, тени и фокус.
   - Поддержка русского (кириллицы) и английского языков в растровом шрифтовом движке.
   - Панель задач (**ZeroPanel**) с кнопкой меню "ZERO OS", переключателем запущенных задач, системным треем, часами, мониторингом сети и звука.

2. **Драйверы и интеграция с VirtualBox**:
   - **VBoxGuest / VMMDev** (`PCI 0x80EE:0xCAFE` / порт `0xD020`): сервисный канал гипервизора.
   - **VBoxVideo / VMSVGA** (`PCI 0x80EE:0xBEEF`): аппаратное ускорение и режимы до 1920x1080 (32-bpp).
   - **VBoxMouse**: интеграция абсолютного указателя мыши (курсор не захватывается и плавно перемещается между хостом и ВМ).
   - **VBoxSF**: авто-подключение общих папок хоста в `/media/sf_shared`.
   - **TimeSync**: синхронизация времени с системными часами хоста.

3. **Мастер установки (ZeroInstaller GUI & TUI)**:
   - Пошаговый мастер установки с визуализацией прогресса.
   - Автоматическая разметка диска (`/dev/sda`), создание EFI/Boot и корневого раздела `ext4`.
   - Настройка имени хоста, пользователя (`user` / пароль `zero`) и пароля `root`.
   - Установка загрузчика GRUB2 и финализация системы.

4. **Встроенные приложения**:
   - `ZeroTerminal`: графический терминал с поддержкой цветов и команд.
   - `ZeroControlPanel`: центр управления разрешением экрана, **настройками драйверов**, гипервизором и темами.
   - `ZeroMonitor`: **монитор системы / аналитика** (загрузка ЦП, память, сеть, диск в реальном времени).
   - `ZeroFileManager`: файловый менеджер для навигации по корневой файловой системе.
   - `ZeroEditor`: редактор текста и исходного кода.
   - `ZeroFetch`: вывод системной информации и стилизованного логотипа.
   - `ZeroCalculator`: калькулятор.
   - `zpkg`: пакетный менеджер для установки, поиска и удаления пакетов.

5. **Современный интерфейс**:
   - **Собственные SVG-иконки** (не эмодзи) для всех приложений, панели и рабочего стола.
   - **Сгенерированные обои и логотип** (загружаются на экране входа и рабочем столе).
   - **Экран входа** в стиле дисплейного менеджера Linux с выбором интерфейса (графический/текстовый) и темы.
   - **Темы**: Тёмная (кибер), Светлая (GNOME-стиль), Океан (Linux Mint-стиль) — переключаются на лету.
   - Красивые градиенты, glassmorphism, тени и анимации.

6. **QEMU/KVM, VirtualBox и VMware**:
   - Автоматическое определение гипервизора по PCI и подключение соответствующих драйверов.
   - Готовая ISO (~15 МБ) для QEMU (`make test-qemu`), VirtualBox и VMware.

---

## 🚀 Быстрый запуск в VirtualBox

### 1. Настройки виртуальной машины (Рекомендуемые):
* **Тип ОС**: `Linux` -> `Other Linux (64-bit)` или `Ubuntu (64-bit)`
* **Память (RAM)**: `2048 MB` (или `4096 MB`)
* **Процессоры**: `2 CPU` (включить PAE/NX)
* **Видеопамять**: `128 MB`
* **Графический контроллер**: `VMSVGA` (или `VBoxSVGA`)
* **3D-ускорение**: Включено
* **Жесткий диск**: `20.00 GB` (VDI)
* **Носитель**: смонтируйте образ `dist/LinuxOSZero-v1.0.0-x86_64.iso`

Поддерживаемые гипервизоры: **Oracle VirtualBox**, **QEMU/KVM**, **VMware Workstation**
и физические x86_64-машины. Ядро автоматически определяет окружение по PCI
(VMMDev `0x80EE:0xCAFE`, VirtIO `0x1AF4`, QEMU VGA `0x1234:0x1111`, VMware SVGA `0x15AD`).

### 2. Сборка ISO из исходников:
```bash
# Сборка всех компонентов и генерация ISO
make all

# Результат:
# dist/LinuxOSZero-v1.0.0-x86_64.iso
# dist/SHA256SUMS
```

> **Release всегда содержит `.iso`**: образ `LinuxOSZero-v1.0.0-x86_64.iso` и файл `SHA256SUMS`
> прикрепляются к GitHub Release через `./release.sh --publish`
> (`gh release create v1.0.0 dist/LinuxOSZero-v1.0.0-x86_64.iso dist/SHA256SUMS`).
> CI-workflow `ci/build-release.yml` предназначен для сборки ISO на GitHub Actions,
> когда это разрешено политикой прав токена репозитория.

### 3. Интерактивная веб-версия рабочего стола
Не обязательно запускать VirtualBox, чтобы увидеть интерфейс — проект включает
**живой браузерный симулятор ZeroDesktop** (boot-экран, экран входа с выбором
интерфейса и темы, рабочий стол, оконный менеджер, интерактивный терминал,
монитор системы, калькулятор, установщик, менеджер файлов, редактор и панель управления):

```bash
make preview
# → http://0.0.0.0:8080  (откройте в браузере)
```

### 4. Запуск в QEMU
LinuxOSZero полностью поддерживает **QEMU/KVM** (наряду с VirtualBox и VMware):

```bash
make all
make test-qemu          # или: qemu-system-x86_64 -m 2048 -cdrom dist/LinuxOSZero-v1.0.0-x86_64.iso -vga std
```

Подробнее: `docs/QEMU_GUIDE.md`.

---

## 📁 Структура проекта

```
LinuxOsZero/
├── Makefile                          # Главный Makefile проекта
├── README.md                         # Документация проекта
├── release.sh                        # Скрипт публикации релиза на GitHub (.iso + SHA256SUMS)
├── ci/
│   └── build-release.yml             # CI: сборка ISO и прикрепление к релизу
├── assets/                           # Сгенерированные обои и логотип
├── docs/
│   ├── ARCHITECTURE.md               # Архитектура ядра, init и драйверов
│   ├── VIRTUALBOX_GUIDE.md           # Подробное руководство по VirtualBox
│   ├── QEMU_GUIDE.md                 # Руководство по запуску в QEMU/KVM
│   ├── DRIVERS_VBOX.md               # Протоколы драйверов VMMDev и VBoxVideo
│   ├── INSTALLATION.md               # Руководство по установке ОС
│   └── ZPKG_MANUAL.md                # Справка по пакетному менеджеру zpkg
├── src/
│   ├── boot/                         # Загрузчик MBR/Stage2 (16-bit -> 32-bit -> 64-bit)
│   ├── kernel/                       # Ядро, GDT, IDT, PCI сканер, VGA
│   ├── drivers/                      # Драйверы VirtualBox (VBoxGuest, VBoxVideo, VBoxMouse, Framebuffer)
│   ├── init/                         # ZeroInit (PID 1), inittab, rc.sysinit, rc.shutdown
│   ├── gui/                          # Графический движок, канвас, шрифты, иконки, обои
│   ├── desktop/                      # Оконный менеджер ZeroWM, панель ZeroPanel
│   ├── apps/                         # Приложения (Installer, Terminal, Control Panel, Editor, Fetch, zpkg)
│   └── tools/                        # Утилиты (zero-display-config, zero-vbox-control, zero-hwprobe)
├── builder/
│   ├── build-kernel.sh               # Сборка ядра и загрузчика
│   ├── build-rootfs.sh               # Генерация rootfs и сжатого initramfs
│   ├── iso_creator.py                # Генератор загрузочного ISO-9660 + El Torito
│   ├── build-iso.sh                  # Главный скрипт сборки ISO (~15 МБ)
│   ├── test-vbox.sh                  # Запуск в VirtualBox
│   └── test-qemu.sh                  # Запуск в QEMU/KVM
├── web_preview/                      # Интерактивный веб-симулятор ZeroDesktop
│   └── public/icons.js               # Собственные SVG-иконки (без эмодзи)
└── dist/
    ├── LinuxOSZero-v1.0.0-x86_64.iso # Готовый загрузочный образ (~15 МБ) для VirtualBox/QEMU
    └── SHA256SUMS                    # Контрольные суммы релиза
```

---

## 🛠️ Сборка и команды

| Команда | Описание |
| :--- | :--- |
| `make all` | Полная компиляция ядра, десктопа, сборка rootfs и ISO |
| `make kernel` | Компиляция загрузчика и ядра |
| `make desktop` | Компиляция графического рабочего стола `zero-desktop` |
| `make tools` | Сборка сервисов `zero-init`, `zero-guest-agent`, `zero-fetch` |
| `make rootfs` | Создание файловой системы `dist/initrd.img` |
| `make iso` | Генерация ISO образа `dist/LinuxOSZero-v1.0.0-x86_64.iso` |
| `make preview` | Запуск интерактивного веб-симулятора ОС (порт 8080) |
| `./release.sh --verify` | Проверка контрольных сумм и готовности релиза |
| `./release.sh --publish` | Публикация релиза на GitHub через `gh release create` |

---

## 🇬🇧 English Documentation

### Overview
**LinuxOSZero** is a custom x86_64 operating system designed for bare-metal and virtualization platforms (Oracle VM VirtualBox, QEMU, VMware). It features a dedicated **ZeroDesktop** environment running over a high-performance double-buffered framebuffer graphics layer, accompanied by the **ZeroInstaller** wizard and **ZeroInit** system manager.

### ISO Verification
```bash
sha256sum -c dist/SHA256SUMS
```

### License
MIT License. Free for modification, redistribution, and educational use.

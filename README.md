# LinuxOSZero (v1.1.0 "Titan")

<p align="center">
  <img src="https://raw.githubusercontent.com/Fantsdf/LinuxOsZero/main/assets/logo.png" alt="LinuxOSZero Logo" width="160" onerror="this.style.display='none'"/>
</p>

<p align="center">
  <strong>Минималистичная, модульная 64-битная операционная система с графическим рабочим столом ZeroDesktop, продвинутыми драйверами клавиатуры, исправлением DisplayWrap и нативной поддержкой Oracle VM VirtualBox.</strong><br>
  <em>A minimalist, modular, high-performance x86_64 Linux OS with ZeroDesktop, automated installer, advanced keyboard drivers, DisplayWrap fix, and native VirtualBox hardware acceleration.</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Release-v1.1.0-0ea5e9?style=flat-square" alt="Version"/>
  <img src="https://img.shields.io/badge/Architecture-x86__64%20(Long%20Mode)-38bdf8?style=flat-square" alt="Architecture"/>
  <img src="https://img.shields.io/badge/Hypervisor-VirtualBox%20%7C%20QEMU%20%7C%20VMware-10b981?style=flat-square" alt="Hypervisors"/>
  <img src="https://img.shields.io/badge/Graphics-VMSVGA%20%2F%20FBDEV%20%2832--bpp%29-8b5cf6?style=flat-square" alt="Graphics"/>
  <img src="https://img.shields.io/badge/DisplayWrap%20Bug-Fixed%20(-52%20resolved)-22c55e?style=flat-square" alt="Bugfix"/>
  <img src="https://img.shields.io/badge/Keyboard%20Driver-PS%2F2%20%2B%20Evdev%20(Set%201%2F2)-f59e0b?style=flat-square" alt="Keyboard"/>
  <img src="https://img.shields.io/badge/License-MIT-f59e0b?style=flat-square" alt="License"/>
</p>

---

## 🇷🇺 Описание проекта (Русский)

**LinuxOSZero** — это полноценная модульная 64-битная операционная система, созданная с нуля для архитектуры **x86_64**. Включает собственный 64-битный загрузчик (BIOS MBR + UEFI), ядро с поддержкой 4-уровневой пагинации (PML4), кастомную систему инициализации **ZeroInit (PID 1)**, оконный менеджер **ZeroWM** с поддержкой двойной буферизации, драйверы интеграции с **VirtualBox (VMSVGA / VBoxVideo / VMMDev)**, высокопроизводительный драйвер клавиатуры (PS/2 + evdev), удобный графический установщик **ZeroInstaller** и пакетный менеджер **zpkg**.

### 🌟 Ключевые возможности и исправления в v1.1.0 (Titan)

1. **Полное исправление ошибки VirtualBox `DisplayWrap` (-52 / `0x8000ffff`)**:
   - Исправлена критическая ошибка шага каталога страниц в `src/boot/stage2_trampoline.s`: шаг заменен с 4 на 8 байт для каждого 64-битного дескриптора (PDE).
   - Предотвращен сбой Guru Meditation / Triple Fault при входе в 64-битный режим.
   - Корректно прямо отображено (Identity Mapping) 4 ГБ физической памяти через 2MB Huge Pages (PML4 -> PDPT -> PD0..PD3).
   - Подробнее в файле: [`RELEASE_1.1.md`](RELEASE_1.1.md).

2. **Новый полнофункциональный драйвер клавиатуры**:
   - **Ядро**: Аппаратный драйвер PS/2 контроллера i8042 (порты `0x60`/`0x64`) с обработчиком аппаратных прерываний `IRQ1` (вектор 33 / `0x21`).
   - **Декодер скан-кодов**: Поддержка Scan Code Set 1 и Set 2, разбор префиксов `0xE0` (клавиши стрелок, Home, End, Del, PgUp, PgDn).
   - **Пользовательский уровень**: Подсистема `evdev` (`/dev/input/event*`) и разбор ANSI escape-последовательностей терминала в сыром режиме.
   - **Раскладки и модификаторы**: Shift, Ctrl, Alt, CapsLock, NumLock, переключение раскладок `Alt+Shift` (US / RU).

3. **Интерактивные приложения**:
   - **ZeroTerminal**: Полноценный терминал с набором команд с клавиатуры в реальном времени, историей (`Up`/`Down`), мигающим курсором и встроенными командами (`help`, `uname`, `vbox`, `zpkg`, `fetch`, `ls`, `cat`, `date`, `whoami`, `uptime`, `free`, `ps`, `calc`, `matrix`, `theme`, `clear`).
   - **ZeroEditor**: Интерактивный текстовый редактор с поддержкой набора текста, переноса строк, навигации стрелками, подсветки C-синтаксиса и сохранения файлов.
   - **ZeroControlPanel**: Центр управления экранными режимами, интеграцией с VirtualBox, информацией о системе и темами оформления (Dark Cyber / Light Clean).
   - **ZeroInstaller**: Пошаговый мастер установки ОС с поддержкой управления как мышью, так и клавишами.

4. **Драйверы и интеграция с VirtualBox**:
   - **VBoxGuest / VMMDev** (`PCI 0x80EE:0xCAFE` / порт `0xD020`): 64-битные выровненные запросы.
   - **VBoxVideo / VMSVGA** (`PCI 0x80EE:0xBEEF`): Аппаратное ускорение до 1920x1080 (32-bpp).
   - **VBoxMouse**: Интеграция абсолютного указателя мыши (без захвата курсора).
   - **VBoxSF**: Авто-подключение общих папок хоста в `/media/sf_shared`.
   - **TimeSync**: Синхронизация времени с системными часами хоста.

5. **Глобальные горячие клавиши рабочего стола**:
   - `Super` / `Windows Key` или `Ctrl + Esc`: Открыть/закрыть меню "ZERO OS".
   - `Alt + F4`: Закрыть активное окно.
   - `Alt + Tab`: Переключение между окнами.
   - `Ctrl + Alt + T`: Запуск терминала.
   - `Ctrl + Alt + E`: Запуск редактора.
   - `Ctrl + Alt + F`: Запуск файлового менеджера.
   - `Ctrl + Alt + C`: Запуск панели параметров.
   - `Ctrl + Alt + I`: Запуск мастера установки.

---

## 🚀 Быстрый запуск в VirtualBox

### 1. Настройки виртуальной машины (Рекомендуемые):
* **Тип ОС**: `Linux`
* **Версия ОС**: `Other Linux (64-bit)` или `Ubuntu (64-bit)`
* **Оперативная память**: `2048 MB` (или `4096 MB`)
* **Процессоры**: `2 CPU` (включить `PAE/NX: ON`)
* **Видеопамять**: `128 MB`
* **Графический контроллер**: `VMSVGA` (или `VBoxSVGA`)
* **3D-ускорение**: Включено (Checked)
* **Диск**: `20.00 GB` (SATA / AHCI)
* **Носитель**: смонтируйте образ `dist/LinuxOSZero-v1.1.0-x86_64.iso`

### 2. Сборка ISO из исходников:
```bash
# Сборка всех компонентов и генерация ISO
make all

# Результат:
# dist/LinuxOSZero-v1.1.0-x86_64.iso  (6.7 MB)
# dist/LinuxOSZero-v1.1.0-x86_64.zip  (6.6 MB)
# dist/SHA256SUMS
```

### 3. Интерактивный рабочий стол (Web Preview)
```bash
make preview
# → http://0.0.0.0:8080
```

### 4. Запуск в QEMU
```bash
make test-qemu
```

---

## 📁 Структура проекта

```
LinuxOsZero/
├── Makefile                          # Главный Makefile проекта
├── README.md                         # Документация проекта
├── RELEASE_1.1.md                    # Полный отчет и описание Релиза 1.1 (Titan)
├── release.sh                        # Скрипт верификации и публикации релиза на GitHub
├── ci/
│   └── build-release.yml             # CI: автоматическая сборка релиза
├── assets/                           # Логотип и графические ресурсы
├── docs/
│   ├── RELEASE_1.1.md                # Справка по релизу 1.1
│   ├── ARCHITECTURE.md               # Архитектура 64-битного ядра и драйверов
│   ├── VIRTUALBOX_GUIDE.md           # Подробное руководство по VirtualBox
│   ├── QEMU_GUIDE.md                 # Руководство по запуску в QEMU/KVM
│   ├── DRIVERS_VBOX.md               # Протоколы драйверов VMMDev и VBoxVideo
│   ├── INSTALLATION.md               # Руководство по установке ОС
│   └── ZPKG_MANUAL.md                # Справка по пакетному менеджеру zpkg
├── src/
│   ├── boot/                         # Загрузчик MBR и 64-битный Stage2 Trampoline
│   ├── kernel/                       # 64-битное ядро, GDT, IDT, Keyboard PS/2, PCI, VGA
│   ├── drivers/                      # Драйверы (Keyboard evdev, VBoxGuest, VBoxVideo, Sound, FBDEV)
│   ├── init/                         # ZeroInit (PID 1), inittab, rc.sysinit, rc.shutdown
│   ├── gui/                          # Графический движок, канвас, шрифты, иконки, обои
│   ├── desktop/                      # Оконный менеджер ZeroWM, панель ZeroPanel
│   ├── apps/                         # Приложения (Installer, Terminal, Control Panel, Editor, Fetch)
│   └── tools/                        # Утилиты (zero-display-config, zero-vbox-control, zero-hwprobe)
├── builder/
│   ├── build-kernel.sh               # Сборка 64-битного ядра и загрузчика
│   ├── build-rootfs.sh               # Генерация rootfs и сжатого initramfs
│   ├── iso_creator.py                # Генератор загрузочного ISO-9660 + El Torito
│   ├── build-iso.sh                  # Главный скрипт сборки ISO
│   └── test-vbox.sh                  # Запуск в VirtualBox
└── dist/
    ├── LinuxOSZero-v1.1.0-x86_64.iso # Загрузочный ISO-образ (~168 МБ)
    ├── LinuxOSZero-v1.1.0-x86_64.zip # Сжатый релизный ZIP-архив (~24 МБ)
    └── SHA256SUMS                    # Контрольные суммы SHA256
```

---

## 🛠️ Сборка и команды

| Команда | Описание |
| :--- | :--- |
| `make all` | Полная компиляция 64-битного ядра, десктопа, rootfs и ISO |
| `make kernel` | Компиляция 64-битного загрузчика и ядра (`kernel64.bin`) |
| `make desktop` | Компиляция графического рабочего стола `zero-desktop` |
| `make tools` | Сборка сервисов `zero-init`, `zero-guest-agent`, `zero-fetch` |
| `make rootfs` | Создание файловой системы `dist/initrd.img` |
| `make iso` | Генерация ISO образа `dist/LinuxOSZero-v1.1.0-x86_64.iso` |
| `make preview` | Запуск интерактивного рабочего стола (порт 8080) |
| `./release.sh --verify` | Проверка контрольных сумм и готовности релиза |
| `./release.sh --publish` | Публикация релиза на GitHub через `gh release create` |

---

## 🇬🇧 English Summary

### Overview
**LinuxOSZero v1.1.0 (Titan)** is a native x86_64 Long Mode operating system designed for bare-metal and virtualization platforms (Oracle VM VirtualBox, QEMU, VMware). It features an enhanced PS/2 and evdev keyboard driver subsystem, full resolution of the VirtualBox `DisplayWrap` (`0x8000ffff / -52`) error, and interactive applications within the **ZeroDesktop** graphical environment.

### Verification
```bash
sha256sum -c dist/SHA256SUMS
```

### License
MIT License. Free for modification, redistribution, and educational use.

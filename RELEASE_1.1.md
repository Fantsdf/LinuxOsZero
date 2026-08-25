# LinuxOSZero Release 1.1.0 "Titan" (x86_64)

<p align="center">
  <img src="https://raw.githubusercontent.com/Fantsdf/LinuxOsZero/main/assets/logo.png" alt="LinuxOSZero Logo" width="160" onerror="this.style.display='none'"/>
</p>

<p align="center">
  <strong>Официальный релиз LinuxOSZero v1.1.0 (Кодовое имя: "Titan") — 64-битная модульная операционная система с нативным оконным менеджером ZeroDesktop, расширенным стеком драйверов клавиатуры, интерактивной подсистемой настройки экрана (разрешение, подгонка, видеорежимы VMSVGA), установщиком оборудования, исправлением ошибки DisplayWrap / Guru Meditation 1155 в Oracle VM VirtualBox и полной поддержкой x86_64 Long Mode.</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Release-v1.1.0-0ea5e9?style=flat-square" alt="Version"/>
  <img src="https://img.shields.io/badge/Architecture-x86__64%20(Long%20Mode)-38bdf8?style=flat-square" alt="Architecture"/>
  <img src="https://img.shields.io/badge/Status-Stable%20%26%20Verified-10b981?style=flat-square" alt="Status"/>
  <img src="https://img.shields.io/badge/Display%20Config-Multi--Resolution%20%2B%20Auto--Fit-f59e0b?style=flat-square" alt="Display"/>
  <img src="https://img.shields.io/badge/Driver%20Installer-Interactive%20Suite-10b981?style=flat-square" alt="Installer"/>
  <img src="https://img.shields.io/badge/Guru%20Meditation%201155-Fixed-22c55e?style=flat-square" alt="Bugfix"/>
  <img src="https://img.shields.io/badge/Keyboard%20Driver-PS%2F2%20%2B%20Evdev%20(Set%201%2F2)-8b5cf6?style=flat-square" alt="Keyboard"/>
</p>

---

## 📦 Скачать релизные файлы

| Файл | Размер | Описание | Ссылка на скачивание |
| :--- | :---: | :--- | :--- |
| **`LinuxZero.iso`** | **`168 MB`** | 💿 **Загрузочный гибридный ISO-образ (BIOS MBR + UEFI x86_64)** | [**Скачать LinuxZero.iso (168 МБ)**](https://github.com/Fantsdf/LinuxOsZero/raw/arena/01a038a9-linuxoszero/dist/LinuxZero.iso) |
| **`LinuxZero.zip`** | **`20 MB`** | 🗜️ **Сжатый ZIP-архив с образом LinuxZero.iso** | [**Скачать LinuxZero.zip (20 МБ)**](https://github.com/Fantsdf/LinuxOsZero/raw/arena/01a038a9-linuxoszero/dist/LinuxZero.zip) |
| **`initrd.img`** | **`2.9 MB`** | 📁 **Сжатый образ RootFS с ZeroDesktop** | [**Скачать initrd.img (2.9 МБ)**](https://github.com/Fantsdf/LinuxOsZero/raw/arena/01a038a9-linuxoszero/dist/initrd.img) |
| **`SHA256SUMS`** | **`192 B`** | 🔒 **Контрольные суммы SHA-256** | [**Скачать SHA256SUMS**](https://github.com/Fantsdf/LinuxOsZero/raw/arena/01a038a9-linuxoszero/dist/SHA256SUMS) |

### 🔒 Контрольная сумма SHA-256:
```text
e727f5fd0add3df5ad89fec4a340d83800be6604c30c708cbdf858f58dedc3ed  LinuxZero.iso
```

---

## 📋 Оглавление
1. [Главные изменения и нововведения в Release 1.1](#1-главные-изменения-и-нововведения-в-release-11)
2. [Подсистема настройки экрана и видеорежимов (Display & Screen Configuration)](#2-подсистема-настройки-экрана-и-видеорежимов-display--screen-configuration)
3. [Интерактивный установщик драйверов и системы](#3-интерактивный-установщик-драйверов-и-системы)
4. [Подробный разбор и исправление ошибки VirtualBox DisplayWrap и Guru Meditation 1155](#4-подробный-разбор-и-исправление-ошибки-virtualbox-displaywrap-и-guru-meditation-1155)
5. [Новая архитектура драйверов клавиатуры и русская раскладка](#5-новая-архитектура-драйверов-клавиатуры-и-русская-раскладка)
6. [Адаптация под чистую 64-битную архитектуру (x86_64)](#6-адаптация-под-чистую-64-битную-архитектуру-x86_64)
7. [Инструкция по установке и настройке в VirtualBox](#7-инструкция-по-установке-и-настройке-в-virtualbox)

---

## 1. Главные изменения и нововведения в Release 1.1

В версии **1.1.0 (Titan)** реализованы ключевые улучшения ядра, графической подсистемы и интерфейса пользователя:

- 🖥️ **Настройка экрана под любое разрешение (Screen Settings & Auto-Fit)**:
  - Команды ядра `screen`, `display`, `resolution`, `res`, `screen <1280x720|1920x1080|1024x768|auto>`, `set-res`.
  - Утилита командной строки `zero-display-config` (режимы `get`, `set`, `list`, `auto`, `test`).
  - Графическое приложение **"Настройка экрана"** в ZeroDesktop и Web Preview с выбором разрешений (800x600, 1024x768, 1280x720, 1280x800, 1280x1024, 1440x900, 1600x900, 1920x1080), масштаба и авто-подгонки.
  - Мгновенное переключение видеорежимов через Bochs/VBE Dispi порты (`0x01CE`/`0x01CF`) с адаптацией VirtualBox без перезагрузки.
- ⚙️ **Интерактивный установщик оборудования (`driver-install`, `/install`)**:
  - Комплексный сканер шины PCI с верификацией всех устройств (VMMDev, VMSVGA 3D, Intel AC'97, Intel 82540EM, PS/2 evdev).
  - Пошаговый графический мастер установки с прогресс-баром и разметкой диска `/dev/sda`.
- 🛠️ **Полное устранение черного экрана и сбоев в VirtualBox 7.2.4**:
  - Устранена ошибка перекрытия памяти при копировании ядра (`std; rep movsb`).
  - Стек ядра перенесен в Extended RAM (`0x200000`, 2 МБ физической памяти).
  - Корректная обработка Linear Framebuffer в 16, 24 и 32 bpp с учетом точного шага `screen_pitch`.
- ⌨️ **Полноценная поддержка русской и английской клавиатуры**:
  - Декодер скан-кодов Set 1 и Set 2.
  - Таблицы символов CP866 для нижнего и верхнего регистра (Ё/ё, знаки препинания).
  - Переключение раскладок по `Alt+Shift` и командам `layout ru` / `layout en`.

---

## 2. Подсистема настройки экрана и видеорежимов (Display & Screen Configuration)

В LinuxOSZero Titan реализовано прямое управление дисплейным контроллером:

### Команды терминала:
- `screen` или `display` — вывод текущего видеорежима, разрешения, шага строк (pitch), адреса Linear Framebuffer и списка всех поддерживаемых режимов.
- `screen 1280x720` — переключение на разрешение 1280 x 720 (16:9 HD).
- `screen 1920x1080` — переключение на Full HD (1920 x 1080 @ 32 bpp).
- `screen 1024x768` — возврат к стандартному разрешению VirtualBox.
- `screen auto` — автоматическая подгонка экрана.
- `zero-display-config set 1280 720 32` — системная утилита настройки дисплея.

### Поддерживаемые разрешения:
| Разрешение | Соотношение сторон | Назначение |
| :--- | :---: | :--- |
| **`1024 x 768`** | `4:3` | Стандарт VirtualBox XGA (по умолчанию) |
| **`1280 x 720`** | `16:9` | HD 720p широкоформатный |
| **`1280 x 800`** | `16:10` | WXGA для ноутбуков |
| **`1280 x 1024`** | `5:4` | SXGA классические мониторы |
| **`1440 x 900`** | `16:10` | WXGA+ широкоформатный |
| **`1600 x 900`** | `16:9` | HD+ мониторы |
| **`1920 x 1080`** | `16:9` | Full HD 1080p |
| **`800 x 600`** | `4:3` | SVGA базовый |

---

## 3. Интерактивный установщик драйверов и системы

Команда `driver-install` (или `/install`, `install`, `setup`) запускает интерактивный мастер:
1. **Сканирование шины PCI**: автоматический поиск `0x80EE:0xCAFE` (VMMDev), `0x80EE:0xBEEF` (VMSVGA), `0x8086:0x100E` (E1000), `0x8086:0x2415` (AC'97).
2. **Загрузка Ring-0 драйверов**: подключение канала гостевых дополнений, синхронизация времени хоста, бесшовный указатель мыши.
3. **Настройка видеоподсистемы**: автоматический выбор оптимального Linear Framebuffer.
4. **Монтирование общих папок**: доступ к `/media/sf_shared`.

---

## 4. Подробный разбор и исправление ошибки VirtualBox DisplayWrap и Guru Meditation 1155

### Симптомы проблемы
```text
00:00:24.663542 Changing the VM state from 'RUNNING' to 'GURU_MEDITATION'
00:00:24.663845 !! VCPU0: Guru Meditation 1155 (VINF_EM_TRIPLE_FAULT)
```

### Первопричины и решения:
1. **Размещение стека в области BIOS ROM (`0x000F0000 - 0x000FFFFF`)**:
   Инструкция `mov rsp, 0x100000` вела к записи стековых фреймов в защищенную от записи область ROM.
   **Решение**: перенос стека на `0x200000` (2 МБ в Extended RAM).
2. **Перекрытие памяти при копировании ядра**:
   Копирование вперед `rep movsb` портило тело ядра при размере > 32 КБ.
   **Решение**: обратное копирование `std; rep movsb`.
3. **Несоответствие цветового формата 24bpp / 32bpp**:
   VirtualBox в режиме `0x4118` выдает 24 bpp с pitch 3072 (`0xC00`).
   **Решение**: раздельная отрисовка 16, 24 и 32 bpp с учетом `screen_pitch`.
4. **EPT Lock Contention**:
   Постоянная перерисовка полного фона вызывала блокировки VMSVGA dirty-page tracker.
   **Решение**: дифференциальный рендеринг (полная перерисовка только по запросу/смене режима).

---

## 5. Новая архитектура драйверов клавиатуры и русская раскладка

- Поддержка скан-кодов **Set 1 и Set 2**.
- Обработка префиксов `0xE0` и `0xF0`.
- Полноценные таблицы CP866 для русской раскладки (буквы, регистр, Ё/ё, знаки препинания).
- Переключение по `Alt + Shift` и через команды `layout ru` / `layout en`.

---

## 6. Адаптация под чистую 64-битную архитектуру (x86_64)

- **GDT**: селекторы `0x08` (32-bit Code), `0x10` (64-bit Data), `0x18` (64-bit Kernel Code).
- **IDT**: 256 16-байтовых шлюзов прерываний.
- **Таблицы страниц**: 4-уровневая структура (PML4 -> PDPT -> PD0..PD3) с отображением 4 ГБ физической памяти через 2 МБ Huge Pages.

---

## 7. Инструкция по установке и настройке в VirtualBox

| Параметр | Рекомендуемое значение | Примечание |
| :--- | :--- | :--- |
| **Тип ОС** | `Linux` | Обязательно |
| **Версия ОС** | `Other Linux (64-bit)` или `Ubuntu (64-bit)` | Включает флаги 64-бит |
| **Оперативная память (RAM)** | `2048 MB` (или `4096 MB`) | Рекомендуется |
| **Процессоры (CPU)** | `2 CPU` | Включить `PAE/NX: ON` |
| **Видеопамять (VRAM)** | `128 MB` | Рекомендуется |
| **Графический контроллер** | `VMSVGA` (или `VBoxSVGA`) | Полное ускорение |
| **3D-ускорение** | `Включено (Checked)` | Аппаратный рендеринг |
| **Диск** | `20.0 GB (VDI dynamic)` | Контроллер SATA / AHCI |
| **Мышь** | `USB Tablet` (Абсолютный курсор) | Плавный ввод без захвата |

---
*LinuxOSZero Core Team & Arena.ai Agent Mode — August 2026*

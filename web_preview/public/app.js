// =============================================================================
// LinuxOSZero — ZeroDesktop Live Preview (Genesis v1.0.0)
// Boot → Login/session chooser → Desktop OR Text-mode terminal.
// Custom SVG icons, System Monitor (analytics), driver settings, multi-hypervisor.
// =============================================================================

const bootScreen = document.getElementById('boot-screen');
const loginScreen = document.getElementById('login-screen');
const osContainer = document.getElementById('os-container');
const windowsLayer = document.getElementById('windows-layer');
const taskList = document.getElementById('task-list');
const startMenu = document.getElementById('start-menu');
const startItems = document.getElementById('start-items');
const clockElem = document.getElementById('clock');
const desktopIcons = document.getElementById('desktop-icons');

let zIndexCount = 100;
let windows = {};
let nextWinId = 1;

// ---------------------------------------------------------------------------
// Hypervisor state (QEMU / VirtualBox / VMware / bare-metal)
// ---------------------------------------------------------------------------
let hypervisor = 'QEMU';          // detected hypervisor
const HV_META = {
  QEMU:      { label: 'QEMU (KVM)',         vmmdev: 'N/A (std VGA / VirtIO)',   badge: 'QEMU',        color: '#f97316' },
  VirtualBox:{ label: 'Oracle VirtualBox',  vmmdev: 'VMMDev 0xD020',            badge: 'VBox VMSVGA', color: '#22c55e' },
  VMware:    { label: 'VMware Workstation', vmmdev: 'VMX net / SVGA II',        badge: 'VMware SVGA', color: '#3b82f6' },
  BareMetal: { label: 'Физическая машина',  vmmdev: 'VBE framebuffer',          badge: 'Bare Metal',  color: '#a855f7' }
};

function setHypervisor(hv) {
  hypervisor = hv;
  const meta = HV_META[hv];
  const badge = document.getElementById('hv-badge');
  const status = document.getElementById('hv-status');
  if (badge) { badge.textContent = meta.badge; badge.style.borderColor = meta.color; badge.style.color = meta.color; }
  if (status) status.textContent = '● ' + meta.label + ' — гостевые драйверы активны';
  localStorage.setItem('zero-hv', hv);
}

// Auto-detect a hypervisor (demo: QEMU default, tappable via settings)
function detectHypervisor() {
  const saved = localStorage.getItem('zero-hv');
  setHypervisor(saved && HV_META[saved] ? saved : 'QEMU');
}

// ---------------------------------------------------------------------------
// Clock
// ---------------------------------------------------------------------------
function updateClock() {
  const now = new Date();
  clockElem.textContent = now.toTimeString().split(' ')[0];
}
setInterval(updateClock, 1000);
updateClock();

// ---------------------------------------------------------------------------
// Theme
// ---------------------------------------------------------------------------
function applyTheme(theme) {
  document.body.setAttribute('data-theme', theme);
  localStorage.setItem('zero-theme', theme);
}

// ---------------------------------------------------------------------------
// Application registry (single source of truth for icons & launchers)
// ---------------------------------------------------------------------------
const APPS = [
  { id: 'installer',  name: 'Установка ОС',    icon: 'installer',   launch: 'installer' },
  { id: 'terminal',   name: 'Терминал',        icon: 'terminal',    launch: 'terminal' },
  { id: 'filemanager',name: 'Файлы',           icon: 'filemanager', launch: 'filemanager' },
  { id: 'control',    name: 'Параметры',       icon: 'settings',    launch: 'control' },
  { id: 'analytics',  name: 'Монитор системы', icon: 'analytics',   launch: 'analytics' },
  { id: 'editor',     name: 'Редактор',        icon: 'editor',      launch: 'editor' },
  { id: 'fetch',      name: 'О системе',       icon: 'info',        launch: 'fetch' },
  { id: 'calculator', name: 'Калькулятор',     icon: 'calculator',  launch: 'calculator' }
];

function openApp(appName) {
  switch (appName) {
    case 'installer': createInstallerWindow(); break;
    case 'terminal': createTerminalWindow(); break;
    case 'filemanager': createFileManagerWindow(); break;
    case 'control': createControlPanelWindow(); break;
    case 'analytics': createAnalyticsWindow(); break;
    case 'editor': createEditorWindow(); break;
    case 'fetch': createFetchWindow(); break;
    case 'calculator': createCalculatorWindow(); break;
    case 'welcome': createWelcomeWindow(); break;
  }
}

// Populate desktop icons & start menu from APPS registry
function buildDesktopAndMenu() {
  desktopIcons.innerHTML = APPS.map(a =>
    `<div class="desktop-icon" data-app="${a.launch}"><div class="icon-img">${svgIcon(a.icon, 30)}</div><span>${a.name}</span></div>`
  ).join('');
  desktopIcons.querySelectorAll('.desktop-icon').forEach(el => {
    el.addEventListener('click', () => openApp(el.dataset.app));
  });

  startItems.innerHTML = APPS.map(a =>
    `<div class="start-item" data-app="${a.launch}"><span class="start-icon">${svgIcon(a.icon, 20)}</span><div><div class="item-title">${a.name}</div></div></div>`
  ).join('');
  startItems.querySelectorAll('.start-item').forEach(el => {
    el.addEventListener('click', () => { openApp(el.dataset.app); toggleStartMenu(); });
  });

  // Fill tray icons + search + power
  document.getElementById('net-icon').innerHTML = svgIcon('network', 16);
  document.getElementById('audio-icon').innerHTML = svgIcon('audio', 16);
  document.getElementById('start-search-icon').innerHTML = svgIcon('search', 14);
}

// Start menu search filter
const startSearch = document.getElementById('start-search');
if (startSearch) startSearch.addEventListener('input', (e) => {
  const q = e.target.value.toLowerCase();
  startItems.querySelectorAll('.start-item').forEach(it => {
    it.style.display = it.textContent.toLowerCase().includes(q) ? '' : 'none';
  });
});

document.getElementById('power-btn').addEventListener('click', logout);

// ---------------------------------------------------------------------------
// Boot sequence
// ---------------------------------------------------------------------------
const BOOT_LINES = [
  { text: 'BIOS: запуск загрузчика LinuxOSZero', cls: 'ok' },
  { text: 'Детектирование гипервизора (QEMU/VirtualBox/VMware)…', cls: 'ok' },
  { text: 'PCI: сканирование шин … найдены устройства', cls: 'ok' },
  { text: 'Загрузка ядра 6.1.0-zero-x86_64 (long mode)', cls: '' },
  { text: 'Драйвер дисплея: VGA / VMSVGA (32-bpp) инициализирован', cls: 'ok' },
  { text: 'Драйвер мыши: бесшовная интеграция', cls: 'ok' },
  { text: 'Монтирование /proc /sys /dev /tmp … готово', cls: 'ok' },
  { text: 'Запуск zero-init (PID 1)', cls: 'ok' },
  { text: 'Запуск службы zero-guest-agent …', cls: 'ok' },
  { text: 'Запуск ZeroDesktop и ZeroWM …', cls: '' },
  { text: 'LinuxOSZero Genesis Edition v1.0.0 готова.', cls: 'ok' }
];

function bootSequence() {
  const log = document.getElementById('boot-log');
  const pbar = document.getElementById('boot-pbar');
  log.innerHTML = '';
  let i = 0;
  const maxLines = 10;
  const timer = setInterval(() => {
    if (i < BOOT_LINES.length) {
      const line = BOOT_LINES[i];
      const div = document.createElement('div');
      div.innerHTML = `<span class="dim">[</span><span class="${line.cls || 'dim'}">OK</span><span class="dim">]</span> ${line.text}`;
      log.appendChild(div);
      while (log.childElementCount > maxLines) log.removeChild(log.firstChild);
      log.scrollTop = log.scrollHeight;
      pbar.style.width = Math.round(((i + 1) / BOOT_LINES.length) * 100) + '%';
      i++;
    } else {
      clearInterval(timer);
      setTimeout(showLogin, 350);
    }
  }, 110);
}
let bootTimer = setTimeout(bootSequence, 0);

function showLogin() {
  bootScreen.classList.add('boot-done');
  loginScreen.classList.remove('hidden');
  const saved = localStorage.getItem('zero-theme') || 'dark';
  applyTheme(saved);
  document.getElementById('login-theme').value = saved;
  detectHypervisor();
  const ls = document.getElementById('login-status');
  if (ls) ls.textContent = '● Детектирован гипервизор: ' + HV_META[hypervisor].label;
  setTimeout(() => bootScreen.remove(), 700);
  setTimeout(() => document.getElementById('login-pass').focus(), 200);
}
document.addEventListener('keydown', (e) => {
  if (!bootScreen.classList.contains('boot-done')) { clearTimeout(bootTimer); showLogin(); }
}, { once: true });

document.getElementById('login-theme').addEventListener('change', (e) => applyTheme(e.target.value));
document.getElementById('login-btn').addEventListener('click', () => {
  const session = document.getElementById('login-session').value;
  applyTheme(document.getElementById('login-theme').value);
  enterSession(session);
});
document.getElementById('login-pass').addEventListener('keydown', (e) => {
  if (e.key === 'Enter') document.getElementById('login-btn').click();
});

function enterSession(session) {
  loginScreen.classList.add('hidden');
  if (session === 'terminal') showTextSession();
  else showDesktop();
}
function logout() {
  Object.keys(windows).forEach(id => closeWin(id));
  document.getElementById('text-session')?.remove();
  osContainer.classList.add('hidden');
  loginScreen.classList.remove('hidden');
}
function showDesktop() {
  osContainer.classList.remove('hidden');
  openApp('welcome');
  openApp('terminal');
}

// ---------------------------------------------------------------------------
// Text-mode session
// ---------------------------------------------------------------------------
function showTextSession() {
  const tty = document.createElement('div');
  tty.id = 'text-session';
  tty.style.cssText = `position:fixed;inset:0;z-index:3000;background:#05070d;color:#e2e8f0;font-family:"Courier New",monospace;font-size:14px;line-height:1.5;display:flex;flex-direction:column;padding:14px 18px;`;
  tty.innerHTML = `
    <div style="color:#38bdf8;font-weight:bold;border-bottom:1px solid #1e293b;padding-bottom:8px;">
      LinuxOSZero — текстовый режим (ZeroTerminal) — <span style="color:#22c55e;">${HV_META[hypervisor].label} TTY</span>
    </div>
    <div id="tty-output" style="flex:1;overflow-y:auto;white-space:pre-wrap;margin-top:8px;"></div>
    <div style="display:flex;"><span class="term-prompt">user@linuxoszero:~$</span>
    <input id="tty-input" style="flex:1;background:transparent;border:none;outline:none;color:#e2e8f0;font-family:inherit;font-size:14px;caret-color:#38bdf8;" autocomplete="off" spellcheck="false"></div>`;
  document.body.appendChild(tty);
  const out = document.getElementById('tty-output');
  out.innerHTML = `<span class="dim">LinuxOSZero текстовый режим — введите 'help'. Выход: 'logout'</span>\n`;
  const input = document.getElementById('tty-input');
  setTimeout(() => input.focus(), 60);
  input.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      const v = input.value;
      out.insertAdjacentHTML('beforeend', `<span class="term-prompt">user@linuxoszero:~$</span> ` + v.replace(/</g, '&lt;') + '\n');
      runTermCommand(v, out);
      out.scrollTop = out.scrollHeight;
      input.value = '';
    }
  });
  out.addEventListener('click', () => input.focus());
}

// ---------------------------------------------------------------------------
// Terminal command engine
// ---------------------------------------------------------------------------
function termLine(text, cls = '') { return `<div class="${cls}">${text}</div>`; }

function runTermCommand(cmd, outputEl) {
  const c = cmd.trim(); const p = c.split(/\s+/); const prog = p[0] || ''; const args = p.slice(1);
  let out = '';
  switch (prog.toLowerCase()) {
    case '': break;
    case 'help':
      out = termLine(`Доступные команды:
  help            Справка
  uname           Информация о ядре
  neofetch        Системная информация
  hwprobe         Драйверы и гипервизор
  vboxstatus      Статус интеграции гостя
  monitor         Монитор системы (аналитика)
  ls [путь]       Список файлов
  cat <файл>      Вывести содержимое
  zpkg list       Установленные пакеты
  theme dark|light|ocean   Сменить тему
  clear           Очистить экран
  logout          Выход из сеанса`); break;
    case 'uname':
      out = termLine(args.includes('-a') ? 'Linux linuxoszero 6.1.0-zero #1 SMP PREEMPT x86_64 GNU/Linux' : 'LinuxOSZero 6.1.0-zero x86_64'); break;
    case 'neofetch':
    case 'fetch':
      out = termLine(`
       .---.
      /     \\
     | () () |
      \\  _  /
     .-'   '-.
    /  ZERO   \\
   |  LINUXOS  |
    \\  v1.0   /
     '-------'

<span class="term-success">user@linuxoszero</span>
--------------------------------
ОС         : LinuxOSZero 1.0.0 (Genesis)
Гипервизор : ${HV_META[hypervisor].label}
Ядро       : 6.1.0-zero-x86_64
Разрешение : 1920x1080 (${hypervisor === 'QEMU' ? 'std VGA' : 'VMSVGA'})
WM         : ZeroWM (двойная буферизация)
Пакетов    : 45 (zpkg)
Память     : 1.2 GB / 4 GB`); break;
    case 'hwprobe':
    case 'zero-hwprobe':
      out = termLine(`[*] Гипервизор   : <span class="term-success">${HV_META[hypervisor].label}</span>
[*] Дисплей      : <span class="term-success">${hypervisor === 'QEMU' ? 'std VGA (VBE 0x01CE)' : 'VMSVGA 1920x1080x32'}</span>
[*] ${HV_META[hypervisor].vmmdev === 'N/A (std VGA / VirtIO)' ? 'VirtIO' : 'VMMDev'}       : <span class="term-success">подключено</span>
[*] Мышь         : <span class="term-success">бесшовная интеграция</span>`); break;
    case 'vboxstatus':
      out = termLine(`${hypervisor}: <span class="term-success">ПОДКЛЮЧЕНО</span>
Мышь        : <span class="term-success">бесшовная интеграция</span>
Авто-размер : <span class="term-success">Включено</span>
Общие папки : /media/sf_shared
Буфер обмена: <span class="term-success">двунаправленный</span>`); break;
    case 'monitor':
      out = termLine(`ЦП: ████████░░ 82%   Память: ███████░░░ 74%   Сеть: 12.4 MB/s`); break;
    case 'ls':
      out = termLine(`<span class="term-success">bin</span>  <span class="term-success">boot</span>  <span class="term-success">dev</span>  <span class="term-success">etc</span>  <span class="term-success">home</span>  <span class="term-success">media</span>  <span class="term-success">proc</span>  <span class="term-success">root</span>  <span class="term-success">sys</span>  <span class="term-success">usr</span>  <span class="term-success">var</span>`); break;
    case 'cat':
      out = termLine(args[0] === '/etc/zero-release' || args[0] === 'zero-release'
        ? 'LinuxOSZero Genesis Edition v1.0.0 (x86_64) — ОС для QEMU/VirtualBox'
        : `<span class="term-warn">cat: ${args[0] || ''}: файл не найден</span>`); break;
    case 'zpkg':
      out = termLine(args[0] === 'list'
        ? `base-system (1.0.0)   zero-wm (1.0.0)
vbox-guest (6.1)     zero-installer (1.0)
qemu-guest (1.0)     zero-monitor (1.0)
zero-terminal (1.0)  zero-editor (1.0)`
        : 'Использование: zpkg list | zpkg install <пакет>'); break;
    case 'echo': out = termLine(args.join(' ')); break;
    case 'whoami': out = termLine('user'); break;
    case 'date': out = termLine(new Date().toString()); break;
    case 'theme':
      if (args[0]) { applyTheme(args[0]); out = termLine(`Тема изменена: ${args[0]}`); } else out = termLine('Темы: dark | light | ocean'); break;
    case 'clear': outputEl.innerHTML = ''; outputEl.scrollTop = 0; return;
    case 'logout': out = termLine('Выход из сеанса…'); setTimeout(logout, 400); return;
    default: out = termLine(`<span class="term-warn">zero: команда не найдена: ${prog}</span> <span class="dim">(попробуйте 'help')</span>`);
  }
  if (out) outputEl.insertAdjacentHTML('beforeend', out);
  outputEl.scrollTop = outputEl.scrollHeight;
}

// ---------------------------------------------------------------------------
// Window management
// ---------------------------------------------------------------------------
function createWindow(title, iconName, width, height, contentHtml, opts = {}) {
  const id = 'win-' + nextWinId++;
  const win = document.createElement('div');
  win.className = 'window active'; win.id = id;
  const offset = (nextWinId * 24) % 110;
  const left = Math.max(30, (window.innerWidth - width) / 2 + offset);
  const top = Math.max(24, (window.innerHeight - height) / 2 - 30 + offset);
  win.style.cssText = `width:${width}px;height:${height}px;left:${left}px;top:${top}px;z-index:${++zIndexCount};`;
  win.innerHTML = `
    <div class="titlebar" onmousedown="startDrag('${id}', event)" ondblclick="maximizeWin('${id}')">
      <div class="titlebar-left"><span class="title-icon">${svgIcon(iconName, 16)}</span><span>${title}</span></div>
      <div class="win-controls">
        <button class="win-btn win-btn-min" onclick="minimizeWin('${id}')"></button>
        <button class="win-btn win-btn-max" onclick="maximizeWin('${id}')"></button>
        <button class="win-btn win-btn-close" onclick="closeWin('${id}')"></button>
      </div>
    </div>
    <div class="win-content" ${opts.contentId ? `id="${opts.contentId}"` : ''}>${contentHtml}</div>`;
  win.addEventListener('mousedown', () => focusWin(id));
  windowsLayer.appendChild(win);
  const taskBtn = document.createElement('div');
  taskBtn.className = 'task-item active'; taskBtn.id = 'task-' + id;
  taskBtn.innerHTML = `<span class="task-icon">${svgIcon(iconName, 14)}</span> <span>${title}</span>`;
  taskBtn.onclick = () => {
    if (win.style.display === 'none') { win.style.display = 'flex'; focusWin(id); }
    else if (win.classList.contains('active')) minimizeWin(id);
    else focusWin(id);
  };
  taskList.appendChild(taskBtn);
  windows[id] = { elem: win, taskElem: taskBtn, isMax: false };
  focusWin(id);
  return id;
}
function focusWin(id) {
  document.querySelectorAll('.window').forEach(w => w.classList.remove('active'));
  document.querySelectorAll('.task-item').forEach(t => t.classList.remove('active'));
  if (windows[id]) { windows[id].elem.classList.add('active'); windows[id].elem.style.zIndex = ++zIndexCount; windows[id].taskElem.classList.add('active'); }
}
function closeWin(id) { if (windows[id]) { windows[id].elem.remove(); windows[id].taskElem.remove(); delete windows[id]; } }
function minimizeWin(id) { if (windows[id]) { windows[id].elem.style.display = 'none'; windows[id].elem.classList.remove('active'); windows[id].taskElem.classList.remove('active'); } }
function maximizeWin(id) {
  if (!windows[id]) return;
  const w = windows[id];
  if (!w.isMax) {
    w.prevLeft = w.elem.style.left; w.prevTop = w.elem.style.top; w.prevWidth = w.elem.style.width; w.prevHeight = w.elem.style.height;
    w.elem.style.left = '0px'; w.elem.style.top = '0px'; w.elem.style.width = '100vw'; w.elem.style.height = 'calc(100vh - 44px)'; w.isMax = true;
  } else {
    w.elem.style.left = w.prevLeft; w.elem.style.top = w.prevTop; w.elem.style.width = w.prevWidth; w.elem.style.height = w.prevHeight; w.isMax = false;
  }
}
let dragWin = null, dragOffX = 0, dragOffY = 0;
function startDrag(id, e) { if (e.target.closest('.win-controls')) return; dragWin = windows[id].elem; focusWin(id); const r = dragWin.getBoundingClientRect(); dragOffX = e.clientX - r.left; dragOffY = e.clientY - r.top; document.addEventListener('mousemove', onDrag); document.addEventListener('mouseup', stopDrag); }
function onDrag(e) { if (dragWin) { dragWin.style.left = Math.max(0, e.clientX - dragOffX) + 'px'; dragWin.style.top = Math.max(0, e.clientY - dragOffY) + 'px'; } }
function stopDrag() { dragWin = null; document.removeEventListener('mousemove', onDrag); document.removeEventListener('mouseup', stopDrag); }
function toggleStartMenu() { startMenu.classList.toggle('hidden'); }
document.addEventListener('click', (e) => { if (!startMenu.contains(e.target) && !e.target.closest('#start-btn')) startMenu.classList.add('hidden'); });
document.getElementById('start-btn').addEventListener('click', toggleStartMenu);

// ---------------------------------------------------------------------------
// App: Terminal
// ---------------------------------------------------------------------------
function createTerminalWindow() {
  const contentId = 'term-content-' + nextWinId;
  const html = `<div class="term-window"><div class="term-output" id="${contentId}"></div>
    <div class="term-input-row"><span class="term-prompt">user@linuxoszero:~$</span>
    <input class="term-input" id="term-input-${contentId}" autocomplete="off" spellcheck="false" placeholder="введите 'help'"></div></div>`;
  createWindow('Терминал', 'terminal', 660, 400, html, { contentId });
  const outEl = document.getElementById(contentId);
  outEl.innerHTML = `<div class="dim">LinuxOSZero Терминал v1.0.0 (x86_64) — интерактивная оболочка</div><br>`;
  const inputEl = document.getElementById('term-input-' + contentId);
  setTimeout(() => inputEl.focus(), 50);
  inputEl.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      const val = inputEl.value;
      outEl.insertAdjacentHTML('beforeend', '<div><span class="term-prompt">user@linuxoszero:~$</span> ' + val.replace(/</g, '&lt;') + '</div>');
      runTermCommand(val, outEl); outEl.scrollTop = outEl.scrollHeight; inputEl.value = '';
    }
  });
  outEl.addEventListener('click', () => inputEl.focus());
}

// ---------------------------------------------------------------------------
// App: System Monitor (analytics)
// ---------------------------------------------------------------------------
function createAnalyticsWindow() {
  const contentId = 'mon-content-' + nextWinId;
  const html = `
    <div style="display:flex;flex-direction:column;gap:14px;" id="${contentId}">
      <h3 style="color:var(--accent);">Монитор системы — анализ ресурсов</h3>
      <div class="card"><strong>ЦП (${hypervisor === 'QEMU' ? '4 ядра' : '2 ядра'})</strong>
        <div class="bar-bg"><div class="bar-fill bar-cpu" style="width:62%"></div></div>
        <div class="mon-row"><span>Ядро 1</span><div class="bar-bg sm"><div class="bar-fill" style="width:78%"></div></div></div>
        <div class="mon-row"><span>Ядро 2</span><div class="bar-bg sm"><div class="bar-fill" style="width:45%"></div></div></div>
        <div class="mon-row"><span>Ядро 3</span><div class="bar-bg sm"><div class="bar-fill" style="width:61%"></div></div></div>
        <div class="mon-row"><span>Ядро 4</span><div class="bar-bg sm"><div class="bar-fill" style="width:22%"></div></div></div>
      </div>
      <div class="card"><strong>Память — 2.9 GB / 4 GB</strong>
        <div class="bar-bg"><div class="bar-fill bar-mem" style="width:74%"></div></div>
        <div class="mon-row"><span>Ядро ОС</span><span class="dim">210 MB</span></div>
        <div class="mon-row"><span>ZeroDesktop</span><span class="dim">380 MB</span></div>
        <div class="mon-row"><span>Свободно</span><span class="dim">1.1 GB</span></div>
      </div>
      <div class="card"><strong>Сеть — ${HV_META[hypervisor].label}</strong>
        <div class="mon-row"><span>⬇ Загрузка</span><span class="dim">12.4 MB/s</span></div>
        <div class="mon-row"><span>⬆ Отдача</span><span class="dim">2.1 MB/s</span></div>
        <div class="mon-row"><span>Активные соединения</span><span class="dim">8</span></div>
      </div>
      <div class="card"><strong>Диск — /dev/${hypervisor === 'QEMU' ? 'vda' : 'sda'} (20 GB)</strong>
        <div class="bar-bg"><div class="bar-fill bar-disk" style="width:31%"></div></div>
        <div class="mon-row"><span>Занято</span><span class="dim">6.2 GB</span><span>Свободно</span><span class="dim">13.8 GB</span></div>
      </div>
    </div>`;
  createWindow('Монитор системы', 'analytics', 560, 520, html, { contentId });
  // Live-update bars
  const timers = [];
  const jitter = setInterval(() => {
    const cpu = document.querySelector('.bar-cpu');
    if (cpu) cpu.style.width = (30 + Math.round(Math.random() * 60)) + '%';
    const mem = document.querySelector('.bar-mem');
    if (mem) mem.style.width = (60 + Math.round(Math.random() * 25)) + '%';
  }, 900);
  timers.push(jitter);
  // Stop timers when window closes
  const origClose = closeWin;
}

// ---------------------------------------------------------------------------
// App: Calculator
// ---------------------------------------------------------------------------
function createCalculatorWindow() {
  const contentId = 'calc-content-' + nextWinId;
  let expr = '';
  const html = `
    <div style="display:flex;flex-direction:column;height:100%;">
      <div class="calc-display" id="${contentId}-disp">0</div>
      <div class="calc-grid">
        ${['C','⌫','%','÷','7','8','9','×','4','5','6','−','1','2','3','+','±','0',',','=']
          .map(k => `<button class="calc-btn ${['÷','×','−','+','='].includes(k) ? 'calc-op' : (k==='C'?'calc-clear':'')}" data-k="${k}">${k}</button>`).join('')}
      </div>
    </div>`;
  createWindow('Калькулятор', 'calculator', 300, 420, html, { contentId });
  const disp = document.getElementById(contentId + '-disp');
  document.querySelectorAll('.calc-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const k = btn.dataset.k;
      if (k === 'C') { expr = ''; disp.textContent = '0'; }
      else if (k === '⌫') { expr = expr.slice(0, -1); disp.textContent = expr || '0'; }
      else if (k === '=') {
        try { const clean = expr.replace(/×/g,'*').replace(/−/g,'-').replace(/÷/g,'/').replace(/,/g,'.'); disp.textContent = String(Function('"use strict";return (' + clean + ')')()).replace('.',','); expr = disp.textContent.replace(',','.'); }
        catch(e) { disp.textContent = 'Ошибка'; expr = ''; }
      }
      else { expr += k; disp.textContent = expr; }
    });
  });
}

// ---------------------------------------------------------------------------
// App: Control panel (driver settings + hypervisor + resolution + theme)
// ---------------------------------------------------------------------------
function createControlPanelWindow() {
  const html = `
    <div style="display:flex;flex-direction:column;gap:16px;">
      <h3 style="color:var(--accent);">Настройки драйверов и системы</h3>
      <div class="card">
        <strong>Гипервизор:</strong>
        <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px;">
          ${Object.keys(HV_META).map(hv =>
            `<button class="btn-secondary hv-btn" data-hv="${hv}" style="${hv === hypervisor ? 'border:1px solid var(--accent);' : ''}">${svgIcon(hv === 'VirtualBox' ? 'chip' : hv === 'QEMU' ? 'terminal' : 'network', 14)} ${HV_META[hv].label}</button>`
          ).join('')}
        </div>
      </div>
      <div class="card">
        <strong>Разрешение экрана:</strong>
        <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:10px;">
          <button class="btn-secondary res-btn" data-res="1024x768">1024 x 768</button>
          <button class="btn-secondary res-btn" data-res="1280x720">1280 x 720</button>
          <button class="btn-secondary res-btn" data-res="1920x1080" style="border:1px solid var(--accent);">1920 x 1080 (активно)</button>
          <button class="btn-secondary res-btn" data-res="2560x1440">2560 x 1440</button>
        </div>
        <p id="res-status" style="color:var(--accent);margin-top:10px;font-size:12px;">Текущее: 1920x1080</p>
      </div>
      <div class="card">
        <strong>Гостевые драйверы:</strong>
        <div class="toggle-row"><span>Бесшовная мышь</span><label class="switch"><input type="checkbox" checked id="tg-mouse"><span class="slider"></span></label></div>
        <div class="toggle-row"><span>Авто-изменение размера</span><label class="switch"><input type="checkbox" checked id="tg-resize"><span class="slider"></span></label></div>
        <div class="toggle-row"><span>Общие папки (/media/sf_shared)</span><label class="switch"><input type="checkbox" checked id="tg-sf"><span class="slider"></span></label></div>
        <div class="toggle-row"><span>Буфер обмена (двунаправленный)</span><label class="switch"><input type="checkbox" checked id="tg-cb"><span class="slider"></span></label></div>
      </div>
      <div class="card">
        <strong>Тема оформления:</strong>
        <div style="display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-top:8px;">
          <button class="btn-secondary theme-btn" data-theme="dark">Тёмная</button>
          <button class="btn-secondary theme-btn" data-theme="light">Светлая</button>
          <button class="btn-secondary theme-btn" data-theme="ocean">Океан</button>
        </div>
      </div>
      <div style="display:flex;gap:8px;">
        <button class="btn-secondary" onclick="openApp('analytics')">Монитор системы</button>
        <button class="btn-secondary" style="margin-left:auto;" onclick="logout()">Выйти из сеанса</button>
      </div>
    </div>`;
  createWindow('Параметры и драйверы', 'settings', 600, 620, html);
  document.querySelectorAll('.hv-btn').forEach(btn => btn.addEventListener('click', () => {
    setHypervisor(btn.dataset.hv);
    document.querySelectorAll('.hv-btn').forEach(b => b.style.border = '1px solid var(--border)');
    btn.style.border = '1px solid var(--accent)';
  }));
  document.querySelectorAll('.res-btn').forEach(btn => btn.addEventListener('click', () => {
    document.querySelectorAll('.res-btn').forEach(b => b.style.border = '1px solid var(--border)');
    btn.style.border = '1px solid var(--accent)';
    const st = document.getElementById('res-status'); if (st) st.textContent = 'Текущее: ' + btn.dataset.res;
  }));
  document.querySelectorAll('.theme-btn').forEach(btn => btn.addEventListener('click', () => applyTheme(btn.dataset.theme)));
}

// ---------------------------------------------------------------------------
// App: File manager
// ---------------------------------------------------------------------------
const fmTree = {
  '/': [
    { n: 'bin', t: 'Системные программы', d: true }, { n: 'boot', t: 'Ядро и GRUB', d: true },
    { n: 'dev', t: 'Устройства', d: true }, { n: 'etc', t: 'Конфигурация', d: true },
    { n: 'home', t: 'Домашние каталоги', d: true }, { n: 'media', t: 'vboxsf', d: true },
    { n: 'opt', t: 'Дополнительно', d: true }, { n: 'proc', t: 'Proc FS', d: true },
    { n: 'root', t: 'root', d: true }, { n: 'sys', t: 'Sys FS', d: true },
    { n: 'tmp', t: 'Временные', d: true }, { n: 'usr', t: 'Пользовательские', d: true },
    { n: 'var', t: 'Переменные данные', d: true },
    { n: 'zero-init', t: 'ZeroInit (PID 1)', d: false, s: '128 KB' },
    { n: 'vmlinuz', t: 'Образ ядра', d: false, s: '13 KB' }
  ],
  '/home': [{ n: 'user', t: 'Домашний каталог пользователя', d: true }],
  '/home/user': [
    { n: 'Документы', t: 'Папка', d: true }, { n: 'wallpaper.png', t: 'Изображение', d: false, s: '1.8 MB' },
    { n: 'hello.c', t: 'Исходный код C', d: false, s: '1.1 KB' }, { n: 'zero.conf', t: 'Конфигурация', d: false, s: '620 B' }
  ],
  '/etc': [
    { n: 'inittab', t: 'Конфигурация init', d: false, s: '1.2 KB' },
    { n: 'zero-release', t: 'Версия ОС', d: false, s: '160 B' },
    { n: 'fstab', t: 'Таблица монтирования', d: false, s: '420 B' },
    { n: 'vboxsf', t: 'Общие папки', d: true }
  ]
};
function createFileManagerWindow() {
  const contentId = 'fm-content-' + nextWinId;
  const html = `<div style="display:flex;flex-direction:column;height:100%;">
    <div style="display:flex;gap:8px;margin-bottom:12px;">
      <button class="btn-secondary fm-back">&#8592;</button>
      <input type="text" id="fm-loc" value="Расположение: /" style="flex:1;background:var(--surface);border:1px solid var(--border);color:var(--text);padding:4px 8px;border-radius:6px;" readonly>
    </div>
    <table style="width:100%;border-collapse:collapse;font-size:13px;">
      <thead><tr style="color:var(--text-dim);border-bottom:1px solid var(--border);text-align:left;"><th style="padding:6px;">Имя</th><th>Тип</th><th>Размер</th></tr></thead>
      <tbody id="${contentId}"></tbody></table></div>`;
  createWindow('Файловый менеджер', 'filemanager', 600, 400, html, { contentId });
  let currentPath = '/';
  const body = document.getElementById(contentId);
  function render(path) {
    const loc = document.getElementById('fm-loc'); loc.value = 'Расположение: ' + path;
    const items = fmTree[path] || []; body.innerHTML = '';
    if (path !== '/') {
      const up = document.createElement('tr');
      up.innerHTML = `<td style="padding:6px;color:var(--warn);cursor:pointer;">${svgIcon('filemanager',14)} ..</td><td>Вверх</td><td>DIR</td>`;
      up.onclick = () => { const parts = path.split('/').filter(Boolean); parts.pop(); currentPath = '/' + parts.join('/'); render(currentPath); };
      body.appendChild(up);
    }
    items.forEach(it => {
      const tr = document.createElement('tr');
      tr.innerHTML = `<td style="padding:6px;color:${it.d ? 'var(--accent)' : 'var(--text)'};cursor:pointer;">${svgIcon(it.d ? 'filemanager' : (it.n === 'wallpaper.png' ? 'image' : 'editor'), 14)} ${it.n}</td><td>${it.t}</td><td>${it.d ? 'DIR' : (it.s || '—')}</td>`;
      if (it.d) tr.addEventListener('dblclick', () => { const np = (path === '/' ? '' : path) + '/' + it.n; if (fmTree[np]) { currentPath = np; render(currentPath); } });
      else if (it.n === 'zero-release') tr.addEventListener('dblclick', () => createEditorWindow('LinuxOSZero Genesis Edition v1.0.0 (x86_64)'));
      else if (it.n === 'wallpaper.png') tr.addEventListener('dblclick', () => { const w = window.open('', '_blank'); if (w) w.document.write('<img src="img/wallpaper.jpg" style="width:100%">'); });
      body.appendChild(tr);
    });
  }
  render(currentPath);
}

// ---------------------------------------------------------------------------
// App: Editor
// ---------------------------------------------------------------------------
function createEditorWindow(initialText) {
  const defaultText = `/* LinuxOSZero — ядро и рабочий стол */
#include <zero/os.h>
#include <zero/hypervisor.h>

int main(void) {
    detect_hypervisor();           // QEMU / VirtualBox / VMware
    printf("Привет из LinuxOSZero!\\n");
    enable_mouse_integration(true);
    zerowm_start_session();
    return 0;
}
`;
  const html = `<div style="display:flex;flex-direction:column;height:100%;">
    <div style="display:flex;gap:8px;margin-bottom:8px;">
      <button class="btn-primary editor-save">Сохранить</button><button class="btn-secondary">Открыть</button>
      <span style="color:var(--text-dim);margin-left:12px;align-self:center;font-size:12px;">main.c (исходный код C)</span>
    </div>
    <textarea class="editor-area" style="flex:1;width:100%;background:#090d16;color:#38bdf8;border:1px solid var(--border);border-radius:6px;padding:10px;font-family:monospace;resize:none;">${(initialText || defaultText).replace(/</g, '&lt;')}</textarea></div>`;
  createWindow('Редактор — main.c', 'editor', 600, 400, html);
  const save = document.querySelector('.editor-save');
  if (save) save.onclick = () => { const area = save.closest('.window').querySelector('.editor-area'); area.style.borderColor = '#22c55e'; setTimeout(() => area.style.borderColor = '', 800); alert('Файл сохранён!'); };
}

// ---------------------------------------------------------------------------
// App: Fetch
// ---------------------------------------------------------------------------
function createFetchWindow() {
  const html = `
    <div style="display:flex;gap:20px;align-items:flex-start;">
      <img src="img/logo.png" style="width:110px;height:110px;border-radius:18px;box-shadow:0 8px 24px var(--accent-glow);" alt="logo">
      <div style="font-size:13px;line-height:1.7;">
        <div style="color:var(--accent);font-weight:bold;">user@linuxoszero</div>
        <div style="color:var(--text-dim);">--------------------------------</div>
        <div><strong>ОС</strong>         : LinuxOSZero 1.0.0 (Genesis)</div>
        <div><strong>Гипервизор</strong> : ${HV_META[hypervisor].label}</div>
        <div><strong>Ядро</strong>       : 6.1.0-zero-x86_64</div>
        <div><strong>Разрешение</strong> : 1920x1080 (${hypervisor === 'QEMU' ? 'std VGA' : 'VMSVGA'})</div>
        <div><strong>WM</strong>         : ZeroWM (двойная буферизация)</div>
        <div><strong>Пакетов</strong>    : 45 (zpkg)</div>
        <div><strong>Память</strong>     : 1.2 GB / 4 GB</div>
      </div>
    </div>`;
  createWindow('О системе', 'info', 540, 330, html);
}

// ---------------------------------------------------------------------------
// App: Welcome
// ---------------------------------------------------------------------------
function createWelcomeWindow() {
  const html = `
    <div style="display:flex;flex-direction:column;align-items:center;text-align:center;padding:12px;">
      <img src="img/logo.png" style="width:72px;height:72px;border-radius:16px;box-shadow:0 8px 24px var(--accent-glow);margin-bottom:12px;" alt="">
      <h3>Добро пожаловать в LinuxOSZero</h3>
      <p style="color:var(--text-dim);margin-top:6px;font-size:13px;">Ваша операционная система с поддержкой QEMU, VirtualBox и VMware.</p>
      <div style="display:flex;gap:10px;margin-top:18px;">
        <button class="btn-primary" onclick="openApp('installer')">Установить ОС</button>
        <button class="btn-secondary" onclick="openApp('analytics')">Монитор системы</button>
      </div>
    </div>`;
  createWindow('Добро пожаловать', 'info', 500, 280, html);
}

// ---------------------------------------------------------------------------
// App: Installer
// ---------------------------------------------------------------------------
let currentStep = 1;
function createInstallerWindow() { currentStep = 1; createWindow('Установка LinuxOSZero v1.0', 'installer', 660, 460, getInstallerStepHtml(1), { contentId: 'installer-content' }); }
function getInstallerStepHtml(step) {
  let stepContent = '';
  if (step === 1) stepContent = `
      <h3>Добро пожаловать в установку LinuxOSZero</h3>
      <p style="color:var(--text-dim);margin-top:6px;">Мастер установит LinuxOSZero на ваш диск.</p>
      <div class="card"><strong style="color:var(--accent);">Проверка совместимости:</strong>
        <p style="color:var(--success);margin-top:8px;">✔ Драйверы ${HV_META[hypervisor].label} найдены</p>
        <p style="color:var(--success);">✔ ОЗУ 4 GB подтверждено</p>
        <p style="color:var(--success);">✔ Доступен целевой диск (&gt;10 GB)</p>
        <p style="color:var(--success);">✔ Режим x86_64 и ACPI готов</p></div>
      <div class="installer-actions"><button class="btn-primary" onclick="setInstallerStep(2)">Далее &gt;</button></div>`;
  else if (step === 2) stepContent = `
      <h3>Выбор целевого диска</h3>
      <p style="color:var(--text-dim);margin-top:6px;">Выберите диск для автоматической разметки.</p>
      <div class="card" style="border-color:var(--accent);">
        <strong>📁 /dev/${hypervisor === 'QEMU' ? 'vda' : 'sda'} — 20.0 GB (${hypervisor === 'QEMU' ? 'QEMU virtio' : 'VirtualBox VDI'})</strong>
        <p style="color:var(--text-dim);font-size:12px;margin-top:4px;">Разметка: 512MB EFI/Загрузочный + 19.5GB ext4 RootFS</p></div>
      <div class="installer-actions"><button class="btn-secondary" onclick="setInstallerStep(1)">&lt; Назад</button><button class="btn-primary" onclick="setInstallerStep(3)">Далее &gt;</button></div>`;
  else if (step === 3) stepContent = `
      <h3>Пользователь и система</h3>
      <p style="color:var(--text-dim);margin-top:6px;">Настройте пользователя и системные данные.</p>
      <div class="card">
        <p><strong>Имя хоста :</strong> linuxoszero</p>
        <p><strong>Пользователь :</strong> user</p>
        <p><strong>Пароль :</strong> zero (доступ sudo)</p>
        <p><strong>Часовой пояс :</strong> UTC (авто-синхронизация)</p></div>
      <div class="installer-actions"><button class="btn-secondary" onclick="setInstallerStep(2)">&lt; Назад</button><button class="btn-primary" onclick="setInstallerStep(4); startInstallSim();">Установить &gt;&gt;</button></div>`;
  else if (step === 4) stepContent = `
      <h3>Установка LinuxOSZero...</h3>
      <p style="color:var(--text-dim);margin-top:6px;">Копирование ядра, rootfs и драйверов...</p>
      <div class="progress-bar-bg"><div id="inst-pbar" class="progress-bar-fill"></div></div>
      <p id="inst-status" style="color:var(--accent);font-size:13px;margin-top:8px;">Форматирование раздела ext4...</p>`;
  else if (step === 5) stepContent = `
      <h3 style="color:var(--success);">Установка завершена!</h3>
      <p style="color:var(--text-dim);margin-top:6px;">LinuxOSZero v1.0.0 успешно установлена.</p>
      <div class="card" style="border-color:var(--success);">
        <p>✔ Ядро: LinuxOSZero 6.1 x86_64</p>
        <p>✔ Загрузчик: GRUB2 (MBR/EFI) установлен</p>
        <p>✔ Драйверы ${HV_META[hypervisor].label}: настроены</p>
        <p>✔ Пользователь: user (пароль: zero)</p></div>
      <div class="installer-actions"><button class="btn-primary" style="background:var(--success);" onclick="location.reload()">Перезагрузить</button></div>`;
  return `<div class="installer-box">
    <div class="installer-sidebar">
      <div class="inst-step ${step === 1 ? 'active' : (step > 1 ? 'done' : '')}"><div class="step-dot"></div> 1. Приветствие</div>
      <div class="inst-step ${step === 2 ? 'active' : (step > 2 ? 'done' : '')}"><div class="step-dot"></div> 2. Диск</div>
      <div class="inst-step ${step === 3 ? 'active' : (step > 3 ? 'done' : '')}"><div class="step-dot"></div> 3. Пользователь</div>
      <div class="inst-step ${step === 4 ? 'active' : (step > 4 ? 'done' : '')}"><div class="step-dot"></div> 4. Установка</div>
      <div class="inst-step ${step === 5 ? 'active' : (step > 5 ? 'done' : '')}"><div class="step-dot"></div> 5. Готово</div>
    </div>
    <div class="installer-main">${stepContent}</div></div>`;
}
function setInstallerStep(step) { currentStep = step; const c = document.getElementById('installer-content'); if (c) c.innerHTML = getInstallerStepHtml(step); }
function startInstallSim() {
  let progress = 10;
  const pbar = document.getElementById('inst-pbar'); const status = document.getElementById('inst-status');
  const timer = setInterval(() => {
    progress += 20; if (pbar) pbar.style.width = progress + '%';
    if (progress === 30 && status) status.textContent = 'Форматирование корневой ФС ext4...';
    if (progress === 50 && status) status.textContent = 'Распаковка системных библиотек и ZeroDesktop...';
    if (progress === 70 && status) status.textContent = 'Установка драйверов гостя...';
    if (progress === 90 && status) status.textContent = 'Настройка загрузчика GRUB2...';
    if (progress >= 100) { clearInterval(timer); setTimeout(() => setInstallerStep(5), 500); }
  }, 400);
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
buildDesktopAndMenu();

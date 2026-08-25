// =============================================================================
// LinuxOSZero — ZeroDesktop Live Preview (Genesis v1.0.0)
// Boot → Login/Session chooser → Desktop OR Text-mode terminal.
// =============================================================================

const bootScreen = document.getElementById('boot-screen');
const loginScreen = document.getElementById('login-screen');
const osContainer = document.getElementById('os-container');
const windowsLayer = document.getElementById('windows-layer');
const taskList = document.getElementById('task-list');
const startMenu = document.getElementById('start-menu');
const clockElem = document.getElementById('clock');

let zIndexCount = 100;
let windows = {};
let nextWinId = 1;

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
// Theme management
// ---------------------------------------------------------------------------
function applyTheme(theme) {
  document.body.setAttribute('data-theme', theme);
  localStorage.setItem('zero-theme', theme);
}

// ---------------------------------------------------------------------------
// Boot sequence
// ---------------------------------------------------------------------------
const BOOT_LINES = [
  { text: 'BIOS: обнаружен гипервизор Oracle VirtualBox', cls: 'ok' },
  { text: 'VMMDev I/O порт 0xD020 : подключено', cls: 'ok' },
  { text: 'VBoxVideo (VMSVGA) 0x80EE:0xBEEF : инициализировано', cls: 'ok' },
  { text: 'Драйвер PCI : устройства найдены', cls: 'ok' },
  { text: 'Загрузка ядра 6.1.0-zero-x86_64', cls: '' },
  { text: 'Переход в long mode ... готово', cls: 'ok' },
  { text: 'Монтирование /proc /sys /dev /tmp ... готово', cls: 'ok' },
  { text: 'Запуск zero-init (PID 1)', cls: 'ok' },
  { text: 'Запуск службы zero-guest-agent ...', cls: 'ok' },
  { text: 'Запуск ZeroDisplay 1024x768x32', cls: 'ok' },
  { text: 'Запуск ZeroDesktop и ZeroWM ...', cls: '' },
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
  }, 120);
}

function skipBoot() {
  clearTimeout(bootTimer);
  showLogin();
}
let bootTimer;
function showLogin() {
  bootScreen.classList.add('boot-done');
  loginScreen.classList.remove('hidden');
  // Preload saved theme
  const saved = localStorage.getItem('zero-theme') || 'dark';
  applyTheme(saved);
  document.getElementById('login-theme').value = saved;
  setTimeout(() => bootScreen.remove(), 700);
  setTimeout(() => document.getElementById('login-pass').focus(), 200);
}
bootTimer = setTimeout(bootSequence, 0);

document.addEventListener('keydown', (e) => {
  if (!bootScreen.classList.contains('boot-done')) {
    clearTimeout(bootTimer);
    skipBoot();
  }
}, { once: true });

// Live theme preview in login screen
document.getElementById('login-theme').addEventListener('change', (e) => {
  applyTheme(e.target.value);
});

// ---------------------------------------------------------------------------
// Login → enter session
// ---------------------------------------------------------------------------
document.getElementById('login-btn').addEventListener('click', () => {
  const session = document.getElementById('login-session').value;
  const theme = document.getElementById('login-theme').value;
  applyTheme(theme);
  enterSession(session);
});

document.getElementById('login-pass').addEventListener('keydown', (e) => {
  if (e.key === 'Enter') document.getElementById('login-btn').click();
});

function enterSession(session) {
  loginScreen.classList.add('hidden');
  if (session === 'terminal') {
    showTextSession();
  } else {
    showDesktop();
  }
}

function logout() {
  // Close all windows, return to login
  Object.keys(windows).forEach(id => closeWin(id));
  document.getElementById('text-session')?.remove();
  osContainer.classList.add('hidden');
  loginScreen.classList.remove('hidden');
}

// ---------------------------------------------------------------------------
// Desktop session
// ---------------------------------------------------------------------------
function showDesktop() {
  osContainer.classList.remove('hidden');
  openApp('welcome');
  openApp('terminal');
}

// ---------------------------------------------------------------------------
// Text-mode terminal session (fullscreen, like a real TTY)
// ---------------------------------------------------------------------------
function showTextSession() {
  const tty = document.createElement('div');
  tty.id = 'text-session';
  tty.style.cssText = `
    position: fixed; inset: 0; z-index: 3000; background: #05070d; color: #e2e8f0;
    font-family: "Courier New", monospace; font-size: 14px; line-height: 1.5;
    display: flex; flex-direction: column; padding: 14px 18px;
  `;
  tty.innerHTML = `
    <div style="color:#38bdf8; font-weight:bold; border-bottom:1px solid #1e293b; padding-bottom:8px;">
      LinuxOSZero — текстовый режим (ZeroTerminal) — <span style="color:#22c55e;">VirtualBox TTY</span>
    </div>
    <div id="tty-output" style="flex:1; overflow-y:auto; white-space:pre-wrap; margin-top:8px;"></div>
    <div style="display:flex;">
      <span class="term-prompt">user@linuxoszero:~$</span>
      <input id="tty-input" style="flex:1; background:transparent; border:none; outline:none; color:#e2e8f0; font-family:inherit; font-size:14px; caret-color:#38bdf8;" autocomplete="off" spellcheck="false">
    </div>`;
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
// Shared terminal command engine (used by both desktop & text sessions)
// ---------------------------------------------------------------------------
function termLine(text, cls = '') {
  return `<div class="${cls}">${text}</div>`;
}

function runTermCommand(cmd, outputEl) {
  const c = cmd.trim();
  const p = c.split(/\s+/);
  const prog = p[0] || '';
  const args = p.slice(1);
  let out = '';

  switch (prog.toLowerCase()) {
    case '':
      break;
    case 'help':
      out = termLine(`Доступные команды:
  help            Справка
  uname           Информация о ядре
  neofetch        Системная информация
  zero-hwprobe    Драйверы и оборудование VirtualBox
  vboxstatus      Статус интеграции VirtualBox
  ls [путь]       Список файлов
  cat <файл>      Вывести содержимое
  zpkg list       Установленные пакеты
  echo <текст>    Вывести текст
  whoami          Текущий пользователь
  date            Дата и время
  theme dark|light|ocean   Сменить тему
  clear           Очистить экран
  logout          Выход из сеанса`);
      break;
    case 'uname':
      out = termLine(args.includes('-a')
        ? 'Linux linuxoszero 6.1.0-zero #1 SMP PREEMPT x86_64 GNU/Linux'
        : 'LinuxOSZero 6.1.0-zero x86_64');
      break;
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
Хост       : Oracle VM VirtualBox (vboxguest)
Ядро       : 6.1.0-zero-x86_64
Разрешение : 1024x768 (VMSVGA, аппаратное ускорение)
WM         : ZeroWM (двойная буферизация)
Пакетов    : 42 (zpkg)
Память     : 245 MB / 2048 MB`);
      break;
    case 'zero-hwprobe':
      out = termLine(`[*] Гипервизор   : <span class="term-success">Oracle VM VirtualBox</span>
[*] VMMDev       : <span class="term-success">I/O порт 0xD020 подключён</span>
[*] Видео        : <span class="term-success">VMSVGA 1024x768x32 (ускорение)</span>
[*] Мышь         : <span class="term-success">бесшовная интеграция</span>
[*] Общие папки  : /media/sf_shared`);
      break;
    case 'vboxstatus':
      out = termLine(`VMMDev        : <span class="term-success">ПОДКЛЮЧЕНО</span>
Мышь          : <span class="term-success">бесшовная интеграция</span>
Авто-изменение размера : <span class="term-success">Включено</span>
Общие папки   : /media/sf_shared
Буфер обмена  : <span class="term-success">двунаправленный</span>`);
      break;
    case 'ls':
      out = termLine(`
<span class="term-success">bin</span>  <span class="term-success">boot</span>  <span class="term-success">dev</span>  <span class="term-success">etc</span>  <span class="term-success">home</span>  <span class="term-success">media</span>  <span class="term-success">proc</span>  <span class="term-success">root</span>  <span class="term-success">sbin</span>  <span class="term-success">sys</span>  <span class="term-success">tmp</span>  <span class="term-success">usr</span>  <span class="term-success">var</span>
zero-init  zero-desktop  zero-guest-agent  zero-fetch  vmlinuz  initrd.img`);
      break;
    case 'cat':
      if (args[0] === '/etc/zero-release' || args[0] === 'zero-release') {
        out = termLine('LinuxOSZero Genesis Edition v1.0.0 (x86_64) — ОС с поддержкой VirtualBox');
      } else {
        out = termLine(`<span class="term-warn">cat: ${args[0] || ''}: файл не найден</span>`);
      }
      break;
    case 'zpkg':
      if (args[0] === 'list') {
        out = termLine(`base-system (1.0.0)      zero-wm (1.0.0)
vbox-guest (6.1)      zero-installer (1.0)
zero-terminal (1.0)   zero-editor (1.0)
zero-filemanager (1.0) zpkg (1.0)`);
      } else {
        out = termLine('Использование: zpkg list | zpkg search <имя> | zpkg install <пакет>');
      }
      break;
    case 'echo':
      out = termLine(args.join(' '));
      break;
    case 'whoami':
      out = termLine('user');
      break;
    case 'date':
      out = termLine(new Date().toString());
      break;
    case 'theme':
      if (args[0]) { applyTheme(args[0]); out = termLine(`Тема изменена: ${args[0]}`); }
      else out = termLine('Темы: dark | light | ocean');
      break;
    case 'logout':
      out = termLine('Выход из сеанса...');
      setTimeout(logout, 400);
      return;
    case 'clear':
      outputEl.innerHTML = '';
      outputEl.scrollTop = 0;
      return;
    case 'reboot':
    case 'shutdown':
      out = termLine('<span class="term-warn">Меню питания: используйте кнопку ⏻ в меню «Пуск».</span>');
      break;
    default:
      out = termLine(`<span class="term-warn">zero: команда не найдена: ${prog}</span> <span class="dim">(попробуйте 'help')</span>`);
  }
  if (out) outputEl.insertAdjacentHTML('beforeend', out);
  outputEl.scrollTop = outputEl.scrollHeight;
}

// ---------------------------------------------------------------------------
// Window management
// ---------------------------------------------------------------------------
function createWindow(title, icon, width, height, contentHtml, opts = {}) {
  const id = 'win-' + nextWinId++;
  const win = document.createElement('div');
  win.className = 'window active';
  win.id = id;
  const offset = (nextWinId * 24) % 110;
  const left = Math.max(30, (window.innerWidth - width) / 2 + offset);
  const top = Math.max(24, (window.innerHeight - height) / 2 - 30 + offset);
  win.style.width = width + 'px';
  win.style.height = height + 'px';
  win.style.left = left + 'px';
  win.style.top = top + 'px';
  win.style.zIndex = ++zIndexCount;
  win.innerHTML = `
    <div class="titlebar" onmousedown="startDrag('${id}', event)" ondblclick="maximizeWin('${id}')">
      <div class="titlebar-left"><span>${icon}</span><span>${title}</span></div>
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
  taskBtn.className = 'task-item active';
  taskBtn.id = 'task-' + id;
  taskBtn.innerHTML = `<span>${icon}</span> <span>${title}</span>`;
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
  if (windows[id]) {
    windows[id].elem.classList.add('active');
    windows[id].elem.style.zIndex = ++zIndexCount;
    windows[id].taskElem.classList.add('active');
  }
}
function closeWin(id) {
  if (windows[id]) { windows[id].elem.remove(); windows[id].taskElem.remove(); delete windows[id]; }
}
function minimizeWin(id) {
  if (windows[id]) { windows[id].elem.style.display = 'none'; windows[id].elem.classList.remove('active'); windows[id].taskElem.classList.remove('active'); }
}
function maximizeWin(id) {
  if (!windows[id]) return;
  const w = windows[id];
  if (!w.isMax) {
    w.prevLeft = w.elem.style.left; w.prevTop = w.elem.style.top;
    w.prevWidth = w.elem.style.width; w.prevHeight = w.elem.style.height;
    w.elem.style.left = '0px'; w.elem.style.top = '0px';
    w.elem.style.width = '100vw'; w.elem.style.height = 'calc(100vh - 44px)';
    w.isMax = true;
  } else {
    w.elem.style.left = w.prevLeft; w.elem.style.top = w.prevTop;
    w.elem.style.width = w.prevWidth; w.elem.style.height = w.prevHeight;
    w.isMax = false;
  }
}

let dragWin = null, dragOffX = 0, dragOffY = 0;
function startDrag(id, e) {
  if (e.target.closest('.win-controls')) return;
  dragWin = windows[id].elem; focusWin(id);
  const rect = dragWin.getBoundingClientRect();
  dragOffX = e.clientX - rect.left; dragOffY = e.clientY - rect.top;
  document.addEventListener('mousemove', onDrag);
  document.addEventListener('mouseup', stopDrag);
}
function onDrag(e) {
  if (dragWin) { dragWin.style.left = Math.max(0, e.clientX - dragOffX) + 'px'; dragWin.style.top = Math.max(0, e.clientY - dragOffY) + 'px'; }
}
function stopDrag() { dragWin = null; document.removeEventListener('mousemove', onDrag); document.removeEventListener('mouseup', stopDrag); }

function toggleStartMenu() { startMenu.classList.toggle('hidden'); }
document.addEventListener('click', (e) => {
  if (!startMenu.contains(e.target) && !e.target.closest('#start-btn')) startMenu.classList.add('hidden');
});

// ---------------------------------------------------------------------------
// App launchers
// ---------------------------------------------------------------------------
function openApp(appName) {
  switch (appName) {
    case 'installer': createInstallerWindow(); break;
    case 'terminal': createTerminalWindow(); break;
    case 'filemanager': createFileManagerWindow(); break;
    case 'control': createControlPanelWindow(); break;
    case 'editor': createEditorWindow(); break;
    case 'fetch': createFetchWindow(); break;
    case 'welcome': createWelcomeWindow(); break;
  }
}

// ---------------------------------------------------------------------------
// Terminal app (windowed)
// ---------------------------------------------------------------------------
function createTerminalWindow() {
  const contentId = 'term-content-' + nextWinId;
  const html = `
    <div class="term-window">
      <div class="term-output" id="${contentId}"></div>
      <div class="term-input-row">
        <span class="term-prompt">user@linuxoszero:~$</span>
        <input class="term-input" id="term-input-${contentId}" autocomplete="off" spellcheck="false" placeholder="введите 'help'">
      </div>
    </div>`;
  createWindow('Терминал', '>_', 640, 390, html, { contentId });
  const outEl = document.getElementById(contentId);
  outEl.innerHTML = `<div class="dim">LinuxOSZero Терминал v1.0.0 (x86_64) — интерактивная оболочка</div><br>`;
  const inputEl = document.getElementById('term-input-' + contentId);
  setTimeout(() => inputEl.focus(), 50);
  inputEl.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      const val = inputEl.value;
      outEl.insertAdjacentHTML('beforeend', '<div><span class="term-prompt">user@linuxoszero:~$</span> ' + val.replace(/</g, '&lt;') + '</div>');
      runTermCommand(val, outEl);
      outEl.scrollTop = outEl.scrollHeight;
      inputEl.value = '';
    }
  });
  outEl.addEventListener('click', () => inputEl.focus());
}

// ---------------------------------------------------------------------------
// Control panel
// ---------------------------------------------------------------------------
function createControlPanelWindow() {
  const html = `
    <div style="display:flex; flex-direction:column; gap:16px;">
      <h3 style="color:var(--accent);">Параметры экрана и VirtualBox</h3>
      <div class="card">
        <strong>Разрешение экрана (VBoxVideo / VMSVGA):</strong>
        <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px; margin-top:10px;">
          <button class="btn-secondary res-btn" data-res="1024x768" style="border:1px solid var(--accent);">1024 x 768 (активно)</button>
          <button class="btn-secondary res-btn" data-res="1280x720">1280 x 720 (HD)</button>
          <button class="btn-secondary res-btn" data-res="1280x800">1280 x 800 (WXGA)</button>
          <button class="btn-secondary res-btn" data-res="1920x1080">1920 x 1080 (FHD)</button>
        </div>
        <p id="res-status" style="color:var(--accent); margin-top:10px; font-size:12px;">Текущее: 1024x768 (активно)</p>
      </div>
      <div class="card">
        <strong>Интеграция VirtualBox:</strong>
        <p style="color:var(--success); margin-top:8px;">✔ VMMDev I/O порт 0xD020 : подключено</p>
        <p style="color:var(--success);">✔ Мышь : бесшовная интеграция</p>
        <p style="color:var(--success);">✔ Общие папки : /media/sf_shared</p>
        <p style="color:var(--success);">✔ Синхронизация времени с хостом : активно</p>
      </div>
      <div class="card">
        <strong>Интерфейс и тема:</strong>
        <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px; margin-top:8px;">
          <button class="btn-secondary theme-btn" data-theme="dark">Тёмная</button>
          <button class="btn-secondary theme-btn" data-theme="light">Светлая</button>
          <button class="btn-secondary theme-btn" data-theme="ocean">Океан</button>
          <button class="btn-secondary" onclick="logout()">Выйти из сеанса</button>
        </div>
      </div>
    </div>`;
  createWindow('Параметры и драйверы', '⚙️', 580, 500, html);
  document.querySelectorAll('.res-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const status = document.getElementById('res-status');
      document.querySelectorAll('.res-btn').forEach(b => b.style.border = '1px solid var(--border)');
      btn.style.border = '1px solid var(--accent)';
      if (status) status.textContent = 'Текущее: ' + btn.dataset.res + ' (активно)';
    });
  });
  document.querySelectorAll('.theme-btn').forEach(btn => {
    btn.addEventListener('click', () => applyTheme(btn.dataset.theme));
  });
}

// ---------------------------------------------------------------------------
// File manager
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
    { n: 'vmlinuz', t: 'Образ ядра', d: false, s: '8.7 KB' }
  ],
  '/home': [{ n: 'user', t: 'Домашний каталог пользователя', d: true }],
  '/home/user': [
    { n: 'Документы', t: 'Папка', d: true }, { n: 'wallpaper.bmp', t: 'Изображение', d: false, s: '4.2 MB' },
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
  const html = `
    <div style="display:flex; flex-direction:column; height:100%;">
      <div style="display:flex; gap:8px; margin-bottom:12px;">
        <button class="btn-secondary fm-back">&#8592;</button>
        <input type="text" id="fm-loc" value="Расположение: /" style="flex:1; background:var(--surface); border:1px solid var(--border); color:var(--text); padding:4px 8px; border-radius:6px;" readonly>
      </div>
      <table style="width:100%; border-collapse:collapse; font-size:13px;">
        <thead><tr style="color:var(--text-dim); border-bottom:1px solid var(--border); text-align:left;">
          <th style="padding:6px;">Имя</th><th>Тип</th><th>Размер</th>
        </tr></thead>
        <tbody id="${contentId}"></tbody>
      </table>
    </div>`;
  createWindow('Файловый менеджер', '📁', 580, 390, html, { contentId });
  let currentPath = '/';
  const body = document.getElementById(contentId);
  function render(path) {
    const loc = document.getElementById('fm-loc');
    loc.value = 'Расположение: ' + path;
    const items = fmTree[path] || [];
    body.innerHTML = '';
    if (path !== '/') {
      const up = document.createElement('tr');
      up.innerHTML = '<td style="padding:6px; color:var(--warn); cursor:pointer;">📁 ..</td><td>Вверх</td><td>DIR</td>';
      up.onclick = () => { const parts = path.split('/').filter(Boolean); parts.pop(); currentPath = '/' + parts.join('/'); render(currentPath); };
      body.appendChild(up);
    }
    items.forEach(it => {
      const tr = document.createElement('tr');
      const color = it.d ? 'var(--accent)' : 'var(--text)';
      tr.innerHTML = `<td style="padding:6px; color:${color}; cursor:pointer;">${it.d ? '📁' : '📄'} ${it.n}</td><td>${it.t}</td><td>${it.d ? 'DIR' : (it.s || '—')}</td>`;
      if (it.d) {
        tr.addEventListener('dblclick', () => {
          const newPath = (path === '/' ? '' : path) + '/' + it.n;
          if (fmTree[newPath]) { currentPath = newPath; render(currentPath); }
        });
      } else if (it.n === 'zero-release') {
        tr.addEventListener('dblclick', () => createEditorWindow('LinuxOSZero Genesis Edition v1.0.0 (x86_64)'));
      }
      body.appendChild(tr);
    });
  }
  render(currentPath);
}

// ---------------------------------------------------------------------------
// Editor
// ---------------------------------------------------------------------------
function createEditorWindow(initialText) {
  const defaultText = `/* LinuxOSZero — ядро и рабочий стол */
#include <zero/os.h>
#include <zero/vbox.h>

int main(void) {
    printf("Привет из LinuxOSZero!\\n");
    vbox_enable_mouse_integration(true);
    zerowm_start_session();
    return 0;
}
`;
  const html = `
    <div style="display:flex; flex-direction:column; height:100%;">
      <div style="display:flex; gap:8px; margin-bottom:8px;">
        <button class="btn-primary editor-save">Сохранить</button>
        <button class="btn-secondary">Открыть</button>
        <span style="color:var(--text-dim); margin-left:12px; align-self:center; font-size:12px;">main.c (исходный код C)</span>
      </div>
      <textarea class="editor-area" style="flex:1; width:100%; background:#090d16; color:#38bdf8; border:1px solid var(--border); border-radius:6px; padding:10px; font-family:monospace; resize:none;">${(initialText || defaultText).replace(/</g, '&lt;')}</textarea>
    </div>`;
  createWindow('Редактор — main.c', '📝', 580, 390, html);
  const save = document.querySelector('.editor-save');
  if (save) save.onclick = () => {
    const area = save.closest('.window').querySelector('.editor-area');
    area.style.borderColor = '#22c55e';
    setTimeout(() => area.style.borderColor = '', 800);
    alert('Файл сохранён!');
  };
}

// ---------------------------------------------------------------------------
// Fetch
// ---------------------------------------------------------------------------
function createFetchWindow() {
  const html = `
    <div style="display:flex; gap:20px;">
      <pre style="color:var(--accent); font-weight:bold; font-size:13px;">
       .---.
      /     \\
     | () () |
      \\  _  /
     .-'   '-.
    /  ZERO   \\
   |  LINUXOS  |
    \\  v1.0   /
     '-------'
      </pre>
      <div style="font-size:13px; line-height:1.7;">
        <div style="color:var(--accent); font-weight:bold;">user@linuxoszero</div>
        <div style="color:var(--text-dim);">--------------------------------</div>
        <div><strong>ОС</strong>         : LinuxOSZero 1.0.0 (Genesis)</div>
        <div><strong>Хост</strong>       : Oracle VM VirtualBox (vboxguest)</div>
        <div><strong>Ядро</strong>       : 6.1.0-zero-x86_64</div>
        <div><strong>Разрешение</strong> : 1024x768 (VMSVGA, ускорение)</div>
        <div><strong>WM</strong>         : ZeroWM (двойная буферизация)</div>
        <div><strong>Пакетов</strong>    : 42 (zpkg)</div>
        <div><strong>Память</strong>     : 245 MB / 2048 MB</div>
      </div>
    </div>`;
  createWindow('О системе', 'ℹ️', 540, 330, html);
}

// ---------------------------------------------------------------------------
// Welcome
// ---------------------------------------------------------------------------
function createWelcomeWindow() {
  const html = `
    <div style="display:flex; flex-direction:column; align-items:center; text-align:center; padding:12px;">
      <div style="width:64px;height:64px;border-radius:50%;border:3px solid var(--accent);display:flex;align-items:center;justify-content:center;font-size:32px;font-weight:900;color:var(--accent);box-shadow:0 0 20px var(--accent-glow);margin-bottom:12px;">Z</div>
      <h3>Добро пожаловать в LinuxOSZero</h3>
      <p style="color:var(--text-dim); margin-top:6px; font-size:13px;">Ваша собственная операционная система с поддержкой VirtualBox и графическим рабочим столом.</p>
      <div style="display:flex; gap:10px; margin-top:18px;">
        <button class="btn-primary" onclick="openApp('installer')">Установить ОС</button>
        <button class="btn-secondary" onclick="openApp('terminal')">Открыть терминал</button>
      </div>
    </div>`;
  createWindow('Добро пожаловать', '🚀', 480, 270, html);
}

// ---------------------------------------------------------------------------
// Installer wizard
// ---------------------------------------------------------------------------
let currentStep = 1;
function createInstallerWindow() {
  currentStep = 1;
  createWindow('Установка LinuxOSZero v1.0', '💾', 640, 440, getInstallerStepHtml(1), { contentId: 'installer-content' });
}
function getInstallerStepHtml(step) {
  let stepContent = '';
  if (step === 1) {
    stepContent = `
      <h3>Добро пожаловать в установку LinuxOSZero</h3>
      <p style="color:var(--text-dim); margin-top:6px;">Этот мастер установит LinuxOSZero на ваш виртуальный или физический диск.</p>
      <div class="card">
        <strong style="color:var(--accent);">Проверка совместимости:</strong>
        <p style="color:var(--success); margin-top:8px;">✔ Обнаружены драйверы VMMDev и VMSVGA VirtualBox</p>
        <p style="color:var(--success);">✔ ОЗУ 2048 MB подтверждено</p>
        <p style="color:var(--success);">✔ Доступен целевой диск (&gt;10 GB)</p>
        <p style="color:var(--success);">✔ Режим x86_64 и ACPI готов</p>
      </div>
      <div class="installer-actions"><button class="btn-primary" onclick="setInstallerStep(2)">Далее &gt;</button></div>`;
  } else if (step === 2) {
    stepContent = `
      <h3>Выбор целевого диска</h3>
      <p style="color:var(--text-dim); margin-top:6px;">Выберите диск для автоматической разметки и форматирования.</p>
      <div class="card" style="border-color:var(--accent);">
        <strong>📁 /dev/sda — 20.0 GB (диск VirtualBox VDI)</strong>
        <p style="color:var(--text-dim); font-size:12px; margin-top:4px;">Разметка: 512MB EFI/Загрузочный + 19.5GB ext4 RootFS</p>
      </div>
      <div class="installer-actions">
        <button class="btn-secondary" onclick="setInstallerStep(1)">&lt; Назад</button>
        <button class="btn-primary" onclick="setInstallerStep(3)">Далее &gt;</button>
      </div>`;
  } else if (step === 3) {
    stepContent = `
      <h3>Пользователь и система</h3>
      <p style="color:var(--text-dim); margin-top:6px;">Настройте пользователя и системные данные.</p>
      <div class="card">
        <p><strong>Имя хоста :</strong> linuxoszero</p>
        <p><strong>Пользователь :</strong> user</p>
        <p><strong>Пароль :</strong> zero (доступ sudo)</p>
        <p><strong>Часовой пояс :</strong> UTC (авто-синхронизация VirtualBox)</p>
      </div>
      <div class="installer-actions">
        <button class="btn-secondary" onclick="setInstallerStep(2)">&lt; Назад</button>
        <button class="btn-primary" onclick="setInstallerStep(4); startInstallSim();">Установить &gt;&gt;</button>
      </div>`;
  } else if (step === 4) {
    stepContent = `
      <h3>Установка LinuxOSZero...</h3>
      <p style="color:var(--text-dim); margin-top:6px;">Копирование ядра, rootfs и драйверов VirtualBox...</p>
      <div class="progress-bar-bg"><div id="inst-pbar" class="progress-bar-fill"></div></div>
      <p id="inst-status" style="color:var(--accent); font-size:13px; margin-top:8px;">Форматирование раздела ext4...</p>`;
  } else if (step === 5) {
    stepContent = `
      <h3 style="color:var(--success);">Установка завершена!</h3>
      <p style="color:var(--text-dim); margin-top:6px;">LinuxOSZero v1.0.0 успешно установлена на /dev/sda.</p>
      <div class="card" style="border-color:var(--success);">
        <p>✔ Ядро: LinuxOSZero 6.1 x86_64</p>
        <p>✔ Загрузчик: GRUB2 (MBR/EFI) установлен</p>
        <p>✔ Гостевые дополнения VirtualBox: настроены</p>
        <p>✔ Пользователь: user (пароль: zero)</p>
      </div>
      <div class="installer-actions">
        <button class="btn-primary" style="background:var(--success);" onclick="location.reload()">Перезагрузить</button>
      </div>`;
  }
  return `
    <div class="installer-box">
      <div class="installer-sidebar">
        <div class="inst-step ${step === 1 ? 'active' : (step > 1 ? 'done' : '')}"><div class="step-dot"></div> 1. Приветствие</div>
        <div class="inst-step ${step === 2 ? 'active' : (step > 2 ? 'done' : '')}"><div class="step-dot"></div> 2. Диск</div>
        <div class="inst-step ${step === 3 ? 'active' : (step > 3 ? 'done' : '')}"><div class="step-dot"></div> 3. Пользователь</div>
        <div class="inst-step ${step === 4 ? 'active' : (step > 4 ? 'done' : '')}"><div class="step-dot"></div> 4. Установка</div>
        <div class="inst-step ${step === 5 ? 'active' : (step > 5 ? 'done' : '')}"><div class="step-dot"></div> 5. Готово</div>
      </div>
      <div class="installer-main">${stepContent}</div>
    </div>`;
}
function setInstallerStep(step) {
  currentStep = step;
  const content = document.getElementById('installer-content');
  if (content) content.innerHTML = getInstallerStepHtml(step);
}
function startInstallSim() {
  let progress = 10;
  const pbar = document.getElementById('inst-pbar');
  const status = document.getElementById('inst-status');
  const timer = setInterval(() => {
    progress += 20;
    if (pbar) pbar.style.width = progress + '%';
    if (progress === 30 && status) status.textContent = 'Форматирование корневой ФС ext4...';
    if (progress === 50 && status) status.textContent = 'Распаковка системных библиотек и ZeroDesktop...';
    if (progress === 70 && status) status.textContent = 'Установка драйверов VMMDev и VMSVGA...';
    if (progress === 90 && status) status.textContent = 'Настройка загрузчика GRUB2...';
    if (progress >= 100) { clearInterval(timer); setTimeout(() => setInstallerStep(5), 500); }
  }, 400);
}

// =============================================================================
// LinuxOSZero - ZeroDesktop Live Simulator (Genesis Edition v1.0.0)
// Interactive browser preview of the LinuxOSZero desktop environment.
// =============================================================================

const windowsLayer = document.getElementById('windows-layer');
const taskList = document.getElementById('task-list');
const startMenu = document.getElementById('start-menu');
const clockElem = document.getElementById('clock');
const osContainer = document.getElementById('os-container');
const bootScreen = document.getElementById('boot-screen');

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
// Boot sequence
// ---------------------------------------------------------------------------
const BOOT_LINES = [
  { text: 'BIOS: Oracle VirtualBox hypervisor detected', cls: 'ok' },
  { text: 'VMMDev I/O port 0xD020 : connected', cls: 'ok' },
  { text: 'VBoxVideo (VMSVGA) 0x80EE:0xBEEF : initialized', cls: 'ok' },
  { text: 'Fast A20 gate ... enabled', cls: 'ok' },
  { text: 'Loading kernel 6.1.0-zero-x86_64', cls: '' },
  { text: 'Switching to long mode ... done', cls: 'ok' },
  { text: 'Mounting /proc /sys /dev /tmp ... done', cls: 'ok' },
  { text: 'Starting zero-init (PID 1)', cls: 'ok' },
  { text: 'Starting zero-guest-agent daemon ...', cls: 'ok' },
  { text: 'Starting ZeroDisplay framebuffer 1024x768x32', cls: 'ok' },
  { text: 'Starting ZeroDesktop & ZeroWM ...', cls: '' },
  { text: 'LinuxOSZero Genesis Edition v1.0.0 ready.', cls: 'ok' }
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
      setTimeout(() => {
        bootScreen.classList.add('boot-done');
        osContainer.classList.remove('hidden');
        setTimeout(() => bootScreen.remove(), 700);
      }, 400);
    }
  }, 120);
}
bootSequence();

// Skip boot on any key
document.addEventListener('keydown', () => {
  if (bootScreen && !bootScreen.classList.contains('boot-done')) {
    osContainer.classList.remove('hidden');
    bootScreen.classList.add('boot-done');
    setTimeout(() => bootScreen.remove(), 700);
  }
}, { once: true });

// ---------------------------------------------------------------------------
// Start menu
// ---------------------------------------------------------------------------
function toggleStartMenu() {
  startMenu.classList.toggle('hidden');
}

document.addEventListener('click', (e) => {
  if (!startMenu.contains(e.target) && !e.target.closest('#start-btn')) {
    startMenu.classList.add('hidden');
  }
});

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
      <div class="titlebar-left">
        <span>${icon}</span>
        <span>${title}</span>
      </div>
      <div class="win-controls">
        <button class="win-btn win-btn-min" onclick="minimizeWin('${id}')"></button>
        <button class="win-btn win-btn-max" onclick="maximizeWin('${id}')"></button>
        <button class="win-btn win-btn-close" onclick="closeWin('${id}')"></button>
      </div>
    </div>
    <div class="win-content" ${opts.contentId ? `id="${opts.contentId}"` : ''}>
      ${contentHtml}
    </div>
  `;

  win.addEventListener('mousedown', () => focusWin(id));
  windowsLayer.appendChild(win);

  const taskBtn = document.createElement('div');
  taskBtn.className = 'task-item active';
  taskBtn.id = 'task-' + id;
  taskBtn.innerHTML = `<span>${icon}</span> <span>${title}</span>`;
  taskBtn.onclick = () => {
    if (win.style.display === 'none') {
      win.style.display = 'flex';
      focusWin(id);
    } else if (win.classList.contains('active')) {
      minimizeWin(id);
    } else {
      focusWin(id);
    }
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
  if (windows[id]) {
    windows[id].elem.remove();
    windows[id].taskElem.remove();
    delete windows[id];
  }
}

function minimizeWin(id) {
  if (windows[id]) {
    windows[id].elem.style.display = 'none';
    windows[id].elem.classList.remove('active');
    windows[id].taskElem.classList.remove('active');
  }
}

function maximizeWin(id) {
  if (!windows[id]) return;
  const w = windows[id];
  if (!w.isMax) {
    w.prevLeft = w.elem.style.left;
    w.prevTop = w.elem.style.top;
    w.prevWidth = w.elem.style.width;
    w.prevHeight = w.elem.style.height;
    w.elem.style.left = '0px';
    w.elem.style.top = '0px';
    w.elem.style.width = '100vw';
    w.elem.style.height = 'calc(100vh - 40px)';
    w.isMax = true;
  } else {
    w.elem.style.left = w.prevLeft;
    w.elem.style.top = w.prevTop;
    w.elem.style.width = w.prevWidth;
    w.elem.style.height = w.prevHeight;
    w.isMax = false;
  }
}

let dragWin = null;
let dragOffX = 0;
let dragOffY = 0;

function startDrag(id, e) {
  if (e.target.closest('.win-controls')) return;
  dragWin = windows[id].elem;
  focusWin(id);
  const rect = dragWin.getBoundingClientRect();
  dragOffX = e.clientX - rect.left;
  dragOffY = e.clientY - rect.top;
  document.addEventListener('mousemove', onDrag);
  document.addEventListener('mouseup', stopDrag);
}

function onDrag(e) {
  if (dragWin) {
    dragWin.style.left = Math.max(0, e.clientX - dragOffX) + 'px';
    dragWin.style.top = Math.max(0, e.clientY - dragOffY) + 'px';
  }
}

function stopDrag() {
  dragWin = null;
  document.removeEventListener('mousemove', onDrag);
  document.removeEventListener('mouseup', stopDrag);
}

// ---------------------------------------------------------------------------
// Application launchers
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
// Interactive Terminal
// ---------------------------------------------------------------------------
const termState = { output: '' };

function termLine(text, cls = '') {
  return `<div class="${cls}">${text}</div>`;
}

function termPrompt() {
  return '<span class="term-prompt">user@linuxoszero:~$</span> ';
}

function runTermCommand(cmd, outputEl) {
  const c = cmd.trim();
  const p = c.split(/\s+/);
  const prog = p[0] || '';
  const args = p.slice(1);

  let out = '';
  const banner = `<div class="dim">LinuxOSZero Terminal v1.0.0 (x86_64) — type 'help'</div>`;

  switch (prog.toLowerCase()) {
    case '':
      break;
    case 'help':
      out = termLine(`Available commands:
  help          Show this help
  uname         Kernel & system info
  neofetch      System information (ASCII art)
  zero-hwprobe  VirtualBox hardware & driver status
  ls [path]     List directory contents
  cat <file>    Print file contents
  zpkg list     Installed packages
  echo <text>   Print text
  whoami        Show current user
  date          Show current date & time
  vboxstatus    VirtualBox guest integration status
  clear         Clear the terminal
  reboot        Restart the session`, '');
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
OS         : LinuxOSZero 1.0.0 (Genesis)
Host       : Oracle VM VirtualBox (vboxguest)
Kernel     : 6.1.0-zero-x86_64
Resolution : 1024x768 (VMSVGA Accelerated)
WM         : ZeroWM (Double-Buffered)
Packages   : 42 (zpkg)
Memory     : 245 MB / 2048 MB`);
      break;
    case 'zero-hwprobe':
      out = termLine(`[*] Hypervisor : <span class="term-success">Oracle VM VirtualBox</span>
[*] VMMDev     : <span class="term-success">I/O port 0xD020 connected</span>
[*] Video Mode : <span class="term-success">VMSVGA 1024x768x32 (Hardware Accelerated)</span>
[*] Mouse      : <span class="term-success">Seamless Pointer Integration Active</span>
[*] Shared Folders : /media/sf_shared`);
      break;
    case 'vboxstatus':
      out = termLine(`VMMDev     : <span class="term-success">CONNECTED</span>
Mouse      : <span class="term-success">Seamless integration</span>
Auto-resize: <span class="term-success">Enabled</span>
Shared Folders : /media/sf_shared
Clipboard  : <span class="term-success">Bidirectional</span>`);
      break;
    case 'ls':
      out = termLine(`
<span class="term-success">bin</span>  <span class="term-success">boot</span>  <span class="term-success">dev</span>  <span class="term-success">etc</span>  <span class="term-success">home</span>  <span class="term-success">media</span>  <span class="term-success">proc</span>  <span class="term-success">root</span>  <span class="term-success">sbin</span>  <span class="term-success">sys</span>  <span class="term-success">tmp</span>  <span class="term-success">usr</span>  <span class="term-success">var</span>
zero-init  zero-desktop  zero-guest-agent  zero-fetch  vmlinuz  initrd.img`);
      break;
    case 'cat':
      if (args[0] === '/etc/zero-release' || args[0] === 'zero-release') {
        out = termLine('LinuxOSZero Genesis Edition v1.0.0 (x86_64) — Custom OS with VirtualBox guest support');
      } else {
        out = termLine(`<span class="term-warn">cat: ${args[0] || ''}: No such file or directory</span>`);
      }
      break;
    case 'zpkg':
      if (args[0] === 'list') {
        out = termLine(`base-system (1.0.0)      zero-wm (1.0.0)
vbox-guest (6.1)      zero-installer (1.0)
zero-terminal (1.0)   zero-editor (1.0)
zero-filemanager (1.0) zpkg (1.0)`);
      } else {
        out = termLine('Usage: zpkg list | zpkg search <name> | zpkg install <pkg>');
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
    case 'clear':
      termState.output = '';
      outputEl.innerHTML = banner + '<br>';
      return;
    case 'reboot':
    case 'shutdown':
      out = termLine('<span class="term-warn">Power menu: please use the power button in the Start menu.</span>');
      break;
    default:
      out = termLine(`<span class="term-warn">zero: command not found: ${prog}</span> <span class="dim">(try 'help')</span>`);
  }

  if (out) outputEl.insertAdjacentHTML('beforeend', out);
  outputEl.scrollTop = outputEl.scrollHeight;
}

function createTerminalWindow() {
  const contentId = 'term-content-' + nextWinId;
  const html = `
    <div class="term-window">
      <div class="term-output" id="${contentId}"></div>
      <div class="term-input-row">
        <span class="term-prompt">user@linuxoszero:~$</span>
        <input class="term-input" id="term-input-${contentId}" autocomplete="off" spellcheck="false" placeholder="type 'help'">
      </div>
    </div>`;

  createWindow('Zero Terminal', '>_', 620, 380, html, { contentId: contentId });

  const outEl = document.getElementById(contentId);
  outEl.innerHTML = `<div class="dim">LinuxOSZero Terminal v1.0.0 (x86_64) — interactive shell</div><br>`;

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

  // Keep focus when clicking the terminal output
  outEl.addEventListener('click', () => inputEl.focus());
}

// ---------------------------------------------------------------------------
// Control Panel
// ---------------------------------------------------------------------------
function createControlPanelWindow() {
  const html = `
    <div style="display:flex; flex-direction:column; gap:16px;">
      <h3 style="color:#38bdf8;">VirtualBox Display & Guest Settings</h3>
      <div class="card">
        <strong>Display Resolution (VBoxVideo / VMSVGA):</strong>
        <div style="display:grid; grid-template-columns: 1fr 1fr; gap:8px; margin-top:10px;">
          <button class="btn-secondary res-btn" data-res="1024x768" style="border:1px solid #0ea5e9;">1024 x 768 (Active)</button>
          <button class="btn-secondary res-btn" data-res="1280x720">1280 x 720 (HD)</button>
          <button class="btn-secondary res-btn" data-res="1280x800">1280 x 800 (WXGA)</button>
          <button class="btn-secondary res-btn" data-res="1920x1080">1920 x 1080 (FHD)</button>
        </div>
        <p id="res-status" style="color:#38bdf8; margin-top:10px; font-size:12px;">Current: 1024x768 (Active)</p>
      </div>
      <div class="card">
        <strong>VirtualBox Guest Integration Status:</strong>
        <p style="color:#22c55e; margin-top:8px;">✔ VMMDev I/O Port 0xD020 : Connected</p>
        <p style="color:#22c55e;">✔ Mouse Pointer Integration : Seamless</p>
        <p style="color:#22c55e;">✔ Shared Folders : /media/sf_shared</p>
        <p style="color:#22c55e;">✔ Host RTC Clock Sync : Active</p>
      </div>
      <div class="card">
        <strong>Theme:</strong>
        <div style="display:flex; gap:8px; margin-top:8px;">
          <button class="btn-secondary theme-btn" data-theme="dark">Dark Cyber (Default)</button>
          <button class="btn-secondary theme-btn" data-theme="midnight">Midnight Blue</button>
          <button class="btn-secondary theme-btn" data-theme="emerald">Emerald</button>
        </div>
      </div>
    </div>`;

  createWindow('Control Panel & Drivers', '⚙️', 560, 460, html);

  // Resolution switching
  document.querySelectorAll('.res-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const status = document.getElementById('res-status');
      const res = btn.dataset.res;
      document.querySelectorAll('.res-btn').forEach(b => b.style.border = '1px solid #334155');
      btn.style.border = '1px solid #0ea5e9';
      if (status) status.textContent = 'Current: ' + res + ' (Active)';
    });
  });

  // Theme switching
  document.querySelectorAll('.theme-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const t = btn.dataset.theme;
      if (t === 'midnight') {
        document.querySelectorAll('.window').forEach(w => w.style.background = '#0f2440');
        document.querySelectorAll('.win-content').forEach(w => w.style.background = '#0b1a30');
      } else if (t === 'emerald') {
        document.querySelectorAll('.window').forEach(w => w.style.background = '#0f2e23');
        document.querySelectorAll('.win-content').forEach(w => w.style.background = '#0b2018');
      } else {
        document.querySelectorAll('.window').forEach(w => w.style.background = '');
        document.querySelectorAll('.win-content').forEach(w => w.style.background = '');
      }
    });
  });
}

// ---------------------------------------------------------------------------
// File Manager
// ---------------------------------------------------------------------------
const fmTree = {
  '/': [
    { n: 'bin', t: 'System Binaries', d: true }, { n: 'boot', t: 'Kernel & GRUB', d: true },
    { n: 'dev', t: 'Device Nodes', d: true }, { n: 'etc', t: 'Init & Config', d: true },
    { n: 'home', t: 'User Directory', d: true }, { n: 'media', t: 'vboxsf Mounts', d: true },
    { n: 'opt', t: 'Optional', d: true }, { n: 'proc', t: 'Proc FS', d: true },
    { n: 'root', t: 'Root Home', d: true }, { n: 'sys', t: 'Sys FS', d: true },
    { n: 'tmp', t: 'Temp', d: true }, { n: 'usr', t: 'Userland', d: true },
    { n: 'var', t: 'Variable Data', d: true },
    { n: 'zero-init', t: 'ZeroInit (PID 1)', d: false, s: '128 KB' },
    { n: 'vmlinuz', t: 'Kernel Image', d: false, s: '8.7 KB' }
  ],
  '/home': [{ n: 'user', t: 'Default User Home', d: true }],
  '/home/user': [
    { n: 'Documents', t: 'Folder', d: true }, { n: 'wallpaper.bmp', t: 'Bitmap', d: false, s: '4.2 MB' },
    { n: 'hello.c', t: 'C Source', d: false, s: '1.1 KB' }, { n: 'zero.conf', t: 'Config', d: false, s: '620 B' }
  ],
  '/etc': [
    { n: 'inittab', t: 'Init Config', d: false, s: '1.2 KB' },
    { n: 'zero-release', t: 'OS Release', d: false, s: '160 B' },
    { n: 'fstab', t: 'Mount Table', d: false, s: '420 B' },
    { n: 'vboxsf', t: 'Shared Folder Config', d: true }
  ]
};

function createFileManagerWindow() {
  const contentId = 'fm-content-' + nextWinId;
  const html = `
    <div style="display:flex; flex-direction:column; height:100%;">
      <div style="display:flex; gap:8px; margin-bottom:12px;">
        <button class="btn-secondary fm-back">&#8592;</button>
        <input type="text" id="fm-loc" value="Location: /" style="flex:1; background:#1e293b; border:1px solid #334155; color:#fff; padding:4px 8px; border-radius:4px;" readonly>
      </div>
      <table style="width:100%; border-collapse:collapse; font-size:13px;">
        <thead><tr style="color:#94a3b8; border-bottom:1px solid #334155; text-align:left;">
          <th style="padding:6px;">Name</th><th>Type</th><th>Size</th>
        </tr></thead>
        <tbody id="${contentId}"></tbody>
      </table>
    </div>`;

  createWindow('Zero File Manager', '📁', 560, 380, html, { contentId: contentId });

  let currentPath = '/';
  const body = document.getElementById(contentId);

  function render(path) {
    const loc = document.getElementById('fm-loc');
    loc.value = 'Location: ' + path;
    const items = fmTree[path] || [];
    body.innerHTML = '';
    if (path !== '/') {
      const up = document.createElement('tr');
      up.innerHTML = '<td style="padding:6px; color:#eab308; cursor:pointer;">📁 ..</td><td>Parent</td><td>DIR</td>';
      up.onclick = () => {
        const parts = path.split('/').filter(Boolean);
        parts.pop();
        currentPath = '/' + parts.join('/');
        render(currentPath);
      };
      body.appendChild(up);
    }
    items.forEach(it => {
      const tr = document.createElement('tr');
      const color = it.d ? '#38bdf8' : '#f8fafc';
      tr.innerHTML = `<td style="padding:6px; color:${color}; cursor:pointer;">${it.d ? '📁' : '📄'} ${it.n}</td><td>${it.t}</td><td>${it.d ? 'DIR' : (it.s || '—')}</td>`;
      if (it.d) {
        tr.style.cursor = 'pointer';
        tr.addEventListener('dblclick', () => {
          const newPath = (path === '/' ? '' : path) + '/' + it.n;
          if (fmTree[newPath]) { currentPath = newPath; render(currentPath); }
        });
      } else if (it.n === 'zero-release') {
        tr.style.cursor = 'pointer';
        tr.addEventListener('dblclick', () => {
          createEditorWindow('LinuxOSZero Genesis Edition v1.0.0 (x86_64)');
        });
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
  const defaultText = `/* LinuxOSZero - Custom OS Kernel & Desktop */
#include <zero/os.h>
#include <zero/vbox.h>

int main(void) {
    printf("Hello from LinuxOSZero!\\n");
    vbox_enable_mouse_integration(true);
    zerowm_start_session();
    return 0;
}
`;
  const html = `
    <div style="display:flex; flex-direction:column; height:100%;">
      <div style="display:flex; gap:8px; margin-bottom:8px;">
        <button class="btn-primary editor-save">Save</button>
        <button class="btn-secondary">Open</button>
        <span style="color:#94a3b8; margin-left:12px; align-self:center; font-size:12px;">main.c (C Source)</span>
      </div>
      <textarea class="editor-area" style="flex:1; width:100%; background:#090d16; color:#38bdf8; border:1px solid #334155; border-radius:4px; padding:10px; font-family:monospace; resize:none;">${(initialText || defaultText).replace(/</g, '&lt;')}</textarea>
    </div>`;

  createWindow('Zero Editor - main.c', '📝', 560, 380, html);
  const save = document.querySelector('.editor-save');
  if (save) save.onclick = () => {
    const area = save.closest('.window').querySelector('.editor-area');
    area.style.borderColor = '#22c55e';
    setTimeout(() => area.style.borderColor = '#334155', 800);
    alert('File saved successfully!');
  };
}

// ---------------------------------------------------------------------------
// ZeroFetch
// ---------------------------------------------------------------------------
function createFetchWindow() {
  const html = `
    <div style="display:flex; gap:20px;">
      <pre style="color:#38bdf8; font-weight:bold; font-size:13px;">
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
        <div style="color:#38bdf8; font-weight:bold;">user@linuxoszero</div>
        <div style="color:#64748b;">--------------------------------</div>
        <div><strong>OS</strong>         : LinuxOSZero 1.0.0 (Genesis)</div>
        <div><strong>Host</strong>       : Oracle VM VirtualBox (vboxguest)</div>
        <div><strong>Kernel</strong>     : 6.1.0-zero-x86_64</div>
        <div><strong>Resolution</strong> : 1024x768 (VMSVGA Accelerated)</div>
        <div><strong>WM</strong>         : ZeroWM (Double-Buffered)</div>
        <div><strong>Packages</strong>   : 42 (zpkg)</div>
        <div><strong>Memory</strong>     : 245 MB / 2048 MB</div>
      </div>
    </div>`;
  createWindow('ZeroFetch - System Info', 'ℹ️', 520, 320, html);
}

// ---------------------------------------------------------------------------
// Welcome window (extra polish)
// ---------------------------------------------------------------------------
function createWelcomeWindow() {
  const html = `
    <div style="display:flex; flex-direction:column; align-items:center; text-align:center; padding:12px;">
      <div style="width:64px;height:64px;border-radius:50%;border:3px solid #0ea5e9;display:flex;align-items:center;justify-content:center;font-size:32px;font-weight:900;color:#38bdf8;box-shadow:0 0 20px rgba(14,165,233,.5);margin-bottom:12px;">Z</div>
      <h3>Welcome to LinuxOSZero</h3>
      <p style="color:#94a3b8; margin-top:6px; font-size:13px;">Your custom Operating System with full VirtualBox support and a graphical desktop.</p>
      <div style="display:flex; gap:10px; margin-top:18px;">
        <button class="btn-primary" onclick="openApp('installer')">Install OS</button>
        <button class="btn-secondary" onclick="openApp('terminal')">Open Terminal</button>
      </div>
    </div>`;
  createWindow('Welcome', '🚀', 460, 260, html);
}

// ---------------------------------------------------------------------------
// Installer wizard
// ---------------------------------------------------------------------------
let currentStep = 1;

function createInstallerWindow() {
  currentStep = 1;
  createWindow('Install LinuxOSZero v1.0', '💾', 620, 430, getInstallerStepHtml(1), { contentId: 'installer-content' });
}

function getInstallerStepHtml(step) {
  let stepContent = '';
  if (step === 1) {
    stepContent = `
      <h3>Welcome to LinuxOSZero Installation</h3>
      <p style="color:#94a3b8; margin-top:6px;">This setup wizard will install LinuxOSZero onto your virtual or physical disk.</p>
      <div class="card">
        <strong style="color:#38bdf8;">System Compatibility Verification:</strong>
        <p style="color:#22c55e; margin-top:8px;">✔ VirtualBox VMMDev & VMSVGA Drivers detected</p>
        <p style="color:#22c55e;">✔ 2048 MB System RAM verified</p>
        <p style="color:#22c55e;">✔ Target Storage Drive available (&gt;10 GB)</p>
        <p style="color:#22c55e;">✔ x86_64 Long Mode & ACPI support ready</p>
      </div>
      <div class="installer-actions"><button class="btn-primary" onclick="setInstallerStep(2)">Next &gt;</button></div>`;
  } else if (step === 2) {
    stepContent = `
      <h3>Select Target Storage Drive</h3>
      <p style="color:#94a3b8; margin-top:6px;">Choose the destination drive for automatic partitioning and formatting.</p>
      <div class="card" style="border-color:#0ea5e9;">
        <strong>📁 /dev/sda - 20.0 GB (VirtualBox VDI Disk)</strong>
        <p style="color:#94a3b8; font-size:12px; margin-top:4px;">Partition layout: 512MB EFI/Boot + 19.5GB ext4 RootFS</p>
      </div>
      <div class="installer-actions">
        <button class="btn-secondary" onclick="setInstallerStep(1)">&lt; Back</button>
        <button class="btn-primary" onclick="setInstallerStep(3)">Next &gt;</button>
      </div>`;
  } else if (step === 3) {
    stepContent = `
      <h3>User & System Configuration</h3>
      <p style="color:#94a3b8; margin-top:6px;">Set up default user and administrative credentials.</p>
      <div class="card">
        <p><strong>Hostname   :</strong> linuxoszero</p>
        <p><strong>Username   :</strong> user</p>
        <p><strong>Password   :</strong> zero (sudo enabled)</p>
        <p><strong>Timezone   :</strong> UTC (VirtualBox Auto-sync)</p>
      </div>
      <div class="installer-actions">
        <button class="btn-secondary" onclick="setInstallerStep(2)">&lt; Back</button>
        <button class="btn-primary" onclick="setInstallerStep(4); startInstallSim();">Install Now &gt;&gt;</button>
      </div>`;
  } else if (step === 4) {
    stepContent = `
      <h3>Installing LinuxOSZero...</h3>
      <p style="color:#94a3b8; margin-top:6px;">Copying kernel, rootfs, and VirtualBox guest drivers...</p>
      <div class="progress-bar-bg"><div id="inst-pbar" class="progress-bar-fill"></div></div>
      <p id="inst-status" style="color:#38bdf8; font-size:13px; margin-top:8px;">Formatting ext4 partition...</p>`;
  } else if (step === 5) {
    stepContent = `
      <h3 style="color:#22c55e;">Installation Complete!</h3>
      <p style="color:#94a3b8; margin-top:6px;">LinuxOSZero v1.0.0 is successfully installed on /dev/sda.</p>
      <div class="card" style="border-color:#22c55e;">
        <p>✔ Kernel: LinuxOSZero 6.1 x86_64</p>
        <p>✔ Bootloader: GRUB2 Hybrid MBR/EFI installed</p>
        <p>✔ VirtualBox Guest Additions: Configured</p>
        <p>✔ Default User: user (password: zero)</p>
      </div>
      <div class="installer-actions">
        <button class="btn-primary" style="background:#22c55e;" onclick="rebootSession()">Reboot Now</button>
      </div>`;
  }

  return `
    <div class="installer-box">
      <div class="installer-sidebar">
        <div class="inst-step ${step === 1 ? 'active' : (step > 1 ? 'done' : '')}"><div class="step-dot"></div> 1. Welcome</div>
        <div class="inst-step ${step === 2 ? 'active' : (step > 2 ? 'done' : '')}"><div class="step-dot"></div> 2. Disk Setup</div>
        <div class="inst-step ${step === 3 ? 'active' : (step > 3 ? 'done' : '')}"><div class="step-dot"></div> 3. User Config</div>
        <div class="inst-step ${step === 4 ? 'active' : (step > 4 ? 'done' : '')}"><div class="step-dot"></div> 4. Install</div>
        <div class="inst-step ${step === 5 ? 'active' : (step > 5 ? 'done' : '')}"><div class="step-dot"></div> 5. Finished</div>
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
    if (progress === 30 && status) status.textContent = 'Formatting ext4 root filesystem...';
    if (progress === 50 && status) status.textContent = 'Extracting system libraries and ZeroDesktop...';
    if (progress === 70 && status) status.textContent = 'Installing VirtualBox VMMDev & VMSVGA drivers...';
    if (progress === 90 && status) status.textContent = 'Configuring GRUB2 bootloader...';
    if (progress >= 100) {
      clearInterval(timer);
      setTimeout(() => setInstallerStep(5), 500);
    }
  }, 400);
}

function rebootSession() {
  location.reload();
}

// ---------------------------------------------------------------------------
// Open default apps on launch
// ---------------------------------------------------------------------------
window.addEventListener('DOMContentLoaded', () => {
  // Apps open after the boot sequence reveals the desktop
  setTimeout(() => {
    openApp('welcome');
    openApp('terminal');
  }, 750);
});

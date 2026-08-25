// =============================================================================
// LinuxOSZero — ZeroDesktop (Titan v1.1.0 x86_64)
// Interactive desktop: Display & Screen Settings, Hardware Driver Installer,
// App Store, working Browser, Discord, Steam, Games, Terminal,
// File Manager, Editor, Network settings, 3D demo.
// =============================================================================

const $ = id => document.getElementById(id);
const bootScreen = $('boot-screen'), loginScreen = $('login-screen');
const osContainer = $('os-container'), windowsLayer = $('windows-layer');
const taskList = $('task-list'), startMenu = $('start-menu'), startItems = $('start-items');
const clockElem = $('clock'), desktopIcons = $('desktop-icons');

let zIndexCount = 100, windows = {}, nextWinId = 1;
let bootMode = 'live', grubIndex = 0;
let currentUser = localStorage.getItem('zero-user') || 'user';
let online = true;
let currentRes = localStorage.getItem('zero-res') || '1024x768';
let currentScale = localStorage.getItem('zero-scale') || '100';

function getUsername() { return currentUser; }
function getUserInitials() { return (currentUser[0] || 'U').toUpperCase(); }

// ---------------------------------------------------------------------------
// Window manager
// ---------------------------------------------------------------------------
function createWindow(title, iconName, width, height, contentHtml, opts = {}) {
  const id = 'win-' + nextWinId++;
  const win = document.createElement('div');
  win.className = 'window active'; win.id = id;
  const offset = (nextWinId * 24) % 110;
  const left = Math.max(24, (window.innerWidth - width) / 2 + offset);
  const top = Math.max(20, (window.innerHeight - height) / 2 - 30 + offset);
  win.style.cssText = `width:${width}px;height:${height}px;left:${left}px;top:${top}px;z-index:${++zIndexCount};`;
  win.innerHTML = `
    <div class="titlebar" onmousedown="startDrag('${id}', event)" ondblclick="maximizeWin('${id}')">
      <div class="titlebar-left"><span class="title-icon">${svgIcon(iconName,16)}</span><span>${title}</span></div>
      <div class="win-controls">
        <button class="win-btn win-btn-min" onclick="minimizeWin('${id}')"></button>
        <button class="win-btn win-btn-max" onclick="maximizeWin('${id}')"></button>
        <button class="win-btn win-btn-close" onclick="closeWin('${id}')"></button>
      </div>
    </div>
    <div class="win-content" ${opts.contentId?`id="${opts.contentId}"`:''}>${contentHtml}</div>`;
  win.addEventListener('mousedown', () => focusWin(id));
  windowsLayer.appendChild(win);
  const tb = document.createElement('div');
  tb.className = 'task-item active'; tb.id = 'task-'+id;
  tb.innerHTML = `<span class="task-icon">${svgIcon(iconName,14)}</span> <span>${title}</span>`;
  tb.onclick = () => { if(win.style.display==='none'){win.style.display='flex';focusWin(id);} else if(win.classList.contains('active')) minimizeWin(id); else focusWin(id); };
  taskList.appendChild(tb);
  windows[id] = { elem:win, taskElem:tb, isMax:false };
  focusWin(id);
  return id;
}
function focusWin(id){ document.querySelectorAll('.window').forEach(w=>w.classList.remove('active')); document.querySelectorAll('.task-item').forEach(t=>t.classList.remove('active')); if(windows[id]){ windows[id].elem.classList.add('active'); windows[id].elem.style.zIndex=++zIndexCount; windows[id].taskElem.classList.add('active'); } }
function closeWin(id){ if(windows[id]){ windows[id].elem.remove(); windows[id].taskElem.remove(); delete windows[id]; } }
function minimizeWin(id){ if(windows[id]){ windows[id].elem.style.display='none'; windows[id].elem.classList.remove('active'); windows[id].taskElem.classList.remove('active'); } }
function maximizeWin(id){ if(!windows[id])return; const w=windows[id]; if(!w.isMax){ w.prevLeft=w.elem.style.left;w.prevTop=w.elem.style.top;w.prevWidth=w.elem.style.width;w.prevHeight=w.elem.style.height; w.elem.style.left='0px';w.elem.style.top='0px';w.elem.style.width='100vw';w.elem.style.height='calc(100vh - 44px)';w.isMax=true;} else { w.elem.style.left=w.prevLeft;w.elem.style.top=w.prevTop;w.elem.style.width=w.prevWidth;w.elem.style.height=w.prevHeight;w.isMax=false; } }
let dragWin=null,dragOffX=0,dragOffY=0;
function startDrag(id,e){ if(e.target.closest('.win-controls'))return; dragWin=windows[id].elem;focusWin(id);const r=dragWin.getBoundingClientRect();dragOffX=e.clientX-r.left;dragOffY=e.clientY-r.top;document.addEventListener('mousemove',onDrag);document.addEventListener('mouseup',stopDrag); }
function onDrag(e){ if(dragWin){dragWin.style.left=Math.max(0,e.clientX-dragOffX)+'px';dragWin.style.top=Math.max(0,e.clientY-dragOffY)+'px';} }
function stopDrag(){ dragWin=null;document.removeEventListener('mousemove',onDrag);document.removeEventListener('mouseup',stopDrag); }
function toggleStartMenu(){ startMenu.classList.toggle('hidden'); }
document.addEventListener('click',e=>{ if(!startMenu.contains(e.target)&&!e.target.closest('#start-btn')) startMenu.classList.add('hidden'); });
$('start-btn').addEventListener('click',toggleStartMenu);
$('power-btn').addEventListener('click',()=>{ Object.keys(windows).forEach(id=>closeWin(id)); osContainer.classList.add('hidden'); loginScreen.classList.remove('hidden'); });

// Clock
function updateClock(){ const n=new Date(); clockElem.textContent=n.toTimeString().split(' ')[0]; }
setInterval(updateClock,1000); updateClock();

// Theme
function applyTheme(t){ document.body.setAttribute('data-theme',t); localStorage.setItem('zero-theme',t); }

// Resolution & Screen scaling
function applyResolution(res, scale) {
  currentRes = res;
  currentScale = scale || currentScale;
  localStorage.setItem('zero-res', res);
  localStorage.setItem('zero-scale', currentScale);
  const badge = $('res-badge');
  if (badge) badge.textContent = res;
}

// ---------------------------------------------------------------------------
// App registry + desktop/menu
// ---------------------------------------------------------------------------
const APPS = [
  { id:'installer',  name:'Установщик ОС',       icon:'installer',   launch:'installer' },
  { id:'display',    name:'Настройка экрана',    icon:'screen',      launch:'display' },
  { id:'store',      name:'Магазин приложений',  icon:'store',       launch:'store' },
  { id:'browser',    name:'Браузер',             icon:'browser',     launch:'browser' },
  { id:'discord',    name:'Discord',             icon:'discord',     launch:'discord' },
  { id:'steam',      name:'Steam',               icon:'steam',       launch:'steam' },
  { id:'filemanager',name:'Файлы',               icon:'filemanager', launch:'filemanager' },
  { id:'terminal',   name:'Терминал',            icon:'terminal',    launch:'terminal' },
  { id:'editor',     name:'Редактор',            icon:'editor',      launch:'editor' },
  { id:'internet',   name:'Настройки сети',      icon:'wifi',        launch:'internet' },
  { id:'gpu',        name:'3D / 2D Драйверы',    icon:'chip',        launch:'gpu' },
  { id:'games',      name:'Игры',                icon:'gamepad',     launch:'games' },
  { id:'analytics',  name:'Монитор системы',     icon:'analytics',   launch:'analytics' },
  { id:'calculator', name:'Калькулятор',         icon:'calculator',  launch:'calculator' },
  { id:'fetch',      name:'О системе',           icon:'info',        launch:'fetch' },
  { id:'user',       name:'Пользователь',        icon:'settings',    launch:'user' }
];

function openApp(n){
  switch(n){
    case 'installer': createInstallerWindow(); break;
    case 'display': createDisplayWindow(); break;
    case 'store': createStoreWindow(); break;
    case 'browser': createBrowserWindow(); break;
    case 'discord': createDiscordWindow(); break;
    case 'steam': createSteamWindow(); break;
    case 'filemanager': createFileManagerWindow(); break;
    case 'terminal': createTerminalWindow(); break;
    case 'editor': createEditorWindow(); break;
    case 'internet': createInternetWindow(); break;
    case 'gpu': createGpuWindow(); break;
    case 'games': createGamesWindow(); break;
    case 'analytics': createAnalyticsWindow(); break;
    case 'calculator': createCalculatorWindow(); break;
    case 'fetch': createFetchWindow(); break;
    case 'user': createUserWindow(); break;
    case 'welcome': createWelcomeWindow(); break;
  }
}

function buildDesktopAndMenu(){
  desktopIcons.innerHTML = APPS.map(a=>`<div class="desktop-icon" data-app="${a.launch}"><div class="icon-img">${svgIcon(a.icon,30)}</div><span>${a.name}</span></div>`).join('');
  desktopIcons.querySelectorAll('.desktop-icon').forEach(el=>el.addEventListener('click',()=>openApp(el.dataset.app)));
  startItems.innerHTML = APPS.map(a=>`<div class="start-item" data-app="${a.launch}"><span class="start-icon">${svgIcon(a.icon,20)}</span><div><div class="item-title">${a.name}</div></div></div>`).join('');
  startItems.querySelectorAll('.start-item').forEach(el=>el.addEventListener('click',()=>{openApp(el.dataset.app);toggleStartMenu();}));
  $('net-icon').innerHTML = svgIcon(online?'wifi':'network',16);
  $('audio-icon').innerHTML = svgIcon('audio',16);
  $('start-search-icon').innerHTML = svgIcon('search',14);
  $('start-user').textContent = currentUser + '@linuxoszero';

  // Add resolution badge to system tray if not present
  if (!$('res-badge')) {
    const rBadge = document.createElement('div');
    rBadge.className = 'tray-badge';
    rBadge.id = 'res-badge';
    rBadge.style.cursor = 'pointer';
    rBadge.style.color = '#38bdf8';
    rBadge.title = 'Настройка разрешения экрана';
    rBadge.textContent = currentRes;
    rBadge.onclick = () => openApp('display');
    $('system-tray').insertBefore(rBadge, $('hv-badge'));
  }
}
$('start-search').addEventListener('input',e=>{ const q=e.target.value.toLowerCase(); startItems.querySelectorAll('.start-item').forEach(it=>it.style.display=it.textContent.toLowerCase().includes(q)?'':'none'); });

// ---------------------------------------------------------------------------
// Boot flow
// ---------------------------------------------------------------------------
const GRUB_ITEMS = [
  { name:'Запустить LinuxOSZero Titan (графический режим)', mode:'live' },
  { name:'Установить LinuxOSZero (мастер установки драйверов и ОС)', mode:'install' },
  { name:'LinuxOSZero — безопасная графика (VESA VBE LFB)', mode:'safe' },
  { name:'LinuxOSZero — консоль восстановления', mode:'rescue' },
  { name:'Перезагрузка', mode:'reboot' }, { name:'Выключение', mode:'poweroff' }
];
function initGrub(){
  $('grub-menu').innerHTML = GRUB_ITEMS.map((it,i)=>`<div class="grub-item ${i===0?'selected':''}" data-i="${i}"><span class="grub-caret">&#9656;</span>${it.name}</div>`).join('');
  $('grub-menu').querySelectorAll('.grub-item').forEach(el=>el.addEventListener('click',()=>{grubIndex=+el.dataset.i;selectGrub(grubIndex);grubBoot(GRUB_ITEMS[grubIndex].mode);}));
  document.addEventListener('keydown',e=>{
    if(!$('grub-screen').classList.contains('hidden')){
      if(e.key==='ArrowDown'||e.key==='j'){grubIndex=(grubIndex+1)%GRUB_ITEMS.length;selectGrub(grubIndex);}
      else if(e.key==='ArrowUp'||e.key==='k'){grubIndex=(grubIndex-1+GRUB_ITEMS.length)%GRUB_ITEMS.length;selectGrub(grubIndex);}
      else if(e.key==='Enter') grubBoot(GRUB_ITEMS[grubIndex].mode);
    }
  });
}
function selectGrub(i){ document.querySelectorAll('.grub-item').forEach((el,idx)=>el.classList.toggle('selected',idx===i)); }
function grubBoot(mode){
  bootMode=mode;
  $('grub-screen').classList.add('hidden');
  $('loading-screen').classList.remove('hidden');
  let p=0; const msgs=['Загрузка 64-битного ядра…','Инициализация драйвера VMSVGA & VMMDev…','Запуск служб и подсистем…','Подготовка рабочего стола Titan…'];
  const t=setInterval(()=>{ p+=8; $('loading-fill').style.width=Math.min(p,100)+'%'; $('loading-text').textContent='Loading LinuxOSZero… '+Math.min(p,100)+'%'; $('loading-sub').textContent=msgs[Math.floor(p/25)%msgs.length]; if(p>=100){clearInterval(t);$('loading-screen').classList.add('hidden');setTimeout(bootSequence,250);} },60);
}
const BOOT_LINES=[
  {text:'BIOS: запуск GRUB2 (LinuxOSZero Titan x86_64)',cls:'ok'},
  {text:'PML4/PDPT: 4-уровневые таблицы страниц Long Mode загружены',cls:'ok'},
  {text:'PCI: шина сканирована … Oracle VirtualBox VMMDev + VMSVGA найдены',cls:'ok'},
  {text:'Дисплей: VMSVGA 1024x768x32 Linear Framebuffer активен',cls:'ok'},
  {text:'Клавиатура: PS/2 i8042 контроллер (Скан-коды Set 1/2 + US/RU)',cls:'ok'},
  {text:'Монтирование /proc /sys /dev /media/sf_shared … готово',cls:'ok'},
  {text:'Сеть: интерфейс eth0 Intel 82540EM настроен (DHCP)',cls:'ok'},
  {text:'Звук: Intel 82801AA AC\'97 инициализирован',cls:'ok'},
  {text:'Запуск zero-init (PID 1) и ZeroDesktop',cls:'ok'},
  {text:'LinuxOSZero Titan v1.1.0 готова к работе.',cls:'ok'}
];
function bootSequence(){
  bootScreen.classList.remove('hidden');
  const log=$('boot-log'); const pbar=$('boot-pbar'); log.innerHTML=''; let i=0;
  const t=setInterval(()=>{ if(i<BOOT_LINES.length){ const line=BOOT_LINES[i]; const d=document.createElement('div'); d.innerHTML=`<span class="dim">[</span><span class="${line.cls||'dim'}">OK</span><span class="dim">]</span> ${line.text}`; log.appendChild(d); while(log.childElementCount>10)log.removeChild(log.firstChild); log.scrollTop=log.scrollHeight; pbar.style.width=Math.round(((i+1)/BOOT_LINES.length)*100)+'%'; i++; } else { clearInterval(t); setTimeout(showLogin,350); } },110);
}
initGrub();
function showLogin(){
  bootScreen.classList.add('boot-done'); loginScreen.classList.remove('hidden');
  applyTheme(localStorage.getItem('zero-theme')||'dark'); $('login-theme').value=document.body.dataset.theme;
  $('login-username').textContent=currentUser; $('login-avatar').textContent=getUserInitials();
  $('login-status').textContent='● Система готова, драйверы VirtualBox активны';
  setTimeout(()=>bootScreen.remove(),600);
}
document.addEventListener('keydown',e=>{ if((e.code==='Space'||e.code==='Escape')&&loginScreen.classList.contains('hidden')){ $('grub-screen').classList.add('hidden'); $('loading-screen').classList.add('hidden'); showLogin(); } });
$('login-theme').addEventListener('change',e=>applyTheme(e.target.value));
$('login-btn').addEventListener('click',()=>{ applyTheme($('login-theme').value); loginScreen.classList.add('hidden'); if($('login-session').value==='terminal') showTextSession(); else showDesktop(); });
$('login-pass').addEventListener('keydown',e=>{ if(e.key==='Enter') $('login-btn').click(); });

function showDesktop(){ osContainer.classList.remove('hidden'); openApp('welcome'); if(bootMode==='install') openApp('installer'); else openApp('terminal'); }

// Text session
function showTextSession(){
  const tty=document.createElement('div'); tty.id='text-session';
  tty.style.cssText='position:fixed;inset:0;z-index:3000;background:#05070d;color:#e2e8f0;font-family:"Courier New",monospace;font-size:14px;line-height:1.5;display:flex;flex-direction:column;padding:14px 18px;';
  tty.innerHTML=`<div style="color:#38bdf8;font-weight:bold;border-bottom:1px solid #1e293b;padding-bottom:8px;">LinuxOSZero — текстовый режим <span style="color:#22c55e;">TTY</span> (введите 'help' или 'screen')</div><div id="tty-output" style="flex:1;overflow-y:auto;white-space:pre-wrap;margin-top:8px;"></div><div style="display:flex;"><span class="term-prompt">${currentUser}@linuxoszero:~$</span><input id="tty-input" style="flex:1;background:transparent;border:none;outline:none;color:#e2e8f0;font-family:inherit;font-size:14px;caret-color:#38bdf8;" autocomplete="off" spellcheck="false"></div>`;
  document.body.appendChild(tty);
  const out=$('tty-output'); out.innerHTML=''; const input=$('tty-input'); setTimeout(()=>input.focus(),60);
  input.addEventListener('keydown',e=>{ if(e.key==='Enter'){ const v=input.value; out.insertAdjacentHTML('beforeend',`<span class="term-prompt">${currentUser}@linuxoszero:~$</span> `+v.replace(/</g,'&lt;')+'\n'); runTermCommand(v,out); out.scrollTop=out.scrollHeight; input.value=''; } });
}

// ---------------------------------------------------------------------------
// Terminal command engine (with real VFS file creation)
// ---------------------------------------------------------------------------
function termLine(t,c=''){ return `<div class="${c}">${t}</div>`; }
function runTermCommand(cmd,out){
  const c=cmd.trim(); if(!c)return;
  const parts=c.split(/\s+/); const rawProg=parts[0].toLowerCase(); const prog=rawProg.startsWith('/')?rawProg.slice(1):rawProg; const args=parts.slice(1);
  let res='';
  switch(prog){
    case 'help': case '?':
      res=termLine(`
<span class="term-prompt" style="font-weight:bold;">=== LinuxOSZero v1.1.0 (Titan) Доступные Команды ===</span>
<span style="color:#eab308;font-weight:bold;">[СИСТЕМА]</span>
  <b>uname -a</b>        - Архитектура ядра и версия ОС
  <b>fetch / neofetch</b> - Системная информация и цветной логотип
  <b>whoami</b>          - Текущий пользователь и права доступа
  <b>uptime</b>          - Время непрерывной работы системы
  <b>date</b>            - Текущая дата и системное время
  <b>free</b>            - Использование оперативной памяти (RAM)
  <b>ps</b>              - Список активных процессов
  <b>clear</b>           - Очистить экран терминала
<span style="color:#22c55e;font-weight:bold;">[НАСТРОЙКА ЭКРАНА И ДРАЙВЕРЫ]</span>
  <b>screen / display</b> - <span style="color:#38bdf8;">Настройка разрешения экрана и видеорежимов</span>
  <b>screen &lt;1280x720|1920x1080|1024x768|auto&gt;</b> - Изменить разрешение экрана
  <b>driver-install</b>   - <span style="color:#22c55e;">Автоматический интерактивный установщик драйверов</span> (/install)
  <b>vbox</b>             - Диагностика VirtualBox VMMDev и VMSVGA
  <b>pci</b>              - Сканирование и список устройств на шине PCI
  <b>video</b>            - Разрешение экрана и 3D-ускоритель VMSVGA
  <b>audio</b>            - Статус звукового контроллера Intel AC'97
  <b>layout &lt;en|ru&gt;</b>   - Переключение раскладки (или Alt+Shift)
<span style="color:#38bdf8;font-weight:bold;">[УТИЛИТЫ И ФАЙЛЫ]</span>
  <b>ls [путь]</b>        - Список файлов и директорий
  <b>cat &lt;файл&gt;</b>       - Просмотр содержимого файла
  <b>touch &lt;файл&gt;</b>     - Создать пустой файл
  <b>mkdir &lt;папка&gt;</b>    - Создать директорию
  <b>rm &lt;файл&gt;</b>        - Удалить файл
  <b>echo &lt;текст&gt;</b>     - Вывод текста в терминал (или echo &gt; file.txt)
  <b>edit [файл]</b>      - Открыть текстовый редактор ZeroEditor
  <b>calc &lt;выражение&gt;</b> - Интерактивный калькулятор (e.g. calc 100 * 4)
  <b>matrix</b>           - Цифровой дождь матрицы
  <b>theme &lt;dark|light&gt;</b> - Переключение темы оформления
  <b>zpkg list</b>        - Список установленных пакетов
  <b>apps</b>             - Открыть центр приложений
  <b>reboot</b>           - Перезагрузка системы
  <b>logout</b>           - Завершить сеанс пользователя
`);
      break;
    case 'screen': case 'display': case 'resolution': case 'res': case 'set-res':
      if(args.length === 0){
        res=termLine(`
<span class="term-prompt" style="font-weight:bold;">================== Настройки Экрана и Дисплея ==================</span>
Текущее разрешение: <b>${currentRes}</b> (32 bpp, Scanline Pitch: 4096 байт)
Адрес Framebuffer : 0xE0000000 | 3D VMSVGA: <span style="color:#22c55e;">Активно (VirtualBox)</span>

<span style="color:#eab308;font-weight:bold;">Поддерживаемые режимы экрана:</span>
  1. <b>screen 1024x768</b>   - 1024 x 768  (4:3  Стандарт VirtualBox)
  2. <b>screen 1280x720</b>   - 1280 x 720  (16:9 HD 720p)
  3. <b>screen 1280x800</b>   - 1280 x 800  (16:10 WXGA)
  4. <b>screen 1280x1024</b>  - 1280 x 1024 (5:4  SXGA)
  5. <b>screen 1440x900</b>   - 1440 x 900  (16:10 WXGA+)
  6. <b>screen 1600x900</b>   - 1600 x 900  (16:9 HD+)
  7. <b>screen 1920x1080</b>  - 1920 x 1080 (16:9 Full HD 1080p)
  8. <b>screen 800x600</b>    - 800 x 600   (4:3  SVGA)
  9. <b>screen auto</b>       - Авто-подгонка под размер экрана

<span style="color:#38bdf8;">Пример: введите 'screen 1280x720' или откройте приложение 'Настройка экрана'</span>
`);
      } else {
        const target = args[0].toLowerCase();
        if(target === 'auto' || target === 'fit'){
          applyResolution('1024x768');
          res=termLine('<span style="color:#22c55e;">[✓] Авто-подгонка выполнена: установлено оптимальное разрешение 1024x768 (32 bpp)</span>');
        } else {
          applyResolution(target);
          res=termLine(`<span style="color:#22c55e;">[✓] Разрешение экрана успешно изменено на: <b>${target}</b> (32 bpp)</span>`);
        }
      }
      break;
    case 'driver-install': case 'install': case 'install-drivers': case 'setup':
      res=termLine(`
<span class="term-prompt" style="font-weight:bold;">[*] ===========================================================</span>
<span class="term-prompt" style="font-weight:bold;">[*]     Установщик оборудования LinuxOSZero (Titan Edition)     </span>
<span class="term-prompt" style="font-weight:bold;">[*] ===========================================================</span>
<span style="color:#94a3b8;">[+] Сканирование шины PCI и конфигурационного пространства...</span>
<span style="color:#22c55e;">[✓] Обнаружен: Oracle VirtualBox VMMDev (0x80EE:0xCAFE, Port 0xD020)</span>
    -> Загрузка Ring-0 драйвера гостевых дополнений... [<span style="color:#22c55e;">OK</span>]
<span style="color:#22c55e;">[✓] Обнаружен: Oracle VirtualBox VMSVGA 3D (0x80EE:0xBEEF)</span>
    -> Настройка ${currentRes} 3D Linear Framebuffer... [<span style="color:#22c55e;">OK</span>]
<span style="color:#22c55e;">[✓] Обнаружен: Intel 82540EM Gigabit Ethernet (0x8086:0x100E)</span>
    -> Инициализация сети NAT / DHCP... [<span style="color:#22c55e;">OK</span>]
<span style="color:#22c55e;">[✓] Обнаружен: Intel 82801AA AC'97 Audio Controller (0x8086:0x2415)</span>
    -> Инициализация драйвера звука WASAPI/Host... [<span style="color:#22c55e;">OK</span>]
<span style="color:#22c55e;">[✓] Обнаружен: PS/2 i8042 Контроллер клавиатуры и мыши</span>
    -> Включение скан-кодов Set 1/2 + раскладки US/RU... [<span style="color:#22c55e;">OK</span>]
<span style="color:#22c55e;">[✓] Общие папки VirtualBox (/media/sf_shared)... [СМОНТИРОВАНО]</span>
<span style="color:#22c55e;">[✓] Абсолютное позиционирование мыши (Seamless Mouse)... [АКТИВНО]</span>
<span style="color:#22c55e;font-weight:bold;">[+] Статус установки драйверов: [ 100% ЗАВЕРШЕНО ]</span>
<span style="color:#f8fafc;">[+] Все аппаратные драйверы успешно установлены и работают стабильно!</span>
`);
      break;
    case 'vbox': case 'zero-hwprobe':
      res=termLine(`
<span class="term-prompt" style="font-weight:bold;">[*] Диагностика гипервизора Oracle VM VirtualBox 7.2.4 (x86_64 Long Mode)</span>
<span style="color:#22c55e;">[OK] VMMDev Channel (PCI 0x80EE:0xCAFE, Port 0xD020): ПОДКЛЮЧЁН</span>
<span style="color:#22c55e;">[OK] VMSVGA Display: ${currentRes} с аппаратным 3D-ускорением (DisplayWrap Fixed)</span>
<span style="color:#22c55e;">[OK] Guru Meditation 1155 (Triple Fault): УСТРАНЁН (Стек в Extended RAM 0x200000)</span>
<span style="color:#22c55e;">[OK] Драйвер клавиатуры PS/2: АКТИВЕН (Скан-коды Set 1/2 + переключение раскладки)</span>
<span style="color:#22c55e;">[OK] Интеграция указателя мыши (USB Tablet): АКТИВНА</span>
<span style="color:#22c55e;">[OK] Общие папки (/media/sf_shared): СМОНТИРОВАНЫ</span>
`);
      break;
    case 'pci':
      res=termLine(`
<span class="term-prompt" style="font-weight:bold;">Обнаруженные устройства на шине PCI:</span>
  [00:00.0] Host Bridge       : Intel Corporation 82440FX (PIIX3)
  [00:01.0] ISA Bridge        : Intel Corporation 82371SB PIIX3
  [00:01.1] IDE Storage       : Intel Corporation 82371AB PIIX4 IDE
  [00:02.0] VGA Controller    : InnoTek / Oracle VMSVGA Graphics Adapter
  [00:03.0] Network Controller: Intel Corporation 82540EM Gigabit Ethernet
  [00:04.0] System Peripheral : Oracle VM VirtualBox Guest Additions (VMMDev)
  [00:05.0] Audio Controller  : Intel Corporation 82801AA AC'97 Audio
  [00:06.0] USB Controller    : Apple Computer KeyLargo USB OHCI
  [00:0b.0] USB Controller    : Intel Corporation 82801FB/FBM USB2 EHCI
  [00:0d.0] SATA Controller   : Intel Corporation 82801HM/HEM AHCI Controller
`);
      break;
    case 'video':
      res=termLine(`
<span class="term-prompt">Видеоподсистема:</span> InnoTek/VirtualBox VMSVGA (0x80EE:0xBEEF)
  Разрешение: ${currentRes} @ 32 bpp (Linear Framebuffer 0xE0000000)
  Pitch     : 4096 байт на строку
  Статус    : Аппаратное 2D/3D ускорение активно
  Настройка : введите 'screen' для выбора разрешения
`);
      break;
    case 'audio':
      res=termLine(`
<span class="term-prompt">Аудиоподсистема:</span> Intel 82801AA AC'97 Audio Controller
  Порты     : 0xD100 (NAM) / 0xD200 (NABM)
  Каналы    : Stereo 16-bit 48000 Hz HostAudioWas
  Статус    : Микшер разглушен, вывод звука активен
`);
      break;
    case 'layout':
      if(args[0]==='ru'){ res=termLine('Раскладка клавиатуры переключена на: <b>RU (Русская)</b>'); }
      else { res=termLine('Раскладка клавиатуры переключена на: <b>US (English)</b>'); }
      break;
    case 'zpkg': case 'pkg':
      res=termLine(`
<span class="term-prompt">База данных пакетов zpkg v1.1.0:</span>
  base-system-1.1.0-x86_64       [<span style="color:#22c55e;">установлен</span>]
  zero-kernel-titan-x86_64       [<span style="color:#22c55e;">установлен</span>]
  zero-desktop-wm-1.1.0          [<span style="color:#22c55e;">установлен</span>]
  vbox-guest-additions-7.2.4     [<span style="color:#22c55e;">установлен</span>]
  zero-display-config-1.1.0      [<span style="color:#22c55e;">установлен</span>]
  zero-apps-suite-titan          [<span style="color:#22c55e;">установлен</span>]
  ps2-evdev-keyboard-drivers     [<span style="color:#22c55e;">установлен</span>]
`);
      break;
    case 'pwd': res=termLine('/home/'+currentUser); break;
    case 'ls': {
      const p=ZERO_FS.normalizePath(args[0]||cwd());
      const names=ZERO_FS.listNames(p);
      res=termLine(names.map(n=>ZERO_FS.stat(p==='/'?('/'+n):(p==='/'?'/':p)+'/'+n)?.type==='dir'?'<span class="term-success">'+n+'</span>':n).join('  ')||'(пусто)');
      break; }
    case 'cat': { const c2=ZERO_FS.readFile(ZERO_FS.normalizePath(args[0]||'')); res=termLine(c2??`cat: ${args[0]}: нет такого файла`); break; }
    case 'touch': { const p=ZERO_FS.normalizePath(args[0]||''); ZERO_FS.writeFile(p,''); res=termLine('Создан файл: '+p); break; }
    case 'mkdir': { const p=ZERO_FS.normalizePath(args[0]||''); ZERO_FS.mkdir(p); res=termLine('Создана папка: '+p); break; }
    case 'rm': { ZERO_FS.remove(ZERO_FS.normalizePath(args[0]||'')); res=termLine('Удалено: '+args[0]); break; }
    case 'echo': {
      const body=args.slice(1).join(' ').replace(/^"/,'').replace(/"$/,'');
      if(args[0]==='>'){ ZERO_FS.writeFile(cwd()+'/'+(args[1]||'file.txt'),body); res=termLine('Записано в файл'); }
      else res=termLine(c.slice(5));
      break; }
    case 'edit': { openApp('editor'); res=termLine('Открыт редактор ZeroEditor'); break; }
    case 'uname': res=termLine('Linux linuxoszero 6.1.0-zero-titan #1 SMP PREEMPT x86_64 GNU/Linux'); break;
    case 'fetch': case 'neofetch':
      res=termLine(`
<span style="color:#38bdf8;">   .---.       </span><span class="term-success">${currentUser}@linuxoszero</span>
<span style="color:#38bdf8;">  /     \\      </span>----------------------------------------
<span style="color:#38bdf8;"> | () () |     </span><b>ОС</b>     : LinuxOSZero 1.1.0 (Titan Edition) x86_64
<span style="color:#38bdf8;">  \\  _  /      </span><b>Хост</b>   : Oracle VM VirtualBox 7.2.4
<span style="color:#38bdf8;">   '---'       </span><b>Ядро</b>   : 6.1.0-zero-titan x86_64 Long Mode
               <b>Дисплей</b>: VMSVGA ${currentRes} @ 32 bpp (LFB 0xE0000000)
               <b>ОЗУ</b>    : 245 МБ / 2048 МБ
               <b>Драйверы</b>: VMMDev, VMSVGA, AC97, E1000, PS/2 [<span style="color:#22c55e;">АКТИВНЫ</span>]
`);
      break;
    case 'whoami': res=termLine('user (UID 1000, GID 1000, Группы: wheel, video, audio, vboxsf, sudo)'); break;
    case 'date': res=termLine(new Date().toUTCString()); break;
    case 'uptime': res=termLine('up 2 hours, 10 mins, 1 user, load average: 0.02, 0.01, 0.00'); break;
    case 'free':
      res=termLine(`
               total        used        free      shared  buff/cache   available
Mem:         2048000      250880     1797120        4096       32768     1793024
Swap:              0           0           0
`);
      break;
    case 'ps':
      res=termLine(`
  PID TTY          TIME CMD
    1 ?        00:00:01 zero-init (PID 1)
   42 ?        00:00:00 zero-guest-agent (VMMDev)
  100 tty1     00:00:05 zero-desktop (ZeroWM)
  105 tty1     00:00:01 zero-terminal
`);
      break;
    case 'calc': {
      try {
        const expr = args.join(' ');
        if (!expr) { res=termLine('Использование: calc &lt;выражение&gt; (например: calc 42 * 2 + 10)'); }
        else {
          const sanitized = expr.replace(/[^0-9+\-*/(). %]/g, '');
          const val = Function('"use strict";return (' + sanitized + ')')();
          res=termLine(`= <span style="color:#22c55e;font-weight:bold;">${val}</span>`);
        }
      } catch(e) { res=termLine('calc: ошибка вычисления выражения'); }
      break;
    }
    case 'matrix':
      res=termLine('<span style="color:#22c55e;">Wake up, Neo... LinuxOSZero 64-bit Long Mode has you.<br>Follow the white rabbit. VirtualBox and PS/2 keyboard drivers: [OK]</span>');
      break;
    case 'theme':
      if (args[0] === 'light') { applyTheme('light'); res=termLine('Установлена светлая тема'); }
      else { applyTheme('dark'); res=termLine('Установлена тёмная кибер-тема'); }
      break;
    case 'net': res=termLine('eth0: 10.0.2.15 (DHCP / NAT) — подключён к сети'); break;
    case 'apps': { openApp('store'); res=termLine('Открыт магазин приложений'); break; }
    case 'reboot': res=termLine('<span style="color:#eab308;">Перезагрузка виртуальной машины...</span>'); setTimeout(()=>location.reload(), 1000); return;
    case 'logout': setTimeout(()=>{ document.getElementById('text-session')?.remove(); osContainer.classList.add('hidden'); loginScreen.classList.remove('hidden'); },300); return;
    case 'clear': out.innerHTML=''; out.scrollTop=0; return;
    default: res=termLine(`zero: команда не найдена: ${rawProg}. Введите <b>help</b> или <b>/help</b> для списка.`);
  }
  if(res) out.insertAdjacentHTML('beforeend',res);
  out.scrollTop=out.scrollHeight;
}
function cwd(){ return ZERO_FS.home(); }

function createTerminalWindow(){
  const id='term-'+nextWinId;
  const html=`<div class="term-window"><div class="term-output" id="${id}"></div><div class="term-input-row"><span class="term-prompt">${currentUser}@linuxoszero:~$</span><input class="term-input" id="${id}-in" autocomplete="off" spellcheck="false" placeholder="help"></div></div>`;
  createWindow('Терминал','terminal',680,420,html,{contentId:id});
  const outEl=document.getElementById(id);
  outEl.innerHTML='<div class="dim">LinuxOSZero Терминал — настоящая оболочка. Введите "help", "screen" или "driver-install"</div><br>';
  const inputEl=document.getElementById(id+'-in');
  setTimeout(()=>inputEl.focus(),60);
  inputEl.addEventListener('keydown',e=>{ if(e.key==='Enter'){ const v=inputEl.value; outEl.insertAdjacentHTML('beforeend',`<div><span class="term-prompt">${currentUser}@linuxoszero:~$</span> ${v.replace(/</g,'&lt;')}</div>`); runTermCommand(v,outEl); outEl.scrollTop=outEl.scrollHeight; inputEl.value=''; } });
  outEl.addEventListener('click',()=>inputEl.focus());
}

// ---------------------------------------------------------------------------
// DISPLAY & SCREEN SETTINGS APP
// ---------------------------------------------------------------------------
const SCREEN_RESOLUTIONS = [
  { res: '1024x768',  name: '1024 x 768',  aspect: '4:3',   desc: 'Стандарт VirtualBox (XGA)' },
  { res: '1280x720',  name: '1280 x 720',  aspect: '16:9',  desc: 'HD 720p широкоформатный' },
  { res: '1280x800',  name: '1280 x 800',  aspect: '16:10', desc: 'WXGA для ноутбуков' },
  { res: '1280x1024', name: '1280 x 1024', aspect: '5:4',   desc: 'SXGA мониторы' },
  { res: '1440x900',  name: '1440 x 900',  aspect: '16:10', desc: 'WXGA+ широкоформатный' },
  { res: '1600x900',  name: '1600 x 900',  aspect: '16:9',  desc: 'HD+ мониторы' },
  { res: '1920x1080', name: '1920 x 1080', aspect: '16:9',  desc: 'Full HD 1080p' },
  { res: '800x600',   name: '800 x 600',   aspect: '4:3',   desc: 'SVGA базовый' }
];

function createDisplayWindow(){
  const id='disp-'+nextWinId;
  const renderResCards = () => {
    return SCREEN_RESOLUTIONS.map(r => `
      <div class="card disp-res-card ${r.res === currentRes ? 'active-res' : ''}" data-res="${r.res}" style="cursor:pointer;display:flex;justify-content:space-between;align-items:center;padding:10px 14px;border:1px solid ${r.res === currentRes ? 'var(--accent)' : 'var(--border)'};background:${r.res === currentRes ? 'rgba(56,189,248,0.12)' : 'var(--card-bg)'};border-radius:6px;margin-bottom:8px;">
        <div>
          <div style="font-weight:bold;color:${r.res === currentRes ? 'var(--accent)' : 'var(--text)'}">${r.name} <span class="dim" style="font-size:12px;">(${r.aspect})</span></div>
          <div class="dim" style="font-size:12px;margin-top:2px;">${r.desc}</div>
        </div>
        <div>
          <button class="btn-${r.res === currentRes ? 'primary' : 'secondary'} disp-choose-btn" data-res="${r.res}">${r.res === currentRes ? 'Активно ✓' : 'Выбрать'}</button>
        </div>
      </div>
    `).join('');
  };

  const html = `
    <div style="display:flex;flex-direction:column;gap:14px;height:100%;overflow-y:auto;padding:4px;">
      <div style="display:flex;justify-content:space-between;align-items:center;">
        <h3 style="color:var(--accent);margin:0;">Настройка Экрана и Разрешения</h3>
        <span class="dim" style="font-size:12px;">Драйвер: VMSVGA (VirtualBox)</span>
      </div>

      <!-- Current Mode Info Card -->
      <div class="card" style="background:#0f172a;border:1px solid #334155;">
        <div style="display:flex;justify-content:space-between;align-items:center;">
          <div>
            <div style="font-size:12px;color:#94a3b8;">Текущий видеорежим:</div>
            <div style="font-size:18px;font-weight:bold;color:#38bdf8;" id="${id}-cur-label">${currentRes} @ 32 bpp</div>
          </div>
          <div style="text-align:right;">
            <div style="font-size:12px;color:#22c55e;">✔ 3D Ускорение активно</div>
            <div style="font-size:12px;color:#94a3b8;">VRAM: 128 МБ</div>
          </div>
        </div>
      </div>

      <!-- Resolution List -->
      <div style="font-weight:bold;color:var(--text);font-size:13px;">Выберите разрешение экрана:</div>
      <div id="${id}-res-list">${renderResCards()}</div>

      <!-- Advanced Display Options -->
      <div class="card">
        <div style="font-weight:bold;color:var(--text);margin-bottom:10px;">Масштабирование и подгонка</div>
        <div style="display:flex;gap:10px;align-items:center;margin-bottom:8px;">
          <label style="font-size:13px;color:var(--text);">Масштаб интерфейса:</label>
          <select class="field-input" id="${id}-scale" style="width:140px;">
            <option value="100" ${currentScale==='100'?'selected':''}>100% (Обычный)</option>
            <option value="125" ${currentScale==='125'?'selected':''}>125%</option>
            <option value="150" ${currentScale==='150'?'selected':''}>150% (Крупный)</option>
            <option value="200" ${currentScale==='200'?'selected':''}>200% (HiDPI)</option>
          </select>
        </div>
        <div style="display:flex;gap:10px;align-items:center;">
          <label style="font-size:13px;color:var(--text);">Режим подгонки:</label>
          <button class="btn-secondary" id="${id}-autofit">Авто-подгонка под размер окна</button>
        </div>
      </div>

      <div style="display:flex;justify-content:space-between;align-items:center;margin-top:auto;padding-top:8px;">
        <span id="${id}-status" style="color:var(--success);font-size:13px;"></span>
        <button class="btn-primary" id="${id}-apply-btn">Применить настройки</button>
      </div>
    </div>
  `;

  createWindow('Настройка экрана','screen',620,520,html);
  const content = windows[Object.keys(windows).pop()].elem;

  const wireResButtons = () => {
    content.querySelectorAll('.disp-choose-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        const targetRes = btn.dataset.res;
        applyResolution(targetRes);
        content.querySelector('#' + id + '-res-list').innerHTML = renderResCards();
        content.querySelector('#' + id + '-cur-label').textContent = targetRes + ' @ 32 bpp';
        const st = content.querySelector('#' + id + '-status');
        st.textContent = '✓ Разрешение ' + targetRes + ' успешно применено!';
        wireResButtons();
      });
    });
  };
  wireResButtons();

  content.querySelector('#' + id + '-autofit').addEventListener('click', () => {
    applyResolution('1024x768');
    content.querySelector('#' + id + '-res-list').innerHTML = renderResCards();
    content.querySelector('#' + id + '-cur-label').textContent = '1024x768 @ 32 bpp';
    content.querySelector('#' + id + '-status').textContent = '✓ Авто-подгонка 1024x768 выполнена!';
    wireResButtons();
  });

  content.querySelector('#' + id + '-apply-btn').addEventListener('click', () => {
    const sc = content.querySelector('#' + id + '-scale').value;
    applyResolution(currentRes, sc);
    content.querySelector('#' + id + '-status').textContent = '✓ Настройки экрана сохранены и активны!';
    setTimeout(() => {
      const st = content.querySelector('#' + id + '-status');
      if (st) st.textContent = '';
    }, 3000);
  });
}

// ---------------------------------------------------------------------------
// HARDWARE DRIVER & OS INSTALLER APP
// ---------------------------------------------------------------------------
function createInstallerWindow(){
  let step = 1;
  const id = 'inst-' + nextWinId;

  const renderStep = () => {
    if (step === 1) {
      return `
        <div style="display:flex;flex-direction:column;gap:14px;">
          <h3 style="color:var(--accent);margin:0;">Мастер установки оборудования и LinuxOSZero</h3>
          <p class="dim" style="font-size:13px;line-height:1.6;">Данный мастер проверит драйверы VirtualBox и настроит систему Titan v1.1.0.</p>
          <div class="card">
            <strong style="color:var(--text);">Проверка аппаратных компонентов:</strong>
            <div style="color:var(--success);margin-top:8px;">✔ VirtualBox VMMDev (0x80EE:0xCAFE) — подключён</div>
            <div style="color:var(--success);margin-top:4px;">✔ VMSVGA 3D Графика (0x80EE:0xBEEF) — 1024x768x32</div>
            <div style="color:var(--success);margin-top:4px;">✔ Клавиатура PS/2 i8042 (Set 1/2 + US/RU) — готова</div>
            <div style="color:var(--success);margin-top:4px;">✔ Сетевой адаптер Intel 82540EM — активен</div>
            <div style="color:var(--success);margin-top:4px;">✔ Звук Intel AC'97 (Host Audio) — разглушен</div>
          </div>
          <div style="display:flex;justify-content:flex-end;gap:10px;margin-top:12px;">
            <button class="btn-primary" id="${id}-next-1">Далее ></button>
          </div>
        </div>
      `;
    } else if (step === 2) {
      return `
        <div style="display:flex;flex-direction:column;gap:14px;">
          <h3 style="color:var(--accent);margin:0;">Выбор диска для установки</h3>
          <p class="dim" style="font-size:13px;">Автоматическая разметка дискового пространства:</p>
          <div class="card" style="border:1px solid var(--accent);background:rgba(56,189,248,0.08);">
            <div style="font-weight:bold;color:var(--accent);">/dev/sda — 20.0 GB (VirtualBox VDI HardDisk)</div>
            <div class="dim" style="font-size:12px;margin-top:4px;">Разделы: /dev/sda1 (512MB EFI/Boot) + /dev/sda2 (19.5GB ext4 Root)</div>
          </div>
          <div style="display:flex;justify-content:space-between;margin-top:12px;">
            <button class="btn-secondary" id="${id}-back-2">< Назад</button>
            <button class="btn-primary" id="${id}-next-2">Установить сейчас >></button>
          </div>
        </div>
      `;
    } else if (step === 3) {
      return `
        <div style="display:flex;flex-direction:column;gap:14px;">
          <h3 style="color:var(--accent);margin:0;">Установка драйверов и системы...</h3>
          <p class="dim" style="font-size:13px;" id="${id}-progress-text">Инициализация процесса установки...</p>
          <div class="bar-bg" style="height:16px;border-radius:8px;">
            <div class="bar-fill" id="${id}-pbar" style="width:0%;height:100%;border-radius:8px;background:linear-gradient(90deg, #38bdf8, #22c55e);transition:width 0.3s;"></div>
          </div>
          <div class="card" id="${id}-log" style="font-family:monospace;font-size:12px;height:120px;overflow-y:auto;background:#05070d;color:#94a3b8;">
            <div>[+] Создание GPT таблицы разделов...</div>
          </div>
        </div>
      `;
    } else {
      return `
        <div style="display:flex;flex-direction:column;gap:14px;text-align:center;align-items:center;">
          <div style="font-size:36px;color:#22c55e;margin-top:10px;">✔</div>
          <h3 style="color:#22c55e;margin:0;">Установка успешно завершена!</h3>
          <p class="dim" style="font-size:13px;max-width:440px;">Все аппаратные драйверы VirtualBox и операционная система LinuxOSZero Titan v1.1.0 установлены и готовы к работе.</p>
          <button class="btn-primary" id="${id}-finish-btn" style="margin-top:16px;">Готово</button>
        </div>
      `;
    }
  };

  createWindow('Установщик ОС и драйверов','installer',580,420,'<div id="' + id + '-body">' + renderStep() + '</div>');
  const content = windows[Object.keys(windows).pop()].elem;

  const wireSteps = () => {
    const next1 = content.querySelector('#' + id + '-next-1');
    if (next1) next1.onclick = () => { step = 2; updateView(); };

    const back2 = content.querySelector('#' + id + '-back-2');
    if (back2) back2.onclick = () => { step = 1; updateView(); };

    const next2 = content.querySelector('#' + id + '-next-2');
    if (next2) next2.onclick = () => {
      step = 3;
      updateView();
      runInstallSim();
    };

    const fin = content.querySelector('#' + id + '-finish-btn');
    if (fin) fin.onclick = () => { closeWin(Object.keys(windows).pop()); };
  };

  const updateView = () => {
    const body = content.querySelector('#' + id + '-body');
    if (body) {
      body.innerHTML = renderStep();
      wireSteps();
    }
  };

  const runInstallSim = () => {
    let pct = 0;
    const logEl = content.querySelector('#' + id + '-log');
    const pbar = content.querySelector('#' + id + '-pbar');
    const txt = content.querySelector('#' + id + '-progress-text');
    const msgs = [
      '[+] Форматирование ext4 корневой файловой системы...',
      '[+] Установка 64-битного ядра LinuxOSZero Titan...',
      '[+] Установка Ring-0 драйвера VMMDev...',
      '[+] Настройка видеорежима VMSVGA 3D LFB...',
      '[+] Настройка драйвера клавиатуры PS/2 evdev...',
      '[+] Настройка сети Intel E1000 Gigabit...',
      '[+] Настройка звука Intel AC97 HostAudio...',
      '[+] Генерация GRUB2 конфигурации...',
      '[+] Финализация установки...'
    ];
    const timer = setInterval(() => {
      pct += 12;
      if (pbar) pbar.style.width = Math.min(pct, 100) + '%';
      const msgIdx = Math.floor((pct / 100) * msgs.length);
      if (txt && msgs[msgIdx]) txt.textContent = msgs[msgIdx];
      if (logEl && msgs[msgIdx]) {
        logEl.insertAdjacentHTML('beforeend', '<div>' + msgs[msgIdx] + '</div>');
        logEl.scrollTop = logEl.scrollHeight;
      }
      if (pct >= 100) {
        clearInterval(timer);
        setTimeout(() => {
          step = 4;
          updateView();
        }, 400);
      }
    }, 250);
  };

  wireSteps();
}

// ---------------------------------------------------------------------------
// APP STORE
// ---------------------------------------------------------------------------
const STORE_CATALOG = [
  { name:'Firefox',       cat:'Браузеры',      icon:'browser', desc:'Полноценный браузер', size:'82 МБ', popular:true },
  { name:'Chromium',      cat:'Браузеры',      icon:'globe',    desc:'Браузер с интернетом', size:'95 МБ', popular:true },
  { name:'Discord',       cat:'Мессенджеры',   icon:'discord',  desc:'Голос и чат', size:'120 МБ', popular:true },
  { name:'Steam',         cat:'Игры',          icon:'steam',    desc:'Магазин игр и запуск', size:'210 МБ', popular:true },
  { name:'Minecraft',     cat:'Игры',          icon:'game',     desc:'Блочная песочница', size:'320 МБ' },
  { name:'Змейка',        cat:'Игры',          icon:'gamepad',  desc:'Классическая игра', size:'2 МБ' },
  { name:'Понг',          cat:'Игры',          icon:'gamepad',  desc:'Аркада на двоих', size:'1 МБ' },
  { name:'VLC',           cat:'Мультимедиа',   icon:'play',     desc:'Видеоплеер', size:'48 МБ' },
  { name:'Муз. плеер',    cat:'Мультимедиа',   icon:'music',    desc:'Слушать музыку', size:'9 МБ' },
  { name:'Terminal',      cat:'Утилиты',       icon:'terminal', desc:'Командная строка', size:'4 МБ' },
  { name:'Файлы',         cat:'Утилиты',       icon:'filemanager', desc:'Файловый менеджер', size:'6 МБ' },
  { name:'GIMP',          cat:'Графика',       icon:'paint',    desc:'Редактор изображений', size:'140 МБ' }
];
const installedApps = JSON.parse(localStorage.getItem('zero-installed')||'["terminal","filemanager","analytics","calculator","editor","fetch","display","installer"]');

function createStoreWindow(){
  const cat='all';
  const renderCat = catName => {
    const list = STORE_CATALOG.filter(a=>catName==='all'||a.cat===catName);
    return list.map(a=>`
      <div class="store-card">
        <div class="store-icon" style="color:var(--accent)">${svgIcon(a.icon,34)}</div>
        <div class="store-info"><div class="store-name">${a.name}</div><div class="store-desc">${a.desc}</div><div class="store-size">${a.size}</div></div>
        <button class="btn-primary store-btn" data-name="${a.name}">${installedApps.includes(a.name)?'Запустить':'Установить'}</button>
      </div>`).join('');
  };
  const cats=['all','Браузеры','Мессенджеры','Игры','Мультимедиа','Утилиты','Графика'];
  const html=`<div class="store"><div class="store-head"><h3 style="color:var(--accent)">Магазин приложений — ZeroApp</h3>
    <div class="store-cats">${cats.map(c=>`<button class="btn-secondary store-cat ${c==='all'?'active':''}" data-c="${c}">${c==='all'?'Все':c}</button>`).join('')}</div>
    <div class="store-search-row"><span>${svgIcon('search',14)}</span><input class="field-input store-q" placeholder="Найти приложение…"></div>
    <div class="store-list" id="store-list">${renderCat('all')}</div></div></div>`;
  createWindow('Магазин приложений','store',760,560,html,{contentId:'store-list'});
  const content=windows[Object.keys(windows).pop()].elem;
  const list=content.querySelector('#store-list');
  content.querySelectorAll('.store-cat').forEach(b=>b.addEventListener('click',()=>{ content.querySelectorAll('.store-cat').forEach(x=>x.classList.remove('active')); b.classList.add('active'); list.innerHTML=renderCat(b.dataset.c); wireStoreBtns(list); }));
  const q=content.querySelector('.store-q');
  q.addEventListener('input',()=>{ const s=q.value.toLowerCase(); list.querySelectorAll('.store-card').forEach(c=>c.style.display=c.textContent.toLowerCase().includes(s)?'':'none'); });
  wireStoreBtns(list);
}
function wireStoreBtns(list){
  list.querySelectorAll('.store-btn').forEach(btn=>{
    btn.addEventListener('click',()=>{
      const name=btn.dataset.name;
      if(installedApps.includes(name)){ const map={Firefox:'browser',Chromium:'browser',Discord:'discord',Steam:'steam','Змейка':'games',Понг:'games',Terminal:'terminal','Файлы':'filemanager'}; openApp(map[name]||'store'); }
      else { installedApps.push(name); localStorage.setItem('zero-installed',JSON.stringify(installedApps)); btn.textContent='Запустить'; btn.classList.add('installed'); }
    });
  });
}

// ---------------------------------------------------------------------------
// BROWSER (real iframe — actual working internet browser)
// ---------------------------------------------------------------------------
function createBrowserWindow(){
  const id='browser-'+nextWinId;
  const startUrl = localStorage.getItem('zero-homepage') || 'https://www.wikipedia.org/';
  const html=`<div class="browser"><div class="browser-bar">
    <button class="btn-secondary bb-back">&#8592;</button><button class="btn-secondary bb-fwd">&#8594;</button>
    <input class="field-input bb-url" value="${startUrl}" spellcheck="false">
    <button class="btn-primary bb-go">Перейти</button>
  </div><div class="browser-body"><iframe id="${id}-frame" class="browser-frame" src="${startUrl}" sandbox="allow-scripts allow-same-origin allow-forms allow-popups"></iframe>
  <div class="browser-loading hidden" id="${id}-loading">Загрузка…</div></div></div>`;
  createWindow('Браузер','browser',900,600,html);
  const content=windows[Object.keys(windows).pop()].elem;
  const url=content.querySelector('.bb-url'); const frame=content.querySelector('iframe'); const loading=content.querySelector('.browser-loading');
  const go=()=>{ let u=url.value.trim(); if(!/^https?:/.test(u)) u='https://'+u; loading.classList.remove('hidden'); frame.src=u; };
  content.querySelector('.bb-go').addEventListener('click',go);
  url.addEventListener('keydown',e=>{ if(e.key==='Enter') go(); });
  frame.addEventListener('load',()=>{ loading.classList.add('hidden'); url.value=frame.src; });
  content.querySelector('.bb-back').addEventListener('click',()=>frame.contentWindow.history.back());
  content.querySelector('.bb-fwd').addEventListener('click',()=>frame.contentWindow.history.forward());
}

// ---------------------------------------------------------------------------
// DISCORD (working local chat)
// ---------------------------------------------------------------------------
const discordMsgs = JSON.parse(localStorage.getItem('zero-discord')||'[]');
if(discordMsgs.length===0) discordMsgs.push({u:'Система',t:'Добро пожаловать в LinuxOSZero Discord!'});
function createDiscordWindow(){
  const id='disc-'+nextWinId;
  const channels=['#общий','#игры','#разработка','#музыка'];
  const html=`<div class="discord">
    <div class="disc-side">${channels.map(c=>`<div class="disc-chan ${c==='#общий'?'active':''}">${c}</div>`).join('')}</div>
    <div class="disc-main"><div class="disc-head">LinuxOSZero Discord — общий</div>
    <div class="disc-msgs" id="${id}-msgs"></div>
    <div class="disc-input"><input class="field-input" id="${id}-in" placeholder="Написать сообщение…" autocomplete="off"><button class="btn-primary disc-send">Отправить</button></div></div></div>`;
  createWindow('Discord','discord',720,520,html,{contentId:id+'-msgs'});
  const content=windows[Object.keys(windows).pop()].elem;
  const msgs=content.querySelector('.disc-msgs'); const input=content.querySelector('.disc-input input');
  const renderMsgs=()=>{ msgs.innerHTML=discordMsgs.map(m=>`<div class="disc-msg"><b style="color:var(--accent)">${m.u}</b><span>${m.t.replace(/</g,'&lt;')}</span></div>`).join(''); msgs.scrollTop=msgs.scrollHeight; };
  renderMsgs();
  const send=()=>{ const t=input.value.trim(); if(!t)return; discordMsgs.push({u:currentUser,t}); localStorage.setItem('zero-discord',JSON.stringify(discordMsgs)); input.value=''; renderMsgs(); };
  content.querySelector('.disc-send').addEventListener('click',send);
  input.addEventListener('keydown',e=>{ if(e.key==='Enter') send(); });
  content.querySelectorAll('.disc-chan').forEach(c=>c.addEventListener('click',()=>{ content.querySelectorAll('.disc-chan').forEach(x=>x.classList.remove('active')); c.classList.add('active'); }));
}

// ---------------------------------------------------------------------------
// STEAM
// ---------------------------------------------------------------------------
const STEAM_GAMES=[
  {name:'Minecraft',    tag:'Блоки',    price:'2499 ₽'},
  {name:'Counter-Strike', tag:'Шутер', price:'0 ₽'},
  {name:'The Witcher 3', tag:'RPG',     price:'1999 ₽'},
  {name:'ZERO 2D Racer', tag:'Гонки',   price:'Free'},
  {name:'Cyberpunk',     tag:'RPG',     price:'2999 ₽'}
];
function createSteamWindow(){
  const html=`<div class="steam">
    <div class="steam-head">Steam — Магазин</div>
    <div class="steam-body">
      <div class="steam-featured"><div class="steam-feat-inner"><h2>ZERO 2D Racer</h2><p>Бесплатно в библиотеке LinuxOSZero</p><button class="btn-primary" onclick="openApp('games')">Играть</button></div></div>
      <div class="steam-grid">${STEAM_GAMES.map(g=>`<div class="steam-card"><div class="steam-icon">${svgIcon('gamepad',30)}</div><div class="steam-gname">${g.name}</div><div class="steam-tag">${g.tag}</div><div class="steam-price">${g.price}</div><button class="btn-secondary steam-add">В библиотеку</button></div>`).join('')}</div>
    </div></div>`;
  createWindow('Steam','steam',780,560,html);
  const content=windows[Object.keys(windows).pop()].elem;
  content.querySelectorAll('.steam-add').forEach(b=>b.addEventListener('click',()=>{ b.textContent='Установлено ✓'; b.disabled=true; }));
}

// ---------------------------------------------------------------------------
// GAMES (playable Snake + Pong)
// ---------------------------------------------------------------------------
function createGamesWindow(){
  const html=`<div class="games"><div class="steam-head">Игры LinuxOSZero</div><div class="steam-grid">
    <div class="steam-card"><div class="steam-icon">${svgIcon('gamepad',30)}</div><div class="steam-gname">Змейка</div><button class="btn-primary" onclick="openGame('snake')">Играть</button></div>
    <div class="steam-card"><div class="steam-icon">${svgIcon('gamepad',30)}</div><div class="steam-gname">Понг</div><button class="btn-primary" onclick="openGame('pong')">Играть</button></div>
    <div class="steam-card"><div class="steam-icon">${svgIcon('gamepad',30)}</div><div class="steam-gname">3D Демо (Драйвер)</div><button class="btn-primary" onclick="openGame('3d')">Запустить</button></div>
  </div></div>`;
  createWindow('Игры','gamepad',520,360,html);
}
function openGame(g){
  if(g==='snake') createSnakeGame();
  else if(g==='pong') createPongGame();
  else if(g==='3d') createGpuWindow();
}
function createSnakeGame(){
  const id='snake-'+nextWinId;
  const html=`<div style="display:flex;flex-direction:column;height:100%;"><div class="snake-hud" id="${id}-score">Счёт: 0</div><div class="snake-grid" id="${id}-grid"></div><div class="dim" style="text-align:center;font-size:11px;margin-top:4px;">Стрелки — управление</div></div>`;
  createWindow('Змейка','game',360,430,html,{contentId:id+'-grid'});
  const content=windows[Object.keys(windows).pop()].elem;
  const grid=content.querySelector('.snake-grid'); const scoreEl=content.querySelector('.snake-hud');
  const N=16; let cells=[];
  for(let i=0;i<N*N;i++){ const c=document.createElement('div'); c.className='snake-cell'; grid.appendChild(c); cells.push(c); }
  let snake=[Math.floor(N*N/2),Math.floor(N*N/2)+1,Math.floor(N*N/2)+2]; let dir='up'; let apple; let score=0; let over=false;
  const spawn=()=>{ let p; do{ p=Math.floor(Math.random()*N*N); }while(snake.includes(p)); apple=p; };
  spawn();
  const render=()=>{ cells.forEach(c=>c.classList.remove('head','body','apple')); snake.forEach((p,i)=>{ if(cells[p]) cells[p].classList.add(i===0?'head':'body'); }); if(cells[apple])cells[apple].classList.add('apple'); scoreEl.textContent='Счёт: '+score; };
  const step=()=>{ if(over)return; const h=snake[0]; const r=Math.floor(h/N),c2=h%N; let nr=r,nc=c2;
    if(dir==='up')nr--;else if(dir==='down')nr++;else if(dir==='left')nc--;else nc++;
    if(nr<0||nr>=N||nc<0||nc>=N){over=true;scoreEl.textContent='Игра окончена! Счёт: '+score;return;}
    const np=nr*N+nc; if(snake.includes(np)){over=true;scoreEl.textContent='Игра окончена! Счёт: '+score;return;}
    snake.unshift(np); if(np===apple){score++;spawn();} else snake.pop(); render(); };
  const key=e=>{ if(e.key==='ArrowUp'&&dir!=='down')dir='up'; else if(e.key==='ArrowDown'&&dir!=='up')dir='down'; else if(e.key==='ArrowLeft'&&dir!=='right')dir='left'; else if(e.key==='ArrowRight'&&dir!=='left')dir='right'; };
  document.addEventListener('keydown',key);
  render(); const t=setInterval(step,140);
}

function createPongGame(){
  const id='pong-'+nextWinId;
  const html=`<div style="display:flex;flex-direction:column;height:100%;"><canvas id="${id}" class="pong-canvas" width="480" height="300"></canvas><div class="dim" style="text-align:center;font-size:11px;">↑ ↓ — ракетка</div></div>`;
  createWindow('Понг','game',520,380,html);
  const content=windows[Object.keys(windows).pop()].elem;
  const cv=content.querySelector('canvas'); const ctx=cv.getContext('2d');
  let bx=240,by=150,dx=3,dy=2,py=120,px=460,score=0;
  document.addEventListener('keydown',e=>{ if(e.key==='ArrowUp')py-=14; if(e.key==='ArrowDown')py+=14; });
  const t=setInterval(()=>{ bx+=dx;by+=dy; if(by<5||by>295)dy*=-1; if(bx<5){bx=240;by=150;dx=Math.abs(dx);} if(bx>455){ if(by>py-30&&by<py+70){dx*=-1;score++;} else {bx=240;by=150;dx=-Math.abs(dx);} }
    ctx.fillStyle='#0f172a';ctx.fillRect(0,0,480,300); ctx.fillStyle='#38bdf8'; ctx.fillRect(px,py,8,60); ctx.fillRect(12,by-8,8,16); ctx.beginPath();ctx.arc(bx,by,6,0,7);ctx.fill(); ctx.fillStyle='#64748b'; ctx.font='12px monospace'; ctx.fillText('Счёт: '+score,210,16); },16);
}

// ---------------------------------------------------------------------------
// INTERNET SETTINGS
// ---------------------------------------------------------------------------
function createInternetWindow(){
  const html=`<div style="display:flex;flex-direction:column;gap:16px;">
    <h3 style="color:var(--accent)">Настройки интернета и сети</h3>
    <div class="card"><div class="toggle-row"><span>Wi-Fi / Ethernet</span><label class="switch"><input type="checkbox" checked id="net-on"><span class="slider"></span></label></div>
      <div class="toggle-row"><span>Подключение (DHCP)</span><label class="switch"><input type="checkbox" checked id="net-dhcp"><span class="slider"></span></label></div></div>
    <div class="card"><label class="field-label">IP-адрес</label><input class="field-input" id="net-ip" value="192.168.56.10"></div>
    <div class="card"><label class="field-label">Маска</label><input class="field-input" id="net-mask" value="255.255.255.0"></div>
    <div class="card"><label class="field-label">Шлюз</label><input class="field-input" id="net-gw" value="192.168.56.1"></div>
    <div class="card"><label class="field-label">Домашняя страница браузера</label><input class="field-input" id="net-home" value="${localStorage.getItem('zero-homepage')||'https://www.wikipedia.org/'}"></div>
    <div style="display:flex;gap:8px;"><button class="btn-primary" onclick="saveNetSettings()">Применить</button><span class="dim" style="align-self:center" id="net-status"></span></div>
  </div>`;
  createWindow('Настройки сети','wifi',520,520,html);
}
function saveNetSettings(){
  localStorage.setItem('zero-homepage',document.getElementById('net-home').value);
  const st=document.getElementById('net-status'); st.textContent='✓ Настройки применены';
  const ip=document.getElementById('net-ip').value; if(ip) $('hv-status').textContent='● IP: '+ip;
}

// ---------------------------------------------------------------------------
// GPU / 3D-2D drivers
// ---------------------------------------------------------------------------
function createGpuWindow(){
  const id='gpu-'+nextWinId;
  const html=`<div style="display:flex;flex-direction:column;gap:14px;">
    <h3 style="color:var(--accent)">Графические драйверы — 3D / 2D</h3>
    <div class="card"><strong>Видеокарта</strong><p style="color:var(--text);margin-top:6px;">${svgIcon('chip',15)} VMSVGA / std VGA (0x80EE:0xBEEF) — 128 МБ</p><p style="color:var(--success);margin-top:6px;">✔ 2D: аппаратное ускорение</p><p style="color:var(--success);">✔ 3D: программный растеризатор</p></div>
    <div class="card"><strong>3D-тест (вращающийся куб)</strong><canvas id="${id}-c" class="gpu-canvas" width="360" height="240"></canvas></div>
    <div class="card"><strong>2D-тест (градиенты)</strong><canvas id="${id}-g" class="gpu-canvas" width="360" height="120"></canvas></div>
  </div>`;
  createWindow('3D / 2D Драйверы','chip',520,600,html);
  const content=windows[Object.keys(windows).pop()].elem;
  const c=content.querySelector('#gpu-'+(parseInt(id.split('-')[1]))+'-c')||content.querySelector('.gpu-canvas');
  const ctx=c.getContext('2d');
  const pts=[]; for(let x=-1;x<=1;x+=2)for(let y=-1;y<=1;y+=2)for(let z=-1;z<=1;z+=2) pts.push([x,y,z]);
  const edges=[[0,1],[0,2],[1,3],[2,3],[4,5],[4,6],[5,7],[6,7],[0,4],[1,5],[2,6],[3,7]];
  let a=0;
  const t=setInterval(()=>{ a+=0.02; ctx.fillStyle='#0f172a'; ctx.fillRect(0,0,360,240);
    const ca=Math.cos(a),sa=Math.sin(a);
    const pr=pts.map(p=>{ const x=p[0],y=p[1],z=p[2]; const y1=y*ca-z*sa,z1=y*sa+z*ca; const x2=x*ca+z1*sa,z2=-x*sa+z1*ca; const s=300/(z2+4); return [120+x2*s,120+y1*s]; });
    ctx.strokeStyle='#38bdf8'; ctx.lineWidth=2;
    edges.forEach(e=>{ ctx.beginPath(); ctx.moveTo(pr[e[0]][0],pr[e[0]][1]); ctx.lineTo(pr[e[1]][0],pr[e[1]][1]); ctx.stroke(); });
  },30);
  const g2=content.querySelectorAll('.gpu-canvas')[1]; const cg=g2.getContext('2d');
  const gr=cg.createLinearGradient(0,0,360,0); gr.addColorStop(0,'#0ea5e9'); gr.addColorStop(1,'#8b5cf6'); cg.fillStyle=gr; cg.fillRect(0,0,360,120);
}

// ---------------------------------------------------------------------------
// FILE MANAGER — create files/folders
// ---------------------------------------------------------------------------
function createFileManagerWindow(){
  const id='fm-'+nextWinId;
  const html=`<div style="display:flex;flex-direction:column;height:100%;">
    <div style="display:flex;gap:8px;margin-bottom:8px;">
      <button class="btn-secondary fm-up">&#8592;</button>
      <input type="text" class="field-input fm-loc" value="Расположение: /home/${currentUser}" readonly>
      <button class="btn-secondary" onclick="fmNewFile()">Новый файл</button>
      <button class="btn-secondary" onclick="fmNewFolder()">Папка</button>
    </div>
    <div id="${id}-list" style="flex:1;overflow:auto;"></div></div>`;
  createWindow('Файлы','filemanager',660,440,html,{contentId:id+'-list'});
  const content=windows[Object.keys(windows).pop()].elem;
  const list=content.querySelector('#'+id+'-list'); const loc=content.querySelector('.fm-loc');
  let cur=ZERO_FS.home();
  const render=()=>{ loc.value='Расположение: '+cur; const names=ZERO_FS.listNames(cur);
    let h='<table class="fm-table"><thead><tr><th>Имя</th><th>Тип</th></tr></thead><tbody>';
    if(cur!=='/') h+=`<tr onclick="fmNav('${ZERO_FS.normalizePath(cur+'/..')}')"><td style="color:var(--warn);cursor:pointer;">..</td><td>Вверх</td></tr>`;
    names.forEach(n=>{ const full=cur==='/'?('/'+n):(cur+'/'+n); const s=ZERO_FS.stat(full); const isDir=s&&s.type==='dir';
      h+=`<tr ${isDir?'onclick="fmNav(\''+full+'\')"':''} ondblclick="fmOpenFile('${full}')"><td style="color:${isDir?'var(--accent)':'var(--text)'};cursor:pointer;">${svgIcon(isDir?'filemanager':'filePlus',13)} ${n}</td><td>${isDir?'Папка':'Файл'}</td></tr>`; });
    h+='</tbody></table>'; list.innerHTML=h; };
  content.querySelector('.fm-up').addEventListener('click',()=>{ cur=cur==='/'?'/':ZERO_FS.normalizePath(cur+'/..'); render(); });
  window.fmNav=(p)=>{ if(ZERO_FS.stat(p)?.type==='dir'){cur=p;render();} };
  window.fmNewFile=()=>{ const n=prompt('Имя нового файла:','файл.txt'); if(n) { ZERO_FS.writeFile(cur+'/'+n,''); render(); } };
  window.fmNewFolder=()=>{ const n=prompt('Имя новой папки:','Новая папка'); if(n){ ZERO_FS.mkdir(cur+'/'+n); render(); } };
  window.fmOpenFile=(p)=>{ const s=ZERO_FS.stat(p); if(s&&s.type==='file'){ createEditorWindow(s.content,p); } };
  render();
}

// ---------------------------------------------------------------------------
// EDITOR (save to VFS)
// ---------------------------------------------------------------------------
function createEditorWindow(initialText, filePath){
  const content = initialText || '#include <stdio.h>\nint main() {\n    printf("Привет из LinuxOSZero Titan!\\n");\n    return 0;\n}\n';
  const path = filePath || ZERO_FS.home()+'/новый-файл.c';
  const html=`<div style="display:flex;flex-direction:column;height:100%;">
    <div style="display:flex;gap:8px;margin-bottom:8px;align-items:center;">
      <button class="btn-primary ed-save">Сохранить</button>
      <input class="field-input ed-path" value="${path}" style="flex:1;" spellcheck="false">
    </div>
    <textarea class="editor-area" style="flex:1;width:100%;background:#090d16;color:#38bdf8;border:1px solid var(--border);border-radius:6px;padding:10px;font-family:monospace;resize:none;">${content.replace(/</g,'&lt;')}</textarea></div>`;
  createWindow('Редактор','editor',640,440,html);
  const contentEl=windows[Object.keys(windows).pop()].elem;
  const save=contentEl.querySelector('.ed-save');
  save.addEventListener('click',()=>{ const p=contentEl.querySelector('.ed-path').value; const txt=contentEl.querySelector('.editor-area').value; ZERO_FS.writeFile(p,txt); alert('Файл сохранён: '+p); });
}

// ---------------------------------------------------------------------------
// USER, FETCH, ANALYTICS, CALCULATOR, WELCOME
// ---------------------------------------------------------------------------
function createUserWindow(){
  const html=`<div style="display:flex;flex-direction:column;gap:16px;">
    <h3 style="color:var(--accent)">Настройка пользователя</h3>
    <div style="display:flex;align-items:center;gap:16px;"><div class="profile-avatar" id="user-avatar">${getUserInitials()}</div>
      <div><div style="font-size:16px;font-weight:700;color:var(--text)" id="user-name">${currentUser}</div><div class="dim">@linuxoszero</div></div></div>
    <div class="card"><label class="field-label">Имя пользователя</label><input class="field-input" id="set-user" value="${currentUser}">
      <label class="field-label">Пароль</label><input type="password" class="field-input" id="set-pass" placeholder="Новый пароль"></div>
    <div style="display:flex;gap:8px;"><button class="btn-primary" onclick="saveUser()">Сохранить</button><button class="btn-secondary" onclick="resetUser()">Сбросить</button></div></div>`;
  createWindow('Пользователь','settings',460,420,html);
}
function saveUser(){ const u=document.getElementById('set-user').value.trim()||'user'; currentUser=u; localStorage.setItem('zero-user',u); const p=document.getElementById('set-pass').value; if(p)localStorage.setItem('zero-pass',p); $('start-user').textContent=u+'@linuxoszero'; document.getElementById('user-name').textContent=u; document.getElementById('user-avatar').textContent=u[0].toUpperCase(); alert('Пользователь сохранён: '+u); }
function resetUser(){ localStorage.removeItem('zero-user'); location.reload(); }

function createFetchWindow(){
  const html=`<div style="display:flex;gap:20px;align-items:flex-start;">
    <img src="img/logo.png" style="width:100px;height:100px;border-radius:18px;box-shadow:0 8px 24px var(--accent-glow)" alt="">
    <div style="font-size:13px;line-height:1.8;">
      <div style="color:var(--accent);font-weight:bold">${currentUser}@linuxoszero</div>
      <div class="dim">----------------------------</div>
      <div><strong>ОС</strong>: LinuxOSZero 1.1.0 (Titan Edition) x86_64</div>
      <div><strong>Ядро</strong>: 6.1.0-zero-titan x86_64</div>
      <div><strong>Видео</strong>: VMSVGA 3D (${currentRes})</div>
      <div><strong>Интернет</strong>: <span style="color:var(--success)">подключён</span></div>
      <div><strong>Память</strong>: 2.1 GB / 4 GB</div>
    </div></div>`;
  createWindow('О системе','info',520,300,html);
}
function createWelcomeWindow(){
  const html=`<div style="display:flex;flex-direction:column;align-items:center;text-align:center;padding:12px;">
    <img src="img/logo.png" style="width:70px;height:70px;border-radius:16px;box-shadow:0 8px 24px var(--accent-glow);margin-bottom:12px" alt="">
    <h3>Добро пожаловать в LinuxOSZero Titan</h3>
    <p class="dim" style="margin-top:6px;font-size:13px;">Полноценный рабочий стол: настройка экрана, установщик драйверов, магазин, браузер, терминал, игры.</p>
    <div style="display:flex;gap:10px;margin-top:18px;flex-wrap:wrap;justify-content:center;">
      <button class="btn-primary" onclick="openApp('display')">Настройка экрана</button>
      <button class="btn-primary" onclick="openApp('installer')">Установщик драйверов</button>
      <button class="btn-secondary" onclick="openApp('terminal')">Терминал</button>
      <button class="btn-secondary" onclick="openApp('store')">Магазин</button>
    </div></div>`;
  createWindow('Добро пожаловать','info',540,320,html);
}
function createAnalyticsWindow(){
  const html=`<div style="display:flex;flex-direction:column;gap:14px;">
    <h3 style="color:var(--accent)">Монитор системы</h3>
    <div class="card"><strong>ЦП (4 ядра)</strong><div class="bar-bg"><div class="bar-fill bar-cpu" style="width:58%"></div></div></div>
    <div class="card"><strong>Память — 2.1 / 4 GB</strong><div class="bar-bg"><div class="bar-fill bar-mem" style="width:53%"></div></div></div>
    <div class="card"><strong>Сеть — онлайн</strong><div class="mon-row"><span>⬇ 12.4 MB/s</span><span>⬆ 2.1 MB/s</span></div></div>
    <div class="card"><strong>Графика 3D (VMSVGA)</strong><div class="bar-bg"><div class="bar-fill bar-disk" style="width:42%"></div></div></div></div>`;
  createWindow('Монитор системы','analytics',460,400,html);
  const content=windows[Object.keys(windows).pop()].elem;
  setInterval(()=>{ const c=content.querySelector('.bar-cpu'); if(c)c.style.width=(30+Math.round(Math.random()*60))+'%'; },900);
}
function createCalculatorWindow(){
  let expr=''; const id='calc-'+nextWinId;
  const html=`<div style="display:flex;flex-direction:column;height:100%;"><div class="calc-display" id="${id}-d">0</div><div class="calc-grid">
    ${['C','⌫','%','÷','7','8','9','×','4','5','6','−','1','2','3','+','±','0',',','='].map(k=>`<button class="calc-btn ${['÷','×','−','+','='].includes(k)?'calc-op':(k==='C'?'calc-clear':'')}" data-k="${k}">${k}</button>`).join('')}</div></div>`;
  createWindow('Калькулятор','calculator',300,420,html,{contentId:id+'-d'});
  const content=windows[Object.keys(windows).pop()].elem;
  const disp=content.querySelector('.calc-display');
  content.querySelectorAll('.calc-btn').forEach(b=>b.addEventListener('click',()=>{ const k=b.dataset.k;
    if(k==='C'){expr='';disp.textContent='0';}
    else if(k==='⌫'){expr=expr.slice(0,-1);disp.textContent=expr||'0';}
    else if(k==='='){ try{ const cl=expr.replace(/×/g,'*').replace(/−/g,'-').replace(/÷/g,'/').replace(/,/g,'.'); disp.textContent=String(Function('"use strict";return ('+cl+')')()).replace('.',','); expr=disp.textContent.replace(',','.'); }catch(e){disp.textContent='Ошибка';expr='';} }
    else { expr+=k; disp.textContent=expr; } }));
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------
buildDesktopAndMenu();

// =============================================================================
// LinuxOSZero — ZeroDesktop (Genesis v1.0.0)
// Interactive desktop: App Store, working Browser, Discord, Steam, Games,
// Terminal, File Manager (Рабочая Самка), Editor, Internet settings, 3D demo.
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

// ---------------------------------------------------------------------------
// App registry + desktop/menu
// ---------------------------------------------------------------------------
const APPS = [
  { id:'store',      name:'Магазин приложений', icon:'store',       launch:'store' },
  { id:'browser',    name:'Браузер',            icon:'browser',     launch:'browser' },
  { id:'discord',    name:'Discord',            icon:'discord',     launch:'discord' },
  { id:'steam',      name:'Steam',              icon:'steam',       launch:'steam' },
  { id:'filemanager',name:'Рабочая Самка',      icon:'filemanager', launch:'filemanager' },
  { id:'terminal',   name:'Терминал',           icon:'terminal',    launch:'terminal' },
  { id:'editor',     name:'Редактор',           icon:'editor',      launch:'editor' },
  { id:'internet',   name:'Настройки сети',     icon:'wifi',        launch:'internet' },
  { id:'gpu',        name:'3D / 2D Драйверы',   icon:'chip',        launch:'gpu' },
  { id:'games',      name:'Игры',               icon:'gamepad',     launch:'games' },
  { id:'analytics',  name:'Монитор системы',    icon:'analytics',   launch:'analytics' },
  { id:'calculator', name:'Калькулятор',        icon:'calculator',  launch:'calculator' },
  { id:'fetch',      name:'О системе',          icon:'info',        launch:'fetch' },
  { id:'user',       name:'Пользователь',       icon:'settings',    launch:'user' }
];

function openApp(n){
  switch(n){
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
}
$('start-search').addEventListener('input',e=>{ const q=e.target.value.toLowerCase(); startItems.querySelectorAll('.start-item').forEach(it=>it.style.display=it.textContent.toLowerCase().includes(q)?'':'none'); });

// ---------------------------------------------------------------------------
// Boot flow
// ---------------------------------------------------------------------------
const GRUB_ITEMS = [
  { name:'Запустить LinuxOSZero (графический режим)', mode:'live' },
  { name:'Установить LinuxOSZero (установщик)', mode:'install' },
  { name:'LinuxOSZero — безопасная графика (VESA)', mode:'safe' },
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
  let p=0; const msgs=['Загрузка ядра…','Инициализация 3D/2D драйверов…','Запуск служб…','Подготовка рабочего стола…'];
  const t=setInterval(()=>{ p+=8; $('loading-fill').style.width=Math.min(p,100)+'%'; $('loading-text').textContent='Loading LinuxOSZero… '+Math.min(p,100)+'%'; $('loading-sub').textContent=msgs[Math.floor(p/25)%msgs.length]; if(p>=100){clearInterval(t);$('loading-screen').classList.add('hidden');setTimeout(bootSequence,250);} },60);
}
const BOOT_LINES=[
  {text:'BIOS: запуск GRUB2 (LinuxOSZero)',cls:'ok'},
  {text:'PCI: сканирование шин … устройства найдены',cls:'ok'},
  {text:'Драйвер 2D: аппаратное ускорение включено',cls:'ok'},
  {text:'Драйвер 3D: программный растеризатор активен',cls:'ok'},
  {text:'Дисплей: VMSVGA / std VGA (32-bpp)',cls:'ok'},
  {text:'Монтирование /proc /sys /dev … готово',cls:'ok'},
  {text:'Сеть: интерфейс eth0 настроен (DHCP)',cls:'ok'},
  {text:'Запуск zero-init (PID 1)',cls:'ok'},
  {text:'Запуск ZeroDesktop …',cls:''},
  {text:'LinuxOSZero готова.',cls:'ok'}
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
  $('login-status').textContent='● Система готова, интернет подключён';
  setTimeout(()=>bootScreen.remove(),600);
}
document.addEventListener('keydown',e=>{ if((e.code==='Space'||e.code==='Escape')&&loginScreen.classList.contains('hidden')){ $('grub-screen').classList.add('hidden'); $('loading-screen').classList.add('hidden'); showLogin(); } });
$('login-theme').addEventListener('change',e=>applyTheme(e.target.value));
$('login-btn').addEventListener('click',()=>{ applyTheme($('login-theme').value); loginScreen.classList.add('hidden'); if($('login-session').value==='terminal') showTextSession(); else showDesktop(); });
$('login-pass').addEventListener('keydown',e=>{ if(e.key==='Enter') $('login-btn').click(); });

function showDesktop(){ osContainer.classList.remove('hidden'); openApp('welcome'); if(bootMode==='install') openApp('store'); else openApp('store'); }

// Text session
function showTextSession(){
  const tty=document.createElement('div'); tty.id='text-session';
  tty.style.cssText='position:fixed;inset:0;z-index:3000;background:#05070d;color:#e2e8f0;font-family:"Courier New",monospace;font-size:14px;line-height:1.5;display:flex;flex-direction:column;padding:14px 18px;';
  tty.innerHTML=`<div style="color:#38bdf8;font-weight:bold;border-bottom:1px solid #1e293b;padding-bottom:8px;">LinuxOSZero — текстовый режим <span style="color:#22c55e;">TTY</span> (введите 'help')</div><div id="tty-output" style="flex:1;overflow-y:auto;white-space:pre-wrap;margin-top:8px;"></div><div style="display:flex;"><span class="term-prompt">${currentUser}@linuxoszero:~$</span><input id="tty-input" style="flex:1;background:transparent;border:none;outline:none;color:#e2e8f0;font-family:inherit;font-size:14px;caret-color:#38bdf8;" autocomplete="off" spellcheck="false"></div>`;
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
  const parts=c.split(/\s+/); const prog=parts[0].toLowerCase(); const args=parts.slice(1);
  let res='';
  switch(prog){
    case 'help': res=termLine('Доступно: help, ls, cd, pwd, cat, touch, mkdir, rm, echo, edit, uname, neofetch, net, apps, clear, logout'); break;
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
    case 'edit': { openApp('editor'); res=termLine('Открыт редактор'); break; }
    case 'uname': res=termLine('Linux linuxoszero 6.1.0-zero x86_64'); break;
    case 'neofetch': res=termLine('\n<span class="term-success">'+currentUser+'@linuxoszero</span>\nОС: LinuxOSZero 1.0.0 (Genesis)\nЯдро: 6.1.0-zero-x86_64\nГрафика: 3D/2D ускорение\nИнтернет: подключён'); break;
    case 'net': res=termLine('eth0: 192.168.56.10 (DHCP) — онлайн'); break;
    case 'apps': { openApp('store'); res=termLine('Открыт магазин приложений'); break; }
    case 'logout': setTimeout(()=>{ document.getElementById('text-session')?.remove(); osContainer.classList.add('hidden'); loginScreen.classList.remove('hidden'); },300); return;
    case 'clear': out.innerHTML=''; out.scrollTop=0; return;
    default: res=termLine(`zero: команда не найдена: ${prog} (help)`);
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
  outEl.innerHTML='<div class="dim">LinuxOSZero Терминал — настоящая оболочка. Попробуйте: touch, mkdir, echo, cat, edit</div><br>';
  const inputEl=document.getElementById(id+'-in');
  setTimeout(()=>inputEl.focus(),60);
  inputEl.addEventListener('keydown',e=>{ if(e.key==='Enter'){ const v=inputEl.value; outEl.insertAdjacentHTML('beforeend',`<div><span class="term-prompt">${currentUser}@linuxoszero:~$</span> ${v.replace(/</g,'&lt;')}</div>`); runTermCommand(v,outEl); outEl.scrollTop=outEl.scrollHeight; inputEl.value=''; } });
  outEl.addEventListener('click',()=>inputEl.focus());
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
  { name:'Рабочая Самка', cat:'Утилиты',       icon:'filemanager', desc:'Файловый менеджер', size:'6 МБ' },
  { name:'GIMP',          cat:'Графика',       icon:'paint',    desc:'Редактор изображений', size:'140 МБ' }
];
const installedApps = JSON.parse(localStorage.getItem('zero-installed')||'["terminal","filemanager","analytics","calculator","editor","fetch"]');

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
      if(installedApps.includes(name)){ const map={Firefox:'browser',Chromium:'browser',Discord:'discord',Steam:'steam','Змейка':'games',Понг:'games',Terminal:'terminal','Рабочая Самка':'filemanager'}; openApp(map[name]||'store'); }
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
  const orig=closeWin; // game continues in background; ok
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
  // 3D rotating cube (software projection)
  const pts=[]; for(let x=-1;x<=1;x+=2)for(let y=-1;y<=1;y+=2)for(let z=-1;z<=1;z+=2) pts.push([x,y,z]);
  const edges=[[0,1],[0,2],[1,3],[2,3],[4,5],[4,6],[5,7],[6,7],[0,4],[1,5],[2,6],[3,7]];
  let a=0;
  const t=setInterval(()=>{ a+=0.02; ctx.fillStyle='#0f172a'; ctx.fillRect(0,0,360,240);
    const ca=Math.cos(a),sa=Math.sin(a);
    const pr=pts.map(p=>{ const x=p[0],y=p[1],z=p[2]; const y1=y*ca-z*sa,z1=y*sa+z*ca; const x2=x*ca+z1*sa,z2=-x*sa+z1*ca; const s=300/(z2+4); return [120+x2*s,120+y1*s]; });
    ctx.strokeStyle='#38bdf8'; ctx.lineWidth=2;
    edges.forEach(e=>{ ctx.beginPath(); ctx.moveTo(pr[e[0]][0],pr[e[0]][1]); ctx.lineTo(pr[e[1]][0],pr[e[1]][1]); ctx.stroke(); });
  },30);
  // 2D gradients
  const g2=content.querySelectorAll('.gpu-canvas')[1]; const cg=g2.getContext('2d');
  const gr=cg.createLinearGradient(0,0,360,0); gr.addColorStop(0,'#0ea5e9'); gr.addColorStop(1,'#8b5cf6'); cg.fillStyle=gr; cg.fillRect(0,0,360,120);
}

// ---------------------------------------------------------------------------
// FILE MANAGER (Рабочая Самка) — create files/folders
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
  createWindow('Рабочая Самка','filemanager',660,440,html,{contentId:id+'-list'});
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
  const content = initialText || '#include <stdio.h>\nint main(){ printf("Привет из LinuxOSZero!\\n"); return 0; }\n';
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
      <div><strong>ОС</strong>: LinuxOSZero 1.0.0 (Genesis)</div>
      <div><strong>Ядро</strong>: 6.1.0-zero-x86_64</div>
      <div><strong>Видео</strong>: VMSVGA 3D/2D</div>
      <div><strong>Интернет</strong>: <span style="color:var(--success)">подключён</span></div>
      <div><strong>Память</strong>: 2.1 GB / 4 GB</div>
    </div></div>`;
  createWindow('О системе','info',520,300,html);
}
function createWelcomeWindow(){
  const html=`<div style="display:flex;flex-direction:column;align-items:center;text-align:center;padding:12px;">
    <img src="img/logo.png" style="width:70px;height:70px;border-radius:16px;box-shadow:0 8px 24px var(--accent-glow);margin-bottom:12px" alt="">
    <h3>Добро пожаловать в LinuxOSZero</h3>
    <p class="dim" style="margin-top:6px;font-size:13px;">Полный рабочий стол: магазин приложений, браузер, Discord, Steam, игры, файлы, интернет.</p>
    <div style="display:flex;gap:10px;margin-top:18px;">
      <button class="btn-primary" onclick="openApp('store')">Открыть магазин</button>
      <button class="btn-secondary" onclick="openApp('browser')">Браузер</button>
      <button class="btn-secondary" onclick="openApp('games')">Игры</button>
    </div></div>`;
  createWindow('Добро пожаловать','info',520,300,html);
}
function createAnalyticsWindow(){
  const html=`<div style="display:flex;flex-direction:column;gap:14px;">
    <h3 style="color:var(--accent)">Монитор системы</h3>
    <div class="card"><strong>ЦП (4 ядра)</strong><div class="bar-bg"><div class="bar-fill bar-cpu" style="width:58%"></div></div></div>
    <div class="card"><strong>Память — 2.1 / 4 GB</strong><div class="bar-bg"><div class="bar-fill bar-mem" style="width:53%"></div></div></div>
    <div class="card"><strong>Сеть — онлайн</strong><div class="mon-row"><span>⬇ 12.4 MB/s</span><span>⬆ 2.1 MB/s</span></div></div>
    <div class="card"><strong>Графика 3D</strong><div class="bar-bg"><div class="bar-fill bar-disk" style="width:42%"></div></div></div></div>`;
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

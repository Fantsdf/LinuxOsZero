// =============================================================================
// LinuxOSZero — Virtual File System (VFS)
// A persistent in-browser filesystem backed by localStorage. Files you create
// in the terminal / file manager / editor are saved and survive reloads.
// =============================================================================
const ZERO_FS = (() => {
  const KEY = 'linuxoszero-fs-v1';
  let store = null;

  function load() {
    if (store) return store;
    try {
      const raw = localStorage.getItem(KEY);
      store = raw ? JSON.parse(raw) : defaultFS();
    } catch (e) {
      store = defaultFS();
    }
    return store;
  }
  function save() { try { localStorage.setItem(KEY, JSON.stringify(store)); } catch (e) {} }

  function defaultFS() {
    return {
      '/': {
        'home': { type: 'dir', children: {} },
        'etc': { type: 'dir', children: {
          'zero-release': { type: 'file', content: 'LinuxOSZero Genesis Edition v1.0.0 (x86_64)\n' }
        } },
        'usr': { type: 'dir', children: {} },
        'media': { type: 'dir', children: {
          'sf_shared': { type: 'dir', children: {} }
        } },
        'tmp': { type: 'dir', children: {} }
      },
      '/home': { 'user': { type: 'dir', children: {
        'Документы': { type: 'dir', children: {} },
        'hello.c': { type: 'file', content: '#include <stdio.h>\nint main(){ printf("Привет из LinuxOSZero!\\n"); return 0; }\n' },
        'заметки.txt': { type: 'file', content: 'Добро пожаловать в LinuxOSZero!\n' }
      } } }
    };
  }

  function node(path) {
    const parts = path.split('/').filter(Boolean);
    let dir = store['/'];
    for (let i = 0; i < parts.length - 1; i++) {
      const d = dir[parts[i]];
      if (!d || d.type !== 'dir') return null;
      dir = d.children;
    }
    const name = parts[parts.length - 1];
    return name === undefined ? { dir } : { dir, name, entry: dir[name] };
  }

  function list(path) {
    const n = node(path === '/' ? '/' : path);
    if (!n) return [];
    if (path === '/') return Object.keys(store['/']).map(k => store['/'][k]);
    return n.dir ? Object.entries(n.dir).map(([k, v]) => v) : [];
  }

  function listNames(path) {
    const names = [];
    if (path === '/') { for (const k in store['/']) names.push(k); return names; }
    const n = node(path);
    if (n && n.dir) { for (const k in n.dir) names.push(k); }
    return names;
  }

  function stat(path) {
    const n = node(path);
    if (!n) return null;
    if (path === '/') return { name: '/', type: 'dir' };
    return { name: n.name, type: n.entry.type, content: n.entry.content };
  }

  function mkdir(path) {
    const n = node(path);
    if (n && n.entry) return false;
    if (!n || !n.dir) return false;
    n.dir[n.name] = { type: 'dir', children: {} };
    save(); return true;
  }

  function writeFile(path, content) {
    const n = node(path);
    if (n && n.entry) { n.entry.type = 'file'; n.entry.content = content; save(); return true; }
    if (n && n.dir) { n.dir[n.name] = { type: 'file', content: String(content) }; save(); return true; }
    return false;
  }

  function readFile(path) {
    const n = node(path);
    if (n && n.entry && n.entry.type === 'file') return n.entry.content;
    return null;
  }

  function remove(path) {
    const n = node(path);
    if (!n || !n.dir || n.entry === undefined) return false;
    delete n.dir[n.name];
    save(); return true;
  }

  function normalizePath(p) {
    if (!p) return '/';
    if (!p.startsWith('/')) p = '/' + p;
    const parts = [];
    for (const part of p.split('/')) {
      if (!part || part === '.') continue;
      if (part === '..') parts.pop(); else parts.push(part);
    }
    return '/' + parts.join('/');
  }

  function home() { return '/home/user'; }

  return { list, listNames, stat, mkdir, writeFile, readFile, remove, normalizePath, home, save };
})();

// LinuxOSZero — SVG icon set (no emoji)
function svgIcon(name, size) {
  const s = size || 24;
  const stroke = 'currentColor';
  const attrs = `fill="none" stroke="${stroke}" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"`;
  const body = ICON_PATHS[name] || ICON_PATHS.info;
  return `<svg width="${s}" height="${s}" viewBox="0 0 24 24" ${attrs}>${body}</svg>`;
}

const ICON_PATHS = {
  installer: `<circle cx="12" cy="12" r="8"/><circle cx="12" cy="12" r="2.5"/><path d="M12 9.5v5M9.8 11.3l2.2-2.2 2.2 2.2"/>`,
  terminal: `<rect x="3" y="4" width="18" height="16" rx="3"/><path d="M7 9l3 3-3 3M12.5 15H17"/>`,
  filemanager: `<path d="M3 7a2 2 0 0 1 2-2h4l2 3h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>`,
  settings: `<path d="M4 7h10M18 7h2M4 17h4M12 17h8"/><circle cx="16" cy="7" r="2"/><circle cx="10" cy="17" r="2"/>`,
  editor: `<path d="M6 3h9l4 4v13a1 1 0 0 1-1 1H6a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1z"/><path d="M14 3v4h4M9 13l2.5-2.5a1.5 1.5 0 0 1 2.1 0l.9.9a1.5 1.5 0 0 1 0 2.1L12 16"/>`,
  info: `<circle cx="12" cy="12" r="9"/><path d="M12 8h.01M12 11v5"/>`,
  analytics: `<path d="M4 20V10M10 20V4M16 20v-8M21 20H3"/>`,
  calculator: `<rect x="5" y="3" width="14" height="18" rx="2"/><path d="M8 7h8M8 12h.01M11 12h.01M14 12h.01M8 15h.01M11 15h.01M14 15h.01M8 18h.01M11 18h.01M14 18h.01"/>`,
  network: `<rect x="4" y="9" width="16" height="10" rx="2"/><path d="M2 9l10-5 10 5M8 13v2M12 13v2M16 13v2"/>`,
  audio: `<path d="M4 10v4h4l5 4V6l-5 4H4z"/><path d="M17 8.5a5 5 0 0 1 0 7M19.5 6a9 9 0 0 1 0 12"/>`,
  chip: `<rect x="6" y="6" width="12" height="12" rx="2"/><rect x="9" y="9" width="6" height="6" rx="1"/><path d="M9 3v3M15 3v3M9 18v3M15 18v3M3 9h3M3 15h3M18 9h3M18 15h3"/>`,
  search: `<circle cx="11" cy="11" r="7"/><path d="M21 21l-4-4"/>`,
  power: `<path d="M12 3v9M6.3 6.3a8 8 0 1 0 11.4 0"/>`,
  screen: `<rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8M12 17v4"/>`,
  display: `<rect x="2" y="3" width="20" height="14" rx="2"/><path d="M8 21h8M12 17v4"/>`,
  // Store / shopping
  store: `<path d="M4 8h16l-1 12H5L4 8z"/><path d="M4 8l1.5-4h13L20 8M9 8V6a3 3 0 0 1 6 0v2"/>`,
  bag: `<rect x="4" y="7" width="16" height="14" rx="2"/><path d="M8 7V6a4 4 0 0 1 8 0v1"/>`,
  // Browser / globe
  browser: `<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c2.5 2.4 3.8 5.6 3.8 9S14.5 18.6 12 21M12 3C9.5 5.4 8.2 8.6 8.2 12s1.3 6.6 3.8 9"/>`,
  // Discord
  discord: `<path d="M8 6c-3 1-4 4-4 7v5h4l1.5-2h5L16 18h4v-5c0-3-1-6-4-7l-1.5 2h-5L8 6z"/><circle cx="9" cy="12" r="1"/><circle cx="15" cy="12" r="1"/>`,
  chat: `<path d="M4 5h16v11H9l-5 4V5z"/><path d="M8 9h8M8 12h5"/>`,
  // Steam
  steam: `<path d="M3 12a6 6 0 0 1 12-2.8M12 12a4 4 0 0 1 4 4"/><circle cx="12" cy="12" r="9"/><circle cx="8" cy="15" r="2"/><path d="M9.5 13.5l5-3"/>`,
  gamepad: `<path d="M7 9h10l1 3h1.5a1.5 1.5 0 0 1 1.5 1.5V15a3 3 0 0 1-3 3h-1.4a2 2 0 0 1-1.5-.7l-.8-.9a1 1 0 0 0-1.6 0l-.8.9a2 2 0 0 1-1.5.7H7a3 3 0 0 1-3-3v-1.5A1.5 1.5 0 0 1 5.5 12H7l1-3z"/><path d="M9.5 14v-1M9 13.5h1M17 13.5h.01M17.5 15h.01"/>`,
  image: `<rect x="3" y="4" width="18" height="16" rx="3"/><circle cx="9" cy="10" r="1.5"/><path d="M4 18l5-5 3 3 3-3 5 5"/>`,
  download: `<path d="M12 3v12M7 10l5 5 5-5M4 21h16"/>`,
  wifi: `<path d="M2 9a12 12 0 0 1 20 0M5 13a8 8 0 0 1 14 0M9 17a4 4 0 0 1 6 0"/><circle cx="12" cy="20" r="1"/>`,
  globe: `<circle cx="12" cy="12" r="9"/><path d="M3 12h18M12 3c2.5 2.4 3.8 5.6 3.8 9S14.5 18.6 12 21M12 3C9.5 5.4 8.2 8.6 8.2 12s1.3 6.6 3.8 9"/>`,
  game: `<path d="M6 4h12l2 12H4L6 4z"/><circle cx="9" cy="12" r="1"/><circle cx="15" cy="12" r="1"/><path d="M9 14v-1M8.5 13.5h1"/>`,
  music: `<path d="M8 18V6l10-2v11"/><circle cx="6" cy="18" r="2"/><circle cx="16" cy="15" r="2"/>`,
  folderPlus: `<path d="M3 7a2 2 0 0 1 2-2h4l2 3h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/><path d="M12 11v4M10 13h4"/>`,
  filePlus: `<path d="M6 3h9l4 4v13a1 1 0 0 1-1 1H6a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1z"/><path d="M14 3v4h4M12 11v4M10 13h4"/>`,
  refresh: `<path d="M20 11a8 8 0 1 0-2.3 6M20 4v5h-5"/>`,
  paint: `<path d="M4 15a8 8 0 1 1 16 0 3 3 0 0 1-3 3h-2.5a1.5 1.5 0 0 0-1.5 1.5c0 1-.8 1.5-1.5 1.5H8a4 4 0 0 1-4-4z"/><circle cx="9" cy="10" r="1"/><circle cx="12" cy="7" r="1"/><circle cx="15" cy="10" r="1"/>`
};

ICON_PATHS.system_info = ICON_PATHS.info;
ICON_PATHS.browser2 = ICON_PATHS.browser;
ICON_PATHS.wm = ICON_PATHS.apps;
ICON_PATHS.downloads = ICON_PATHS.download;
ICON_PATHS.photos = ICON_PATHS.image;

// =============================================================================
// LinuxOSZero — Custom SVG icon set (no emoji)
// Icons are hand-drawn, clean, flat/line-style with the accent color.
// =============================================================================

function svgIcon(name, size) {
  const s = size || 24;
  const stroke = 'currentColor';
  const common = {
    fill: 'none',
    stroke: stroke,
    'stroke-width': 1.8,
    'stroke-linecap': 'round',
    'stroke-linejoin': 'round'
  };
  const attrs = Object.entries(common).map(([k, v]) => `${k}="${v}"`).join(' ');
  const body = ICON_PATHS[name] || ICON_PATHS.info;
  return `<svg width="${s}" height="${s}" viewBox="0 0 24 24" ${attrs}>${body}</svg>`;
}

// Icon path bodies (24x24 grid, stroke-based)
const ICON_PATHS = {
  // Installer: optical disc with arrow
  installer: `
    <circle cx="12" cy="12" r="8"/>
    <circle cx="12" cy="12" r="2.5"/>
    <path d="M12 9.5v5M9.8 11.3l2.2-2.2 2.2 2.2"/>`,
  // Terminal: prompt chevron + underscore
  terminal: `
    <rect x="3" y="4" width="18" height="16" rx="3"/>
    <path d="M7 9l3 3-3 3M12.5 15H17"/>`,
  // File manager: folder
  filemanager: `
    <path d="M3 7a2 2 0 0 1 2-2h4l2 3h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/>`,
  // Settings: sliders
  settings: `
    <path d="M4 7h10M18 7h2M4 17h4M12 17h8"/>
    <circle cx="16" cy="7" r="2"/>
    <circle cx="10" cy="17" r="2"/>`,
  // Editor: pencil in document
  editor: `
    <path d="M6 3h9l4 4v13a1 1 0 0 1-1 1H6a1 1 0 0 1-1-1V4a1 1 0 0 1 1-1z"/>
    <path d="M14 3v4h4M9 13l2.5-2.5a1.5 1.5 0 0 1 2.1 0l.9.9a1.5 1.5 0 0 1 0 2.1L12 16"/>`,
  // Fetch / about: circle-info
  info: `
    <circle cx="12" cy="12" r="9"/>
    <path d="M12 8h.01M12 11v5"/>`,
  // Analytics / monitor: bar chart
  analytics: `
    <path d="M4 20V10M10 20V4M16 20v-8M21 20H3"/>`,
  // Calculator
  calculator: `
    <rect x="5" y="3" width="14" height="18" rx="2"/>
    <path d="M8 7h8M8 12h.01M11 12h.01M14 12h.01M8 15h.01M11 15h.01M14 15h.01M8 18h.01M11 18h.01M14 18h.01"/>`,
  // Wallpaper / image
  image: `
    <rect x="3" y="4" width="18" height="16" rx="3"/>
    <circle cx="9" cy="10" r="1.5"/>
    <path d="M4 18l5-5 3 3 3-3 5 5"/>`,
  // Power / logout
  power: `
    <path d="M12 3v9M6.3 6.3a8 8 0 1 0 11.4 0"/>`,
  // Driver / chip
  chip: `
    <rect x="6" y="6" width="12" height="12" rx="2"/>
    <rect x="9" y="9" width="6" height="6" rx="1"/>
    <path d="M9 3v3M15 3v3M9 18v3M15 18v3M3 9h3M3 15h3M18 9h3M18 15h3"/>`,
  // Search
  search: `
    <circle cx="11" cy="11" r="7"/><path d="M21 21l-4-4"/>`,
  // Grid / apps
  apps: `
    <rect x="4" y="4" width="5" height="5" rx="1"/><rect x="15" y="4" width="5" height="5" rx="1"/>
    <rect x="4" y="15" width="5" height="5" rx="1"/><rect x="15" y="15" width="5" height="5" rx="1"/>`,
  // Network
  network: `
    <rect x="4" y="9" width="16" height="10" rx="2"/>
    <path d="M2 9l10-5 10 5M8 13v2M12 13v2M16 13v2"/>`,
  // Audio
  audio: `
    <path d="M4 10v4h4l5 4V6l-5 4H4z"/>
    <path d="M17 8.5a5 5 0 0 1 0 7M19.5 6a9 9 0 0 1 0 12"/>`
};

// Legacy alias names used by older code
ICON_PATHS.system_info = ICON_PATHS.info;

# MISC Workspace — Psyerns Framework Web & WordPress

## Zwei Deployment-Varianten

Nutze **eine** der beiden Varianten — nicht beide gleichzeitig:

| | WordPress Plugin | Standalone PHP |
|---|---|---|
| **Ordner** | `wordpress-plugin/` | `standalone/` |
| **Braucht** | WordPress + MySQL | Jedes PHP 7.4+ Hosting |
| **Storage** | MySQL (wpdb) | JSON-Dateien |
| **Setup** | Plugin installieren + aktivieren | Ordner hochladen, `config.php` anpassen |
| **Admin** | WordPress Admin-Panel | `admin.html` (API-Key geschützt) |

## Struktur

```
MISC/
├── WORKSPACE.md                         ← Diese Datei
├── pf_API.md                            ← API-Dokumentation (Referenz)
├── prompt.md                            ← Ninjin Leaderboard Prompt
├── prompt-wordpress-templates.md        ← Komplett-Prompt (WP + Standalone + 6 Agents)
│
├── wordpress-plugin/                    ← Variante A: WordPress Plugin
│   └── psyerns-framework/
│       ├── psyerns-framework.php
│       ├── includes/
│       ├── admin/
│       ├── public/
│       └── uninstall.php
│
├── standalone/                          ← Variante B: Ohne WordPress
│   ├── index.html                       ← Dashboard
│   ├── player.html                      ← Player Card
│   ├── admin.html                       ← Admin Panel
│   ├── api/                             ← PHP Backend
│   │   ├── config.php
│   │   ├── receive.php
│   │   ├── leaderboard.php
│   │   ├── status.php
│   │   ├── whitelist.php
│   │   ├── players.php
│   │   ├── avatar.php
│   │   └── ping.php
│   ├── data/                            ← JSON Storage
│   ├── cache/                           ← Avatar Cache
│   └── assets/css+js+img/              ← Frontend Assets
│
└── templates/                           ← Standalone HTML Templates (iframe-fähig)
    ├── leaderboard.html
    ├── top3-monthly.html
    ├── top3-deadliest.html
    ├── player-card.html
    └── assets/
```

## Themes

Alle Templates und beide Varianten nutzen die gleichen CSS-Klassen:
- `.pf-dark` → DayZ/Survival Dark Theme
- `.pf-light` → Clean Modern Theme

## Spielerbilder

- Automatisch von Steam API (SteamID → Avatar URL)
- Cache: 24h (WordPress: Transient, Standalone: Datei-Cache)
- Fallback: `default-avatar.png`

# Prompt: WordPress Plugin + HTML Templates für Psyerns Framework

Du bist ein erfahrener WordPress-Entwickler. Erstelle ein vollständiges,
produktionsreifes WordPress-Plugin nach WordPress Coding Standards.

Lies zuerst diese Dateien für Kontext:
- **API-Doku:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\pf_API.md`
- **WP Rules:** `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\wordpress-plugin.md`
- **WP Prompt-Template:** `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\wordpress-plugin-prompt.md`

**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\`

---

## Plugin-Metadaten

- Plugin-Name: Psyerns Framework
- Plugin-Slug: psyerns-framework
- Kurzbeschreibung: DayZ Server Integration — Leaderboard, Server Status, Whitelist, KillFeed
- Version: 1.0.0
- Autor: Psyern
- Autor-URL: https://deadmansecho.com
- Textdomain: psyerns-framework
- WP Mindestversion: 5.8
- PHP Mindestversion: 7.4
- Lizenz: GPL-2.0+

---

## Gewünschte Funktionen

### REST-API Endpunkte (Namespace: `psyern/v1`)

Private Endpoints (API-Key als Query-Parameter `?api_key=...`):
- [x] `POST /server/status` — Empfängt Server-Status, speichert als Transient (120s TTL)
- [x] `POST /upload` — Empfängt Leaderboard-Daten, Upsert per steam_id in DB
- [x] `GET /whitelist/check?steam_id=...` — Prüft Whitelist, gibt `{"whitelisted": true/false}`
- [x] `POST /whitelist/add` — Fügt Spieler zur Whitelist hinzu (Body: `{"steamId":"...","name":"..."}`)
- [x] `POST /whitelist/remove` — Entfernt Spieler (Body: `{"steamId":"..."}`)
- [x] `GET /players/lookup?steam_id=...` — Spielerdaten aus DB
- [x] `GET /players/online` — Alle Online-Spieler
- [x] `GET /ping` — Healthcheck `{"status": "ok"}`

Öffentliche Endpoints (kein API-Key):
- [x] `GET /public/leaderboard?type=pve|pvp&limit=20` — Top-Spieler aus DB mit Avatar-URL
- [x] `GET /public/status` — Server-Status aus Transient
- [x] `GET /public/top3?type=monthly|deadliest` — Top 3 Spieler
- [x] `GET /public/avatar?steam_id=...` — Steam Avatar URL (gecacht)

### Admin-Einstellungsseite

Menü-Position: Eigenes Top-Level-Menü "Psyerns Framework"
- Gruppe "API Settings":
  - `pf_api_key`: Text-Input, Label "API Key", Standard ""
  - `pf_steam_api_key`: Text-Input, Label "Steam API Key", Standard ""
  - `pf_default_theme`: Select mit Optionen: dark, light
- Unterseite "Whitelist":
  - Tabelle aller Whitelist-Einträge (Steam ID, Name, Datum)
  - Formular zum Hinzufügen (Steam ID + Name)
  - Lösch-Button pro Eintrag (mit Nonce)

### Datenbanktabellen

Erstelle bei Plugin-Aktivierung via `dbDelta()`:

1. `{$wpdb->prefix}pf_whitelist`:
   - id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY
   - steam_id VARCHAR(20) NOT NULL UNIQUE
   - name VARCHAR(255) NOT NULL DEFAULT ''
   - added_at DATETIME DEFAULT CURRENT_TIMESTAMP

2. `{$wpdb->prefix}pf_leaderboard`:
   - id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY
   - steam_id VARCHAR(20) NOT NULL
   - player_name VARCHAR(255) NOT NULL DEFAULT ''
   - kills INT DEFAULT 0
   - deaths INT DEFAULT 0
   - ai_kills INT DEFAULT 0
   - longest_shot FLOAT DEFAULT 0
   - playtime FLOAT DEFAULT 0
   - pve_points INT DEFAULT 0
   - pvp_points INT DEFAULT 0
   - pve_deaths INT DEFAULT 0
   - pvp_deaths INT DEFAULT 0
   - board_type ENUM('pve','pvp') NOT NULL DEFAULT 'pve'
   - category_kills TEXT (JSON string)
   - category_deaths TEXT (JSON string)
   - category_longest_ranges TEXT (JSON string)
   - is_online TINYINT DEFAULT 0
   - last_login VARCHAR(30) DEFAULT ''
   - updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
   - UNIQUE KEY steam_board (steam_id, board_type)

3. `{$wpdb->prefix}pf_killfeed`:
   - id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY
   - killer_steam_id VARCHAR(20) DEFAULT ''
   - killer_name VARCHAR(255) DEFAULT 'Unknown'
   - killer_weapon VARCHAR(255) DEFAULT 'Unknown'
   - victim_steam_id VARCHAR(20) DEFAULT ''
   - victim_name VARCHAR(255) DEFAULT 'Unknown'
   - pos_x FLOAT DEFAULT 0
   - pos_y FLOAT DEFAULT 0
   - pos_z FLOAT DEFAULT 0
   - distance FLOAT DEFAULT 0
   - killed_at DATETIME DEFAULT CURRENT_TIMESTAMP

### Steam Avatar Integration

Klasse `PF_Steam`:
- `static get_avatar(string $steam_id)` → Avatar-URL oder Fallback
- Steam Web API: `https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/`
- Cache als Transient: `pf_avatar_{steam_id}` (TTL: 24h)
- Fallback: `PF_PLUGIN_URL . 'public/assets/img/default-avatar.png'`
- Steam API Key aus `get_option('pf_steam_api_key')`

### Shortcodes

Alle Shortcodes geben einen Container-`<div>` mit data-Attributen aus. JS lädt Daten per `fetch()` und rendert. Output immer als Return-Wert (kein echo).

- `[pf_leaderboard type="pve" limit="20" theme="dark"]`
- `[pf_server_status theme="dark"]`
- `[pf_top3_monthly theme="dark"]`
- `[pf_top3_deadliest theme="dark"]`
- `[pf_player_card steam_id="76561198..." theme="dark"]`

### CSS Themes (zwei Varianten per Klasse)

Theme-Klasse auf Container: `.pf-dark` oder `.pf-light`

**DayZ Dark Theme** (`pf-dark.css`):
- Hintergrund: `#1a1a2e`, Cards: `#16213e`
- Accent/Gold: `#f0c040`, Silber: `#c0c0c0`, Bronze: `#cd7f32`
- Text: `#e0e0e0`, Muted: `#8892a0`
- Cards: `background: rgba(22, 33, 62, 0.85); backdrop-filter: blur(8px);`
- Top 3 Cards: Border-Glow in Gold/Silber/Bronze

**Clean Modern Theme** (`pf-light.css`):
- Hintergrund: `#f5f5f5`, Cards: `#ffffff`
- Accent: `#3498db`
- Cards: `box-shadow: 0 2px 8px rgba(0,0,0,0.08); border-radius: 8px;`

**Gemeinsame Basis** (`pf-common.css`):
- `.pf-container`, `.pf-tabs`, `.pf-table`, `.pf-card`, `.pf-avatar`
- `.pf-badge-online` / `.pf-badge-offline`, `.pf-rank-1/2/3`, `.pf-top3-grid`
- Responsive: 768px + 480px Breakpoints
- iframe-fähig

### JavaScript (`pf-templates.js`)

Shared JS für Shortcodes und Standalone-Templates:
- `fetchLeaderboard(type, limit)` → Promise
- `fetchServerStatus()` → Promise
- `fetchTop3(type)` → Promise
- `renderLeaderboardTable(players, type, container)` → DOM
- `renderTop3Cards(players, container)` → DOM
- `renderPlayerCard(player, container)` → DOM
- `renderServerStatus(status, container)` → DOM
- `startAutoRefresh(callback, interval)` → Interval-ID
- `formatNumber(n)`, `formatDate(iso)`, `getKDRatio(kills, deaths)`

---

## Technische Anforderungen

- Architektur: OOP (Klassen), jede Klasse eigene Datei
- Sicherheit: Nonces, Capability-Checks, Input-Sanitizing, Output-Escaping
- Datenbankzugriff: `$wpdb->prepare()` — kein direktes SQL
- Assets: CSS/JS nur auf relevanten Seiten laden
- Übersetzbar: Alle Strings mit `__()` / `esc_html__()` wrappen, Textdomain `psyerns-framework`
- Coding Standard: WordPress PHP Coding Standards (Tabs, snake_case, Klammern auf gleicher Zeile)
- Uninstall-Hook: Tabellen droppen, Optionen löschen, Transients aufräumen

---

## Dateien die erstellt werden sollen

Erstelle ALLE Dateien mit vollständigem, funktionstüchtigem Code:

```
wordpress-plugin/psyerns-framework/
├── psyerns-framework.php                    ← Plugin-Header + Bootstrap
├── uninstall.php                            ← Cleanup (Tabellen, Optionen, Transients)
├── includes/
│   ├── class-pf-database.php                ← DB Schema, dbDelta, CRUD
│   ├── class-pf-auth.php                    ← API-Key Validierung
│   ├── class-pf-steam.php                   ← Steam Avatar Resolver + Cache
│   ├── class-pf-api.php                     ← REST Route Registrierung
│   ├── class-pf-server-status.php           ← POST Handler + Transient
│   ├── class-pf-leaderboard.php             ← POST Upload + Upsert + Public GET
│   ├── class-pf-whitelist.php               ← Check/Add/Remove
│   └── class-pf-players.php                 ← Lookup/Online
├── admin/
│   ├── class-pf-admin.php                   ← Menü, Settings, Whitelist-Page
│   ├── views/
│   │   ├── settings-page.php                ← Settings HTML
│   │   └── whitelist-page.php               ← Whitelist Management HTML
│   └── css/
│       └── pf-admin.css                     ← Admin Styles
├── public/
│   ├── class-pf-shortcodes.php              ← Shortcode Registrierung + Output
│   └── assets/
│       ├── css/
│       │   ├── pf-common.css                ← Layout, Grid, Responsive
│       │   ├── pf-dark.css                  ← DayZ Dark Theme
│       │   └── pf-light.css                 ← Clean Modern Theme
│       ├── js/
│       │   └── pf-templates.js              ← Fetch, Render, Auto-Refresh
│       └── img/
│           └── default-avatar.png           ← Fallback Avatar (erstelle als 1x1 Platzhalter)
└── languages/
    └── psyerns-framework.pot                ← Übersetzungs-Template
```

---

## Standalone HTML Templates (iframe-fähig)

Zusätzlich zum Plugin: 4 standalone `.html` Dateien mit eingebettetem CSS+JS. Konfigurierbare API-URL oben im Script. Nutzen die gleichen CSS-Klassen und JS-Funktionen wie das Plugin.

```
templates/
├── leaderboard.html          ← Tabs PvE/PvP, Top 20, Theme-Switch, Auto-Refresh
├── top3-monthly.html         ← 3 Cards, sortiert nach kombinierten Punkten
├── top3-deadliest.html       ← 3 Cards, sortiert nach Gesamtkills
└── player-card.html          ← Einzelne Card per ?steam_id=..., Stats-Grid
```

Jedes Template: Standalone, keine externen Dependencies, iframe-fähig, responsive, Theme-Switch Button.

---

## Standalone PHP Edition (ohne WordPress)

Für Hoster ohne WordPress. JSON-Storage statt MySQL. Gleiche API-Responses wie das WP Plugin.

```
standalone/
├── index.html                ← Dashboard (Tabs: Leaderboard, Top 3, Status)
├── player.html               ← Player Card (?steam_id=...)
├── admin.html                ← Admin Panel (API-Key geschützt)
├── api/
│   ├── config.php            ← API Key, Pfade, CORS, Hilfsfunktionen
│   ├── receive.php           ← POST Router (status, upload, whitelist_add/remove)
│   ├── leaderboard.php       ← GET öffentlich
│   ├── status.php            ← GET öffentlich
│   ├── whitelist.php         ← GET/POST mit API-Key
│   ├── players.php           ← GET mit API-Key
│   ├── avatar.php            ← GET öffentlich, Steam Avatar Proxy + Datei-Cache
│   └── ping.php              ← GET öffentlich
├── data/
│   └── .htaccess             ← Deny from all
├── cache/
│   └── .htaccess             ← Deny from all
└── assets/                   ← Kopie der CSS/JS/IMG aus dem WP Plugin
```

**Wichtig:** Gleiche JSON-Response-Formate wie das WordPress Plugin, damit die Templates mit beiden funktionieren.

---

## Whitelist Response Format (Kritisch)

Der DayZ-Client parst die Whitelist-Response per String-Matching:
```json
{"whitelisted": true}
```
Kein anderes Format, keine zusätzlichen Felder vor `whitelisted`. Sowohl WP als auch Standalone müssen exakt dieses Format liefern.

---

## Agent-Orchestrierung

Teile die Arbeit auf 6 parallele Agents auf. Starte alle 6 gleichzeitig mit dem Agent-Tool.

### Agent 1: Plugin Core & Database
**Erstellt:** `psyerns-framework.php`, `class-pf-database.php`, `class-pf-auth.php`, `class-pf-steam.php`, `uninstall.php`, `languages/psyerns-framework.pot`
**Kontext:** Lies `pf_API.md` für Datenstrukturen. DB-Schema wie oben. Alle Strings mit `__()` wrappen. `dbDelta()` für Tabellen. DB-Version in Options speichern.

### Agent 2: REST API Endpoints
**Erstellt:** `class-pf-api.php`, `class-pf-server-status.php`, `class-pf-leaderboard.php`, `class-pf-whitelist.php`, `class-pf-players.php`
**Kontext:** Lies `pf_API.md` für exakte Signaturen. `PF_Auth::validate_api_key()` für private Endpoints. `permission_callback` korrekt implementieren. Alle Inputs sanitizen. Öffentliche Endpoints unter `/public/` ohne Key.

### Agent 3: Admin Backend
**Erstellt:** `class-pf-admin.php`, `views/settings-page.php`, `views/whitelist-page.php`, `css/pf-admin.css`
**Kontext:** WordPress Settings API. Nonces für alle Formulare. `current_user_can('manage_options')`. Alle Outputs escapen. Admin-CSS nur auf Plugin-Seiten laden.

### Agent 4: CSS Themes
**Erstellt:** `pf-common.css`, `pf-dark.css`, `pf-light.css`
**Kontext:** Themes per `.pf-dark`/`.pf-light` Klasse. Responsive 768px + 480px. iframe-fähig. Farben wie oben definiert. Wird von WP-Shortcodes UND Standalone-Templates genutzt.

### Agent 5: HTML Templates + JavaScript
**Erstellt:** `pf-templates.js`, `class-pf-shortcodes.php`, `templates/leaderboard.html`, `templates/top3-monthly.html`, `templates/top3-deadliest.html`, `templates/player-card.html`
**Kontext:** Shortcode-Output als Return (kein echo). Standalone-HTMLs mit eingebettetem CSS+JS. API-URL konfigurierbar. Auto-Refresh 5 Min.

### Agent 6: Standalone PHP Edition
**Erstellt:** Alle Dateien unter `standalone/`
**Kontext:** JSON-Storage mit `flock()`. Gleiche Response-Formate wie WP Plugin. CORS-Header. `.htaccess` Schutz. Kopiere CSS/JS von Agent 4/5.

---

## Ausführungsreihenfolge

```
┌──────────────────────────────────────────────────────────────────┐
│                   Alle 6 Agents parallel starten                │
├─────────┬───────────┬──────────┬─────────┬──────────┬───────────┤
│ Agent 1 │  Agent 2  │ Agent 3  │ Agent 4 │ Agent 5  │  Agent 6  │
│ Core+DB │  REST API │  Admin   │   CSS   │ HTML+JS  │ Standalone│
├─────────┴───────────┴──────────┴─────────┴──────────┴───────────┤
│                              ↓                                  │
│           Orchestrator prüft: Response-Formate gleich,          │
│           CSS-Klassen konsistent, require_once Pfade korrekt    │
└─────────────────────────────────────────────────────────────────┘
```

## Hinweise für den Orchestrator

1. **Starte alle 6 Agents gleichzeitig** in einer Message
2. **Jeder Agent liest zuerst `pf_API.md`** und `rules/wordpress-plugin.md`
3. **Keine Abhängigkeiten zwischen Agents** — jeder schreibt nur eigene Dateien
4. **Nach Abschluss prüfen:** require_once Pfade, Response-Format-Kompatibilität WP↔Standalone, CSS-Klassen-Konsistenz
5. **Verzeichnisse erstellen** mit `mkdir -p` vor dem Schreiben
6. **ZIP erstellen** nach Abschluss: `wordpress-plugin/psyerns-framework/` → `psyerns-framework.zip`

## Installations- und Testanleitung

1. ZIP in WordPress hochladen → Plugins → Installieren → Aktivieren
2. Psyerns Framework → Settings → API Key + Steam API Key setzen
3. `curl https://deine-seite.de/wp-json/psyern/v1/ping?api_key=DEIN_KEY` → `{"status":"ok"}`
4. DayZ Server Config: WordPress Endpoint URL + API Key eintragen, Enabled = true
5. Server starten → Daten fließen → Shortcode `[pf_leaderboard]` auf beliebiger Seite einfügen

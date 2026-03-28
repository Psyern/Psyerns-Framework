# Psyerns Framework — HTML/PHP Standalone Plugin
## Multi-Agent Orchestration Prompt

> **Zweck:** Diesen Prompt in Claude.ai einfügen um ein vollständiges, eigenständiges HTML/PHP Leaderboard-Plugin zu bauen, das **ohne WordPress** funktioniert — auf jedem PHP-Webserver (Apache/Nginx + PHP 7.4+).

---

## Kontext & Ausgangslage

Das **Psyerns Framework** ist ein DayZ-Server Leaderboard-System.  
Es existiert bereits als WordPress-Plugin. Dieses Projekt ist die **Standalone HTML/PHP Version** — für einfache Webseiten ohne CMS.

### Was bereits existiert (Referenz-Code)

**Standalone Basis** (`MISC/standalone/`):
- `index.html` — Hauptseite mit Tabs (Leaderboard, Top3, Status)
- `admin.html` — Admin-Panel mit API-Key Auth (Whitelist-Verwaltung)
- `player.html` — Einzelne Spieler-Karte via `?steam_id=...`
- `api/config.php` — Zentrale Config + Helper-Funktionen (file-based JSON Storage)
- `api/leaderboard.php` — GET Leaderboard-Daten
- `api/receive.php` — POST Eingang für DayZ-Server Daten (upload/status/whitelist)
- `api/status.php` — Server-Status abrufen
- `api/whitelist.php` — Whitelist CRUD
- `api/players.php` — Einzelspieler lookup
- `api/avatar.php` — Steam Avatar Cache
- `api/top3.php` — Top 3 Abruf
- `api/ping.php` — Health Check

**Frontend JS** (`public/assets/js/pf-templates.js`):
```javascript
const PF = {
    config: { apiUrl: '/api', refreshInterval: 300000 },
    // Methoden:
    fetchLeaderboard(type, limit)     // GET /api/leaderboard.php
    fetchServerStatus()               // GET /api/status.php
    fetchTop3(type)                   // GET /api/top3.php
    renderLeaderboardTable(data, type, container)
    renderTop3Cards(players, container, deadliest)
    renderTop3BossKills(players, container)
    renderPlayerCard(player, container)
    renderServerStatus(status, container)
    factionBadge(faction)
    renderFactionBar(east, west)
    formatNumber(n) / formatDate(iso) / getKDRatio(k, d) / escHtml(s)
}
```

**Theme-Effekte** (`public/js/psyern-theme-effects.js`):
- Stalker: Floating Biohazard `<img>` Icons (opacity + filter, z-index:-1)
- Ops: Matrix Rain Canvas
- Inferno: Ember Particles Canvas
- Bubblegum: Bubble Canvas
- Frostbite: Snowflake Canvas
- Geiger Counter Display (Stalker)
- Scan-Line Animation (Stalker)

**9 CSS Themes**: `military`, `ops`, `stalker`, `outbreak`, `cyberpunk`, `inferno`, `ash`, `frostbite`, `bubblegum`

**Kritische technische Details:**
- WordPress wandelt `☢` Unicode in `<img class="emoji">` um → `color:` funktioniert nicht → stattdessen `opacity` + `filter: sepia(1) saturate(4) hue-rotate(10deg)`
- Biohazard-Icons: `z-index:-1`, `opacity: 0.18–0.25`, 5 individuelle Float-Keyframes
- Pagination: Basis `color: rgba(255,255,255,0.55) !important` — kein `color:inherit` (würde Theme-Farben durchbluten)
- Player-Card: CSS-Klassen statt Inline-Styles für Responsive-Breakpoints

---

## Aufgabe für die Agenten

### 🎯 Ziel

Erstelle ein **produktionsreifes, eigenständiges HTML/PHP Plugin** im Ordner `HTML-Plugin/` mit:

1. **Drop-in Installation** — einfach auf Webserver hochladen, fertig
2. **Null WordPress-Abhängigkeiten**
3. **Gleicher Funktionsumfang** wie das WordPress-Plugin
4. **Einfache Konfiguration** via einer einzigen `config.php`
5. **Saubere URL-Struktur** mit `.htaccess` Router

---

## Agent 1 — Projektstruktur & Config

**Aufgabe:** Erstelle die vollständige Ordnerstruktur und Konfigurations-Dateien.

### Ziel-Dateistruktur

```
HTML-Plugin/
├── config.php                 ← EINZIGE Konfigurationsdatei
├── index.php                  ← Hauptseite (PHP Template)
├── leaderboard.php            ← Leaderboard-Seite (einbettbar)
├── player.php                 ← Spieler-Karte (?steam_id=...)
├── admin.php                  ← Admin-Panel (Session-Auth)
├── admin-login.php            ← Login-Seite
├── .htaccess                  ← URL-Router + Sicherheits-Regeln
├── api/
│   ├── router.php             ← API-Dispatcher (ersetzt WP REST)
│   ├── leaderboard.php
│   ├── status.php
│   ├── top3.php
│   ├── player.php
│   ├── whitelist.php
│   ├── receive.php            ← DayZ → Server POST-Endpoint
│   ├── ping.php
│   └── avatar.php
├── data/                      ← JSON-Datenspeicher (writable)
│   ├── leaderboard.json
│   ├── server-status.json
│   └── whitelist.json
├── cache/                     ← Avatar-Cache (writable)
├── assets/
│   ├── css/
│   │   ├── psyern-leaderboard.css    ← Basis-Styles
│   │   ├── psyern-theme-military.css
│   │   ├── psyern-theme-ops.css
│   │   ├── psyern-theme-stalker.css
│   │   ├── psyern-theme-outbreak.css
│   │   ├── psyern-theme-cyberpunk.css
│   │   ├── psyern-theme-inferno.css
│   │   ├── psyern-theme-ash.css
│   │   ├── psyern-theme-frostbite.css
│   │   └── psyern-theme-bubblegum.css
│   ├── js/
│   │   ├── pf-templates.js           ← Frontend Render-Engine
│   │   └── psyern-theme-effects.js   ← Theme-Animationen
│   └── img/
├── templates/
│   ├── header.php             ← HTML Head + Nav
│   ├── footer.php             ← Scripts + Close-Tags
│   └── leaderboard-base.php  ← Leaderboard HTML-Gerüst
└── README.md
```

### `config.php` Anforderungen

```php
<?php
// ═══════════════════════════════════════
// Psyerns Framework — Standalone Config
// ═══════════════════════════════════════

// API Security
define('PF_API_KEY',        'CHANGE_ME_TO_RANDOM_SECRET');
define('PF_ADMIN_PASSWORD', 'CHANGE_ADMIN_PASSWORD');

// Steam Integration (optional)
define('PF_STEAM_API_KEY',  '');

// Theme (military|ops|stalker|outbreak|cyberpunk|inferno|ash|frostbite|bubblegum)
define('PF_DEFAULT_THEME',  'stalker');

// Server Info
define('PF_SERVER_NAME',    'My DayZ Server');
define('PF_SITE_TITLE',     'Server Leaderboard');

// Paths (auto-detected, normalerweise nicht ändern)
define('PF_DATA_DIR',       __DIR__ . '/data/');
define('PF_CACHE_DIR',      __DIR__ . '/cache/');
define('PF_BASE_URL',       '');  // leer = auto-detect

// Cache Durations
define('PF_AVATAR_CACHE',   86400);   // 24h
define('PF_STATUS_CACHE',   30);      // 30s

// CORS (für DayZ-Server POST)
define('PF_CORS_ORIGIN',    '*');     // oder spezifische IP
```

---

## Agent 2 — PHP API Backend

**Aufgabe:** Erstelle alle PHP API-Endpoints. Kein WordPress, reines PHP mit PDO-fähigem JSON-Storage.

### API-Router (`api/router.php`)

Ersetze den WordPress REST Router:

```
WordPress:  GET /wp-json/psyern/v1/public/leaderboard
Standalone: GET /api/leaderboard.php
            oder GET /api/?route=leaderboard (mit router.php)
```

### Endpoints (alle aus `api/config.php` aufbauend)

#### GET Endpoints (Public)
| Endpoint | Parameter | Beschreibung |
|---|---|---|
| `GET /api/leaderboard.php` | `type=pve\|pvp`, `limit=20` | Leaderboard-Daten |
| `GET /api/status.php` | — | Server-Status |
| `GET /api/top3.php` | `type=monthly\|deadliest` | Top-3 Daten |
| `GET /api/player.php` | `steam_id=...` | Einzelspieler |
| `GET /api/whitelist.php` | `action=list&api_key=...` | Whitelist (Auth!) |
| `GET /api/ping.php` | — | `{"status":"ok"}` |

#### POST Endpoints (API-Key geschützt)
| Endpoint | Body | Beschreibung |
|---|---|---|
| `POST /api/receive.php?endpoint=upload` | Leaderboard JSON | DayZ → Upload |
| `POST /api/receive.php?endpoint=status` | Status JSON | DayZ → Status |
| `POST /api/receive.php?endpoint=whitelist_add` | `{steamId, name}` | Whitelist hinzufügen |
| `POST /api/receive.php?endpoint=whitelist_remove` | `{steamId}` | Whitelist entfernen |

### Auth-System (kein WordPress)

```php
// API Key Validierung (für DayZ-Server Requests)
function pf_validate_key() {
    $key = $_GET['api_key'] ?? $_SERVER['HTTP_X_API_KEY'] ?? '';
    // Auch Header-based für DayZ: X-API-Key: ...
    if (!hash_equals(PF_API_KEY, $key)) {
        http_response_code(401);
        echo json_encode(['error' => 'Unauthorized']);
        exit;
    }
}

// Admin Session Auth (für admin.php)
function pf_require_admin() {
    session_start();
    if (empty($_SESSION['pf_admin'])) {
        header('Location: admin-login.php');
        exit;
    }
}
```

### `api/leaderboard.php` — Vollständig implementieren

Basierend auf dem existierenden Code, aber erweitert um:
- War-Daten: `war_faction`, `war_boss_kills`, `hardline_reputation`
- `globalEastPoints`, `globalWestPoints` für Faction-Bar
- Cache-Header (`Cache-Control: max-age=30`)

### JSON-Datenformat (von DayZ empfangen)

```json
{
  "generatedAt": "2026-03-28T12:00:00Z",
  "playerOnlineCounter": 5,
  "totalPlayers": 150,
  "globalEastPoints": 45000,
  "globalWestPoints": 38000,
  "topPVEPlayers": [
    {
      "playerName": "Psyern",
      "playerID": "76561198...",
      "pvePoints": 12500,
      "pvpPoints": 3200,
      "kills": 145,
      "deaths": 23,
      "aiKills": 890,
      "longestShot": 487.5,
      "playtime": 234.5,
      "isOnline": 1,
      "lastLoginDate": "2026-03-28T11:45:00Z",
      "war_faction": "EAST",
      "war_boss_kills": 3,
      "hardline_reputation": 1250,
      "categoryKills": { "Zombie": 450, "Wolf": 12, "Bear": 3 },
      "categoryLongestRanges": { "Zombie": 125, "Wolf": 87 }
    }
  ],
  "topPVPPlayers": []
}
```

---

## Agent 3 — PHP Frontend Templates

**Aufgabe:** Erstelle die PHP-Template-Seiten ohne WordPress-Template-System.

### `templates/header.php`

```php
<?php
require_once __DIR__ . '/../config.php';
$theme = PF_DEFAULT_THEME;
$title = PF_SITE_TITLE;
?>
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title><?= htmlspecialchars($title) ?></title>
<link rel="stylesheet" href="/assets/css/psyern-leaderboard.css">
<link rel="stylesheet" href="/assets/css/psyern-theme-<?= htmlspecialchars($theme) ?>.css">
<!-- Google Fonts je nach Theme -->
</head>
<body>
<div class="psyern-lb psyern-lb--<?= htmlspecialchars($theme) ?>" id="psyern-lb">
```

### `index.php` — Hauptseite

Anforderungen:
- Theme-Klasse aus `config.php` laden
- Tabs: Leaderboard (PvE/PvP) | Top 3 Monatlich | Top 3 Deadliest | Top Boss Slayer | Server Status
- `pf-templates.js` einbinden
- `psyern-theme-effects.js` einbinden
- `PF.config.apiUrl = '/api'` setzen
- Auto-Refresh alle 5 Minuten

### `leaderboard.php` — Einbettbare Version

Für Sites die nur den Leaderboard-Block einbetten wollen:
- Kein volles HTML-Dokument
- Nur `<div class="psyern-lb ...">` + Scripts
- Kann via `<iframe>` oder PHP `include` eingebettet werden
- Query-Parameter: `?theme=stalker&type=pve&limit=20`

### `player.php` — Spieler-Karte

```
URL: /player.php?steam_id=76561198...
     /player.php?name=Psyern
```

### `admin.php` — Admin-Panel (Session-Auth)

Tabs:
1. **Dashboard** — Server Status Übersicht
2. **Whitelist** — Spieler hinzufügen/entfernen  
3. **Leaderboard** — Daten-Ansicht / manueller Upload
4. **Einstellungen** — Theme, API-Key ändern (in config.php schreiben)

### `admin-login.php`

```php
// POST: Passwort gegen PF_ADMIN_PASSWORD prüfen
// Session setzen: $_SESSION['pf_admin'] = true
// Redirect zu admin.php
```

---

## Agent 4 — Frontend JS Anpassungen

**Aufgabe:** Passe `pf-templates.js` für den Standalone-Einsatz an.

### Änderungen gegenüber WordPress-Version

#### 1. API-URL

```javascript
// WordPress:
config: { apiUrl: '/wp-json/psyern/v1' }

// Standalone:
config: { apiUrl: '/api' }

// Fetch-Calls angepasst:
// WP:         /wp-json/psyern/v1/public/leaderboard
// Standalone: /api/leaderboard.php
```

#### 2. Fetch-Methoden

```javascript
fetchLeaderboard: async function(type, limit) {
    type = type || 'pve'; limit = limit || 20;
    var res = await fetch(PF.config.apiUrl + '/leaderboard.php?type=' + type + '&limit=' + limit);
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
},

fetchServerStatus: async function() {
    var res = await fetch(PF.config.apiUrl + '/status.php');
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
},

fetchTop3: async function(type) {
    type = type || 'monthly';
    var res = await fetch(PF.config.apiUrl + '/top3.php?type=' + type);
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
},

fetchPlayer: async function(steamId) {
    var res = await fetch(PF.config.apiUrl + '/player.php?steam_id=' + encodeURIComponent(steamId));
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
},
```

#### 3. Konfiguration per `data-` Attribut (für Einbettung)

```javascript
// HTML: <div id="psyern-lb" data-api-url="/api" data-theme="stalker">
var container = document.getElementById('psyern-lb');
if (container) {
    if (container.dataset.apiUrl) PF.config.apiUrl = container.dataset.apiUrl;
    if (container.dataset.theme) {
        container.classList.add('psyern-lb--' + container.dataset.theme);
    }
}
```

#### 4. `psyern-theme-effects.js` — WICHTIG: Emoji-Problem

WordPress-spezifisches Problem beachten — aber für Standalone gilt das NICHT:
- In Standalone-HTML wird `☢` NICHT automatisch zu `<img class="emoji">` konvertiert
- **Deshalb:** In der Standalone-Version kann `textContent = '\u2622'` direkt verwendet werden
- Die `opacity` + `filter`-Lösung trotzdem behalten (ist optisch besser als `color`)
- `filter: sepia(1) saturate(4) hue-rotate(10deg) brightness(0.9)` → orangebraune Tönung

---

## Agent 5 — CSS & Themes

**Aufgabe:** Stelle sicher dass alle CSS-Dateien korrekt für Standalone funktionieren.

### Unterschiede WordPress vs. Standalone

| WordPress | Standalone |
|---|---|
| `.psyern-lb` scoped | Gleich — kein Unterschied |
| `wp-content/plugins/...` Pfade | Relative `/assets/css/` Pfade |
| `color: inherit` Problem (WP Theme beeinflusst) | **Kein Problem** — eigene Seite |
| Dashicons | Eigene Icons oder Font Awesome |

### Pagination — kritische Regel

```css
/* WordPress: color:inherit blutete WP-Theme-Farbe durch
   Standalone: Kein WP-Theme vorhanden → trotzdem explizit setzen */
.psyern-lb__page-btn {
    color: rgba(255,255,255,0.55); /* KEIN inherit */
    border: 1px solid rgba(255,255,255,0.25); /* KEIN currentColor */
}
```

### Player Card — Responsive Grid

```css
/* 2-spaltig, bricht bei ≤360px auf 1 Spalte */
.psyern-lb__player-stats-grid {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;
}
@media (max-width: 360px) {
    .psyern-lb__player-stats-grid { grid-template-columns: 1fr; }
}
```

### `.htaccess` für saubere URLs (optional)

```apache
# Verbiete direkten Zugriff auf data/ und cache/
<FilesMatch "\.(json)$">
    Order Allow,Deny
    Deny from all
</FilesMatch>

<Directory "data">
    Order Allow,Deny
    Deny from all
</Directory>

# Optional: Clean URLs
RewriteEngine On
RewriteCond %{REQUEST_FILENAME} !-f
RewriteCond %{REQUEST_FILENAME} !-d
RewriteRule ^api/(.+)$ /api/router.php?route=$1 [QSA,L]
```

---

## Agent 6 — Einbettungs-System (Embed)

**Aufgabe:** Erstelle ein einfaches Embed-System für andere Webseiten.

### Ziel: Leaderboard auf fremder Seite einbetten

#### Option A — iframe

```html
<!-- Einfachste Methode: iframe auf der eigenen Seite -->
<iframe 
    src="https://deinserver.de/leaderboard.php?theme=stalker&type=pve" 
    width="100%" 
    height="800"
    frameborder="0">
</iframe>
```

#### Option B — JavaScript Embed (wie Google Maps)

```html
<!-- Auf fremder Seite: -->
<div id="psyern-leaderboard"></div>
<script 
    src="https://deinserver.de/assets/js/psyern-embed.js"
    data-container="psyern-leaderboard"
    data-api="https://deinserver.de/api"
    data-theme="stalker"
    data-type="pve">
</script>
```

**`psyern-embed.js`** erstellen:
```javascript
(function() {
    var script = document.currentScript;
    var containerId = script.getAttribute('data-container') || 'psyern-leaderboard';
    var apiUrl      = script.getAttribute('data-api')       || '';
    var theme       = script.getAttribute('data-theme')     || 'military';
    var type        = script.getAttribute('data-type')      || 'pve';
    var limit       = script.getAttribute('data-limit')     || '20';

    // CSS laden
    var link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = apiUrl.replace('/api', '') + '/assets/css/psyern-leaderboard.css';
    document.head.appendChild(link);

    var themeLink = document.createElement('link');
    themeLink.rel = 'stylesheet';
    themeLink.href = apiUrl.replace('/api', '') + '/assets/css/psyern-theme-' + theme + '.css';
    document.head.appendChild(themeLink);

    // Container vorbereiten
    var container = document.getElementById(containerId);
    if (!container) return;
    container.className = 'psyern-lb psyern-lb--' + theme;
    container.innerHTML = '<div class="psyern-lb__loading">Loading...</div>';

    // pf-templates.js laden
    var s = document.createElement('script');
    s.src = apiUrl.replace('/api', '') + '/assets/js/pf-templates.js';
    s.onload = function() {
        PF.config.apiUrl = apiUrl;
        PF.fetchLeaderboard(type, parseInt(limit)).then(function(data) {
            PF.renderLeaderboardTable(data, type, container);
        });
    };
    document.head.appendChild(s);
})();
```

#### Option C — PHP Include

```php
<!-- Auf PHP-Seite einfach includen: -->
<?php
define('PF_EMBED_API', 'https://deinserver.de/api');
define('PF_EMBED_THEME', 'stalker');
include 'path/to/psyern-framework/leaderboard.php';
?>
```

---

## Agent 7 — Dokumentation & README

**Aufgabe:** Erstelle vollständige Dokumentation.

### `README.md` Inhalt

```markdown
# Psyerns Framework — HTML/PHP Standalone

Leaderboard-System für DayZ-Server. Läuft auf jedem PHP-Webserver.
Keine Datenbank notwendig — nutzt JSON-Dateien.

## Anforderungen
- PHP 7.4+
- Apache oder Nginx mit PHP-FPM
- `data/` und `cache/` Ordner müssen schreibbar sein (chmod 755)

## Installation
1. Dateien hochladen
2. `config.php` öffnen und anpassen:
   - `PF_API_KEY` — geheimer Schlüssel für DayZ-Server
   - `PF_ADMIN_PASSWORD` — Admin-Panel Passwort
   - `PF_STEAM_API_KEY` — optional für Steam-Avatare
   - `PF_DEFAULT_THEME` — gewünschtes Theme
3. Ordner erstellen: `mkdir data cache` und `chmod 755 data cache`
4. Fertig — Seite im Browser öffnen

## DayZ-Server Anbindung
POST an: `https://deineseite.de/api/receive.php?endpoint=upload&api_key=DEIN_KEY`
Body: JSON (siehe Dokumentation)

## Themes
military | ops | stalker | outbreak | cyberpunk | inferno | ash | frostbite | bubblegum

## Einbetten auf anderer Seite
Option A: iframe
Option B: JavaScript Embed (`psyern-embed.js`)
Option C: PHP include
```

---

## Ausgabe-Anforderungen

### Reihenfolge der Erstellung

```
1. config.php
2. .htaccess
3. api/config.php (Helper-Funktionen)
4. api/leaderboard.php
5. api/status.php
6. api/top3.php
7. api/player.php
8. api/whitelist.php
9. api/receive.php
10. api/ping.php
11. templates/header.php
12. templates/footer.php
13. templates/leaderboard-base.php
14. index.php
15. leaderboard.php (einbettbar)
16. player.php
17. admin-login.php
18. admin.php
19. assets/js/pf-templates.js (angepasst)
20. assets/js/psyern-embed.js (NEU)
21. README.md
```

### Qualitäts-Anforderungen

- **Kein Framework** — reines PHP, kein Laravel/Symfony/etc.
- **Kein npm/node** — kein Build-Prozess
- **Kein Composer** — keine Dependencies
- **Sicher** — SQL-Injection unmöglich (kein SQL), XSS durch `htmlspecialchars()`, CSRF-Token für Admin
- **Performant** — Avatar-Cache, Status-Cache, gzip-fähig
- **Responsive** — Mobile-first CSS
- **Dokumentiert** — jede Datei mit kurzem Kommentar-Header

### Was NICHT neu entwickelt wird (aus WordPress-Version übernehmen)

- ✅ Alle 9 CSS-Theme-Dateien (`psyern-theme-*.css`)
- ✅ `psyern-leaderboard.css` (Basis)
- ✅ `psyern-theme-effects.js` (Theme-Animationen) — direkt übernehmen
- ✅ `pf-templates.js` — NUR `apiUrl` und fetch-Pfade anpassen

---

## Technische Constraints

```
PHP:    7.4+ (kein PHP 8 spezifische Syntax zwingend nötig)
JS:     ES5/ES6 Mix — kein TypeScript, kein bundling
CSS:    Vanilla — kein SASS/LESS
Auth:   PHP Sessions für Admin, API-Key für DayZ
Storage: JSON-Dateien in /data/ (kein MySQL notwendig)
Cache:  Datei-basiert in /cache/
CORS:   Konfigurierbar für DayZ-Server-Kompatibilität
```

---

## Bekannte Probleme aus WordPress-Version (in Standalone irrelevant)

| WordPress-Problem | Standalone-Status |
|---|---|
| WP konvertiert `☢` zu `<img class="emoji">` | ✅ Problem nicht vorhanden |
| WP-Theme beeinflusst `color:inherit` | ✅ Kein WP-Theme |
| `wp-json` API-Prefix | ✅ Direkte PHP-Endpoints |
| `plugins_url()` Pfad-Funktionen | ✅ Relative Pfade |
| Dashicons-Abhängigkeit | ✅ Eigene Icon-Lösung |
| WP Nonce System | ✅ PHP Session + CSRF-Token |

---

## Start-Befehl für Claude

Wenn du diesen Prompt einliest, beginne mit:

```
Ich erstelle jetzt das Psyerns Framework HTML/PHP Standalone Plugin.
Ich starte mit Agent 1 (Projektstruktur & config.php) und arbeite 
mich durch alle 7 Agenten. Für jede Datei zeige ich den vollständigen Code.
```

Arbeite die Agenten **sequentiell** ab. Jeder Agent baut auf dem vorherigen auf.  
Zeige für jede Datei den **vollständigen Code** — keine Platzhalter.

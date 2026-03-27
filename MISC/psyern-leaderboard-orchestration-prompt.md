# Orchestrierungsauftrag: Psyerns Framework WordPress Plugin — Leaderboard UI

## Kontext & Ziel

Du bist der Orchestrator. Koordiniere mehrere spezialisierte Agents, die gemeinsam
das WordPress-Plugin **"Psyerns Framework"** um ein vollständiges Leaderboard-System
erweitern. Das Plugin empfängt bereits Spielerdaten (Kills, Deaths, K/D, Playtime etc.)
vom DayZ-Server über eine REST API. Die Aufgabe ist die Umsetzung eines modularen,
visuell beeindruckenden Leaderboard-Systems mit Shortcode-Integration.

**DayZ Server Framework Doku:** Siehe angehängte README — `Psyerns Framework DayZ Mod`
**WordPress Dev Rules:** `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\wordpress-plugin.md`
**Coding Standards:** Alle WordPress PHP/JS/CSS Coding Standards einhalten (WPCS)

---

## Agent-Aufteilung

### Agent 1 — Backend & REST API (PHP)
**Aufgabe:** WordPress-Plugin-Backend für das Leaderboard

Erstelle alle PHP-Dateien für:

1. **REST-Endpunkte** (Namespace: `psyern/v1`)
   - `GET /leaderboard` — Gibt Leaderboard-Daten zurück
     - Parameter: `mode` (`pvp` | `pve`), `limit` (`10` | `20` | `3` für Top3)
     - Authentifizierung: API-Key via Header oder Query-Parameter
   - `POST /leaderboard` — Empfängt Spielerdaten vom DayZ-Server (bestehender Endpunkt, erweitern)
   - `GET /leaderboard/top3` — Speziell die Top 3 der aktuellen Woche
   - `GET /ping` — Health-Check (bereits vorhanden, beibehalten)

2. **Datenbankschema** (`dbDelta()`)
   - Tabelle `wp_psyern_leaderboard`:
     - `id`, `steam_id`, `player_name`, `kills`, `deaths`, `kd_ratio`,
       `playtime_seconds`, `mode` (`pvp`/`pve`), `score`, `week_number`,
       `year`, `avatar_url`, `last_updated` (DATETIME), `created_at`
   - Tabelle `wp_psyern_weekly_winners` für Top3-Archiv

3. **Admin-Einstellungsseite** (unter Menü: "Psyerns Framework")
   - API Key Verwaltung (anzeigen / regenerieren)
   - Steam API Key Eingabe (für Spieler-Avatare)
   - Connection Test Button mit Ping-Anzeige
   - Modul-Toggles (Switch-Buttons) für:
     - Leaderboard aktivieren/deaktivieren
     - PvP-Modus aktivieren
     - PvE-Modus aktivieren
     - Top3-Wochenanzeige aktivieren
     - Steam Avatar-Laden aktivieren
   - Design-Auswahl: 3 Themes wählbar (`military`, `neon`, `ash`)

4. **Shortcode-System**
   - `[psyern_leaderboard]` — Vollständiges Leaderboard
   - `[psyern_leaderboard mode="pvp" limit="10" theme="military"]`
   - `[psyern_leaderboard mode="pve" limit="20" theme="neon"]`
   - `[psyern_top3 theme="ash"]` — Nur Top 3 der Woche
   - Alle Attribute: `mode`, `limit`, `theme`, `show_avatar`, `show_playtime`

5. **Sicherheit & Standards**
   - `if ( ! defined( 'ABSPATH' ) ) exit;` in jeder Datei
   - Alle Inputs sanitizen, alle Outputs escapen
   - Nonces für Admin-Formulare
   - `current_user_can( 'manage_options' )` für Admin-Aktionen
   - `$wpdb->prepare()` für alle Queries
   - `WP_UNINSTALL_PLUGIN` Check in `uninstall.php`

**Dateistruktur Backend:**
```
psyerns-framework/
├── psyerns-framework.php           ← Plugin-Header + Bootstrap
├── uninstall.php
├── includes/
│   ├── class-psyern-main.php       ← init() Methode, Hook-Registrierung
│   ├── class-psyern-database.php   ← DB-Schema, CRUD-Methoden
│   ├── class-psyern-api.php        ← REST-Endpunkte
│   ├── class-psyern-shortcode.php  ← Shortcode-Registrierung + Render-Logic
│   └── class-psyern-steam.php      ← Steam Avatar API
├── admin/
│   ├── class-psyern-admin.php      ← Admin-Seite, Settings API
│   └── views/
│       └── settings-page.php       ← Admin-Template
└── languages/
    └── psyerns-framework.pot
```

---

### Agent 2 — Frontend UI/UX (3 Themes, PHP-Templates + CSS + JS)

**Aufgabe:** Erstelle 3 vollständige, visuell beeindruckende Leaderboard-Designs
im DayZ-Kontext. Jedes Design muss als Shortcode-Output funktionieren.

**Kontext:** DayZ ist ein postapokalyptisches Survival-Spiel. Rau, dunkel, gnadenlos.
Das UI soll dieses Feeling transportieren — kein generisches Gaming-Widget.

---

#### THEME 1: `military` — Taktisch / Militär-HUD

**Aesthetic Direction:** Militär-Briefing-Terminal. Als ob ein NATO-Geheimdienst
die Überlebenden trackt. Grüne Phosphor-Monitore, CRT-Scanlines, Infrarot-Wärmebilder.

- **Farben:** #0a0f0a (fast schwarz), #1a2a1a, Akzent: #4ade80 (Phosphorgrün), #86efac
- **Typografie:** `Courier Prime` oder `Share Tech Mono` für Daten, `Oswald` für Headlines
- **Effekte:**
  - Scanline-Overlay (CSS `repeating-linear-gradient`)
  - Leichtes CRT-Flimmern via CSS-Animation auf dem Container
  - `>` Cursor-Blinking vor Spielernamen
  - Rang-Badges als militärische Insignien (★ ✦ Streifen via CSS)
  - Horizontale Trennlinien als `- - - - - - -` Muster
  - TOP 3 mit pulsierendem Grün-Glow
  - Pixel-Rauschen im Hintergrund via `background-image: url("data:image/svg...")`

**Layout:**
```
┌─────────────────────────────────────────┐
│ [CLASSIFIED] SURVIVOR REGISTRY v2.4     │
│ MODE: [PVP ●] [PVE ○]  WEEK 13 / 2026  │
├─ TOP OPERATIVES ────────────────────────┤
│ #1 ► PLAYERONE        K:247  D:12  KD:20│
│ #2 ► SURVIVOR_X       K:198  D:31  KD:6 │
│ #3 ► GHOST            K:156  D:8   KD:19│
├─ FULL ROSTER ───────────────────────────┤
│ ...                                     │
└─────────────────────────────────────────┘
```

---

#### THEME 2: `neon` — Cyberpunk / Neon Noir

**Aesthetic Direction:** Eine illegale Wettkampfarena in einer untergegangenen Stadt.
Neon-Leuchtreklamen, Regen, Glasscherben. Der Leaderboard hängt an einer kaputten
LED-Wand irgendwo in Chernarus.

- **Farben:** #050508 (tiefschwarz), #0d0d1a, Akzent: #f0e130 (Neon-Gelb), #ff3860 (Neon-Pink), #00d4ff (Cyan)
- **Typografie:** `Black Ops One` oder `Bebas Neue` für Ränge, `JetBrains Mono` für Zahlen
- **Effekte:**
  - Neon-Glow via `text-shadow` und `box-shadow` (mehrschichtig)
  - Glitch-Animation auf dem Titel (CSS `clip-path` alternierend)
  - Horizontale Glitch-Linien bei Hover auf Tabellenzeilen
  - Neon-Rahmen mit animiertem Gradient-Border
  - Regen-Partikel im Header (JS Canvas oder CSS)
  - TOP 3 mit Podium-Effekt und wechselnden Neon-Farben
  - `::before` Pseudo-Elemente als dekorative Neon-Streifen

**Layout:**
```
╔══════════════════════════════════════════╗
║  ⚡ DEADZONE LEADERBOARD ⚡              ║
║  [PVP] ●────────────── [PVE]            ║
╠══════════════════════════════════════════╣
║  🥇 PLAYERONE    247K  12D  20.5KD      ║  ← pink glow
║  🥈 SURVIVOR_X   198K  31D   6.3KD      ║  ← cyan glow
║  🥉 GHOST        156K   8D  19.5KD      ║  ← yellow glow
╠══════════════════════════════════════════╣
║  4  NOMAD        122K  44D   2.7KD      ║
╚══════════════════════════════════════════╝
```

---

#### THEME 3: `ash` — Postapokalyptisch / Verwittert

**Aesthetic Direction:** Ein handgemachtes Aushangschild in einem verlassenen
Supermarkt. Kreidebeschriftung auf Teer. Die Überlebenden führen ihre eigene
Liste mit dem, was sie noch haben: Stolz, Kills und Wut.

- **Farben:** #1a1714 (Kohle), #2d2825 (Asche), #f5f0e8 (vergilbtes Weiß), Akzent: #c8392b (Blut-Rot), #d4a853 (Rost-Gold)
- **Typografie:** `Libre Baskerville` oder `Playfair Display` für Headers, `DM Mono` für Zahlen
- **Effekte:**
  - Paper-Textur via SVG-Noise-Filter oder `background-image` Data-URI
  - Leichte Rotation auf Karten (-1deg bis +1deg) für handgemachten Look
  - Risse/Kratzer als CSS `::after` mit `border` Trick
  - TOP 3 als "Wanted Poster" Cards mit Foto-Rahmen
  - Tally-Marks statt Zahlen für Kills (optional toggle)
  - Rust-colored Trennlinien
  - Verwitterter Stempel-Look auf Rängen

**Layout:**
```
┌────────────────────────────────────────┐
│  DEAD OR ALIVE — SURVIVOR BOARD        │
│  ══════════════════════════════════    │
│                                        │
│  ⬛ 1st  PLAYERONE  ║ ████ 247 kills   │
│  ⬛ 2nd  SURVIVOR_X ║ ███  198 kills   │
│  ⬛ 3rd  GHOST      ║ ███  156 kills   │
│  ─────────────────────────────────     │
│   4.  NOMAD          122 kills         │
└────────────────────────────────────────┘
```

---

**Technische Anforderungen Frontend:**

```
admin/
└── views/
    └── settings-page.php     ← Theme-Auswahl mit Live-Preview
public/
├── css/
│   ├── psyern-leaderboard.css         ← Basis + Shared Styles
│   ├── psyern-theme-military.css      ← Theme 1
│   ├── psyern-theme-neon.css          ← Theme 2
│   └── psyern-theme-ash.css           ← Theme 3
├── js/
│   └── psyern-leaderboard.js          ← Mode-Switch, AJAX-Reload, Animationen
└── templates/
    ├── leaderboard-base.php           ← Wrapper Template
    ├── leaderboard-top3.php           ← Top3 Widget Template
    └── leaderboard-row.php            ← Einzelne Zeile
```

**JS-Funktionalität (psyern-leaderboard.js):**
- `PvP/PvE Switch-Button` ohne Seitenreload (AJAX → `wp_ajax_` Hooks)
- `Top 10 / Top 20 Toggle` per Button-Gruppe
- Animiertes Einblenden der Rows (staggered `animation-delay`)
- Avatar-Lazy-Loading (falls Steam API aktiv)
- Auto-Refresh alle 60 Sekunden (konfigurierbar per `data-Attribut`)
- Alle Strings via `wp_localize_script()` übersetzbar

---

### Agent 3 — Integration & Qualitätssicherung

**Aufgabe:** Verbinde Backend und Frontend, stelle Kompatibilität sicher

1. **Plugin-Bootstrap** (`psyerns-framework.php`):
   - Plugin-Header mit allen Pflichtfeldern
   - Versions-Konstanten: `PSYERN_VERSION`, `PSYERN_PLUGIN_DIR`, `PSYERN_PLUGIN_URL`
   - Autoload aller Klassen
   - Activation/Deactivation Hooks
   - Kompatibilitäts-Check: PHP ≥ 7.4, WP ≥ 5.8

2. **Asset-Enqueuing** (nur auf Seiten mit Shortcode):
   - Google Fonts laden (die Theme-spezifischen Fonts)
   - Nur das aktive Theme-CSS laden
   - `wp_localize_script()` für AJAX-URL, Nonce, Konfiguration

3. **AJAX-Handler:**
   - `wp_ajax_psyern_get_leaderboard` (eingeloggt)
   - `wp_ajax_nopriv_psyern_get_leaderboard` (öffentlich)
   - Nonce-Validierung, Sanitizing, JSON-Response

4. **Deployment-Checkliste prüfen:**
   - Alle Outputs escaped
   - Alle Inputs sanitized
   - Nonces gesetzt
   - Capability Checks vorhanden
   - Kein direktes SQL
   - `ABSPATH` Check in jeder Datei
   - `readme.txt` im WordPress.org Format

---

## Daten-Mapping: DayZ → WordPress

Die folgenden Felder kommen vom DayZ-Server via `PF_WordPressPayload`:

```json
{
  "generatedAt": "2026-03-23T12:00:00Z",
  "totalPlayers": 42,
  "players": [
    {
      "steamId": "76561198000000000",
      "playerName": "Survivor_X",
      "kills": 198,
      "deaths": 31,
      "kdRatio": 6.38,
      "playtimeSeconds": 86400,
      "mode": "pvp",
      "score": 1580
    }
  ]
}
```

Mappe diese Felder auf `wp_psyern_leaderboard` und berechne:
- `week_number` = PHP `date('W')`
- `kd_ratio` = `kills / max(1, deaths)` (falls nicht mitgeliefert)
- `playtime_display` = Umrechnung Sekunden → `Xh Ym`

---

## Ausgabe-Anforderungen

Jeder Agent liefert:
1. **Alle Dateien** als vollständige Code-Blöcke (Dateiname als Kommentar in Zeile 1)
2. **Keine Platzhalter** — produktionsreifer, lauffähiger Code
3. **WordPress Coding Standards** eingehalten (Tabs, snake_case, PHPDoc, etc.)
4. **Am Ende:** Installations-Anleitung in 5 Schritten

---

## Referenzen

- WordPress Plugin Handbook: https://developer.wordpress.org/plugins/
- WordPress Coding Standards: https://developer.wordpress.org/coding-standards/
- Psyerns Framework DayZ README: (angehängte Dokumentation)
- Plugin Boilerplate: https://wppb.me/
- Lokale Dev-Umgebung: WordPress Studio (empfohlen) oder LocalWP
- Dev Rules: `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\wordpress-plugin.md`

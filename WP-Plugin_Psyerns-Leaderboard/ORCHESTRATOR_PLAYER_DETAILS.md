# Player Details Modal — Multi-Agent Orchestrator Prompt

## Project Identity

- **Feature:** Player Detail Modal on Leaderboard
- **Plugin:** Psyerns Leaderboard (WP) + Psyerns Framework (DayZ EnforceScript)
- **Trigger:** Click on a player row → modal with full kill/death breakdown
- **Author:** Psyern / Deadmans Echo

---

## Mission Statement

> Beim Klick auf einen Spieler im Leaderboard soll ein Modal aufgehen, das zeigt
> *was* der Spieler getötet hat (pro Kategorie), *woran* er gestorben ist,
> seine War-Stats, Hardline-Reputation, Gunplay-Stats und Movement/Time.
> Daten werden von DayZ → WP exportiert und über einen REST-Endpoint abgerufen.

---

## Data Source (Already Tracked)

Aus `TrackingModLeaderboardPlayerData` (Ninjin Leaderboard):

| Feld | Typ | Beispiel-Inhalt |
|---|---|---|
| `categoryKills` | `map<string,int>` | `Zombie:1240`, `Boss_Frostbite:4`, `AI_Bandit:88` |
| `categoryDeaths` | `map<string,int>` | `Zombie:12`, `Player:3`, `Wolf:2` |
| `categoryLongestRanges` | `map<string,int>` | `Player:412`, `Zombie:89` (Meter) |
| `warBossKills` | `int` | `4` |
| `warFaction` / `warLevel` / `warAlignment` | string/int/int | — |
| `hardlineReputation` | `int` | `1820` |
| `shotsFired` / `shotsHit` / `headshots` | int | — |
| `distanceTravelled` / `distanceOnFoot` / `distanceInVehicle` | float (m) | — |
| `playTimeSeconds` / `suicides` / `totalDeaths` | int | — |
| `pvpKills` / `pveKills` / `pvpDeaths` / `pveDeaths` | int | — |
| `pvpLongestShot` / `pveLongestShot` | int | — |

---

## Architecture Overview

```
DayZ Server                   WordPress                Browser
───────────                   ─────────                ───────
PF_LeaderboardExport          /wp-json/psyern/v1/
  ├─ TopList (Upload alle X)    ├─ upload         ◄── DayZ POSTs Top+Details
  └─ PlayerDetails per Spieler  ├─ player/{uid}   ◄── JS fetcht beim Klick
                                └─ players (search optional)
                                       │
                                       ▼
                                DB: wp_pf_player_details (JSON pro UID)
                                       │
                                       ▼
                                Modal-Renderer (Tabs: Kills/Deaths/War/…)
```

---

## Agent Plan (3 Phasen, 5 Agents)

### Phase 1: Foundation (parallel)

#### Agent 1: DayZ Export Extension
**Scope:** `PF_LeaderboardExport.c`, `PF_WordPressPayload.c`, neuer `PF_PlayerDetailPayload.c`

**Aufgabe:**
1. Neues Payload-Struct `PF_PlayerDetailPayload`:
   - Alle Felder aus `TrackingModLeaderboardPlayerData` mappen
   - Maps (`categoryKills` etc.) als JSON-Objects serialisieren (nicht Array)
   - `lastUpdated` Timestamp
2. `PF_WordPressPayload` um `array<ref PF_PlayerDetailPayload> playerDetails` erweitern (alle Spieler, nicht nur Top-N)
3. Export-Intervall bleibt gleich; Konfig-Flag `EnablePlayerDetailsExport` (default `true`)
4. Serializer: nutzt `PF_JsonBuilder` für Maps (escape-safe)

**Output:** Erweiterter JSON-Upload an WP `/upload` enthält `playerDetails[]`.

---

#### Agent 2: WordPress DB + REST Endpoint
**Scope:** `class-pf-database.php`, neuer `class-pf-player-details.php`, REST-Registry

**Aufgabe:**
1. Neue DB-Tabelle `{$wpdb->prefix}pf_player_details`:
   ```sql
   player_uid VARCHAR(32) PRIMARY KEY,
   player_name VARCHAR(64),
   data_json LONGTEXT,             -- komplettes Detail-Payload
   updated_at DATETIME
   ```
2. Upload-Endpoint erweitern: schreibt `playerDetails[]` per `INSERT … ON DUPLICATE KEY UPDATE` in die neue Tabelle.
3. Neuer REST-Endpoint `GET /wp-json/psyern/v1/player/(?P<uid>[A-Za-z0-9]+)`:
   - Public, `permission_callback => __return_true`
   - Liefert `data_json` als parsed JSON
   - 404 wenn nicht gefunden
4. Migrations-Trigger via `pf_db_version` Bump.

**Output:** REST endpoint, der `{ playerName, categoryKills:{...}, categoryDeaths:{...}, war:{...}, gunplay:{...}, movement:{...}, time:{...} }` zurückgibt.

---

#### Agent 3: Category Mapping / Pretty Names
**Scope:** `includes/class-pf-category-map.php` (neu)

**Aufgabe:**
1. Lookup-Map `DayZ-Classname → Display-Name + Group + Icon-Slug`:
   - Beispiele: `ZmbM_*` → "Infected" (Group: Zombies), `Boss_Frostbite` → "Frostbite Boss" (Group: Bosses), `Wolf_Grey` → "Grey Wolf" (Group: Animals), `Survivor*` → "Player" (Group: PvP)
2. Gruppierungs-Logik: gibt Buckets zurück: `zombies`, `players`, `bosses`, `ai`, `animals`, `other`
3. Filter-Hook: `pf_category_label`, `pf_category_group` (für Custom-Mods)
4. Defaults für vanilla + DME-WAR + Custom-Boss-Classnames

**Output:** PHP-Klasse mit statischen Methods `get_label($classname)`, `get_group($classname)`, `bucket_kills(array $raw_kills): array`.

---

### Phase 2: Frontend (sequenziell, nach Phase 1)

#### Agent 4: Modal UI + Templates
**Scope:** `public/templates/player-detail-modal.php` (neu), `public/css/player-detail.css` (neu)

**Aufgabe:**
1. Template-Struktur:
   ```
   ┌──────────────────────────────────────┐
   │ [Avatar] PlayerName       [X]        │
   │ Faction · Level · Reputation         │
   ├──────────────────────────────────────┤
   │ [Tab: Übersicht] [Kills] [Deaths]    │
   │ [War] [Gunplay] [Movement]           │
   ├──────────────────────────────────────┤
   │ <Tab-Inhalt — gruppierte Listen>     │
   └──────────────────────────────────────┘
   ```
2. Tabs:
   - **Übersicht:** KPI-Cards (Total Kills, K/D, Boss-Kills, Playtime, Reputation)
   - **Kills:** Liste gruppiert (Zombies / Players / Bosses / AI / Animals) mit Counter
   - **Deaths:** dieselbe Gruppierung — "Killed by …"
   - **War:** Faction, Level, Boss-Kills, Alignment
   - **Gunplay:** Shots Fired/Hit, Accuracy, Headshots, HS%, longest Shot pro Kategorie
   - **Movement:** Foot/Vehicle/Total km, Playtime, Suicides
3. Theme-aware: nutzt `--pf-*` CSS-Variablen aus dem aktiven Theme, kein Hardcoding.
4. Accessibility: `role="dialog"`, `aria-modal="true"`, Focus-Trap, ESC schließt.

**Output:** Server-rendered Skeleton + CSS, JS füllt es mit Daten.

---

#### Agent 5: JS Wiring + Fetch + Render
**Scope:** `public/js/psyern-leaderboard.js` (erweitern), `public/assets/js/pf-templates.js`

**Aufgabe:**
1. Click-Handler auf `.psyern-leaderboard__row[data-uid]` → öffnet Modal
2. Fetch `wp-json/psyern/v1/player/{uid}` mit Loader-State
3. Render-Funktion pro Tab:
   - Übersicht/War/Gunplay/Movement: KPI-Cards
   - Kills/Deaths: gruppiert nach `pf_category_map.group`, sortiert nach Count desc, Top 20 + "show all"
4. Caching: gleiche UID innerhalb 60s nicht erneut fetchen
5. Mobile: Modal füllt Viewport, Tabs horizontal scrollbar
6. Error-States: 404 → "Keine Daten gefunden", Network-Fehler → "Bitte später erneut"

**Output:** Klickbare Rows, Modal mit allen Daten, sauber gerendert.

---

### Phase 3: Polish (sequenziell)

#### Agent 6 (Main): Integration & Settings
**Scope:** `admin/views/settings-page.php`, `class-pf-admin.php`

**Aufgabe:**
1. Admin-Toggle: "Enable Player Detail Modal" (default `on`)
2. Admin-Toggle: "Show Avatar in Modal" (default `on`)
3. Admin-Toggle: "Max Categories per Group" (default 20)
4. Shortcode-Attribut `enable_details="1|0"` für Override pro Page
5. Compile-Check (DayZ), PHP-Syntax-Check (WP), Browser-Smoketest

---

## Cross-Agent Dependencies

```
Phase 1 (parallel):
  Agent 1: DayZ Export       ─┐
  Agent 2: WP DB + REST       ├── unabhängig
  Agent 3: Category Map       ─┘

Phase 2 (sequenziell, hängt von 1+2+3 ab):
  Agent 4: Modal UI          ─── braucht Category-Map für Gruppen-Labels
  Agent 5: JS Wiring         ─── braucht REST-Endpoint + Modal-Skeleton

Phase 3:
  Agent 6: Settings/Polish   ─── nach allem
```

---

## REST Response Schema (Contract)

```json
GET /wp-json/psyern/v1/player/76561198000000000

{
  "playerUid": "76561198000000000",
  "playerName": "Psyern",
  "survivorType": "SurvivorM_Boris",
  "lastUpdated": "2026-05-16T14:00:00Z",
  "summary": {
    "totalKills": 1410,
    "totalDeaths": 27,
    "kd": 52.22,
    "playTimeSeconds": 234000,
    "isOnline": false
  },
  "war": {
    "faction": "Wolves",
    "level": 12,
    "alignment": 88,
    "bossKills": 4
  },
  "hardline": {
    "reputation": 1820
  },
  "gunplay": {
    "shotsFired": 14200,
    "shotsHit": 3210,
    "headshots": 412,
    "accuracy": 22.6,
    "headshotPercentage": 12.8
  },
  "movement": {
    "distanceTravelled": 412000,
    "distanceOnFoot": 88000,
    "distanceInVehicle": 324000,
    "suicides": 1
  },
  "kills": {
    "zombies":  { "Infected_Citizen": 880, "Infected_Soldier": 312 },
    "players":  { "Player": 14 },
    "bosses":   { "Boss_Frostbite": 4 },
    "ai":       { "AI_Bandit": 88, "AI_NPC_Guard": 22 },
    "animals":  { "Wolf_Grey": 12, "Bear_Brown": 3 },
    "other":    {}
  },
  "deaths": {
    "zombies":  { "Infected_Citizen": 12 },
    "players":  { "Player": 8 },
    "bosses":   {},
    "ai":       { "AI_Bandit": 4 },
    "animals":  { "Wolf_Grey": 2 },
    "other":    { "Suicide": 1 }
  },
  "longestRanges": {
    "Player": 412,
    "Infected": 89,
    "Boss_Frostbite": 14
  }
}
```

---

## Naming Conventions

| Element | Convention | Beispiel |
|---|---|---|
| Klassen DayZ | `PF_PascalCase` | `PF_PlayerDetailPayload` |
| Klassen WP | `PF_Snake_Pascal` | `PF_Player_Details` |
| DB-Tabelle | `{$wpdb->prefix}pf_*` | `wp_pf_player_details` |
| REST namespace | `psyern/v1` | `/player/{uid}` |
| CSS-Klassen (BEM) | `.pf-modal__element--mod` | `.pf-modal__tab--active` |
| JS-Funktionen | `camelCase` | `openPlayerModal(uid)` |
| Options-Keys | `pf_player_details_*` | `pf_player_details_enabled` |

---

## Quality Gates

Vor Markierung als "complete" pro Phase:

**Phase 1:**
- [ ] DayZ-Compile sauber (`#ifdef Psyerns_Framework`)
- [ ] Upload-Payload enthält `playerDetails[]` für alle Spieler
- [ ] DB-Tabelle wird via `dbDelta` korrekt erstellt
- [ ] REST-Endpoint liefert 404 bei unbekannter UID
- [ ] Category-Map deckt mindestens Vanilla-Zombies + WAR-Bosse ab

**Phase 2:**
- [ ] Modal öffnet auf Klick, schließt mit X/ESC/Backdrop-Click
- [ ] Alle Tabs zeigen Daten korrekt
- [ ] Mobile (≤768px) funktioniert
- [ ] Theme-Wechsel ändert Modal-Farben automatisch
- [ ] WCAG: Focus-Trap, ARIA-Attribute

**Phase 3:**
- [ ] Admin-Toggles funktionieren (Modal an/aus)
- [ ] Shortcode-Override greift
- [ ] Keine PHP-Notices/Warnings bei `WP_DEBUG=true`
- [ ] Browser-Konsole frei von Fehlern

---

## Sicherheit (Pflicht)

- [ ] REST-Endpoint: `sanitize_text_field` auf UID, Regex-Whitelist `^[A-Za-z0-9]{1,32}$`
- [ ] DB-Zugriff nur via `$wpdb->prepare()`
- [ ] Upload-Endpoint: API-Key-Check (bestehende `Authorization: Bearer` Logik)
- [ ] Frontend: `wp.apiFetch` mit nonce ODER public endpoint mit Rate-Limit
- [ ] Output-Escape: `esc_html` für Namen, `esc_attr` für UID, `wp_kses_post` für Modal-HTML
- [ ] Direct-file-access: `if ( ! defined( 'ABSPATH' ) ) exit;` in jeder neuen PHP-Datei

---

## Execution Command

```
1. Launch Agents 1, 2, 3 parallel (Phase 1)
2. Wait, verify compile + DB + Category-Map
3. Launch Agent 4 (Modal-Template) → blockiert Agent 5
4. Launch Agent 5 (JS) — verbraucht REST + Modal
5. Run Agent 6 (Settings + Polish + Smoketest)
6. Manual test: Klick auf 3 verschiedene Spieler, alle Tabs, beide Modi (PvP/PvE)
```

---

## Out of Scope (für v1)

- Historische Verläufe (z.B. Kill-Charts über Zeit) — braucht separate Time-Series-Tabelle
- Inventar-Snapshot
- Death-Map (Heatmap) — eigenes Feature
- Player-Vergleich (Side-by-Side) — eigenes Feature
- Steam-Profile-Integration im Modal — bereits via Avatar-URL möglich, kein neuer Code nötig

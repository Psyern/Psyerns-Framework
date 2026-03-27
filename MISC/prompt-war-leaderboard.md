# Prompt: DME-WAR + Hardline Integration ins Leaderboard

## Kontext

Das Ninjin Leaderboard zeigt PvE/PvP Stats. Wir erweitern es um:
1. **DME-WAR Fraktionsdaten** (Fraktion, Punkte, Level, Fraktions-Gesamtpunkte)
2. **DME-WAR Boss-Kills** (Counter pro Spieler — muss erst im Mod implementiert werden)
3. **Expansion Hardline Reputation** (Reputationspunkte pro Spieler)
4. **WordPress API Übergabe** über Psyerns Framework
5. **CSS/HTML Design-Anpassung** aller 6 Themes

Alle Integrationen per `#ifdef` — optional, kein harter Dependency.

**Lies zuerst:** `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\coding-rules.md` (PFLICHT)

---

## Quelldaten-Analyse

### DME-WAR (C:\Users\Administrator\Desktop\DME-WAR\DME_War)

**Player-Daten:** `DMEW_PlayerState` in `scripts/4_World/DMEW_PlayerState.c`
```
Faction: string         → "EAST", "WEST", "NEUTRAL"
Alignment: int          → -10000 bis +10000 (negativ=WEST, positiv=EAST)
Level: int              → 0-6 (Neutral, Neuling, Rekrut, Gefreiter, Veteran, Offizier, General)
Rep: int                → Legacy-Feld, synced mit Alignment
```

**Zugriff:** `DMEW_Storage.GetCached(steamId)` → `DMEW_PlayerState`
**Dateipfad:** `$profile:DeadmansEcho/War/Players/{SteamID}.json`
**#ifdef Guard:** `DME_War` (CfgPatches Klasse)

**Globale Fraktionspunkte:** `DMEW_FactionPoints` in `scripts/4_World/DMEW_FactionPoints.c`
```
DMEW_FactionPointsData.EastPoints: int    → 0-250000
DMEW_FactionPointsData.WestPoints: int    → 0-250000
DMEW_FactionPointsData.WarPoints: int     → Differenz
```
**Zugriff:** `DMEW_FactionPoints.GetData()` → `DMEW_FactionPointsData`

**Boss-System:** `DMEBossSquad` in `scripts/4_World/DMEW_Boss/DMEBossSpawner.c`
- Bosse werden gespawnt und getrackt
- Kill-Detection in `DMEW_PlayerBase.c` EEKilled() (Zeile 399-473)
- Aktuell: Nur Webhook gesendet (`WarWebhookService.SendBossKill`), **kein Counter pro Spieler**
- **MUSS IM MOD ERGÄNZT WERDEN:** `BossKills: int` Feld in `DMEW_PlayerState`

### Expansion Hardline (C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\Hardline)

**Reputation:** `ExpansionHardlinePlayerData` in `Scripts/4_World/.../ExpansionHardlinePlayerData.c`
```
Reputation: int         → 0 bis MaxReputation
FactionReputation: map<int, int>  → Pro-Fraktion Reputation
```

**Zugriff auf PlayerBase:**
```c
int rep = player.Expansion_GetReputation();
int factionRep = player.Expansion_GetFactionReputation(factionID); // #ifdef EXPANSIONMODAI
```

**Zugriff auf Modul:**
```c
ExpansionHardlineModule mod = ExpansionHardlineModule.GetModuleInstance();
ExpansionHardlinePlayerData data = mod.GetPlayerData(player);
```

**#ifdef Guard:** `EXPANSIONMODHARDLINE`
**Storage:** Binary `.bin` Dateien (nicht JSON)

---

## Agent-Orchestrierung (5 Agents, 3 Phasen)

### Phase 1 — Parallel (Mod-Erweiterungen)

#### Agent 1: DME-WAR Boss-Kill Counter
**Typ:** Schreibend
**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\DME-WAR\DME_War`
**Aufgabe:** Boss-Kill-Counter pro Spieler implementieren

**Zu modifizieren:**
- `scripts/4_World/DMEW_PlayerState.c` — Neues Feld `BossKills` (int, default 0) hinzufügen
- `scripts/4_World/DMEW_PlayerBase.c` — In der EEKilled() Boss-Detection (Zeile 399-473): Wenn Boss getötet, `DMEW_PlayerState.BossKills++` und speichern
- `scripts/4_World/DMEW_Storage.c` — Sicherstellen dass BossKills geladen/gespeichert wird (sollte automatisch per JsonFileLoader gehen)

**Regeln:** Enforce Script Pitfalls beachten. Minimale Änderung. Keine mehrzeiligen Concats.

#### Agent 2: Ninjin Leaderboard Web-Export erweitern
**Typ:** Schreibend
**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Ninjin_Leaderboard\Ninjins_LeaderBoard`
**Aufgabe:** Den `LeaderboardWebExport.json` um DME-WAR und Hardline Felder erweitern

**Zu modifizieren:**
- `scripts/3_Game/General Configs/Data/TrackingModLeaderboardData.c` — Neue Felder in `TrackingModWebLeaderboardPlayerData`:
  ```
  string warFaction;        // "EAST", "WEST", "NEUTRAL"
  int warAlignment;         // -10000 bis +10000
  int warLevel;             // 0-6
  int warBossKills;         // Boss-Kill Counter
  int hardlineReputation;   // Expansion Reputation
  ```
- `scripts/3_Game/General Configs/Data/Ninjins_Tracking_Mod_Data.c` — In `CreateWebLeaderboardPlayerData()`: DME-WAR und Hardline Daten auslesen und in Export-Objekt setzen. Alles in `#ifdef DME_War` und `#ifdef EXPANSIONMODHARDLINE` Guards.
- `scripts/3_Game/General Configs/Data/TrackingModLeaderboardData.c` — Auch `TrackingModWebLeaderboardExport` um globale Fraktionspunkte erweitern:
  ```
  int globalEastPoints;
  int globalWestPoints;
  ```

**DME-WAR Datenzugriff (in #ifdef DME_War):**
```c
DMEW_PlayerState warState = DMEW_Storage.GetCached(plainID);
if (warState)
{
    exportPlayer.warFaction = warState.Faction;
    exportPlayer.warAlignment = warState.Alignment;
    exportPlayer.warLevel = warState.Level;
    exportPlayer.warBossKills = warState.BossKills;
}
```

**Hardline Datenzugriff (in #ifdef EXPANSIONMODHARDLINE):**
- Problem: `Expansion_GetReputation()` braucht ein `PlayerBase`-Objekt, nicht nur eine SteamID
- Lösung: In `CreateWebLeaderboardPlayerData()` prüfen ob der Spieler online ist. Wenn ja: `player.Expansion_GetReputation()`. Wenn offline: Hardline .bin Datei direkt laden via `ExpansionHardlineModule`
- Alternative: Die Reputation beim Disconnect im PlayerDeathData speichern (einfacher)

**Empfohlene Lösung für Hardline:**
Neues Feld `HardlineReputation` in `PlayerDeathData` (Ninjin's per-player JSON). Beim Player-Disconnect oder Kill: `playerData.HardlineReputation = player.Expansion_GetReputation()`. So ist der Wert immer in der JSON verfügbar, auch wenn der Spieler offline ist.

**Regeln:** Enforce Script Pitfalls beachten. `#ifdef` ist case-sensitive: `DME_War` und `EXPANSIONMODHARDLINE`.

### Phase 2 — Sequentiell (nach Phase 1)

#### Agent 3: WordPress Plugin API + DB erweitern
**Typ:** Schreibend
**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework`
**Aufgabe:** Neue Felder in DB-Schema und API-Responses

**Zu modifizieren:**
- `includes/class-pf-database.php` — Neue Spalten in `{prefix}pf_leaderboard`:
  ```sql
  war_faction VARCHAR(10) DEFAULT '',
  war_alignment INT DEFAULT 0,
  war_level INT DEFAULT 0,
  war_boss_kills INT DEFAULT 0,
  hardline_reputation INT DEFAULT 0
  ```
  DB-Version hochzählen damit `dbDelta()` migriert.

- `includes/class-pf-leaderboard.php` — In `upsert_players()`:
  ```php
  'war_faction'          => sanitize_text_field( $p['warFaction'] ?? '' ),
  'war_alignment'        => intval( $p['warAlignment'] ?? 0 ),
  'war_level'            => intval( $p['warLevel'] ?? 0 ),
  'war_boss_kills'       => intval( $p['warBossKills'] ?? 0 ),
  'hardline_reputation'  => intval( $p['hardlineReputation'] ?? 0 ),
  ```
  In `format_player_row()` die neuen Felder in die Response aufnehmen.
  In `handle_upload()` die globalen Fraktionspunkte als Transient speichern.

- Neuer öffentlicher Endpoint: `GET /public/factions` → Gibt globale Fraktionspunkte zurück

#### Agent 4: CSS/HTML Design Themes erweitern
**Typ:** Schreibend
**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public`
**Aufgabe:** Alle 6 Themes + Base-Template um Fraktionsdaten erweitern

**Neue UI-Elemente:**
1. **Fraktions-Badge** neben dem Spielernamen — farbig (Rot=EAST, Blau=WEST, Grau=NEUTRAL)
2. **Level-Badge** — Rangsymbol oder Zahl (0-6)
3. **Boss-Kills Spalte** — Totenkopf-Icon + Zahl
4. **Hardline Reputation** — Stern-Icon + Zahl
5. **Fraktions-Bar** im Header — Visueller Balken EAST vs WEST (globale Punkte)

**Zu modifizieren:**
- `templates/leaderboard-base.php` — Neue Spalten in Tabelle: Faction, Level, Boss Kills, Reputation
- `css/psyern-leaderboard.css` — Neue Klassen: `.psyern-lb__faction`, `.psyern-lb__level`, `.psyern-lb__boss-kills`, `.psyern-lb__reputation`, `.psyern-lb__faction-bar`
- `css/psyern-theme-military.css` — Military-spezifische Farben für Fraktionen
- `css/psyern-theme-neon.css` — Neon-Glow für Fraktionsbadges
- `css/psyern-theme-ash.css` — Post-Apokalyptisch
- `css/psyern-theme-echo.css` — Echo-Theme
- `css/psyern-theme-ops.css` — Ops-Theme
- `css/psyern-theme-outbreak.css` — Outbreak-Theme
- `js/psyern-leaderboard.js` — Neue Spalten rendern

**Fraktions-Farben (konsistent über alle Themes):**
- EAST: `#e74c3c` (Rot)
- WEST: `#3498db` (Blau)
- NEUTRAL: `#95a5a6` (Grau)

**Fraktions-Bar Design:**
```
[█████████████████░░░░░░░░] EAST: 156,000 | WEST: 94,000
```
Proportional gefüllt, EAST rot links, WEST blau rechts.

#### Agent 5: PF-Templates.js + Standalone erweitern
**Typ:** Schreibend
**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\assets`
**Aufgabe:** Das alte PF Shortcode-System (pf-templates.js) um die neuen Felder erweitern

**Zu modifizieren:**
- `js/pf-templates.js` — `renderLeaderboardTable()`: Neue Spalten Faction, Level, Boss Kills, Reputation
- `js/pf-templates.js` — `renderTop3Cards()`: Fraktions-Badge + Reputation anzeigen
- `js/pf-templates.js` — `renderPlayerCard()`: Alle neuen Felder im Stats-Grid
- `css/pf-common.css` — Neue Klassen für Fraktions-Badges
- `css/pf-dark.css` — Dark-Theme Farben für Fraktionen
- `css/pf-light.css` — Light-Theme Farben

---

## Ausführungsreihenfolge

```
Phase 1 — Parallel (DayZ Mod Code):
┌──────────────────────┬─────────────────────────────────┐
│      Agent 1         │         Agent 2                 │
│  DME-WAR Boss Counter│  Ninjin Export erweitern        │
│  (DME_War Mod)       │  (Ninjin_Leaderboard Mod)      │
└──────────┬───────────┴────────────────┬────────────────┘
           │                            │
           ▼                            ▼
Phase 2 — Sequentiell (WordPress + Frontend):
┌──────────────────────┬──────────────────┬──────────────┐
│      Agent 3         │    Agent 4       │   Agent 5    │
│  WP Plugin DB+API    │  CSS/HTML Themes │  PF-Templates│
│  (nach Agent 1+2)    │  (parallel zu 3) │  (parallel)  │
└──────────────────────┴──────────────────┴──────────────┘
```

## Hinweise für den Orchestrator

1. **Phase 1:** Agent 1 + 2 parallel starten
2. **Phase 2:** Agent 3, 4, 5 parallel starten (nachdem 1+2 fertig)
3. **Jeder Agent liest `coding-rules.md`** — Enforce Script Pitfalls
4. **#ifdef Guards:** `DME_War` (nicht `DME_WAR`), `EXPANSIONMODHARDLINE` (nicht `ExpansionModHardline`)
5. **Boss-Kill Counter:** Agent 2 hängt von Agent 1 ab (BossKills Feld muss existieren). Falls Agent 1 noch nicht fertig: Agent 2 kann das Feld trotzdem referenzieren, es wird nur 0 sein bis Agent 1 deployed ist.
6. **Hardline Offline-Daten:** Die empfohlene Lösung (Reputation bei Disconnect in PlayerDeathData speichern) ist die einfachste. Alternativ: Hardline .bin Dateien direkt lesen — aber das ist komplex wegen Binary-Format.
7. **DB Migration:** Agent 3 muss DB-Version in `class-pf-database.php` hochzählen und `dbDelta()` aufrufen damit neue Spalten erstellt werden.
8. **Nach Abschluss:** Error-Scan über alle geänderten Dateien. Dann Testserver deployen.

## Neue JSON-Felder in LeaderboardWebExport.json

```json
{
    "generatedAt": "...",
    "playerOnlineCounter": 12,
    "totalPlayers": 150,
    "globalEastPoints": 156000,
    "globalWestPoints": 94000,
    "topPVEPlayers": [
        {
            "playerID": "76561198...",
            "playerName": "Psyern",
            "pvePoints": 8740,
            "pvpPoints": 4250,
            "warFaction": "EAST",
            "warAlignment": 6500,
            "warLevel": 4,
            "warBossKills": 7,
            "hardlineReputation": 3200,
            "categoryKills": {...},
            "categoryDeaths": {...},
            "categoryLongestRanges": {...}
        }
    ]
}
```

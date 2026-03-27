# Multi-Agent Orchestration Prompt
## Feature: Automatische Leaderboard-Synchronisation in Psyerns Framework

---

## Ziel

Das Psyerns Framework soll Leaderboard-Daten **vollautomatisch und nativ** aus dem Ninjins Tracking Mod lesen und an WordPress (oder ein per API-Key verbundenes Standalone-PHP-System) übertragen — ohne PowerShell, ohne externe Tools, komplett in Enforce Script (DayZ Server Mod).

---

## Systemarchitektur-Überblick (für alle Agents)

```
DayZ Server (Enforce Script)
│
├── Psyerns_Framework/
│   ├── scripts/3_Game/Psyerns_Framework/
│   │   ├── Web/
│   │   │   ├── Config/
│   │   │   │   ├── PF_WebConfig.c          ← Singleton Config, JsonFileLoader, Auto-Upgrade
│   │   │   │   └── PF_WebEndpoint.c        ← Endpoint-Datenklasse (Name/BaseUrl/ApiKey/Enabled)
│   │   │   ├── Payload/
│   │   │   │   ├── PF_JsonPayload.c        ← Basisklasse mit Serialize()
│   │   │   │   └── PF_WordPressPayload.c   ← Payload-Klasse mit PF_WP_PlayerData[], Serialize()
│   │   │   └── WebApi/
│   │   │       ├── PF_WebApiBase.c         ← RestApi/RestContext Wrapper, Post()/Get()
│   │   │       └── PF_WordPressApi.c       ← UploadLeaderboard(payload), Ping()
│   │   ├── REST/
│   │   │   ├── Base/PF_RestBase.c          ← Extends PF_WebApiBase, BuildEndpoint(?api_key=)
│   │   │   ├── Config/PF_RestConfig.c      ← Proxy auf PF_WebConfig, IsXxxEnabled()
│   │   │   └── ServerStatus/
│   │   │       └── PF_ServerStatus.c       ← VORLAGE: OnUpdate(timeslice), PushStatus(), Intervall-Timer
│   │   └── Utils/
│   │       ├── PF_JsonBuilder.c            ← Add/AddInt/AddFloat/AddBool/AddRaw/Build()
│   │       └── PF_HttpArguments.c          ← ToQuery("/endpoint") → "/endpoint?key=val"
│   │
│   └── scripts/5_Mission/Psyerns_Framework/
│       ├── PF_MissionInit.c                ← modded MissionServer: OnInit, OnUpdate, OnMissionFinish
│       └── PF_RestInit.c                   ← modded MissionServer: initialisiert alle REST-Module
│
├── Ninjins_Tracking_Mod (externer Mod auf gleichem Server)
│   └── Data/Players/<SteamID64>.json       ← eine JSON-Datei pro Spieler
│
└── WordPress / Standalone PHP
    └── POST /wp-json/psyern/v1/upload      ← erwartet topPVEPlayers[], topPVPPlayers[], Meta
        POST /api/receive.php?endpoint=upload ← Standalone-Alternative
```

---

## Ninjins Tracking Mod — JSON-Feldstruktur

Jede `<SteamID64>.json` enthält exakt diese Felder:

```json
{
  "PlayerID": "76561197968899727",
  "PlayerName": "Rad[Ekwah]",
  "PvEPoints": 17370,
  "PvPPoints": 0,
  "playerIsOnline": 0,
  "WarFaction": "NEUTRAL",
  "WarAlignment": 0,
  "WarLevel": 0,
  "HardlineReputation": 1100,
  "LastLoginDate": "2026-03-25 14:20:00",
  "CategoryKills": {
    "PlayersBased": 56,
    "ZombiesBased": 876,
    "AIBased": 78,
    "AnimalsBased": 134
  },
  "CategoryDeaths": {
    "SelfInflicted": 1,
    "PlayersBased": 45,
    "ZombiesBased": 30,
    "AnimalsBased": 3
  },
  "CategoryLongestRanges": {
    "PlayersBased": 345,
    "ZombiesBased": 38,
    "AIBased": 234,
    "AnimalsBased": 10
  },
  "WarBossKills": 0,
  "survivorType": "...",
  "AvailableMilestones": [],
  "ClaimedMilestones": [],
  "LastDeathPosition": {},
  "LastKillerName": "",
  "LastKillerType": ""
}
```

**Pfad auf dem Server:**
```
$profile:Ninjins_Tracking_Mod/Data/Players/
```
(`$profile:` = DayZ Server Profiles-Verzeichnis, z.B. `./profiles/`)

---

## WordPress Upload-Endpoint — Erwartetes JSON-Format

`POST /wp-json/psyern/v1/upload?api_key=<KEY>`

```json
{
  "apiKey": "pf-xxx",
  "generatedAt": "2026-03-27T12:00:00Z",
  "playerOnlineCounter": 3,
  "totalPlayers": 58,
  "globalEastPoints": 0,
  "globalWestPoints": 0,
  "topPVEPlayers": [
    {
      "playerID": "76561198043039918",
      "playerName": "Psyern",
      "pvePoints": 69030,
      "pvpPoints": 0,
      "kills": 4578,
      "deaths": 45,
      "aiKills": 312,
      "longestShot": 823.0,
      "playtime": 0,
      "isOnline": 0,
      "lastLoginDate": "2026-03-27 10:00:00",
      "warFaction": "NEUTRAL",
      "warAlignment": 0,
      "warLevel": 0,
      "warBossKills": 0,
      "hardlineReputation": 2500,
      "categoryKills": {"PlayersBased":245,"ZombiesBased":3456,"AIBased":312,"AnimalsBased":567},
      "categoryDeaths": {"SelfInflicted":2,"PlayersBased":5,"ZombiesBased":30,"AnimalsBased":8},
      "categoryLongestRanges": {"PlayersBased":823,"ZombiesBased":45,"AIBased":234,"AnimalsBased":12}
    }
  ],
  "topPVPPlayers": [ ... ]
}
```

**PHP-Seite erwartet (class-pf-leaderboard.php → upsert_players):**
- `$p['playerID'] ?? $p['odolozId']` für Steam-ID
- `$p['pvePoints']` für PvE-Punkte
- `$p['pvpPoints']` für PvP-Punkte
- `$p['categoryKills']`, `$p['categoryDeaths']`, `$p['categoryLongestRanges']` als Objekt/Array
- `$p['isOnline']`, `$p['lastLoginDate']`, `$p['warFaction']`, `$p['warAlignment']`, `$p['warLevel']`, `$p['warBossKills']`, `$p['hardlineReputation']`

---

## Zu implementierende Dateien (alle Enforce Script `.c`)

### Agent 1 — Payload-Erweiterung

**Datei:** `scripts/3_Game/Psyerns_Framework/Web/Payload/PF_WordPressPayload.c`

**Aufgabe:** `PF_WP_PlayerData` um alle Ninjins-Felder erweitern und `Serialize()` entsprechend anpassen.

**Aktuelle Felder (unvollständig):**
```c
class PF_WP_PlayerData {
    string playerName;
    string odolozId;   // ← umbenennen zu playerID, odolozId als Alias behalten
    int kills;
    int deaths;
    int aiKills;
    float longestShot;
    float playtime;
}
```

**Neue Felder hinzufügen:**
```c
string playerID;        // Steam64 ID
string playerName;
int pvePoints;
int pvpPoints;
int kills;
int deaths;
int aiKills;
float longestShot;
float playtime;
int isOnline;
string lastLoginDate;
string warFaction;
int warAlignment;
int warLevel;
int warBossKills;
int hardlineReputation;
// CategoryKills/Deaths/Ranges als serialisierte JSON-Strings:
string categoryKillsJson;    // wird als AddRaw serialisiert
string categoryDeathsJson;
string categoryLongestRangesJson;
```

**Serialize-Methode `SerializePlayerArray()` muss ausgeben:**
```json
{
  "playerID":"...", "playerName":"...",
  "pvePoints":0, "pvpPoints":0,
  "kills":0, "deaths":0, "aiKills":0,
  "longestShot":0.0, "playtime":0.0,
  "isOnline":0, "lastLoginDate":"...",
  "warFaction":"NEUTRAL", "warAlignment":0,
  "warLevel":0, "warBossKills":0,
  "hardlineReputation":0,
  "categoryKills":{...},
  "categoryDeaths":{...},
  "categoryLongestRanges":{...}
}
```

**`PF_WordPressPayload` um neue Meta-Felder erweitern:**
```c
int globalEastPoints;
int globalWestPoints;
// Serialize() muss diese ebenfalls serialisieren
```

---

### Agent 2 — Leaderboard-Reader (neues Modul)

**Neue Datei:** `scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardReader.c`

**Aufgabe:** Liest alle `<SteamID64>.json` Dateien aus dem Ninjins Tracking Mod Spielerordner via `FileExist()`, `OpenFile()`, `FGetContent()`, `JsonSerializer` und baut daraus `PF_WP_PlayerData`-Objekte.

**Enforce Script File-API:**
```c
// Verzeichnis-Iteration in Enforce Script:
string fileName;
FindFileHandle handle = FindFile("$profile:Ninjins_Tracking_Mod/Data/Players/*.json", fileName, FileAttr.FILE);
if (handle != INVALID_HANDLE) {
    do {
        string fullPath = "$profile:Ninjins_Tracking_Mod/Data/Players/" + fileName;
        // Datei einlesen mit FGetContent / JsonSerializer
    } while (FindNextFile(handle, fileName));
    CloseFindFile(handle);
}
```

**Daten-Klasse zum Deserialisieren:**
```c
class PF_NinjinPlayerData {
    string PlayerID;
    string PlayerName;
    int PvEPoints;
    int PvPPoints;
    int playerIsOnline;
    string WarFaction;
    int WarAlignment;
    int WarLevel;
    int HardlineReputation;
    string LastLoginDate;
    int WarBossKills;
    ref PF_NinjinCategoryKills CategoryKills;
    ref PF_NinjinCategoryDeaths CategoryDeaths;
    ref PF_NinjinCategoryRanges CategoryLongestRanges;
}

class PF_NinjinCategoryKills {
    int PlayersBased;
    int ZombiesBased;
    int AIBased;
    int AnimalsBased;
}

class PF_NinjinCategoryDeaths {
    int SelfInflicted;
    int PlayersBased;
    int ZombiesBased;
    int AnimalsBased;
}

class PF_NinjinCategoryRanges {
    int PlayersBased;
    int ZombiesBased;
    int AIBased;
    int AnimalsBased;
}
```

**Konvertierungslogik (Ninjin → PF_WP_PlayerData):**
- `totalKills` = Summe aller `CategoryKills.*`
- `totalDeaths` = Summe aller `CategoryDeaths.*`
- `aiKills` = `CategoryKills.AIBased`
- `longestShot` = Maximum aus allen `CategoryLongestRanges.*`
- `warBossKills` = `WarBossKills` (kann 0 oder Array sein → robust abfangen)
- Steam-ID aus Dateiname (`fileName` ohne `.json`)

**Öffentliche Methode:**
```c
class PF_LeaderboardReader {
    // Liest alle Spieler und gibt sie als Array zurück
    static void ReadAllPlayers(out array<ref PF_WP_PlayerData> outPlayers, out int outOnlineCount);
    
    // Hilfsmethode: Ninjin-Daten → PF_WP_PlayerData konvertieren
    protected static PF_WP_PlayerData ConvertPlayer(string steamId, PF_NinjinPlayerData raw);
    
    // Kategorie-JSON serialisieren (PF_NinjinCategoryKills → JSON-String)
    protected static string SerializeCategoryKills(PF_NinjinCategoryKills cat);
    protected static string SerializeCategoryDeaths(PF_NinjinCategoryDeaths cat);
    protected static string SerializeCategoryRanges(PF_NinjinCategoryRanges cat);
}
```

---

### Agent 3 — Leaderboard-Export-Manager (neues Modul)

**Neue Datei:** `scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c`

**Aufgabe:** Intervall-basiertes Modul (analog zu `PF_ServerStatus`). Ruft `PF_LeaderboardReader.ReadAllPlayers()` auf, sortiert nach PvE/PvP-Punkten, baut `PF_WordPressPayload` und sendet via `PF_WordPressApi.UploadLeaderboard()`.

**Vorlage: `PF_ServerStatus`** — exakt gleiche Struktur:
```c
ref PF_LeaderboardExport g_PF_LeaderboardExport;

class PF_LeaderboardExport : PF_RestBase {
    protected float m_Timer;
    protected float m_Interval;
    
    void PF_LeaderboardExport(string baseUrl, string apiKey, int intervalSeconds);
    void OnUpdate(float timeslice);
    void PushLeaderboard();
    protected string GetTimestamp();
}
```

**`PushLeaderboard()`-Logik:**
1. `PF_LeaderboardReader.ReadAllPlayers(players, onlineCount)` aufrufen
2. Spieler nach `pvePoints` absteigend sortieren → `topPVEPlayers`
3. Spieler nach `pvpPoints` absteigend sortieren → `topPVPPlayers`
4. Faction-Punkte summieren: `globalEastPoints` (warFaction == "EAST"), `globalWestPoints` (warFaction == "WEST")
5. `PF_WordPressPayload` befüllen
6. `PF_WordPressApi.UploadLeaderboard(payload)` aufrufen
7. Logging: Anzahl Spieler, Online-Count, Top-PvE-Spieler

**Maximale Spieler pro Liste:** `100` (Konstante `MAX_LEADERBOARD_PLAYERS`)

**Sortierung in Enforce Script** (kein LINQ — manuelles BubbleSort oder SelectionSort über das Array):
```c
// Einfaches Sort-Pattern für Enforce Script:
for (int i = 0; i < players.Count() - 1; i++) {
    for (int j = 0; j < players.Count() - i - 1; j++) {
        if (players[j].pvePoints < players[j+1].pvePoints) {
            PF_WP_PlayerData tmp = players[j];
            players[j] = players[j+1];
            players[j+1] = tmp;
        }
    }
}
```

---

### Agent 4 — Config-Erweiterung

**Datei:** `scripts/3_Game/Psyerns_Framework/Web/Config/PF_WebConfig.c`

**Aufgabe:** Neue Config-Felder für LeaderboardExport hinzufügen.

**Neue Felder in `PF_WebConfig`:**
```c
bool EnableLeaderboardExport;
int LeaderboardExportIntervalSeconds;
string NinjinPlayersPath;  // Default: "$profile:Ninjins_Tracking_Mod/Data/Players"
int LeaderboardMaxPlayers; // Default: 100
```

**`CURRENT_VERSION`** von `2` auf `3` erhöhen.

**`CreateDefaults()` ergänzen:**
```c
EnableLeaderboardExport = false;
LeaderboardExportIntervalSeconds = 600;  // 10 Minuten
NinjinPlayersPath = "$profile:Ninjins_Tracking_Mod/Data/Players";
LeaderboardMaxPlayers = 100;
```

**`PF_RestConfig.c` ergänzen:**
```c
bool IsLeaderboardExportEnabled() { return m_Config.EnableLeaderboardExport; }
int GetLeaderboardExportInterval() { return m_Config.LeaderboardExportIntervalSeconds; }
string GetNinjinPlayersPath() { return m_Config.NinjinPlayersPath; }
int GetLeaderboardMaxPlayers() { return m_Config.LeaderboardMaxPlayers; }
```

---

### Agent 5 — Integration in PF_RestInit

**Datei:** `scripts/5_Mission/Psyerns_Framework/PF_RestInit.c`

**Aufgabe:** `PF_LeaderboardExport` initialisieren und in `OnUpdate()` einbinden.

**In `OnInit()` hinzufügen:**
```c
if (restCfg.IsLeaderboardExportEnabled())
{
    g_PF_LeaderboardExport = new PF_LeaderboardExport(
        baseUrl,
        apiKey,
        restCfg.GetLeaderboardExportInterval()
    );
    enabledCount++;
    PF_Logger.Log("LeaderboardExport initialized (interval: " + restCfg.GetLeaderboardExportInterval().ToString() + "s)");
}
```

**In `OnUpdate()` hinzufügen:**
```c
if (g_PF_LeaderboardExport)
    g_PF_LeaderboardExport.OnUpdate(timeslice);
```

**In `OnMissionFinish()` hinzufügen:**
```c
g_PF_LeaderboardExport = null;
```

**Globale Variable am Dateianfang:**
```c
ref PF_LeaderboardExport g_PF_LeaderboardExport;
```

---

### Agent 6 — scripts/config.cpp aktualisieren

**Datei:** `scripts/config.cpp`

**Aufgabe:** Die neuen Dateien in die Enforce Script Compile-Order eintragen.

**Hinzufügen (nach den bestehenden Einträgen, in korrekter Reihenfolge):**
```cpp
// In class CfgMods / der scripts.c oder direkt in config.cpp:
// Neue Dateien müssen vor PF_RestInit.c geladen werden:
"scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardReader.c",
"scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c",
```

**Wichtig:** `PF_NinjinPlayerData` und Unterklassen müssen **vor** `PF_LeaderboardReader` definiert sein — entweder in der gleichen Datei oder in einer separaten `PF_NinjinData.c` die zuerst geladen wird.

---

## Abhängigkeitsgraph (für Agents)

```
PF_WebConfig (Agent 4)
    └── PF_RestConfig (Agent 4)
            └── PF_LeaderboardExport (Agent 3)
                    ├── PF_LeaderboardReader (Agent 2)
                    │       └── PF_NinjinPlayerData (in Agent 2 Datei)
                    ├── PF_WordPressPayload (Agent 1)
                    └── PF_WordPressApi (existiert bereits)

PF_RestInit (Agent 5)
    └── PF_LeaderboardExport (Agent 3)
```

---

## Enforce Script Constraints (kritisch für alle Agents)

1. **Kein `foreach`-Iterator für Dictionaries** — nur `for`-Schleifen mit Index
2. **Kein `string.Split()`** — Dateiname-Parsing über `StringStart()`, `Substring()`, `IndexOf()`
3. **`JsonFileLoader<T>.JsonLoadFile(path, instance)`** für Deserialisierung (nicht `ConvertFrom-Json`)
4. **`FindFile()` / `FindNextFile()` / `CloseFindFile()`** für Verzeichnis-Iteration
5. **`FGetContent(FileHandle, string)`** für Datei-Inhalt lesen, oder `JsonFileLoader` direkt
6. **Keine Custom HTTP-Header** — API-Key nur als Query-Parameter (`?api_key=...`)
7. **`ref` vor Arrays und Objekte in Klassenfeldern** — `ref array<ref T>`
8. **Sort**: Kein LINQ, kein `.Sort()` mit Lambda — manueller BubbleSort
9. **`string` ist Value-Type** — keine Null-Checks nötig, aber `""` prüfen
10. **`Print()` vs `PF_Logger.Log()`** — immer `PF_Logger` verwenden
11. **`GetTimestamp()`** — `GetYearMonthDay()` + `GetHourMinuteSecond()` + `ToStringLen(N)` für ISO-Format
12. **`Math.RandomInt(min, max)`** — max ist exklusiv

---

## Neue Config-Felder im `PsyernsFrameworkConfig.json` (Endresultat)

```json
{
    "ConfigVersion": 3,
    "EnableLeaderboardExport": false,
    "LeaderboardExportIntervalSeconds": 600,
    "NinjinPlayersPath": "$profile:Ninjins_Tracking_Mod/Data/Players",
    "LeaderboardMaxPlayers": 100
}
```

---

## README.md aktualisieren

**Datei:** `Psyerns_Framework/README.md`

**In der Features-Tabelle (REST Modules) hinzufügen:**
```
- Leaderboard auto-sync (Ninjins Tracking Mod)
```

**Neue Config-Felder in der REST Feature Toggles-Tabelle:**

| Field | Default | Description |
|---|---|---|
| `EnableLeaderboardExport` | `false` | Automatische Leaderboard-Übertragung an WordPress/Standalone |
| `LeaderboardExportIntervalSeconds` | `600` | Sync-Interval in Sekunden (Standard: 10 Minuten) |
| `NinjinPlayersPath` | `$profile:Ninjins_Tracking_Mod/Data/Players` | Pfad zum Ninjins Tracking Mod Spielerordner |
| `LeaderboardMaxPlayers` | `100` | Maximale Spieler pro PvE/PvP-Liste |

---

## Implementierungsreihenfolge für Agents

```
1. Agent 4 → PF_WebConfig + PF_RestConfig (Config-Basis)
2. Agent 1 → PF_WordPressPayload (erweiterte Payload-Klasse)
3. Agent 2 → PF_LeaderboardReader + PF_NinjinPlayerData (Daten einlesen)
4. Agent 3 → PF_LeaderboardExport (Orchestrierung + Übertragung)
5. Agent 5 → PF_RestInit (Integration ins Framework)
6. Agent 6 → scripts/config.cpp (Compile-Order)
7. README.md aktualisieren
```

---

## Qualitätskriterien

- [ ] Kein einziger PowerShell-Call, kein externes Script
- [ ] Interval-basiert wie `PF_ServerStatus` (OnUpdate-Muster)
- [ ] Config-Version auto-upgrade von 2 auf 3 (bestehende Configs werden nicht gelöscht)
- [ ] Fehlerresistenz: fehlende/korrupte JSON-Dateien werden gelogged und übersprungen
- [ ] Logging auf jedem Schritt via `PF_Logger.Log()` und `PF_Logger.Debug()`
- [ ] Bestehende Felder in `PF_WP_PlayerData` bleiben rückwärtskompatibel (`odolozId` als Alias)
- [ ] `PF_LeaderboardExport` ist `null` wenn `EnableLeaderboardExport = false` (kein Overhead)
- [ ] Kein direktes `Print()` — nur `PF_Logger`

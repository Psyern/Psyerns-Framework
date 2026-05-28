# Prompt: Ninjin Leaderboard Web Export via Psyerns Framework

## Aufgabe

Implementiere ein modulares Web-Export-System für das Ninjin_Leaderboard DayZ-Mod. Das System soll:

1. **DayZ-seitig** (EnforceScript): Über `#ifdef PSYERNS_FRAMEWORK` die bereits generierte `LeaderboardWebExport.json` in festem Intervall per HTTP POST an einen konfigurierbaren Endpoint senden
2. **Server-seitig** (PHP): Eine Referenz-Implementierung bereitstellen, die den POST empfängt und die Daten per GET ausliefert
3. **Frontend** (HTML): Eine standalone, iframe-fähige Leaderboard-Seite mit Tabs (PvP/PvE), Top 20 Spielern, erweiterter Spaltenansicht, Clean/Modern Theme und Auto-Refresh

Das System muss **modular** sein — jeder Serverhoster mit beliebigem Setup (Apache, Nginx, eigener Server, WordPress etc.) muss es nutzen können. Die PHP-Dateien sind eine Referenz-Implementierung.

---

## Projektstruktur

### Arbeitsverzeichnis
- **Ninjin_Leaderboard:** `C:\Users\Administrator\Desktop\Ninjin_Leaderboard\Ninjins_LeaderBoard\`
- **Psyerns Framework (Referenz, nicht editieren):** `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\`

### Bestehende Pfad-Konstanten (nicht ändern)
```c
const string TRACKING_MOD_ROOT_FOLDER = "$profile:Ninjins_Tracking_Mod\\";
const string TRACKING_MOD_DATA_DIR = TRACKING_MOD_ROOT_FOLDER + "Data\\";
const string TRACKING_MOD_WEB_EXPORT_FILE = TRACKING_MOD_DATA_DIR + "LeaderboardWebExport.json";
```

---

## Teil 1: DayZ EnforceScript — Web Export Helper

### Neue Datei erstellen
**Pfad:** `Ninjins_LeaderBoard/scripts/3_Game/General Configs/Utils/TrackingModWebExportHelper.c`

### Anforderungen

Die gesamte Datei muss in `#ifdef PSYERNS_FRAMEWORK` / `#endif` stehen, damit das Mod ohne Psyerns Framework kompiliert.

**Klasse `TrackingModWebExportHelper`:**
- Statische Klasse (nur static methods)
- Nutzt `PF_WebApiBase` als Basis für den HTTP-POST (nicht PF_WebClient direkt)
- Liest den "Leaderboard"-Endpoint aus `PF_WebConfig.GetInstance().GetEndpoint("Leaderboard")`
- Liest die bereits existierende `LeaderboardWebExport.json` von Disk (Pfad: `TRACKING_MOD_WEB_EXPORT_FILE`)
- Konvertiert den Dateiinhalt zu einem String und sendet ihn per POST an den konfigurierten Endpoint
- API-Key wird als Query-Parameter oder Header mitgesendet (konsistent mit `PF_WordPressApi` Pattern)

**API für den POST:**
- Endpoint-URL kommt aus Config: `BaseUrl` + `/receive`
- API-Key kommt aus Config: `ApiKey`
- Content-Type: `application/json`
- Body: Der rohe Inhalt der `LeaderboardWebExport.json`

**Referenz-Klasse `PF_WordPressApi` Pattern:**
```c
// So sieht die bestehende WordPress API aus — folge diesem Pattern:
class PF_WordPressApi : PF_WebApiBase
{
    protected string m_ApiKey;

    void PF_WordPressApi(string baseUrl, string apiKey)
    {
        m_BaseUrl = baseUrl;
        m_ApiKey = apiKey;
        m_RestContext = m_Rest.GetRestContext(m_BaseUrl);
        m_RestContext.SetHeader("application/json");
    }

    void UploadLeaderboard(PF_WordPressPayload payload)
    {
        payload.apiKey = m_ApiKey;
        string data = payload.Serialize();
        PF_HttpArguments args = new PF_HttpArguments();
        args.Add("api_key", m_ApiKey);
        string endpoint = args.ToQuery("/upload");
        Post(endpoint, data);
    }
}
```

**Erstelle analog eine `PF_LeaderboardApi` Klasse** (auch in `#ifdef PSYERNS_FRAMEWORK`):
- Erweitert `PF_WebApiBase`
- Methode `SendLeaderboardData(string jsonData)` — sendet den rohen JSON-String per POST
- Nutzt `PF_HttpArguments` für den API-Key als Query-Parameter
- Endpoint: `/receive`

### Timer-basierter Export

**Modifikation:** `Ninjins_LeaderBoard/scripts/5_Mission/MissionServer.c`

In der bestehenden `modded class MissionServer`:

- In `OnInit()`: Innerhalb `#ifdef PSYERNS_FRAMEWORK` Block — initialisiere den WebExportHelper (prüfe ob Endpoint "Leaderboard" konfiguriert und enabled ist)
- In `OnUpdate()` (existiert dort NICHT, muss als neues override hinzugefügt werden): `#ifdef PSYERNS_FRAMEWORK` Block — Timer-Logik:
  - Konfigurierbare Interval-Dauer (default 5 Minuten = 300 Sekunden)
  - Zähle `timeslice` hoch, bei Erreichen des Intervalls: rufe `TrackingModWebExportHelper.SendExport()` auf
  - Reset Timer nach dem Senden

**Bestehende MissionServer.c Struktur (nicht überschreiben, erweitern!):**
```c
modded class MissionServer extends MissionBase
{
    protected ref map<string, int> m_LeaderboardRequestTimes;
    protected const int LEADERBOARD_RATE_LIMIT_MS = 500;

    override void OnInit()
    {
        // ... bestehendes Init
        #ifdef NinjinsPvPPvE
        // ... Zone-Integration
        #endif
    }

    // RPC handlers...
}
```

Füge hinzu:
- Neue Member-Variablen für den Timer (in `#ifdef PSYERNS_FRAMEWORK`)
- `override void OnUpdate(float timeslice)` mit `super.OnUpdate(timeslice)` und Timer-Logik in `#ifdef PSYERNS_FRAMEWORK`

### Config-Erweiterung

**Modifikation:** `Ninjins_LeaderBoard/scripts/3_Game/General Configs/Config/TrackingModConfig.c`

Füge ein neues Feld hinzu:
```c
int WebExportIntervalSeconds; // Default: 300 (5 Minuten)
```

Setze den Default-Wert im Constructor auf `300`.

**WICHTIG:** Die Endpoint-URL und der API-Key werden NICHT in TrackingModConfig gespeichert — sie kommen aus `PsyernsFrameworkConfig.json` über `PF_WebConfig`. Nur das Intervall wird in der Leaderboard-Config gespeichert, da es spielspezifisch ist.

---

## Teil 2: PHP Referenz-Implementierung

### Neue Dateien erstellen unter `Ninjin_Leaderboard/web/`

#### `web/api/receive.php`
- Akzeptiert nur POST-Requests
- Liest `api_key` aus Query-Parameter
- Validiert gegen eine konfigurierbare Variable `$VALID_API_KEY` oben in der Datei
- Liest den JSON-Body aus `php://input`
- Validiert dass es gültiges JSON ist (`json_decode`)
- Speichert als `../data/leaderboard.json`
- Gibt JSON-Response zurück: `{"success": true/false, "message": "..."}`
- HTTP Status Codes: 200 (OK), 401 (Unauthorized), 400 (Bad Request), 405 (Method Not Allowed)

#### `web/api/leaderboard.php`
- Akzeptiert nur GET-Requests
- Setzt CORS-Header: `Access-Control-Allow-Origin: *` (für iframe-Einbettung)
- Setzt `Content-Type: application/json`
- Liest `../data/leaderboard.json` und gibt es zurück
- Falls Datei nicht existiert: HTTP 404 mit `{"error": "No data available"}`

#### `web/data/.htaccess`
- Deny direct access (`Deny from all`)
- Die JSON soll nur über `leaderboard.php` ausgeliefert werden

#### `web/api/config.php`
- Gemeinsame Konfiguration für beide PHP-Dateien
- `$VALID_API_KEY` Variable
- `$DATA_PATH` Variable (Pfad zur leaderboard.json)

---

## Teil 3: HTML Frontend

### Neue Datei: `Ninjin_Leaderboard/web/leaderboard.html`

**Alles in einer einzigen HTML-Datei** (CSS + JS inline) — einfach zu deployen.

### Konfiguration (oben im `<script>` Block)
```javascript
const CONFIG = {
    apiUrl: "./api/leaderboard.php",  // Konfigurierbar
    refreshInterval: 300000,           // 5 Minuten in ms
    maxPlayers: 20                     // Top 20
};
```

### Layout
- **Tabs:** Zwei Tabs oben — "PvE" und "PvP"
- Klick auf Tab wechselt die angezeigte Tabelle
- Aktiver Tab ist visuell hervorgehoben
- Wenn `disablePVELeaderboard` oder `disablePVPLeaderboard` true ist, den entsprechenden Tab ausblenden

### Tabellen-Spalten (Erweitert)
| Spalte | Datenfeld | Beschreibung |
|--------|-----------|--------------|
| Rang | (Index + 1) | Position in der Liste |
| Name | `playerName` | Spielername |
| Punkte | `pvePoints` / `pvpPoints` | Je nach aktivem Tab |
| Kills | `categoryKills` | Aufgeschlüsselt nach Kategorien (dynamisch aus `categoryPreviews`) |
| Deaths | `pveDeaths` / `pvpDeaths` | Je nach Tab |
| Longest Range | `categoryLongestRanges` | Beste Distanz (falls vorhanden) |
| Status | `isOnline` | Online/Offline Indikator (grüner/grauer Punkt) |
| Letzter Login | `lastLoginDate` | Datum/Uhrzeit |

### JSON-Datenstruktur (die das Frontend erwartet)
```json
{
    "generatedAt": "2026-03-22 12:00:00",
    "playerOnlineCounter": 24,
    "totalPlayers": 150,
    "disablePVPLeaderboard": false,
    "disablePVELeaderboard": false,
    "includePlayerIDs": false,
    "exportPlayerLimit": 100,
    "topPVEPlayers": [
        {
            "playerName": "Survivor",
            "pvePoints": 5120,
            "pvpPoints": 320,
            "deathCount": 4,
            "pveDeaths": 3,
            "pvpDeaths": 1,
            "isOnline": 1,
            "survivorType": "Male",
            "lastLoginDate": "2026-03-22 11:45:00",
            "categoryKills": {"Zombies": 150, "Animals": 30, "AI": 10},
            "categoryDeaths": {"Players": 1},
            "categoryLongestRanges": {"Players": 350}
        }
    ],
    "topPVPPlayers": [],
    "categoryPreviews": {"Zombies": "ZombieBase", "Animals": "AnimalBase"}
}
```

### Theme: Clean/Modern
- Heller Hintergrund (`#f5f5f5` / `#ffffff`)
- Klare Typografie — System-Font-Stack (sans-serif)
- Tabelle mit subtilen Zeilen-Trennern und hover-Effekt
- Tab-Leiste oben mit klarer Hervorhebung des aktiven Tabs
- Rang #1-3 mit subtiler goldener/silberner/bronzener Hervorhebung
- Online-Status als kleiner farbiger Punkt (grün = online, grau = offline)
- Responsive: Funktioniert auch in schmalen iframes
- Header-Bereich mit: "Leaderboard", Spieler online, Gesamtspieler, letzte Aktualisierung

### Auto-Refresh
- Alle 5 Minuten (`CONFIG.refreshInterval`) neue Daten per `fetch()` holen
- Bei Fehler: Dezente Fehlermeldung anzeigen, nächsten Refresh abwarten
- "Letzte Aktualisierung" Timestamp anzeigen aus `generatedAt`

### iframe-Fähigkeit
- Keine externen Dependencies (kein CDN, kein Framework)
- Funktioniert standalone und als iframe
- Keine Navigation-Links die aus dem iframe herausführen

---

## Zusammenfassung der zu erstellenden/modifizierenden Dateien

### Neue Dateien
1. `Ninjins_LeaderBoard/scripts/3_Game/General Configs/Utils/TrackingModWebExportHelper.c` — EnforceScript Web Export Helper + PF_LeaderboardApi
2. `Ninjin_Leaderboard/web/api/config.php` — PHP Konfiguration
3. `Ninjin_Leaderboard/web/api/receive.php` — POST Endpoint
4. `Ninjin_Leaderboard/web/api/leaderboard.php` — GET Endpoint
5. `Ninjin_Leaderboard/web/data/.htaccess` — Direktzugriff blockieren
6. `Ninjin_Leaderboard/web/leaderboard.html` — Frontend

### Zu modifizierende Dateien
7. `Ninjins_LeaderBoard/scripts/5_Mission/MissionServer.c` — `#ifdef PSYERNS_FRAMEWORK` Blöcke für Init + Timer in OnUpdate
8. `Ninjins_LeaderBoard/scripts/3_Game/General Configs/Config/TrackingModConfig.c` — `WebExportIntervalSeconds` Feld hinzufügen

---

## Regeln

- **Enforce Script Style:** Folge dem bestehenden Code-Stil des Ninjin_Leaderboard Projekts
- **#ifdef Guards:** Alles was Psyerns Framework referenziert MUSS in `#ifdef PSYERNS_FRAMEWORK` / `#endif` stehen
- **Keine neuen Dependencies:** Das Leaderboard Mod darf NICHT von Psyerns Framework abhängen (optional via #ifdef)
- **Minimale Änderungen:** Bestehende Funktionalität nicht anfassen. Nur hinzufügen.
- **Kein requiredAddon:** `config.cpp` bekommt KEINEN Eintrag für Psyerns Framework
- **PHP Security:** Input validieren, API-Key prüfen, keine Directory Traversal ermöglichen
- **HTML:** Alles in einer Datei, keine externen Abhängigkeiten

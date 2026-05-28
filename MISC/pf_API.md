# Psyerns Framework — API Dokumentation

Vollständige API-Referenz für die Entwicklung eines WordPress-Plugins, das mit dem Psyerns Framework auf einem DayZ-Server kommuniziert.

---

## Übersicht

Das Psyerns Framework sendet HTTP-Requests vom DayZ-Server an externe Endpoints. WordPress muss diese Requests **empfangen** (als REST-Endpoint) und/oder Daten **bereitstellen** (als REST-API).

**Kommunikationsrichtung:** DayZ Server → WordPress (POST/GET)

**Authentifizierung:** API-Key als Query-Parameter `?api_key=...` (DayZ RestContext unterstützt keine Custom-Header wie Authorization).

**Content-Type:** Immer `application/json`

---

## Konfiguration auf DayZ-Seite

### PsyernsFrameworkConfig.json

Pfad: `$profile:DeadmansEcho\PsyernsFramework\PsyernsFrameworkConfig.json`

```json
{
    "EnableDebugLogging": false,
    "DefaultRetryCount": 3,
    "QueueMaxSize": 100,
    "EnableServerStartNotification": true,
    "ServerStartDelaySeconds": 30,
    "ServerName": "Deadmans Echo",
    "Endpoints": [
        {
            "Name": "WordPress",
            "BaseUrl": "https://your-site.com/wp-json/psyern/v1",
            "ApiKey": "YOUR_SECRET_KEY",
            "Enabled": true,
            "RateLimitMs": 5000
        },
        {
            "Name": "Discord",
            "BaseUrl": "https://discord.com/api/webhooks",
            "ApiKey": "WEBHOOK_ID/WEBHOOK_TOKEN",
            "Enabled": true,
            "RateLimitMs": 1000
        }
    ]
}
```

### PF_RestConfig.json

Pfad: `$profile:DeadmansEcho\PsyernsFramework\PF_RestConfig.json`

```json
{
    "BaseUrl": "https://your-site.com/wp-json/psyern/v1",
    "ApiKey": "YOUR_SECRET_KEY",
    "WebhookUrls": [],
    "EnableWhitelist": true,
    "EnablePlayerLookup": true,
    "EnableServerStatus": true,
    "EnableKillFeed": false,
    "EnableDiscordEvents": false,
    "EnableAlertSystem": false,
    "ServerStatusIntervalSeconds": 30,
    "DiscordWebhookId": "",
    "DiscordWebhookToken": "",
    "AlertRules": []
}
```

---

## WordPress REST Endpoints

Das Plugin muss folgende Endpoints registrieren. Alle unter dem Namespace `psyern/v1`.

---

### 1. Server Status

**Empfängt periodische Server-Status-Updates vom DayZ-Server.**

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | `/wp-json/psyern/v1/server/status` |
| **Auth** | `?api_key=...` |
| **Intervall** | Konfigurierbar (default: alle 30 Sekunden) |

**Request Body:**
```json
{
    "serverName": "Deadmans Echo",
    "playerCount": 24,
    "uptimeSeconds": 14400,
    "mapName": "chernarusplus",
    "dayTime": "12:30",
    "timestamp": "2026-03-23T14:32:00Z"
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `serverName` | string | Servername aus PF_WebConfig |
| `playerCount` | int | Aktuell verbundene Spieler |
| `uptimeSeconds` | int | Server-Laufzeit in Sekunden seit Framework-Start |
| `mapName` | string | Kartenname (z.B. "chernarusplus", "enoch") |
| `dayTime` | string | Aktuelle Ingame-Tageszeit "HH:MM" |
| `timestamp` | string | ISO-8601 UTC Zeitstempel |

**Erwartete Response:**
```json
{ "success": true }
```

---

### 2. Leaderboard Upload

**Empfängt Leaderboard-Daten vom DayZ-Server (via PF_WordPressApi).**

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | `/wp-json/psyern/v1/upload` |
| **Auth** | `?api_key=...` + `apiKey` im Body |

**Request Body:**
```json
{
    "apiKey": "YOUR_SECRET_KEY",
    "generatedAt": "2026-03-23T14:32:00Z",
    "playerOnlineCounter": 24,
    "totalPlayers": 150,
    "topPVEPlayers": [
        {
            "playerName": "Survivor",
            "odolozId": "76561198...",
            "kills": 245,
            "deaths": 12,
            "aiKills": 30,
            "longestShot": 423.5,
            "playtime": 128.5
        }
    ],
    "topPVPPlayers": [
        {
            "playerName": "Fighter",
            "odolozId": "76561198...",
            "kills": 89,
            "deaths": 34,
            "aiKills": 5,
            "longestShot": 612.0,
            "playtime": 96.2
        }
    ]
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `apiKey` | string | API-Key (doppelt: Query + Body) |
| `generatedAt` | string | ISO-8601 Zeitstempel der Generierung |
| `playerOnlineCounter` | int | Aktuell online |
| `totalPlayers` | int | Gesamt registrierte Spieler |
| `topPVEPlayers` | array | Top PvE Spieler |
| `topPVPPlayers` | array | Top PvP Spieler |

**Player-Objekt (`PF_WP_PlayerData`):**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `playerName` | string | Spielername |
| `odolozId` | string | Steam64 ID |
| `kills` | int | Gesamte Kills |
| `deaths` | int | Gesamte Tode |
| `aiKills` | int | AI-Kills |
| `longestShot` | float | Weitester Schuss (Meter) |
| `playtime` | float | Spielzeit (Stunden) |

**Erwartete Response:**
```json
{ "success": true }
```

---

### 3. Whitelist Check

**DayZ-Server fragt ab, ob ein Spieler whitelisted ist.**

| | |
|---|---|
| **Methode** | `GET` |
| **Endpoint** | `/wp-json/psyern/v1/whitelist/check` |
| **Auth** | `?api_key=...` |

**Query Parameter:**

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `steam_id` | string | Steam64 ID des Spielers |
| `api_key` | string | API-Key |

**Erwartete Response:**
```json
{
    "whitelisted": true
}
```

> **Wichtig:** Der DayZ-Client parst die Response per String-Matching: sucht nach `"whitelisted":true` oder `"whitelisted": true`. Kein anderes Format verwenden.

---

### 4. Whitelist Add

**DayZ-Server fügt einen Spieler zur Whitelist hinzu.**

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | `/wp-json/psyern/v1/whitelist/add` |
| **Auth** | `?api_key=...` |

**Request Body:**
```json
{
    "steamId": "76561198043039918",
    "name": "Psyern"
}
```

---

### 5. Whitelist Remove

**DayZ-Server entfernt einen Spieler von der Whitelist.**

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | `/wp-json/psyern/v1/whitelist/remove` |
| **Auth** | `?api_key=...` |

**Request Body:**
```json
{
    "steamId": "76561198043039918"
}
```

---

### 6. Player Lookup

**DayZ-Server fragt Spielerdaten ab.**

| | |
|---|---|
| **Methode** | `GET` |
| **Endpoint** | `/wp-json/psyern/v1/players/lookup` |
| **Auth** | `?api_key=...` |

**Query Parameter:**

| Parameter | Typ | Beschreibung |
|-----------|-----|--------------|
| `steam_id` | string | Steam64 ID |
| `api_key` | string | API-Key |

**Erwartete Response:**
```json
{
    "steamId": "76561198043039918",
    "name": "Psyern",
    "online": true,
    "posX": 6425.0,
    "posY": 0.0,
    "posZ": 2817.0,
    "health": 100.0
}
```

---

### 7. Online Players

**DayZ-Server fragt Liste aller Online-Spieler ab.**

| | |
|---|---|
| **Methode** | `GET` |
| **Endpoint** | `/wp-json/psyern/v1/players/online` |
| **Auth** | `?api_key=...` |

**Erwartete Response:**
```json
[
    {
        "steamId": "76561198043039918",
        "name": "Psyern",
        "online": true,
        "posX": 6425.0,
        "posY": 0.0,
        "posZ": 2817.0,
        "health": 100.0
    }
]
```

---

### 8. Ping

**DayZ-Server testet die Verbindung.**

| | |
|---|---|
| **Methode** | `GET` |
| **Endpoint** | `/wp-json/psyern/v1/ping` |
| **Auth** | `?api_key=...` |

**Erwartete Response:**
```json
{ "status": "ok" }
```

---

## KillFeed Webhook (Optional)

**Wird direkt an konfigurierte Webhook-URLs gesendet (nicht über WordPress-Endpoint).**

Falls das WordPress-Plugin auch Kill-Events empfangen soll, muss eine Webhook-URL in `PF_RestConfig.WebhookUrls` eingetragen werden.

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | Konfigurierte Webhook-URL (direkt, ohne Pfad) |
| **Auth** | Keine (URL ist das Secret) |

**Request Body:**
```json
{
    "killerSteamId": "76561198043039918",
    "killerName": "Psyern",
    "killerWeapon": "M4A1",
    "victimSteamId": "76561198000000000",
    "victimName": "Survivor",
    "posX": 6425.32,
    "posY": 12.5,
    "posZ": 2817.88,
    "distance": 245.7,
    "timestamp": "2026-03-23T14:32:00Z"
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `killerSteamId` | string | Steam64 ID des Killers (leer wenn kein Spieler) |
| `killerName` | string | Name des Killers oder Entity-Typ (z.B. "ZombieBase") |
| `killerWeapon` | string | Waffe/Item in den Händen |
| `victimSteamId` | string | Steam64 ID des Opfers |
| `victimName` | string | Name des Opfers |
| `posX/posY/posZ` | float | Position des Todes |
| `distance` | float | Distanz Killer↔Opfer in Metern |
| `timestamp` | string | ISO-8601 UTC |

---

## Alert System Webhook (Optional)

**Wird bei konfigurierten Trigger-Events an Webhook-URLs gesendet.**

| | |
|---|---|
| **Methode** | `POST` |
| **Endpoint** | Konfigurierte Webhook-URL pro AlertRule |
| **Auth** | Keine (URL ist das Secret) |

**Trigger-Typen:** `zone_enter`, `kill`, `loot`

**Request Body:**
```json
{
    "triggerType": "zone_enter",
    "playerName": "Psyern",
    "posX": 6425.32,
    "posY": 12.5,
    "posZ": 2817.88,
    "message": "Player Psyern entered the restricted zone at 6425/2817"
}
```

---

## Authentifizierung

### API-Key Validierung

Alle Requests tragen den API-Key als Query-Parameter:

```
GET /wp-json/psyern/v1/whitelist/check?steam_id=76561198...&api_key=MY_SECRET_KEY
POST /wp-json/psyern/v1/server/status?api_key=MY_SECRET_KEY
```

**WordPress-Plugin muss:**
1. `api_key` aus `$_GET['api_key']` oder `$request->get_param('api_key')` lesen
2. Gegen gespeicherten Key validieren
3. Bei ungültigem Key: HTTP 401 `{ "error": "Unauthorized" }` zurückgeben

### Warum Query-Parameter statt Header?

DayZ's `RestContext.SetHeader()` setzt nur den `Content-Type`. Custom-Header wie `Authorization` oder `X-Api-Key` werden **nicht unterstützt** von der Engine. Daher ist der API-Key immer als Query-Parameter.

---

## Datenfluss-Diagramm

```
DayZ Server                          WordPress
===========                          =========

[PF_ServerStatus]  --POST /server/status-->  [Status Endpoint]
  (alle 30s)                                   → Speichert in DB

[PF_WordPressApi]  --POST /upload-------->  [Leaderboard Endpoint]
  (bei Änderung)                               → Speichert PvE/PvP Daten

[PF_WhitelistMgr]  --GET /whitelist/check->  [Whitelist Endpoint]
  (bei Connect)                                → Gibt whitelisted: true/false

[PF_WhitelistMgr]  --POST /whitelist/add-->  [Whitelist Endpoint]
                    --POST /whitelist/remove   → Bearbeitet Whitelist

[PF_PlayerLookup]  --GET /players/lookup-->  [Player Endpoint]
                   --GET /players/online-->    → Gibt Spielerdaten

[PF_KillFeedMgr]   --POST (webhook URL)--->  [KillFeed Receiver]
  (bei Kill)                                   → Optional

[PF_AlertSystem]    --POST (webhook URL)--->  [Alert Receiver]
  (bei Trigger)                                → Optional
```

---

## WordPress Plugin Struktur (Empfehlung)

```php
psyerns-framework/
├── psyerns-framework.php          // Plugin-Hauptdatei
├── includes/
│   ├── class-pf-api.php           // REST API Registrierung
│   ├── class-pf-auth.php          // API-Key Validierung
│   ├── class-pf-server-status.php // Server Status Handler
│   ├── class-pf-leaderboard.php   // Leaderboard Upload/Display
│   ├── class-pf-whitelist.php     // Whitelist Management
│   ├── class-pf-players.php       // Player Lookup/Online
│   ├── class-pf-killfeed.php      // KillFeed Receiver (optional)
│   └── class-pf-database.php      // DB Schema & Queries
├── admin/
│   ├── settings-page.php          // Admin Settings (API-Key, Feature Toggles)
│   └── whitelist-page.php         // Whitelist Management UI
├── public/
│   ├── shortcodes.php             // [pf_leaderboard], [pf_server_status], etc.
│   └── assets/
│       ├── leaderboard.css
│       └── leaderboard.js
└── uninstall.php
```

### Beispiel: REST API Registrierung

```php
<?php
// class-pf-api.php

class PF_Api {

    const NAMESPACE = 'psyern/v1';

    public function register_routes() {
        // Server Status
        register_rest_route(self::NAMESPACE, '/server/status', [
            'methods'  => 'POST',
            'callback' => [$this, 'handle_server_status'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Leaderboard Upload
        register_rest_route(self::NAMESPACE, '/upload', [
            'methods'  => 'POST',
            'callback' => [$this, 'handle_leaderboard_upload'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Whitelist Check
        register_rest_route(self::NAMESPACE, '/whitelist/check', [
            'methods'  => 'GET',
            'callback' => [$this, 'handle_whitelist_check'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Whitelist Add
        register_rest_route(self::NAMESPACE, '/whitelist/add', [
            'methods'  => 'POST',
            'callback' => [$this, 'handle_whitelist_add'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Whitelist Remove
        register_rest_route(self::NAMESPACE, '/whitelist/remove', [
            'methods'  => 'POST',
            'callback' => [$this, 'handle_whitelist_remove'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Player Lookup
        register_rest_route(self::NAMESPACE, '/players/lookup', [
            'methods'  => 'GET',
            'callback' => [$this, 'handle_player_lookup'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Online Players
        register_rest_route(self::NAMESPACE, '/players/online', [
            'methods'  => 'GET',
            'callback' => [$this, 'handle_online_players'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);

        // Ping
        register_rest_route(self::NAMESPACE, '/ping', [
            'methods'  => 'GET',
            'callback' => [$this, 'handle_ping'],
            'permission_callback' => [$this, 'validate_api_key'],
        ]);
    }

    public function validate_api_key(WP_REST_Request $request) {
        $api_key = $request->get_param('api_key');
        $stored_key = get_option('pf_api_key', '');

        if (empty($stored_key) || $api_key !== $stored_key) {
            return new WP_Error(
                'unauthorized',
                'Invalid API key',
                ['status' => 401]
            );
        }

        return true;
    }

    public function handle_ping(WP_REST_Request $request) {
        return new WP_REST_Response(['status' => 'ok'], 200);
    }

    public function handle_server_status(WP_REST_Request $request) {
        $data = $request->get_json_params();

        // Speichern in Transient oder DB
        set_transient('pf_server_status', [
            'serverName'    => sanitize_text_field($data['serverName'] ?? ''),
            'playerCount'   => intval($data['playerCount'] ?? 0),
            'uptimeSeconds' => intval($data['uptimeSeconds'] ?? 0),
            'mapName'       => sanitize_text_field($data['mapName'] ?? ''),
            'dayTime'       => sanitize_text_field($data['dayTime'] ?? ''),
            'timestamp'     => sanitize_text_field($data['timestamp'] ?? ''),
            'received_at'   => current_time('mysql'),
        ], 120); // 2 Minuten TTL

        return new WP_REST_Response(['success' => true], 200);
    }

    public function handle_whitelist_check(WP_REST_Request $request) {
        $steam_id = sanitize_text_field($request->get_param('steam_id'));

        // DB Query
        global $wpdb;
        $table = $wpdb->prefix . 'pf_whitelist';
        $exists = $wpdb->get_var(
            $wpdb->prepare("SELECT COUNT(*) FROM $table WHERE steam_id = %s", $steam_id)
        );

        return new WP_REST_Response([
            'whitelisted' => ($exists > 0)
        ], 200);
    }
}
```

---

## Wichtige Hinweise für Plugin-Entwicklung

### DayZ Engine Limitierungen

1. **Keine Custom HTTP-Header** — API-Key kommt immer als `?api_key=...` Query-Parameter
2. **Kein HTTPS-Zertifikat-Pinning** — DayZ akzeptiert alle gültigen SSL-Zertifikate
3. **Async & Fire-and-Forget** — Die meisten DayZ-Requests warten nicht auf die Response (außer Whitelist Check). Responses müssen schnell sein.
4. **Rate-Limiting** — Server Status kommt alle 30s, Queue-Processor sendet alle 0.25–2s. Plugin sollte Bursts verarbeiten können.
5. **Request-Größe** — Leaderboard-Upload kann groß sein (bis 1000 Spieler). `post_max_size` und `upload_max_filesize` in PHP prüfen.

### Response-Format

- **Immer JSON** zurückgeben
- **Whitelist Check:** Muss exakt `"whitelisted": true` oder `"whitelisted": false` enthalten (wird per String-Matching geparst)
- **Fehler:** HTTP Status Codes verwenden (401, 400, 404, 500)

### Datenbank-Tabellen (Empfehlung)

```sql
-- Whitelist
CREATE TABLE {prefix}pf_whitelist (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    steam_id VARCHAR(20) NOT NULL UNIQUE,
    name VARCHAR(255) NOT NULL DEFAULT '',
    added_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Leaderboard
CREATE TABLE {prefix}pf_leaderboard (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    player_name VARCHAR(255) NOT NULL,
    odoloz_id VARCHAR(20) DEFAULT '',
    kills INT DEFAULT 0,
    deaths INT DEFAULT 0,
    ai_kills INT DEFAULT 0,
    longest_shot FLOAT DEFAULT 0,
    playtime FLOAT DEFAULT 0,
    board_type ENUM('pve', 'pvp') NOT NULL,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- KillFeed (optional)
CREATE TABLE {prefix}pf_killfeed (
    id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    killer_steam_id VARCHAR(20) DEFAULT '',
    killer_name VARCHAR(255) DEFAULT 'Unknown',
    killer_weapon VARCHAR(255) DEFAULT 'Unknown',
    victim_steam_id VARCHAR(20) DEFAULT '',
    victim_name VARCHAR(255) DEFAULT 'Unknown',
    pos_x FLOAT DEFAULT 0,
    pos_y FLOAT DEFAULT 0,
    pos_z FLOAT DEFAULT 0,
    distance FLOAT DEFAULT 0,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

---

## Feature-Toggles

Nicht alle Features müssen aktiviert sein. Der DayZ-Server sendet nur Daten für aktivierte Features:

| Feature | Config-Feld | WordPress Endpoint |
|---------|-------------|-------------------|
| Server Status | `EnableServerStatus` | `/server/status` |
| Leaderboard | WordPress Endpoint `Enabled` | `/upload` |
| Whitelist | `EnableWhitelist` | `/whitelist/*` |
| Player Lookup | `EnablePlayerLookup` | `/players/*` |
| KillFeed | `EnableKillFeed` | Webhook-URL |
| Discord Events | `EnableDiscordEvents` | — (nur Discord) |
| Alert System | `EnableAlertSystem` | Webhook-URL |
| Server Start | `EnableServerStartNotification` | — (nur Discord) |

---

## Versionsinfo

- **Framework:** Psyerns Framework v1.0.0
- **Autor:** Psyern
- **Community:** Deadmans Echo
- **Generiert:** 2026-03-23

# PsyernsFrameworkConfig.json — Konfigurationsreferenz

Pfad auf dem Server: `profiles/DeadmansEcho/PsyernsFramework/PsyernsFrameworkConfig.json`

Wird automatisch beim ersten Serverstart erstellt. Alle Platzhalter (`YOUR_...`) müssen durch echte Werte ersetzt werden.

---

## Allgemeine Einstellungen

| Feld | Typ | Default | Beschreibung |
|------|-----|---------|--------------|
| `EnableDebugLogging` | bool | `false` | Detailliertes Logging in RPT + Logdatei. Nur zum Debuggen aktivieren. |
| `DefaultRetryCount` | int | `3` | Wie oft ein fehlgeschlagener HTTP-Request wiederholt wird. |
| `QueueMaxSize` | int | `100` | Maximale Anzahl an Requests in der Warteschlange. |
| `EnableServerStartNotification` | bool | `false` | Sendet eine Discord-Nachricht wenn der Server gestartet ist. |
| `ServerStartDelaySeconds` | int | `30` | Wartezeit nach Serverstart bevor die Nachricht gesendet wird (Server muss erst unlocked sein). |
| `ServerName` | string | `"DayZ Server"` | Servername der in Notifications und Status-Meldungen angezeigt wird. |

## Endpoints

Jeder Endpoint definiert eine externe API-Verbindung.

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `Name` | string | Eindeutiger Name. `"WordPress"` und `"Discord"` werden intern referenziert. |
| `BaseUrl` | string | Basis-URL der API. |
| `ApiKey` | string | Authentifizierung. **Discord:** Format `WEBHOOK_ID/WEBHOOK_TOKEN` (aus der Webhook-URL). **WordPress:** Dein geheimer API-Key. |
| `Enabled` | bool | Endpoint aktivieren/deaktivieren. |
| `RateLimitMs` | int | Minimale Millisekunden zwischen Requests an diesen Endpoint. |

### Discord ApiKey — Woher?

1. Discord Server → Servereinstellungen → Integrationen → Webhooks
2. Webhook erstellen → URL kopieren
3. URL-Format: `https://discord.com/api/webhooks/123456789/abcdefghijkl`
4. ApiKey wird: `123456789/abcdefghijkl` (alles nach `/webhooks/`)

### WordPress ApiKey — Woher?

Den API-Key definierst du im WordPress Plugin unter Psyerns Framework → Settings.

## REST Feature Toggles

Jedes Feature kann einzeln aktiviert werden. Deaktivierte Features erzeugen keinen Traffic.

| Feld | Typ | Default | Beschreibung |
|------|-----|---------|--------------|
| `EnableWhitelist` | bool | `false` | Whitelist Check/Add/Remove über WordPress REST API. |
| `EnablePlayerLookup` | bool | `false` | Spielerdaten-Abfrage über WordPress REST API. |
| `EnableServerStatus` | bool | `false` | Periodischer Server-Status Push (Spieleranzahl, Map, Uptime). |
| `EnableKillFeed` | bool | `false` | Kill-Events an Webhook-URLs senden. |
| `EnableDiscordEvents` | bool | `false` | Spieler-Connect/Disconnect/Kill Events an Discord. |
| `EnableAlertSystem` | bool | `false` | Zonenbasierte Alerts an Webhook-URLs. |
| `ServerStatusIntervalSeconds` | int | `300` | Intervall in Sekunden für Server-Status Push (300 = alle 5 Minuten). |

## Discord Events

Separate Discord-Verbindung für Events (Connect, Disconnect, Kills). Nutzt einen eigenen Webhook, unabhängig vom Discord-Endpoint oben.

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `DiscordWebhookId` | string | Webhook ID für Events. |
| `DiscordWebhookToken` | string | Webhook Token für Events. |

## KillFeed

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `WebhookUrls` | string[] | Liste von Webhook-URLs die Kill-Events per POST empfangen. |

## Alert Rules

Jede Rule definiert eine Zone + Trigger. Wenn ein Spieler die Bedingung auslöst, wird ein POST an die WebhookUrl gesendet.

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `TriggerType` | string | `"zone_enter"`, `"kill"`, oder `"loot"` |
| `Radius` | float | Radius der Zone in Metern. |
| `PosX/PosY/PosZ` | float | Zentrum der Zone (Weltkoordinaten). |
| `WebhookUrl` | string | Webhook-URL die den Alert empfängt. |
| `MessageTemplate` | string | Nachricht mit Platzhaltern: `{playerName}`, `{posX}`, `{posY}`, `{posZ}`, `{triggerType}` |

# Psyerns Framework — DayZ Mod

[![DayZ](https://img.shields.io/badge/DayZ-1.29+-blue)](https://dayz.com)
[![Enforce Script](https://img.shields.io/badge/Enforce-Script-orange)](https://community.bistudio.com/wiki/DayZ:Enforce_Script_Syntax)
[![Maintainer](https://img.shields.io/badge/Maintainer-Psyern-green)](https://github.com/Psyern)

> A standalone, dependency-free HTTP/Webhook framework for DayZ mods — built on the engine-native `RestApi`. No external programs, no companion services, no root server access required.

## Features

**Web Transport:**
- Lightweight HTTP client wrapping DayZ's native `RestApi` / `RestContext` / `RestCallback`
- Fluent request builder with GET and POST support
- Async request queue with adaptive rate limiting and automatic retries
- Discord webhook integration with full embed support (embeds, fields, author, timestamps)
- WordPress REST API integration for leaderboard uploads
- Fluent JSON string builder with proper escaping
- Server start notification via Discord webhook

**REST API Modules:**
- Whitelist management (check/add/remove via REST)
- Player lookup and online status queries
- Periodic server status reporting (player count, uptime, map, time of day)
- Kill feed broadcasting to webhook URLs
- Discord event integration (player connect/disconnect, kills)
- Configurable alert system with zone-based triggers

**Infrastructure:**
- Single unified config file — everything in one JSON
- Framework-wide logger with daily log rotation and RPT output
- Server-only execution with clean `MissionServer` bootstrap
- `#ifdef PSYERNS_FRAMEWORK` support for optional integration by other mods

## Project Structure

```text
Psyerns_Framework/
├── config.cpp
├── mod.cpp
├── data/
│   └── PsyernsFrameworkConfig.json
└── scripts/
    ├── config.cpp
    ├── 3_Game/Psyerns_Framework/
    │   ├── Logging/
    │   │   └── PF_Logger.c
    │   ├── Utils/
    │   │   ├── PF_HttpArguments.c
    │   │   └── PF_JsonBuilder.c
    │   ├── REST/
    │   │   ├── Base/PF_RestBase.c
    │   │   ├── Config/PF_RestConfig.c
    │   │   ├── Discord/PF_DiscordIntegration.c
    │   │   ├── PlayerLookup/PF_PlayerLookup.c
    │   │   ├── ServerStatus/PF_ServerStatus.c
    │   │   └── Whitelist/PF_WhitelistManager.c
    │   └── Web/
    │       ├── PF_WebClient.c
    │       ├── PF_WebRequest.c
    │       ├── PF_WebResponse.c
    │       ├── Config/
    │       │   ├── PF_WebConfig.c
    │       │   └── PF_WebEndpoint.c
    │       ├── Payload/
    │       │   ├── PF_JsonPayload.c
    │       │   ├── PF_DiscordPayload.c
    │       │   └── PF_WordPressPayload.c
    │       ├── Queue/
    │       │   ├── PF_WebQueue.c
    │       │   └── PF_WebQueueItem.c
    │       ├── RestCallback/
    │       │   └── PF_RestCallback.c
    │       └── WebApi/
    │           ├── PF_WebApiBase.c
    │           ├── PF_DiscordWebhook.c
    │           └── PF_WordPressApi.c
    ├── 4_World/Psyerns_Framework/
    │   ├── PF_WebQueueProcessor.c
    │   └── REST/
    │       ├── PF_KillFeedHook.c
    │       ├── Alerts/PF_AlertSystem.c
    │       └── KillFeed/PF_KillFeedManager.c
    └── 5_Mission/Psyerns_Framework/
        ├── PF_MissionInit.c
        └── PF_RestInit.c
```

## Profile Structure

```text
profiles/
└── DeadmansEcho/
    └── PsyernsFramework/
        ├── PsyernsFrameworkConfig.json
        └── Logs/
            └── PF_Log_2026-03-23.log
```

All files are created automatically on first server start.

## Configuration

**One file for everything:** `profiles/DeadmansEcho/PsyernsFramework/PsyernsFrameworkConfig.json`

```json
{
    "EnableDebugLogging": false,
    "DefaultRetryCount": 3,
    "QueueMaxSize": 100,
    "EnableServerStartNotification": false,
    "ServerStartDelaySeconds": 30,
    "ServerName": "DayZ Server",
    "Endpoints": [
        {
            "Name": "WordPress",
            "BaseUrl": "https://your-site.com/wp-json/psyern/v1",
            "ApiKey": "YOUR_API_KEY_HERE",
            "Enabled": false,
            "RateLimitMs": 5000
        },
        {
            "Name": "Discord",
            "BaseUrl": "https://discord.com/api/webhooks",
            "ApiKey": "YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN",
            "Enabled": false,
            "RateLimitMs": 1000
        }
    ],
    "EnableWhitelist": false,
    "EnablePlayerLookup": false,
    "EnableServerStatus": false,
    "EnableKillFeed": false,
    "EnableDiscordEvents": false,
    "EnableAlertSystem": false,
    "ServerStatusIntervalSeconds": 30,
    "DiscordWebhookId": "",
    "DiscordWebhookToken": "",
    "WebhookUrls": [],
    "AlertRules": []
}
```

### General Settings

| Field | Default | Description |
|-------|---------|-------------|
| `EnableDebugLogging` | `false` | Verbose debug output to log file and RPT |
| `DefaultRetryCount` | `3` | Retries for failed HTTP requests |
| `QueueMaxSize` | `100` | Maximum queued requests |
| `EnableServerStartNotification` | `false` | Send Discord webhook when server starts |
| `ServerStartDelaySeconds` | `30` | Delay before start notification (wait for unlock) |
| `ServerName` | `"DayZ Server"` | Server name shown in notifications |

### Endpoints

| Field | Description |
|-------|-------------|
| `Name` | Unique identifier (`"WordPress"`, `"Discord"`) |
| `BaseUrl` | Base URL for API requests |
| `ApiKey` | Auth key. For Discord: `webhook_id/webhook_token` |
| `Enabled` | Whether the endpoint is active |
| `RateLimitMs` | Minimum ms between requests |

### REST Feature Toggles

| Field | Default | Description |
|-------|---------|-------------|
| `EnableWhitelist` | `false` | Whitelist check/add/remove via REST API |
| `EnablePlayerLookup` | `false` | Player data queries via REST API |
| `EnableServerStatus` | `false` | Periodic server status push to WordPress |
| `EnableKillFeed` | `false` | Kill events to webhook URLs |
| `EnableDiscordEvents` | `false` | Player connect/disconnect/kill to Discord |
| `EnableAlertSystem` | `false` | Zone-based alert triggers |
| `ServerStatusIntervalSeconds` | `30` | Status push interval |
| `DiscordWebhookId` | `""` | Discord webhook ID for events |
| `DiscordWebhookToken` | `""` | Discord webhook token for events |
| `WebhookUrls` | `[]` | Webhook URLs for kill feed |
| `AlertRules` | `[]` | Alert rule definitions |

## Usage Examples

### Discord Webhook with Embeds

```c
PF_DiscordWebhook discord = new PF_DiscordWebhook("WEBHOOK_ID", "WEBHOOK_TOKEN");
PF_DiscordPayload payload = new PF_DiscordPayload();
payload.username = "My DayZ Server";

PF_DiscordEmbed embed = payload.CreateEmbed();
embed.SetTitle("Server Status");
embed.SetDescription("The server is online.");
embed.SetColor(3066993);
embed.AddField("Players", "42/60", true);
embed.AddField("Uptime", "12h 34m", true);

discord.Send(payload);
```

### WordPress Leaderboard Upload

```c
PF_WordPressApi wordpress = new PF_WordPressApi("https://mysite.com/wp-json/psyern/v1", "MY_KEY");
PF_WordPressPayload payload = new PF_WordPressPayload();
payload.generatedAt = "2026-03-23T12:00:00Z";
payload.totalPlayers = 42;
wordpress.UploadLeaderboard(payload);
```

### Generic HTTP Request

```c
PF_WebClient client = PF_WebClient.GetInstance();
PF_WebRequest req = new PF_WebRequest();
req.SetUrl("https://api.example.com");
req.SetEndpoint("/data");
req.SetBody("{\"key\": \"value\"}");
req.Post();
client.Send(req);
```

### JSON Builder

```c
PF_JsonBuilder b = PF_JsonBuilder.Begin();
b.Add("name", "PlayerOne");
b.AddInt("kills", 15);
b.AddBool("online", true);
string json = b.Build();
// Result: {"name":"PlayerOne","kills":15,"online":true}
```

## Queue System

All HTTP requests are processed through an async queue:

- Requests are queued and sent one at a time
- Adaptive rate limiting: 250ms–2000ms between sends
- On success: interval decreases (faster throughput)
- On failure: interval increases (back-off)
- Failed requests are retried up to `DefaultRetryCount` times
- Queue processing runs on every server frame via `MissionServer.OnUpdate`

## Logging

The framework logs to both server RPT and a dedicated log file:

- Log path: `profiles/DeadmansEcho/PsyernsFramework/Logs/PF_Log_YYYY-MM-DD.log`
- Log levels: `Log` (always), `Error` (always), `Debug` (only when `EnableDebugLogging` is true)
- All entries prefixed with `[Psyerns Framework]` and timestamped

## Dependencies

Required:
- DayZ Standalone 1.29+

Optional:
- None — standalone framework with zero mod dependencies

## Installation

1. Add `Psyerns_Framework` to your server mod load order
2. Start the server once to auto-generate the config
3. Edit `profiles/DeadmansEcho/PsyernsFramework/PsyernsFrameworkConfig.json`
4. Configure your endpoints (WordPress URL + API Key, Discord Webhook ID/Token)
5. Enable the features you want (`EnableServerStatus`, `EnableKillFeed`, etc.)
6. Restart the server

## Integration by Other Mods

Use `#ifdef PSYERNS_FRAMEWORK` for optional integration:

```c
#ifdef PSYERNS_FRAMEWORK
PF_WebClient client = PF_WebClient.GetInstance();
// ... send requests
#endif
```

Integration points:
1. Use `PF_WebClient.GetInstance()` to send requests
2. Extend `PF_WebApiBase` for custom API targets
3. Use `PF_WebConfig.GetInstance().GetEndpoint("name")` to read endpoint config
4. Use `PF_JsonBuilder` for safe JSON construction

## Credits

- Author: Psyern
- Community: Deadmans Echo
- Built as a reusable HTTP framework for the DayZ modding community

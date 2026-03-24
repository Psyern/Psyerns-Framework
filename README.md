# Psyerns Framework

<p align="center">
  <img src="https://img.shields.io/badge/DayZ-1.29+-0074D9?style=for-the-badge&logo=steam&logoColor=white" alt="DayZ 1.29+">
  <img src="https://img.shields.io/badge/Enforce_Script-Enfusion-FF851B?style=for-the-badge" alt="Enforce Script">
  <img src="https://img.shields.io/badge/Dependencies-Zero-2ECC40?style=for-the-badge" alt="Zero Dependencies">
  <img src="https://img.shields.io/badge/License-GPL--2.0-green?style=for-the-badge" alt="License">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Discord-Webhooks-5865F2?style=flat-square&logo=discord&logoColor=white" alt="Discord">
  <img src="https://img.shields.io/badge/WordPress-REST_API-21759B?style=flat-square&logo=wordpress&logoColor=white" alt="WordPress">
  <img src="https://img.shields.io/badge/Steam-Avatar_API-1B2838?style=flat-square&logo=steam&logoColor=white" alt="Steam">
  <img src="https://img.shields.io/badge/REST-Whitelist_%7C_KillFeed_%7C_Alerts-E74C3C?style=flat-square" alt="REST Modules">
</p>

<p align="center">
  <b>A standalone, dependency-free HTTP/Webhook framework for DayZ mods</b><br>
  Built on the engine-native <code>RestApi</code>. No external programs, no companion services, no root server access required.
</p>

<p align="center">
  <a href="https://deadmansecho.com">
    <img src="https://img.shields.io/badge/Community-Deadmans_Echo-F0C040?style=for-the-badge" alt="Deadmans Echo">
  </a>
</p>

---

## Features

<table>
<tr>
<td width="33%" valign="top">

### Web Transport
- HTTP Client (GET/POST)
- Async request queue
- Adaptive rate limiting
- Automatic retries
- Discord webhook embeds
- WordPress REST API
- JSON builder
- Server start notification

</td>
<td width="33%" valign="top">

### REST Modules
- Whitelist management
- Player lookup & online status
- Server status reporting
- Kill feed broadcasting
- Discord event integration
- Zone-based alert system

</td>
<td width="33%" valign="top">

### Infrastructure
- Single unified config file
- Auto-generated API keys
- Daily log rotation
- Debug/Error/Info levels
- `#ifdef` optional integration
- WordPress plugin included

</td>
</tr>
</table>

---

## Quick Start

```
1. Add Psyerns_Framework to server mod load order
2. Start server → config auto-generates with API key
3. Copy API key from config → paste in WordPress plugin
4. Enable endpoints → restart → done
```

---

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
profiles/DeadmansEcho/PsyernsFramework/
├── PsyernsFrameworkConfig.json      ← auto-generated on first start
└── Logs/
    └── PF_Log_2026-03-24.log
```

---

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
    "ServerStatusIntervalSeconds": 300,
    "DiscordWebhookId": "",
    "DiscordWebhookToken": "",
    "WebhookUrls": [],
    "AlertRules": []
}
```

> **Auto-Generated API Keys:** If the WordPress endpoint `ApiKey` is empty or still set to `YOUR_API_KEY_HERE`, the server will auto-generate a secure key on startup (e.g. `pf-a7kx9m2bq4w1n6tp8r3j5hcv`) and save it to the config. Check the server log for: `[Psyerns Framework] Auto-generated API key for endpoint: WordPress → pf-xxxx...`

### General Settings

| Field | Default | Description |
|-------|---------|-------------|
| `EnableDebugLogging` | `false` | Verbose debug output to log file and RPT |
| `DefaultRetryCount` | `3` | Retries for failed HTTP requests |
| `QueueMaxSize` | `100` | Maximum queued requests |
| `EnableServerStartNotification` | `false` | Send Discord webhook when server has fully started (see below) |
| `ServerStartDelaySeconds` | `30` | Delay in seconds after init before notification is sent |
| `ServerName` | `"DayZ Server"` | Server name shown in notifications |

> **Server Start Notification:** The Discord webhook is only sent after the server has fully booted — including BattlEye initialization, map loading, and mod loading. The framework waits `ServerStartDelaySeconds` after `MissionServer.OnInit()` completes, which ensures the server is unlocked and accepting player connections before the notification fires. If the server crashes during startup, no notification is sent.

### Endpoints

| Field | Description |
|-------|-------------|
| `Name` | Unique identifier (`"WordPress"`, `"Discord"`) |
| `BaseUrl` | Base URL for API requests |
| `ApiKey` | Auth key. For Discord: `webhook_id/webhook_token` (see setup below) |
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
| `ServerStatusIntervalSeconds` | `300` | Status push interval (seconds) |
| `DiscordWebhookId` | `""` | Discord webhook ID for events |
| `DiscordWebhookToken` | `""` | Discord webhook token for events |
| `WebhookUrls` | `[]` | Webhook URLs for kill feed |
| `AlertRules` | `[]` | Alert rule definitions |

---

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
payload.generatedAt = "2026-03-24T12:00:00Z";
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

---

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

---

## Installation

### Requirements

| | |
|---|---|
| **DayZ** | 1.29+ |
| **Dependencies** | None — standalone framework |
| **WordPress** | Optional — plugin included for web integration |

### Step-by-Step

1. Add `Psyerns_Framework` to your server mod load order
2. Start the server once — config auto-generates at `profiles/DeadmansEcho/PsyernsFramework/PsyernsFrameworkConfig.json`
3. API key is auto-generated for the WordPress endpoint (check server log)
4. Configure your endpoints (URLs, enable/disable features)
5. Restart the server

---

### <img src="https://img.shields.io/badge/Discord-Setup-5865F2?style=flat-square&logo=discord&logoColor=white" alt="Discord">

1. Discord Server → **Server Settings** → **Integrations** → **Webhooks**
2. Create Webhook → select channel → **Copy URL**
3. URL format: `https://discord.com/api/webhooks/123456789/abcdefghijkl`
4. In config: `ApiKey` = `123456789/abcdefghijkl` (everything after `/webhooks/`)

---

### <img src="https://img.shields.io/badge/WordPress-Plugin_Setup-21759B?style=flat-square&logo=wordpress&logoColor=white" alt="WordPress">

1. Upload the `psyerns-framework` plugin folder to `wp-content/plugins/` → Activate
2. Go to **Psyerns Framework → Settings**
3. Enter the API Key from your DayZ server config (auto-generated on first start)
4. Set the WordPress endpoint in DayZ config:
   - `BaseUrl`: `https://your-site.com/wp-json/psyern/v1`
   - `ApiKey`: the auto-generated key
   - `Enabled`: `true`
5. Restart the DayZ server

---

### <img src="https://img.shields.io/badge/Connection-Test-2ECC40?style=flat-square" alt="Test"> Testing the Connection

After setup, test the API with the Ping endpoint:

```
https://your-site.com/wp-json/psyern/v1/ping?api_key=YOUR_KEY
```

Expected response:
```json
{"status":"ok"}
```

The Ping URL is also shown in the WordPress admin under **Psyerns Framework → Settings → Connection Test**, with a **"Test Now"** button.

---

### <img src="https://img.shields.io/badge/Steam-API_Key-1B2838?style=flat-square&logo=steam&logoColor=white" alt="Steam"> Steam API Key (Optional)

Required for automatic player avatar resolution in the WordPress plugin.

1. Go to **[steamcommunity.com/dev/apikey](https://steamcommunity.com/dev/apikey)**
2. Sign in with your Steam account
3. Enter a domain name (e.g. `deadmansecho.com`)
4. Copy the key → enter it in WordPress under **Psyerns Framework → Settings → Steam API Key**

---

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

---

## Credits

<p align="center">
  <b>Author:</b> Psyern<br>
  <b>Community:</b> <a href="https://deadmansecho.com">Deadmans Echo</a><br><br>
  Built as a reusable HTTP framework for the DayZ modding community.
</p>

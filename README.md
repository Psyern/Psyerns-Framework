# 🌐 Psyerns Framework — DayZ Mod

[![DayZ](https://img.shields.io/badge/DayZ-1.29+-blue)](https://dayz.com)
[![Enforce Script](https://img.shields.io/badge/Enforce-Script-orange)](https://community.bistudio.com/wiki/DayZ:Enforce_Script_Syntax)
[![Maintainer](https://img.shields.io/badge/Maintainer-Psyern-green)](https://github.com/Psyern)

> A standalone, dependency-free HTTP/Webhook framework for DayZ mods — built on the engine-native `RestApi`. No external programs, no companion services, no root server access required.

## ✨ Features

- Lightweight HTTP client wrapping DayZ's native `RestApi` / `RestContext` / `RestCallback`
- Fluent request builder with GET and POST support
- Async request queue with adaptive rate limiting and automatic retries
- JSON configuration with auto-generated defaults on first server start
- Discord webhook integration with full embed support
- WordPress REST API integration for leaderboard uploads
- Fluent JSON string builder with proper escaping
- Framework-wide logger with daily log rotation and RPT output
- Zero dependencies — no CF, no Dabs, no COT required
- Server-only execution with clean `MissionServer` bootstrap

## 📁 Project Structure

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
    │   └── PF_WebQueueProcessor.c
    └── 5_Mission/Psyerns_Framework/
        └── PF_MissionInit.c
```

## 📁 Profile Structure

```text
profiles/
└── Psyerns_Framework/
    ├── PsyernsFrameworkConfig.json
    └── Logs/
        └── PF_Log_2026-03-22.log
```

All files are created automatically on first server start. The config is populated with example endpoints for WordPress and Discord.

## ⚙️ Configuration

File: `profiles/Psyerns_Framework/PsyernsFrameworkConfig.json`

```json
{
    "EnableDebugLogging": false,
    "DefaultRetryCount": 3,
    "QueueMaxSize": 100,
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
            "ApiKey": "",
            "Enabled": false,
            "RateLimitMs": 1000
        }
    ]
}
```

### Fields

- `EnableDebugLogging` — Enables verbose debug output to log file and RPT
- `DefaultRetryCount` — Number of retries for failed HTTP requests (default: 3)
- `QueueMaxSize` — Maximum number of queued requests (default: 100)
- `Endpoints` — List of configured API endpoints

### Endpoint Fields

- `Name` — Unique identifier for the endpoint (e.g. `"WordPress"`, `"Discord"`)
- `BaseUrl` — Base URL for API requests
- `ApiKey` — Authentication key (usage depends on the API implementation)
- `Enabled` — Whether the endpoint is active
- `RateLimitMs` — Minimum milliseconds between requests to this endpoint

## 🚀 Usage Examples

### Simple POST to WordPress

```c
PF_WordPressApi wordpress = new PF_WordPressApi("https://mysite.com/wp-json/psyern/v1", "MY_API_KEY");
PF_WordPressPayload payload = new PF_WordPressPayload();
payload.generatedAt = "2026-03-22T12:00:00Z";
payload.totalPlayers = 42;
wordpress.UploadLeaderboard(payload);
```

### Discord Webhook

```c
PF_DiscordWebhook discord = new PF_DiscordWebhook("WEBHOOK_ID", "WEBHOOK_TOKEN");
discord.SendSimple("Player Kill", "PlayerA killed PlayerB with M4A1", 16711680);
```

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

### Generic HTTP POST

```c
PF_WebClient client = PF_WebClient.GetInstance();
PF_WebRequest req = new PF_WebRequest();
req.SetUrl("https://api.example.com");
req.SetEndpoint("/data");
req.SetBody("{\"key\": \"value\"}");
req.Post();
client.Send(req);
```

### Using the JSON Builder

```c
string json = PF_JsonBuilder.Begin()
    .Add("name", "PlayerOne")
    .AddInt("kills", 15)
    .AddBool("online", true)
    .Build();
// Result: {"name":"PlayerOne","kills":15,"online":true}
```

## 📨 Queue System

All HTTP requests are processed through an async queue with the following behavior:

- Requests are queued and sent one at a time
- Adaptive rate limiting: 250ms–2000ms between sends
- On success, the send interval decreases (faster throughput)
- On failure, the send interval increases (back-off)
- Failed requests are retried up to `DefaultRetryCount` times
- Queue processing runs on every server frame via `MissionServer.OnUpdate`

## 📝 Logging

The framework logs to both the server RPT and a dedicated log file:

- Log path: `profiles/Psyerns_Framework/Logs/PF_Log_YYYY-MM-DD.log`
- Log levels: `Log`, `Error`, `Debug`
- Debug logging is only active when `EnableDebugLogging` is `true`
- All log entries are prefixed with `[Psyerns Framework]` and timestamped

## 🧠 Technical Notes

- Language: Enforce Script
- Engine: DayZ Enfusion
- Script modules: `3_Game`, `4_World`, `5_Mission`
- Uses only engine-native `RestApi`, `RestContext`, `RestCallback`
- All classes prefixed with `PF_` to avoid naming collisions
- Server-only execution — bootstraps via modded `MissionServer`
- Config uses `$profile:` paths for server portability
- Framework is passive — does nothing if no endpoints are enabled

## 🔌 Dependencies

Required:

- DayZ Standalone

Optional:

- None — this is a standalone framework with zero mod dependencies

## 🚀 Installation

1. Add `Psyerns_Framework` to your server mod load order
2. Start the server once to auto-generate the config file
3. Edit `profiles/Psyerns_Framework/PsyernsFrameworkConfig.json`
4. Set your endpoint URLs and API keys
5. Set `Enabled` to `true` for the endpoints you want to use
6. Restart the server

## 🔗 Integration

Psyerns Framework is designed to be used by other mods as a shared HTTP layer. Dependent mods should:

1. Add `Psyerns_Framework` to their `requiredAddons[]` in `config.cpp`
2. Use `PF_WebClient.GetInstance()` to send requests
3. Use `PF_WebApiBase` as a base class for custom API targets
4. Use `PF_WebConfig.GetInstance().GetEndpoint("name")` to read endpoint config

## 👤 Credits

- Author: Psyern
- Community: Deadmans Echo
- Built as a reusable HTTP framework for the DayZ modding community

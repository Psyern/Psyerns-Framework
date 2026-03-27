# Psyerns Framework - Multi-Agent Orchestrator Prompt

## Project Identity

- **Name:** Psyerns Framework
- **Author:** Psyern
- **Community:** Deadmans Echo
- **Type:** Standalone DayZ Mod (EnforceScript Framework)
- **Purpose:** Provides DayZ modders with a simple, dependency-free HTTP/Webhook API built on DayZ's native `RestApi` engine — no external programs, no companion services, no root server access required.

---

## Mission Statement

> DayZ mods have no native way to talk to the outside world. Discord Killfeeds, Leaderboard websites, WordPress plugins, custom APIs — they all need HTTP. Psyerns Framework solves this once, so every modder can use it.

---

## Architecture Overview

```
Psyerns_Framework/
├── config.cpp                                          # DayZ mod definition
├── mod.cpp                                             # Mod metadata
├── scripts/
│   ├── config.cpp                                      # Script module registration
│   ├── 3_Game/
│   │   └── Psyerns_Framework/
│   │       ├── Web/
│   │       │   ├── PF_WebClient.c                      # Core HTTP client (wraps RestApi)
│   │       │   ├── PF_WebRequest.c                     # Request builder (URL, headers, body)
│   │       │   ├── PF_WebResponse.c                    # Response wrapper (data, status, timing)
│   │       │   ├── RestCallback/
│   │       │   │   └── PF_RestCallback.c               # Engine callback handler
│   │       │   ├── WebApi/
│   │       │   │   ├── PF_WebApiBase.c                 # Base class for API targets
│   │       │   │   ├── PF_DiscordWebhook.c             # Discord webhook implementation
│   │       │   │   └── PF_WordPressApi.c               # WordPress REST API implementation
│   │       │   ├── Payload/
│   │       │   │   ├── PF_JsonPayload.c                # Generic JSON payload builder
│   │       │   │   ├── PF_DiscordPayload.c             # Discord-specific embed payloads
│   │       │   │   └── PF_WordPressPayload.c           # WordPress-specific payloads
│   │       │   ├── Queue/
│   │       │   │   ├── PF_WebQueue.c                   # Async request queue (thread-safe)
│   │       │   │   └── PF_WebQueueItem.c               # Individual queue entry
│   │       │   └── Config/
│   │       │       ├── PF_WebConfig.c                  # Main webhook/API configuration
│   │       │       └── PF_WebEndpoint.c                # Single endpoint definition
│   │       ├── Logging/
│   │       │   └── PF_Logger.c                         # Framework logger
│   │       └── Utils/
│   │           ├── PF_HttpArguments.c                  # URL query string builder
│   │           └── PF_JsonBuilder.c                    # Fluent JSON string builder
│   ├── 4_World/
│   │   └── Psyerns_Framework/
│   │       └── PF_WebQueueProcessor.c                  # Queue daemon (processes requests)
│   └── 5_Mission/
│       └── Psyerns_Framework/
│           └── PF_MissionInit.c                        # Framework bootstrap on mission start
└── data/
    └── PsyernsFrameworkConfig.json                     # Default config template
```

---

## Technical Foundation (DayZ Native RestApi)

The framework wraps these **engine-native** classes (no dependencies required):

### Engine Classes Available
```
RestApi              # HTTP client singleton
├── GetRestContext(string baseUrl) → RestContext
└── EnableDebug(bool)

RestContext           # Bound to a base URL
├── SetHeader(string contentType)
├── POST(RestCallback cb, string endpoint, string data)
└── GET(RestCallback cb, string endpoint)   // believed to exist

RestCallback         # Async response handler
├── OnSuccess(string data, int dataSize)
├── OnError(int errorCode)
├── OnTimeout()
└── OnFileCreated(string fileName, int dataSize)

// Global functions
CreateRestApi() → RestApi    // Creates new instance
GetRestApi() → RestApi       // Returns existing (or null)

// Error enum
ERestResultState.EREST_ERROR = 5
```

### Reference Implementations (Already analyzed)
- **Dabs Framework** `WebApiBase` + `DiscordWebhook` → Clean base class pattern
- **COT** `JMWebhookModule` → Queue system with rate limiting + config persistence

---

## Agent Orchestration Plan

This project requires **5 specialized agents** working in phases. Each agent has a clear scope, inputs, and outputs.

### Phase 1: Foundation (Parallel)

#### Agent 1: Core Web Client
```
Scope: PF_WebClient, PF_WebRequest, PF_WebResponse, PF_RestCallback, PF_HttpArguments
```

**Task:** Build the core HTTP abstraction layer.

**Files to create:**
1. `PF_WebClient.c` — Singleton wrapper around `RestApi`/`RestContext`
   - `static PF_WebClient GetInstance()`
   - `PF_WebRequest CreateRequest(string baseUrl)`
   - `void Send(PF_WebRequest request)` → delegates to queue or direct send
   - Manages RestApi lifecycle (CreateRestApi/GetRestApi)

2. `PF_WebRequest.c` — Fluent request builder
   - `PF_WebRequest SetUrl(string url)`
   - `PF_WebRequest SetEndpoint(string endpoint)`
   - `PF_WebRequest SetHeader(string header)`
   - `PF_WebRequest SetBody(string jsonBody)`
   - `PF_WebRequest SetCallback(PF_RestCallback callback)`
   - `PF_WebRequest Post()` / `PF_WebRequest Get()`
   - Internal: stores method, url, endpoint, body, callback

3. `PF_WebResponse.c` — Response data container
   - `bool IsSuccess()`
   - `string GetData()`
   - `int GetDataSize()`
   - `int GetErrorCode()`
   - `string GetErrorString()`
   - `int GetElapsedMs()`

4. `PF_RestCallback.c` — Extends engine `RestCallback`
   - Wraps OnSuccess/OnError/OnTimeout
   - Populates PF_WebResponse
   - Fires user-provided callback function
   - Logs via PF_Logger
   - Tracks timing (start → response)

5. `PF_HttpArguments.c` — URL query builder (based on Dabs pattern)
   - `void Add(string key, string value)`
   - `string ToQuery(string basePath)` → builds `?key=val&key2=val2`
   - Auto-filters empty values

**Design rules:**
- NO dependency on Dabs Framework, CF, or COT
- Uses ONLY engine-native RestApi/RestContext/RestCallback
- Prefix all classes with `PF_` to avoid collisions
- Server-only execution (`#ifdef SERVER` guards where needed)

---

#### Agent 2: Queue System
```
Scope: PF_WebQueue, PF_WebQueueItem, PF_WebQueueProcessor
```

**Task:** Build the async message queue (based on COT's Thread_ProcessQueue pattern).

**Files to create:**
1. `PF_WebQueueItem.c` — Single queued request
   - `PF_WebRequest m_Request`
   - `int m_QueuedAt` (timestamp)
   - `int m_RetryCount`
   - `int m_MaxRetries` (default 3)

2. `PF_WebQueue.c` — Thread-safe FIFO queue
   - `void Enqueue(PF_WebRequest request)`
   - `PF_WebQueueItem Dequeue()`
   - `int Count()`
   - `void Clear()`
   - `bool IsEmpty()`
   - Internal: `ref array<ref PF_WebQueueItem> m_Items`

3. `PF_WebQueueProcessor.c` (4_World) — Background daemon
   - Started via `GetGame().GameScript.Call(this, "ProcessLoop", NULL)`
   - Processes one request at a time
   - Adaptive rate limiting (250ms-2000ms between sends)
   - Retry on failure (up to MaxRetries)
   - Idle polling at 100ms
   - Logging of queue depth, send times, errors

**Design rules:**
- Based on COT's proven Thread_ProcessQueue pattern
- Adaptive throttling to avoid API rate limits
- Queue persists across the mission lifetime
- Failed requests retry with exponential backoff

---

#### Agent 3: Configuration System
```
Scope: PF_WebConfig, PF_WebEndpoint, Config JSON template
```

**Task:** Build the JSON-based configuration for endpoints.

**Files to create:**
1. `PF_WebEndpoint.c` — Single API endpoint definition
   - `string Name` (e.g., "WordPress", "Discord")
   - `string BaseUrl` (e.g., "https://mysite.com/wp-json/psyern/v1")
   - `string ApiKey` (authentication)
   - `bool Enabled`
   - `int RateLimitMs` (minimum ms between requests, default 1000)

2. `PF_WebConfig.c` — Configuration manager
   - `ref array<ref PF_WebEndpoint> Endpoints`
   - `bool EnableDebugLogging`
   - `int DefaultRetryCount`
   - `int QueueMaxSize`
   - `void Load()` → `JsonFileLoader<PF_WebConfig>.JsonLoadFile(path, this)`
   - `void Save()` → `JsonFileLoader<PF_WebConfig>.JsonSaveFile(path, this)`
   - `PF_WebEndpoint GetEndpoint(string name)`
   - Config path: `$profile:Psyerns_Framework\PsyernsFrameworkConfig.json`

3. `PsyernsFrameworkConfig.json` — Default template
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

**Design rules:**
- Auto-create config with defaults if not exists
- Reload-friendly (admin command support)
- Validate URLs and rate limits on load
- Config path uses `$profile:` for server portability

---

### Phase 2: API Implementations (Parallel, after Phase 1)

#### Agent 4: WordPress API + Payload
```
Scope: PF_WordPressApi, PF_WordPressPayload, PF_JsonPayload, PF_JsonBuilder
```

**Task:** Build the WordPress integration — the primary use case.

**Files to create:**
1. `PF_JsonBuilder.c` — Fluent JSON string builder (no engine serializer needed)
   - `PF_JsonBuilder Begin()`
   - `PF_JsonBuilder Add(string key, string value)`
   - `PF_JsonBuilder Add(string key, int value)`
   - `PF_JsonBuilder AddArray(string key, array<string> values)`
   - `PF_JsonBuilder AddObject(string key, PF_JsonBuilder nested)`
   - `string Build()` → returns JSON string
   - Handles escaping of special characters

2. `PF_JsonPayload.c` — Generic JSON payload base class
   - `string Serialize()` → uses JsonSerializer or PF_JsonBuilder
   - Override pattern for custom payloads

3. `PF_WordPressPayload.c` — WordPress-specific data structure
   - Matches what the WordPress plugin expects
   - `string apiKey`
   - `string generatedAt`
   - `int playerOnlineCounter`
   - `int totalPlayers`
   - `ref array<ref PF_WP_PlayerData> topPVEPlayers`
   - `ref array<ref PF_WP_PlayerData> topPVPPlayers`
   - `string Serialize()` → JSON matching WP REST endpoint schema

4. `PF_WebApiBase.c` — Base class for all API targets (based on Dabs WebApiBase)
   - `protected RestApi m_Rest`
   - `protected RestContext m_RestContext`
   - `string GetBaseUrl()` — override in subclasses
   - `void Post(string endpoint, string data, PF_RestCallback callback)`
   - `void Get(string endpoint, PF_RestCallback callback)`
   - Constructor initializes RestApi + RestContext

5. `PF_WordPressApi.c` — WordPress REST API client
   - Extends `PF_WebApiBase`
   - `void UploadLeaderboard(PF_WordPressPayload data)`
   - `void Ping()` — health check endpoint
   - Sets `Authorization: Bearer {apiKey}` header
   - Base URL from config
   - Posts to `/upload` endpoint

**Design rules:**
- WordPress payload MUST match the WordPress plugin's expected schema
- API key sent as query parameter (WordPress REST API compatibility)
- JSON builder handles EnforceScript string escaping quirks
- Payload classes are reusable by other mods

---

#### Agent 5: Discord Webhook + Logging
```
Scope: PF_DiscordWebhook, PF_DiscordPayload, PF_Logger, PF_MissionInit
```

**Task:** Build Discord integration and framework bootstrap.

**Files to create:**
1. `PF_DiscordPayload.c` — Discord embed structure
   - `string username`
   - `string content`
   - `ref array<ref PF_DiscordEmbed> embeds`
   - `PF_DiscordEmbed CreateEmbed()`
   - `string Serialize()`
   - Nested: `PF_DiscordEmbed`, `PF_DiscordEmbedField`, `PF_DiscordEmbedAuthor`

2. `PF_DiscordWebhook.c` — Discord webhook sender
   - Extends `PF_WebApiBase`
   - `void Send(PF_DiscordPayload payload)`
   - `void SendSimple(string title, string message, int color)`
   - Base URL: `https://discord.com/api/webhooks`
   - Endpoint: `/{webhookId}/{webhookToken}`

3. `PF_Logger.c` — Framework-wide logger
   - `static void Log(string message)`
   - `static void Error(string message)`
   - `static void Debug(string message)` (only if EnableDebugLogging)
   - Output to: `$profile:Psyerns_Framework\Logs\PF_Log_YYYY-MM-DD.log`
   - Also prints to server RPT
   - Prefix: `[Psyerns Framework]`

4. `PF_MissionInit.c` (5_Mission) — Framework bootstrap
   - Hooks into `MissionServer.OnInit()` or modded MissionServer
   - Loads PF_WebConfig
   - Initializes PF_WebClient
   - Starts PF_WebQueueProcessor
   - Logs startup message
   - Saves default config if none exists

**Design rules:**
- Discord payload matches Discord API v10 webhook schema
- Logger creates log directory if not exists
- MissionInit is the ONLY entry point — clean bootstrap
- Framework is passive (does nothing if no endpoints configured)

---

### Phase 3: Integration & Config Files (Sequential, after Phase 1+2)

#### Agent 6 (Main Agent): Mod Packaging
```
Scope: config.cpp, mod.cpp, scripts/config.cpp, README
```

**Task:** Package everything as a proper DayZ mod.

**Files to create:**
1. `config.cpp` — Main mod config
```cpp
class CfgPatches
{
    class Psyerns_Framework
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data"};
    };
};

class CfgMods
{
    class Psyerns_Framework
    {
        type = "mod";
        name = "Psyerns Framework";
        author = "Psyern";
        credits = "Psyern, Deadmans Echo Community";
        version = "1.0.0";

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"Psyerns_Framework/scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"Psyerns_Framework/scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"Psyerns_Framework/scripts/5_Mission"};
            };
        };
    };
};
```

2. `scripts/config.cpp` — Script registration

3. `mod.cpp` — Steam Workshop metadata

---

## Cross-Agent Dependencies

```
Phase 1 (Parallel):
  Agent 1: Core Web Client    ─┐
  Agent 2: Queue System        ├── No dependencies between them
  Agent 3: Config System       ─┘

Phase 2 (Parallel, depends on Phase 1):
  Agent 4: WordPress API       ─┐── Both depend on Agent 1 (PF_WebApiBase)
  Agent 5: Discord + Logger    ─┘   and Agent 3 (PF_WebConfig)

Phase 3 (Sequential, depends on all):
  Agent 6: Mod Packaging       ─── Depends on all files existing
```

---

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | `PF_PascalCase` | `PF_WebClient` |
| Files | `PF_PascalCase.c` | `PF_WebClient.c` |
| Member vars | `m_camelCase` | `m_RestContext` |
| Static vars | `s_camelCase` | `s_Instance` |
| Constants | `PF_UPPER_CASE` | `PF_CONFIG_PATH` |
| Config path | `$profile:Psyerns_Framework\` | |
| Log prefix | `[Psyerns Framework]` | |

---

## API Usage Examples (What Modders Will Write)

### Example 1: Simple POST to WordPress
```c
// In any mod's server-side code:
PF_WordPressApi wordpress = new PF_WordPressApi("https://mysite.com/wp-json/psyern/v1", "MY_API_KEY");
PF_WordPressPayload payload = new PF_WordPressPayload();
payload.generatedAt = PF_Utils.GetTimestamp();
payload.totalPlayers = 42;
// ... fill payload
wordpress.UploadLeaderboard(payload);
```

### Example 2: Discord Webhook
```c
PF_DiscordWebhook discord = new PF_DiscordWebhook("WEBHOOK_ID", "WEBHOOK_TOKEN");
discord.SendSimple("Player Kill", "PlayerA killed PlayerB with M4A1", 16711680);
```

### Example 3: Generic HTTP POST
```c
PF_WebClient client = PF_WebClient.GetInstance();
PF_WebRequest req = client.CreateRequest("https://api.example.com");
req.SetEndpoint("/data");
req.SetHeader("application/json");
req.SetBody("{\"key\": \"value\"}");
req.Post();
```

### Example 4: Using the Queue
```c
// Requests are automatically queued and rate-limited
PF_WebClient client = PF_WebClient.GetInstance();
for (int i = 0; i < 50; i++)
{
    PF_WebRequest req = client.CreateRequest("https://api.example.com");
    req.SetEndpoint("/batch/" + i.ToString());
    req.SetBody(someData);
    req.Post(); // Queued, not sent immediately
}
// Queue processor handles rate limiting automatically
```

---

## Integration with Ninjins Leaderboard

The first consumer of Psyerns Framework will be Ninjins Leaderboard mod:

```
Ninjins_LeaderBoard (existing mod)
├── Depends on: Psyerns_Framework
├── On SavePlayerData():
│   1. Build PF_WordPressPayload from TrackingModWebLeaderboardExport
│   2. PF_WordPressApi.UploadLeaderboard(payload)
│   3. Framework handles queue, rate limiting, retries
└── Config: Server admin sets WordPress URL + API key in PsyernsFrameworkConfig.json
```

---

## WordPress Plugin (Companion - Separate Project)

The WordPress plugin is NOT part of this mod but is designed to work with it:

```
WordPress Plugin: "Psyerns DayZ Leaderboard"
├── REST Endpoint: /wp-json/psyern/v1/upload (POST, requires API key)
├── Shortcode: [psyern_leaderboard type="pvp" limit="10"]
├── Admin Panel: API key management, data viewer
└── Storage: WordPress custom table or options
```

---

## Execution Command

To build this framework, run the agents in this order:

```
1. Launch Agents 1, 2, 3 in parallel (Phase 1 - Foundation)
2. Wait for all three to complete
3. Launch Agents 4, 5 in parallel (Phase 2 - Implementations)
4. Wait for both to complete
5. Run Agent 6 sequentially (Phase 3 - Packaging)
6. Integration test: verify all classes compile together
```

---

## Quality Gates

Before marking any agent as complete:

- [ ] All classes use `PF_` prefix
- [ ] No dependencies on Dabs Framework, CF, or COT
- [ ] Only uses engine-native RestApi/RestContext/RestCallback
- [ ] Server-only code guarded with appropriate checks
- [ ] JSON serialization works with EnforceScript's JsonSerializer
- [ ] Config uses `$profile:` paths for server portability
- [ ] Logging uses PF_Logger (not bare Print())
- [ ] Error handling on all HTTP operations
- [ ] Code follows EnforceScript conventions from CLAUDE.md

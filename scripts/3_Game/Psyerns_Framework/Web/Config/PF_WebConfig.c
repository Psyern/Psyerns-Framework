class PF_WebConfig
{
	int ConfigVersion;
	bool EnableDebugLogging;
	int DefaultRetryCount;
	int QueueMaxSize;
	bool EnableServerStartNotification;
	int ServerStartDelaySeconds;
	string ServerName;
	string DiscordAvatarUrl;
	ref array<ref PF_WebEndpoint> Endpoints;

	// REST Features
	bool EnableWhitelist;
	bool EnablePlayerLookup;
	bool EnableServerStatus;
	bool EnableKillFeed;
	bool EnableDiscordEvents;
	bool EnableAlertSystem;
	int ServerStatusIntervalSeconds;
	string DiscordWebhookId;
	string DiscordWebhookToken;
	ref array<string> WebhookUrls;
	ref array<ref PF_AlertRuleConfig> AlertRules;

	// Admin
	ref array<string> AdminIDs;

	// Leaderboard Export
	bool EnableLeaderboardExport;
	int LeaderboardExportIntervalSeconds;
	string NinjinPlayersPath;
	int LeaderboardMaxPlayers;

	// Webhook Notifications
	bool EnableServerStopNotification;
	bool EnableHeartbeat;
	int HeartbeatIntervalSeconds;
	bool EnableModUpdateNotification;
	bool EnableQuestNotifications;

	[NonSerialized()]
	protected static ref PF_WebConfig s_Instance;

	[NonSerialized()]
	static const int CURRENT_VERSION = 3;

	void PF_WebConfig()
	{
		ConfigVersion = 0;
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		EnableServerStartNotification = false;
		ServerStartDelaySeconds = 30;
		ServerName = "DayZ Server";
		DiscordAvatarUrl = "";
		AdminIDs = new array<string>();
		Endpoints = new array<ref PF_WebEndpoint>();

		// REST Features
		EnableWhitelist = false;
		EnablePlayerLookup = false;
		EnableServerStatus = false;
		EnableKillFeed = false;
		EnableDiscordEvents = false;
		EnableAlertSystem = false;
		ServerStatusIntervalSeconds = 30;
		DiscordWebhookId = "";
		DiscordWebhookToken = "";
		WebhookUrls = new array<string>();
		AlertRules = new array<ref PF_AlertRuleConfig>();

		// Leaderboard Export
		EnableLeaderboardExport = false;
		LeaderboardExportIntervalSeconds = 600;
		NinjinPlayersPath = "$profile:Ninjins_Tracking_Mod/Data/Players";
		LeaderboardMaxPlayers = 100;

		// Webhook Notifications
		EnableServerStopNotification = false;
		EnableHeartbeat = false;
		HeartbeatIntervalSeconds = 60;
		EnableModUpdateNotification = false;
		EnableQuestNotifications = false;
	}

	static PF_WebConfig GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new PF_WebConfig();
			s_Instance.Load();
		}

		return s_Instance;
	}

	static string GetConfigDirectory()
	{
		return "$profile:DeadmansEcho\\PsyernsFramework";
	}

	static string GetConfigPath()
	{
		return GetConfigDirectory() + "\\PsyernsFrameworkConfig.json";
	}

	void Load()
	{
		string path = GetConfigPath();

		if (FileExist(path))
		{
			JsonFileLoader<PF_WebConfig>.JsonLoadFile(path, this);
			bool changed = EnsureDefaultEndpoints();
			AutoGenerateApiKeys();

			if (ConfigVersion < CURRENT_VERSION)
			{
				ConfigVersion = CURRENT_VERSION;
				changed = true;
				Print("[Psyerns Framework] Config upgraded to version " + CURRENT_VERSION.ToString() + " — new fields added");
			}

			if (changed)
			{
				Save();
			}

			Print("[Psyerns Framework] Config v" + ConfigVersion.ToString() + " loaded from " + path);
			Print("[Psyerns Framework] Debug logging: " + EnableDebugLogging.ToString() + " | Endpoints: " + Endpoints.Count().ToString() + " | RetryCount: " + DefaultRetryCount.ToString() + " | QueueMax: " + QueueMaxSize.ToString());
		}
		else
		{
			CreateDefaults();
			AutoGenerateApiKeys();
			Save();
			Print("[Psyerns Framework] Default config v" + CURRENT_VERSION.ToString() + " created at " + path);
		}
	}

	protected void AutoGenerateApiKeys()
	{
		bool changed = false;

		for (int i = 0; i < Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = Endpoints[i];
			if (!ShouldAutoGenerateApiKey(ep))
				continue;

			if (ep.ApiKey == "" || ep.ApiKey == "YOUR_API_KEY_HERE")
			{
				ep.ApiKey = GenerateRandomKey();
				PF_Logger.Log("Auto-generated API key for endpoint: " + ep.Name + " → " + ep.ApiKey);
				changed = true;
			}
		}

		if (changed)
			Save();
	}

	protected bool ShouldAutoGenerateApiKey(PF_WebEndpoint endpoint)
	{
		if (!endpoint)
			return false;

		string endpointName = endpoint.Name;
		endpointName.ToLower();

		if (endpointName == "topgames")
			return false;

		return true;
	}

	static string GenerateRandomKey()
	{
		string key = "pf-";
		string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
		for (int i = 0; i < 24; i++)
		{
			int idx = Math.RandomInt(0, 36);
			key = key + chars.Substring(idx, 1);
		}
		return key;
	}

	void Save()
	{
		string dir = GetConfigDirectory();
		if (!FileExist(dir))
		{
			Print("[Psyerns Framework] Creating config directory: " + dir);
			MakeDirectory(dir);
		}

		string path = GetConfigPath();
		JsonFileLoader<PF_WebConfig>.JsonSaveFile(path, this);
		Print("[Psyerns Framework] Config saved to " + path);
	}

	void CreateDefaults()
	{
		ConfigVersion = CURRENT_VERSION;
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		EnableServerStartNotification = false;
		ServerStartDelaySeconds = 30;
		ServerName = "DayZ Server";
		DiscordAvatarUrl = "";
		AdminIDs = new array<string>();
		AdminIDs.Insert("YOUR_STEAM64_ID_HERE");
		Endpoints.Clear();

		// REST Feature Defaults
		EnableWhitelist = false;
		EnablePlayerLookup = false;
		EnableServerStatus = false;
		EnableKillFeed = false;
		EnableDiscordEvents = false;
		EnableAlertSystem = false;
		ServerStatusIntervalSeconds = 300;
		DiscordWebhookId = "YOUR_DISCORD_WEBHOOK_ID";
		DiscordWebhookToken = "YOUR_DISCORD_WEBHOOK_TOKEN";

		// Leaderboard Export
		EnableLeaderboardExport = false;
		LeaderboardExportIntervalSeconds = 600;
		NinjinPlayersPath = "$profile:Ninjins_Tracking_Mod/Data/Players";
		LeaderboardMaxPlayers = 100;

		// Endpoints
		PF_WebEndpoint wp = new PF_WebEndpoint();
		wp.Name = "WordPress";
		wp.BaseUrl = "https://your-site.com/wp-json/psyern/v1";
		wp.ApiKey = "YOUR_API_KEY_HERE";
		wp.Enabled = false;
		wp.RateLimitMs = 5000;
		Endpoints.Insert(wp);

		PF_WebEndpoint discord = new PF_WebEndpoint();
		discord.Name = "Discord";
		discord.BaseUrl = "https://discord.com/api/webhooks";
		discord.ApiKey = "YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN";
		discord.Enabled = false;
		discord.RateLimitMs = 1000;
		Endpoints.Insert(discord);

		PF_WebEndpoint leaderboard = new PF_WebEndpoint();
		leaderboard.Name = "Leaderboard";
		leaderboard.BaseUrl = "https://your-site.com/wp-json/psyern/v1";
		leaderboard.ApiKey = "YOUR_API_KEY_HERE";
		leaderboard.Enabled = false;
		leaderboard.RateLimitMs = 5000;
		Endpoints.Insert(leaderboard);

		PF_WebEndpoint topGames = new PF_WebEndpoint();
		topGames.Name = "TopGames";
		topGames.BaseUrl = "https://api.top-games.net";
		topGames.ApiKey = "";
		topGames.Enabled = false;
		topGames.RateLimitMs = 1000;
		Endpoints.Insert(topGames);

		// Webhook Notifications
		EnableServerStopNotification = false;
		EnableHeartbeat = false;
		HeartbeatIntervalSeconds = 60;
		EnableModUpdateNotification = false;
		EnableQuestNotifications = false;

		// KillFeed Webhook URLs
		WebhookUrls = new array<string>();
		WebhookUrls.Insert("https://your-site.com/wp-json/psyern/v1/killfeed");

		// Alert Rules
		AlertRules = new array<ref PF_AlertRuleConfig>();
		PF_AlertRuleConfig exampleRule = new PF_AlertRuleConfig();
		exampleRule.TriggerType = "zone_enter";
		exampleRule.Radius = 200;
		exampleRule.PosX = 7500;
		exampleRule.PosY = 0;
		exampleRule.PosZ = 7500;
		exampleRule.WebhookUrl = "https://your-site.com/wp-json/psyern/v1/alerts";
		exampleRule.MessageTemplate = "Player {playerName} entered zone at {posX},{posZ}";
		AlertRules.Insert(exampleRule);
	}

	protected bool EnsureDefaultEndpoints()
	{
		bool changed = false;

		changed = EnsureEndpoint("WordPress", "https://your-site.com/wp-json/psyern/v1", "YOUR_API_KEY_HERE", false, 5000) || changed;
		changed = EnsureEndpoint("Discord", "https://discord.com/api/webhooks", "YOUR_WEBHOOK_ID/YOUR_WEBHOOK_TOKEN", false, 1000) || changed;
		changed = EnsureEndpoint("Leaderboard", "https://your-site.com/wp-json/psyern/v1", "YOUR_API_KEY_HERE", false, 5000) || changed;
		changed = EnsureEndpoint("TopGames", "https://api.top-games.net", "", false, 1000) || changed;

		return changed;
	}

	protected bool EnsureEndpoint(string name, string baseUrl, string apiKey, bool enabled, int rateLimitMs)
	{
		PF_WebEndpoint endpoint = GetEndpoint(name);
		if (endpoint)
			return false;

		endpoint = new PF_WebEndpoint();
		endpoint.Name = name;
		endpoint.BaseUrl = baseUrl;
		endpoint.ApiKey = apiKey;
		endpoint.Enabled = enabled;
		endpoint.RateLimitMs = rateLimitMs;
		Endpoints.Insert(endpoint);
		return true;
	}

	PF_WebEndpoint GetEndpoint(string name)
	{
		for (int i = 0; i < Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = Endpoints[i];
			string epName = ep.Name;
			epName.ToLower();
			string searchName = name;
			searchName.ToLower();

			if (epName == searchName)
				return ep;
		}

		return null;
	}

	bool IsEndpointEnabled(string name)
	{
		PF_WebEndpoint ep = GetEndpoint(name);
		if (!ep)
			return false;

		return ep.Enabled;
	}

	bool IsAdmin(string plainId)
	{
		if (!AdminIDs)
			return false;

		for (int i = 0; i < AdminIDs.Count(); i++)
		{
			if (AdminIDs[i] == plainId)
				return true;
		}
		return false;
	}

	static void Reload()
	{
		if (s_Instance)
		{
			string path = GetConfigPath();
			JsonFileLoader<PF_WebConfig>.JsonLoadFile(path, s_Instance);
			PF_Logger.Log("Config reloaded from " + path);
		}
	}
}

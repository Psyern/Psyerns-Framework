class PF_WebConfig
{
	bool EnableDebugLogging;
	int DefaultRetryCount;
	int QueueMaxSize;
	bool EnableServerStartNotification;
	int ServerStartDelaySeconds;
	string ServerName;
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

	// Webhook Notifications
	bool EnableServerStopNotification;
	bool EnableHeartbeat;
	int HeartbeatIntervalSeconds;
	bool EnableModUpdateNotification;
	bool EnableQuestNotifications;

	[NonSerialized()]
	protected static ref PF_WebConfig s_Instance;

	void PF_WebConfig()
	{
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		EnableServerStartNotification = false;
		ServerStartDelaySeconds = 30;
		ServerName = "DayZ Server";
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
			AutoGenerateApiKeys();
			Print("[Psyerns Framework] Config loaded from " + path);
			Print("[Psyerns Framework] Debug logging: " + EnableDebugLogging.ToString() + " | Endpoints: " + Endpoints.Count().ToString() + " | RetryCount: " + DefaultRetryCount.ToString() + " | QueueMax: " + QueueMaxSize.ToString());
		}
		else
		{
			CreateDefaults();
			AutoGenerateApiKeys();
			Save();
			Print("[Psyerns Framework] Default config created at " + path);
		}
	}

	protected void AutoGenerateApiKeys()
	{
		bool changed = false;

		for (int i = 0; i < Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = Endpoints[i];
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
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		EnableServerStartNotification = false;
		ServerStartDelaySeconds = 30;
		ServerName = "DayZ Server";
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
}

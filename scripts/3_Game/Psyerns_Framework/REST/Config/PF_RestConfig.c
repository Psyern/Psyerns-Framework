/**
 * PF_RestConfig — Configuration for the REST API subsystem
 * Loaded from: $profile:DeadmansEcho\PsyernsFramework\PF_RestConfig.json
 *
 * Manages: baseUrl, apiKey, webhookUrls and feature toggle flags
 * for all REST managers (Whitelist, PlayerLookup, ServerStatus, KillFeed, Discord, Alerts).
 */
class PF_RestConfig
{
	string BaseUrl;
	string ApiKey;
	ref array<string> WebhookUrls;
	bool EnableWhitelist;
	bool EnablePlayerLookup;
	bool EnableServerStatus;
	bool EnableKillFeed;
	bool EnableDiscordEvents;
	bool EnableAlertSystem;
	int ServerStatusIntervalSeconds;
	string DiscordWebhookId;
	string DiscordWebhookToken;
	ref array<ref PF_AlertRuleConfig> AlertRules;

	[NonSerialized()]
	protected static ref PF_RestConfig s_Instance;

	void PF_RestConfig()
	{
		WebhookUrls = new array<string>();
		AlertRules = new array<ref PF_AlertRuleConfig>();
		EnableWhitelist = false;
		EnablePlayerLookup = false;
		EnableServerStatus = false;
		EnableKillFeed = false;
		EnableDiscordEvents = false;
		EnableAlertSystem = false;
		ServerStatusIntervalSeconds = 30;
		DiscordWebhookId = "";
		DiscordWebhookToken = "";
	}

	static PF_RestConfig GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new PF_RestConfig();
			s_Instance.Load();
		}
		return s_Instance;
	}

	static string GetConfigPath()
	{
		return PF_WebConfig.GetConfigDirectory() + "\\PF_RestConfig.json";
	}

	void Load()
	{
		string path = GetConfigPath();
		if (FileExist(path))
		{
			JsonFileLoader<PF_RestConfig>.JsonLoadFile(path, this);
			Print("[PF-REST] Config loaded from " + path);
		}
		else
		{
			CreateDefaults();
			Save();
			Print("[PF-REST] Default config created at " + path);
		}
	}

	void Save()
	{
		string dir = PF_WebConfig.GetConfigDirectory();
		if (!FileExist(dir))
			MakeDirectory(dir);

		string path = GetConfigPath();
		JsonFileLoader<PF_RestConfig>.JsonSaveFile(path, this);
	}

	void CreateDefaults()
	{
		BaseUrl = "https://your-api.example.com/api";
		ApiKey = "YOUR_REST_API_KEY";
		WebhookUrls = new array<string>();
		WebhookUrls.Insert("https://your-webhook-endpoint.com/hook");
		EnableWhitelist = false;
		EnablePlayerLookup = false;
		EnableServerStatus = false;
		EnableKillFeed = false;
		EnableDiscordEvents = false;
		EnableAlertSystem = false;
		ServerStatusIntervalSeconds = 30;
		DiscordWebhookId = "YOUR_WEBHOOK_ID";
		DiscordWebhookToken = "YOUR_WEBHOOK_TOKEN";
		AlertRules = new array<ref PF_AlertRuleConfig>();

		PF_AlertRuleConfig exampleRule = new PF_AlertRuleConfig();
		exampleRule.TriggerType = "zone_enter";
		exampleRule.Radius = 200;
		exampleRule.PosX = 7500;
		exampleRule.PosY = 0;
		exampleRule.PosZ = 7500;
		exampleRule.WebhookUrl = "https://your-webhook-endpoint.com/alerts";
		exampleRule.MessageTemplate = "Player {playerName} entered zone at {posX},{posZ}";
		AlertRules.Insert(exampleRule);
	}
}

/**
 * PF_AlertRuleConfig — Serializable alert rule definition for JSON config
 * Used by PF_AlertSystem to evaluate triggers
 */
class PF_AlertRuleConfig
{
	string TriggerType;
	float Radius;
	float PosX;
	float PosY;
	float PosZ;
	string WebhookUrl;
	string MessageTemplate;

	void PF_AlertRuleConfig()
	{
		TriggerType = "";
		Radius = 100;
		PosX = 0;
		PosY = 0;
		PosZ = 0;
		WebhookUrl = "";
		MessageTemplate = "";
	}
}

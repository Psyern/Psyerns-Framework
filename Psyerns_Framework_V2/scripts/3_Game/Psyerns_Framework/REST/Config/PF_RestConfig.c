/**
 * PF_RestConfig — Proxy that reads REST settings from the unified PF_WebConfig
 *
 * All config is now stored in PsyernsFrameworkConfig.json (one file).
 * This class provides backward-compatible access for REST modules.
 */
class PF_RestConfig
{
	[NonSerialized()]
	protected static ref PF_RestConfig s_Instance;

	[NonSerialized()]
	protected PF_WebConfig m_Config;

	void PF_RestConfig()
	{
		m_Config = PF_WebConfig.GetInstance();
	}

	static PF_RestConfig GetInstance()
	{
		if (!s_Instance)
			s_Instance = new PF_RestConfig();

		return s_Instance;
	}

	// REST base URL from WordPress endpoint
	string GetBaseUrl()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("WordPress");
		if (ep)
			return ep.BaseUrl;

		return "";
	}

	// REST API key from WordPress endpoint
	string GetApiKey()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("WordPress");
		if (ep)
			return ep.ApiKey;

		return "";
	}

	string GetLeaderboardBaseUrl()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("Leaderboard");
		if (ep)
			return ep.BaseUrl;

		return GetBaseUrl();
	}

	string GetLeaderboardApiKey()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("Leaderboard");
		if (ep)
			return ep.ApiKey;

		return GetApiKey();
	}

	string GetTopGamesBaseUrl()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("TopGames");
		if (ep)
			return ep.BaseUrl;

		return "";
	}

	string GetTopGamesApiKey()
	{
		PF_WebEndpoint ep = m_Config.GetEndpoint("TopGames");
		if (ep)
			return ep.ApiKey;

		return "";
	}

	bool IsTopGamesEnabled()
	{
		return m_Config.IsEndpointEnabled("TopGames");
	}

	// Feature toggles — read directly from PF_WebConfig
	bool IsWhitelistEnabled() { return m_Config.EnableWhitelist; }
	bool IsPlayerLookupEnabled() { return m_Config.EnablePlayerLookup; }
	bool IsServerStatusEnabled() { return m_Config.EnableServerStatus; }
	bool IsKillFeedEnabled() { return m_Config.EnableKillFeed; }
	bool IsDiscordEventsEnabled() { return m_Config.EnableDiscordEvents; }
	bool IsAlertSystemEnabled() { return m_Config.EnableAlertSystem; }
	int GetServerStatusInterval() { return m_Config.ServerStatusIntervalSeconds; }
	bool IsLeaderboardExportEnabled() { return m_Config.EnableLeaderboardExport; }
	int GetLeaderboardExportInterval() { return m_Config.LeaderboardExportIntervalSeconds; }
	string GetNinjinPlayersPath() { return m_Config.NinjinPlayersPath; }
	int GetLeaderboardMaxPlayers() { return m_Config.LeaderboardMaxPlayers; }
	string GetDiscordWebhookId() { return m_Config.DiscordWebhookId; }
	string GetDiscordWebhookToken() { return m_Config.DiscordWebhookToken; }
	array<string> GetWebhookUrls() { return m_Config.WebhookUrls; }
	array<ref PF_AlertRuleConfig> GetAlertRules() { return m_Config.AlertRules; }
}

/**
 * PF_AlertRuleConfig — Serializable alert rule definition for JSON config
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

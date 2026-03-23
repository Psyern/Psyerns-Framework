// --- Singleton accessor ---
ref PF_WhitelistManager g_PF_WhitelistManager;

PF_WhitelistManager GetPF_WhitelistManager()
{
	return g_PF_WhitelistManager;
}

/**
 * PF_WhitelistManager — Player whitelist management via REST API
 *
 * Endpoints:
 *   GET  /whitelist/check?steam_id=...&api_key=...    -> { "whitelisted": true/false }
 *   POST /whitelist/add     { "steamId":"...", "name":"..." }
 *   POST /whitelist/remove  { "steamId":"..." }
 */
class PF_WhitelistManager : PF_RestBase
{
	void PF_WhitelistManager(string baseUrl, string apiKey)
	{
		Print("[PF-REST] WhitelistManager initialized");
	}

	/**
	 * Check if a player is whitelisted (async — result via callback)
	 * @param steamId  Steam64 ID of the player
	 * @param cb       RestCallback to receive the response
	 */
	void CheckWhitelist(string steamId, RestCallback cb)
	{
		if (steamId == "")
		{
			Print("[PF-REST] [ERROR] WhitelistManager::CheckWhitelist — empty steamId");
			return;
		}

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("steam_id", steamId);
		GetWithArgs("/whitelist/check", args, cb);
		Print("[PF-REST] Checking whitelist for: " + steamId);
	}

	/**
	 * Add a player to the whitelist
	 * @param steamId  Steam64 ID
	 * @param name     Player display name
	 */
	void AddToWhitelist(string steamId, string name)
	{
		if (steamId == "")
		{
			Print("[PF-REST] [ERROR] WhitelistManager::AddToWhitelist — empty steamId");
			return;
		}

		string json = PF_JsonBuilder.Begin()
			.Add("steamId", steamId)
			.Add("name", name)
			.Build();

		PostJson("/whitelist/add", json);
		Print("[PF-REST] Added to whitelist: " + name + " (" + steamId + ")");
	}

	/**
	 * Remove a player from the whitelist
	 * @param steamId  Steam64 ID
	 */
	void RemoveFromWhitelist(string steamId)
	{
		if (steamId == "")
		{
			Print("[PF-REST] [ERROR] WhitelistManager::RemoveFromWhitelist — empty steamId");
			return;
		}

		string json = PF_JsonBuilder.Begin()
			.Add("steamId", steamId)
			.Build();

		PostJson("/whitelist/remove", json);
		Print("[PF-REST] Removed from whitelist: " + steamId);
	}
}

/**
 * PF_CB_WhitelistCheck — Callback for whitelist check requests
 * Logs the result and stores the raw response data.
 */
class PF_CB_WhitelistCheck : RestCallback
{
	protected string m_SteamId;
	protected string m_RawData;
	protected bool m_ResponseReceived;

	void PF_CB_WhitelistCheck(string steamId)
	{
		m_SteamId = steamId;
		m_RawData = "";
		m_ResponseReceived = false;
	}

	override void OnSuccess(string data, int dataSize)
	{
		m_RawData = data;
		m_ResponseReceived = true;
		Print("[PF-REST] WhitelistCheck success for " + m_SteamId + " (" + dataSize.ToString() + " bytes)");
	}

	override void OnError(int errorCode)
	{
		m_ResponseReceived = true;
		Print("[PF-REST] [ERROR] WhitelistCheck failed for " + m_SteamId + " — error: " + errorCode.ToString());
	}

	override void OnTimeout()
	{
		m_ResponseReceived = true;
		Print("[PF-REST] [ERROR] WhitelistCheck timed out for " + m_SteamId);
	}

	string GetSteamId() { return m_SteamId; }
	string GetRawData() { return m_RawData; }
	bool HasResponse()  { return m_ResponseReceived; }
}

/**
 * PF_CB_WhitelistResult — Extended callback that parses the whitelist response
 * Extracts "whitelisted": true/false from the JSON response.
 */
class PF_CB_WhitelistResult : PF_CB_WhitelistCheck
{
	protected bool m_IsWhitelisted;

	void PF_CB_WhitelistResult(string steamId)
	{
		m_IsWhitelisted = false;
	}

	override void OnSuccess(string data, int dataSize)
	{
		super.OnSuccess(data, dataSize);

		// Simple string-based parse for the whitelisted flag
		if (data.Contains("\"whitelisted\":true") || data.Contains("\"whitelisted\": true"))
			m_IsWhitelisted = true;
		else
			m_IsWhitelisted = false;

		Print("[PF-REST] WhitelistResult for " + m_SteamId + ": whitelisted=" + m_IsWhitelisted.ToString());
	}

	override void OnError(int errorCode)
	{
		super.OnError(errorCode);
		m_IsWhitelisted = false;
	}

	bool IsWhitelisted() { return m_IsWhitelisted; }
}

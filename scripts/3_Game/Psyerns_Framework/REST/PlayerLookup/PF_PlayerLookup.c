// --- Singleton accessor ---
ref PF_PlayerLookup g_PF_PlayerLookup;

PF_PlayerLookup GetPF_PlayerLookup()
{
	return g_PF_PlayerLookup;
}

/**
 * PF_PlayerData — Deserialized player information from the REST API
 * Field names must match the JSON response from the server.
 */
class PF_PlayerData
{
	string steamId;
	string name;
	bool online;
	float posX;
	float posY;
	float posZ;
	float health;

	void PF_PlayerData()
	{
		steamId = "";
		name = "";
		online = false;
		posX = 0;
		posY = 0;
		posZ = 0;
		health = 0;
	}
}

/**
 * PF_PlayerLookup — Player data lookup via REST API
 *
 * Endpoints:
 *   GET /players/lookup?steam_id=...&api_key=...  -> PF_PlayerData JSON
 *   GET /players/online?api_key=...               -> array of PF_PlayerData
 */
class PF_PlayerLookup : PF_RestBase
{
	void PF_PlayerLookup(string baseUrl, string apiKey)
	{
		Print("[PF-REST] PlayerLookup initialized");
	}

	/**
	 * Fetch data for a specific player
	 * @param steamId  Steam64 ID
	 * @param cb       RestCallback to receive the response
	 */
	void GetPlayerData(string steamId, RestCallback cb)
	{
		if (steamId == "")
		{
			Print("[PF-REST] [ERROR] PlayerLookup::GetPlayerData — empty steamId");
			return;
		}

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("steam_id", steamId);
		GetWithArgs("/players/lookup", args, cb);
		Print("[PF-REST] Looking up player: " + steamId);
	}

	/**
	 * Fetch list of all online players
	 * @param cb  RestCallback to receive the response
	 */
	void GetOnlinePlayers(RestCallback cb)
	{
		GetWithCallback("/players/online", cb);
		Print("[PF-REST] Fetching online players");
	}
}

/**
 * PF_CB_PlayerData — Callback for player lookup responses
 * Deserializes JSON into PF_PlayerData via JsonSerializer.
 */
class PF_CB_PlayerData : RestCallback
{
	protected ref PF_PlayerData m_PlayerData;
	protected string m_RawData;
	protected bool m_Success;

	void PF_CB_PlayerData()
	{
		m_Success = false;
		m_RawData = "";
	}

	override void OnSuccess(string data, int dataSize)
	{
		m_RawData = data;
		m_Success = true;
		Print("[PF-REST] PlayerData received (" + dataSize.ToString() + " bytes)");

		m_PlayerData = new PF_PlayerData();
		string error;
		bool parsed = JsonSerializer().ReadFromString(m_PlayerData, false, data, error);
		if (!parsed)
		{
			Print("[PF-REST] [ERROR] Failed to parse PlayerData: " + error);
			m_PlayerData = null;
		}
	}

	override void OnError(int errorCode)
	{
		m_Success = false;
		Print("[PF-REST] [ERROR] PlayerData lookup failed — error: " + errorCode.ToString());
	}

	override void OnTimeout()
	{
		m_Success = false;
		Print("[PF-REST] [ERROR] PlayerData lookup timed out");
	}

	PF_PlayerData GetPlayerData() { return m_PlayerData; }
	string GetRawData()           { return m_RawData; }
	bool IsSuccess()              { return m_Success; }
}

/**
 * PF_CB_OnlinePlayers — Callback for online players list
 * Stores the raw JSON response for consumer parsing.
 */
class PF_CB_OnlinePlayers : RestCallback
{
	protected string m_RawData;
	protected bool m_Success;

	void PF_CB_OnlinePlayers()
	{
		m_Success = false;
		m_RawData = "";
	}

	override void OnSuccess(string data, int dataSize)
	{
		m_RawData = data;
		m_Success = true;
		Print("[PF-REST] OnlinePlayers received (" + dataSize.ToString() + " bytes)");
	}

	override void OnError(int errorCode)
	{
		m_Success = false;
		Print("[PF-REST] [ERROR] OnlinePlayers fetch failed — error: " + errorCode.ToString());
	}

	override void OnTimeout()
	{
		m_Success = false;
		Print("[PF-REST] [ERROR] OnlinePlayers fetch timed out");
	}

	string GetRawData() { return m_RawData; }
	bool IsSuccess()    { return m_Success; }
}

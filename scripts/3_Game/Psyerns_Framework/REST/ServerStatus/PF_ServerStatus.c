// --- Singleton accessor ---
ref PF_ServerStatus g_PF_ServerStatus;

PF_ServerStatus GetPF_ServerStatus()
{
	return g_PF_ServerStatus;
}

/**
 * PF_ServerStatus — Periodically pushes server status data via POST
 *
 * Collected data:
 *   - playerCount    (online player count)
 *   - uptimeSeconds  (seconds since framework init)
 *   - mapName        (world name)
 *   - dayTime        (in-game hour:minute)
 *   - serverName     (from PF_WebConfig)
 *
 * Endpoint: POST /server/status?api_key=...
 * Timer: configurable interval (default 30 seconds)
 */
class PF_ServerStatus : PF_RestBase
{
	protected float m_Timer;
	protected float m_Interval;
	protected float m_StartTime;

	void PF_ServerStatus(string baseUrl, string apiKey, int intervalSeconds)
	{
		m_Timer = 0;
		m_Interval = intervalSeconds;
		m_StartTime = GetGame().GetTickTime();
		Print("[PF-REST] ServerStatus initialized (interval: " + intervalSeconds.ToString() + "s)");
	}

	/**
	 * Called every frame from MissionServer.OnUpdate()
	 * Accumulates timeslice and pushes status when interval is reached.
	 */
	void OnUpdate(float timeslice)
	{
		m_Timer += timeslice;
		if (m_Timer >= m_Interval)
		{
			m_Timer = 0;
			PushStatus();
		}
	}

	/**
	 * Collects current server data and sends it via POST
	 */
	void PushStatus()
	{
		// Player count
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		int playerCount = players.Count();

		// Uptime
		float uptimeSeconds = GetGame().GetTickTime() - m_StartTime;
		int uptimeInt = (int)uptimeSeconds;

		// Map name
		string mapName = "unknown";
		if (GetGame().GetWorldName(mapName))
		{
			// mapName is filled by out param
		}

		// In-game time of day
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		string dayTime = hour.ToStringLen(2) + ":" + minute.ToStringLen(2);

		// Server name from config
		string serverName = PF_WebConfig.GetInstance().ServerName;

		// Build JSON payload
		string json = PF_JsonBuilder.Begin()
			.Add("serverName", serverName)
			.AddInt("playerCount", playerCount)
			.AddInt("uptimeSeconds", uptimeInt)
			.Add("mapName", mapName)
			.Add("dayTime", dayTime)
			.Add("timestamp", GetTimestamp())
			.Build();

		PostJson("/server/status", json);
		Print("[PF-REST] ServerStatus pushed: " + playerCount.ToString() + " players, uptime " + uptimeInt.ToString() + "s");
	}

	// Returns ISO-style timestamp string
	protected string GetTimestamp()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2)
			+ "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
	}
}

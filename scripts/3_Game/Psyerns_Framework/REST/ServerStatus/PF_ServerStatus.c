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
		if (g_Game)
			m_StartTime = g_Game.GetTickTime();
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
		if (!g_Game)
			return;

		// Player count
		array<Man> players = new array<Man>();
		g_Game.GetPlayers(players);
		int playerCount = players.Count();

		// Uptime
		float uptimeSeconds = g_Game.GetTickTime() - m_StartTime;
		int uptimeInt = uptimeSeconds;

		// Map name
		string mapName = "unknown";
		g_Game.GetWorldName(mapName);

		// In-game time of day
		int year;
		int month;
		int day;
		int hour;
		int minute;
		if (g_Game.GetWorld())
			g_Game.GetWorld().GetDate(year, month, day, hour, minute);
		string dayTime = hour.ToStringLen(2) + ":" + minute.ToStringLen(2);

		// Server name from config
		string serverName = PF_WebConfig.GetInstance().ServerName;

		// Build JSON payload
		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("serverName", serverName);
		b.AddInt("playerCount", playerCount);
		b.AddInt("uptimeSeconds", uptimeInt);
		b.Add("mapName", mapName);
		b.Add("dayTime", dayTime);
		b.Add("timestamp", GetTimestamp());
		string json = b.Build();

		PostJson("/server/status", json);
		Print("[PF-REST] ServerStatus pushed: " + playerCount.ToString() + " players, uptime " + uptimeInt.ToString() + "s");
	}

	// Returns ISO-style timestamp string
	protected string GetTimestamp()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
	}
}

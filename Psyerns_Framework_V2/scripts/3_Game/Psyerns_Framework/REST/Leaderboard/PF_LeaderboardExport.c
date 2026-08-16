// --- Singleton accessor ---
ref PF_LeaderboardExport g_PF_LeaderboardExport;

PF_LeaderboardExport GetPF_LeaderboardExport()
{
	return g_PF_LeaderboardExport;
}

/**
 * PF_LeaderboardExport — Periodically reads Ninjin player data and uploads to WordPress
 *
 * Follows the same OnUpdate/timer pattern as PF_ServerStatus.
 * Reads from: $profile:Ninjins_Tracking_Mod/Data/Players/*.json
 * Posts to:   /upload?api_key=...
 */
class PF_LeaderboardExport : PF_RestBase
{
	protected float m_Timer;
	protected float m_Interval;
	protected string m_PlayersPath;
	protected int m_MaxPlayers;

	static const int MAX_LEADERBOARD_PLAYERS = 100;

	void PF_LeaderboardExport(string baseUrl, string apiKey, int intervalSeconds, string playersPath, int maxPlayers)
	{
		m_Timer = 0;
		m_Interval = intervalSeconds;
		m_PlayersPath = playersPath;
		m_MaxPlayers = maxPlayers;

		if (m_MaxPlayers <= 0 || m_MaxPlayers > MAX_LEADERBOARD_PLAYERS)
			m_MaxPlayers = MAX_LEADERBOARD_PLAYERS;

		PF_Logger.Log("LeaderboardExport initialized (interval: " + intervalSeconds.ToString() + "s, path: " + playersPath + ", max: " + m_MaxPlayers.ToString() + ")");
	}

	void OnUpdate(float timeslice)
	{
		m_Timer += timeslice;
		if (m_Timer >= m_Interval)
		{
			m_Timer = 0;
			PushLeaderboard();
		}
	}

	void PushLeaderboard()
	{
		// 1. Read all players from Ninjin JSON files
		array<ref PF_WP_PlayerData> allPlayers;
		int onlineCount;
		PF_LeaderboardReader.ReadAllPlayers(m_PlayersPath, allPlayers, onlineCount);

		if (!allPlayers || allPlayers.Count() == 0)
		{
			PF_Logger.Log("LeaderboardExport: No player data found, skipping upload.");
			return;
		}

		int totalPlayers = allPlayers.Count();

		// 2. Sort by PvE points (descending) for topPVEPlayers
		array<ref PF_WP_PlayerData> pveSorted = new array<ref PF_WP_PlayerData>();
		for (int ci = 0; ci < allPlayers.Count(); ci++)
		{
			pveSorted.Insert(allPlayers[ci]);
		}
		SortByPvE(pveSorted);

		// 3. Sort by PvP points (descending) for topPVPPlayers
		array<ref PF_WP_PlayerData> pvpSorted = new array<ref PF_WP_PlayerData>();
		for (int cj = 0; cj < allPlayers.Count(); cj++)
		{
			pvpSorted.Insert(allPlayers[cj]);
		}
		SortByPvP(pvpSorted);

		// 4. Calculate faction points
		int globalEastPoints = 0;
		int globalWestPoints = 0;
		for (int fi = 0; fi < allPlayers.Count(); fi++)
		{
			PF_WP_PlayerData fp = allPlayers[fi];
			if (fp.warFaction == "EAST")
				globalEastPoints += fp.pvePoints + fp.pvpPoints;
			else if (fp.warFaction == "WEST")
				globalWestPoints += fp.pvePoints + fp.pvpPoints;
		}

		// 5. Build payload
		PF_WordPressPayload payload = new PF_WordPressPayload();
		payload.generatedAt = GetTimestamp();
		payload.playerOnlineCounter = onlineCount;
		payload.totalPlayers = totalPlayers;
		payload.globalEastPoints = globalEastPoints;
		payload.globalWestPoints = globalWestPoints;

		// Cap to max players
		int pveLimit = pveSorted.Count();
		if (pveLimit > m_MaxPlayers)
			pveLimit = m_MaxPlayers;

		for (int pi = 0; pi < pveLimit; pi++)
		{
			payload.topPVEPlayers.Insert(pveSorted[pi]);
		}

		int pvpLimit = pvpSorted.Count();
		if (pvpLimit > m_MaxPlayers)
			pvpLimit = m_MaxPlayers;

		for (int pj = 0; pj < pvpLimit; pj++)
		{
			payload.topPVPPlayers.Insert(pvpSorted[pj]);
		}

		// 5b. Populate playerDetails (ALL players, no cap) — gated by config flag.
		int detailsCount = 0;
		if (PF_WebConfig.GetInstance().EnablePlayerDetailsExport)
		{
			for (int di = 0; di < allPlayers.Count(); di++)
			{
				payload.playerDetails.Insert(allPlayers[di]);
			}
			detailsCount = payload.playerDetails.Count();
		}

		// 6. Upload
		payload.apiKey = GetApiKey();
		string json = payload.Serialize();
		PostJson("/upload", json);

		PF_Logger.Log("LeaderboardExport: Uploaded " + totalPlayers.ToString() + " players (" + onlineCount.ToString() + " online), PvE top: " + pveLimit.ToString() + ", PvP top: " + pvpLimit.ToString() + ", playerDetails: " + detailsCount.ToString());

		if (pveSorted.Count() > 0)
		{
			PF_WP_PlayerData topPve = pveSorted[0];
			PF_Logger.Debug("LeaderboardExport: Top PvE player: " + topPve.playerName + " (" + topPve.pvePoints.ToString() + " pts)");
		}
	}

	// Stable Timsort via PF_Sort (see Utils/PF_Sort.c). Keys are extracted
	// once per player instead of being read on every comparison; only an
	// int permutation array moves during the sort. Full descending order,
	// ties keep their input order; the caller still caps at m_MaxPlayers.
	protected void SortByPoints(array<ref PF_WP_PlayerData> players, bool usePvE)
	{
		array<int> keys;
		array<int> order;
		PF_WP_PlayerData entry;
		int points;
		int i;
		int count;

		if (!players)
			return;

		count = players.Count();
		if (count < 2)
			return;

		keys = new array<int>();
		order = new array<int>();

		for (i = 0; i < count; i++)
		{
			entry = players.Get(i);
			points = 0;
			if (entry)
			{
				if (usePvE)
					points = entry.pvePoints;
				else
					points = entry.pvpPoints;
			}
			keys.Insert(points);
		}

		// false == descending
		PF_Sort.BuildOrderByInt(keys, false, order);
		PF_Sort.ApplyOrderPlayers(players, order);
	}

	protected void SortByPvE(array<ref PF_WP_PlayerData> players)
	{
		SortByPoints(players, true);
	}

	protected void SortByPvP(array<ref PF_WP_PlayerData> players)
	{
		SortByPoints(players, false);
	}

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

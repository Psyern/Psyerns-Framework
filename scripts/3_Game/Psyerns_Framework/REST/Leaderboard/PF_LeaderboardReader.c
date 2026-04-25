// --- Ninjin Tracking Mod deserialization ---
// The Ninjin mod writes CategoryKills / CategoryDeaths / CategoryLongestRanges
// as dynamic string-keyed maps, not fixed-field objects. We must mirror that,
// otherwise all category data deserializes to 0.

class PF_NinjinPlayerData
{
	string PlayerID;
	string PlayerName;
	int PvEPoints;
	int PvPPoints;
	int playerIsOnline;
	string WarFaction;
	int WarAlignment;
	int WarLevel;
	int HardlineReputation;
	string LastLoginDate;
	int WarBossKills;
	ref map<string, int> CategoryKills;
	ref map<string, int> CategoryDeaths;
	ref map<string, int> CategoryLongestRanges;

	// Extended stats (Ninjin_LeaderBoard mirror)
	int ShotsFired;
	int ShotsHit;
	int Headshots;
	float DistanceTravelled;
	float DistanceOnFoot;
	float DistanceInVehicle;

	void PF_NinjinPlayerData()
	{
		PlayerID = "";
		PlayerName = "";
		PvEPoints = 0;
		PvPPoints = 0;
		playerIsOnline = 0;
		WarFaction = "NEUTRAL";
		WarAlignment = 0;
		WarLevel = 0;
		HardlineReputation = 0;
		LastLoginDate = "";
		WarBossKills = 0;
		CategoryKills = new map<string, int>();
		CategoryDeaths = new map<string, int>();
		CategoryLongestRanges = new map<string, int>();
		ShotsFired = 0;
		ShotsHit = 0;
		Headshots = 0;
		DistanceTravelled = 0.0;
		DistanceOnFoot = 0.0;
		DistanceInVehicle = 0.0;
	}
}

// --- Leaderboard Reader ---

class PF_LeaderboardReader
{
	static void ReadAllPlayers(string playersPath, out array<ref PF_WP_PlayerData> outPlayers, out int outOnlineCount)
	{
		outPlayers = new array<ref PF_WP_PlayerData>();
		outOnlineCount = 0;

		string fileName;
		FileAttr fileAttr;
		string searchPattern = playersPath + "/*.json";

		FindFileHandle handle = FindFile(searchPattern, fileName, fileAttr, FindFileFlags.ALL);
		if (!handle)
		{
			PF_Logger.Error("LeaderboardReader: Could not open directory: " + playersPath);
			return;
		}

		int totalRead = 0;
		int totalSkipped = 0;

		bool hasFile = true;
		while (hasFile)
		{
			if (fileName != "" && fileName.IndexOf(".json") > 0)
			{
				string fullPath = playersPath + "/" + fileName;
				PF_WP_PlayerData converted = ReadSinglePlayer(fullPath, fileName);

				if (converted)
				{
					outPlayers.Insert(converted);
					totalRead++;

					if (converted.isOnline == 1)
						outOnlineCount++;
				}
				else
				{
					totalSkipped++;
				}
			}

				hasFile = FindNextFile(handle, fileName, fileAttr);
		}

		CloseFindFile(handle);
		PF_Logger.Debug("LeaderboardReader: Read " + totalRead.ToString() + " players, skipped " + totalSkipped.ToString());
	}

	protected static PF_WP_PlayerData ReadSinglePlayer(string fullPath, string fileName)
	{
		PF_NinjinPlayerData raw = new PF_NinjinPlayerData();

		JsonFileLoader<PF_NinjinPlayerData>.JsonLoadFile(fullPath, raw);

		if (!raw.PlayerName || raw.PlayerName == "")
		{
			PF_Logger.Debug("LeaderboardReader: Skipping empty/corrupt file: " + fileName);
			return null;
		}

		// Extract SteamID from filename (remove .json)
		string steamId = fileName;
		int dotIdx = steamId.IndexOf(".");
		if (dotIdx > 0)
			steamId = steamId.Substring(0, dotIdx);

		return ConvertPlayer(steamId, raw);
	}

	protected static PF_WP_PlayerData ConvertPlayer(string steamId, PF_NinjinPlayerData raw)
	{
		PF_WP_PlayerData p = new PF_WP_PlayerData();

		p.playerID = steamId;
		p.playerName = raw.PlayerName;
		p.odolozId = steamId;
		p.pvePoints = raw.PvEPoints;
		p.pvpPoints = raw.PvPPoints;
		p.isOnline = raw.playerIsOnline;
		p.lastLoginDate = raw.LastLoginDate;
		p.warFaction = raw.WarFaction;
		p.warAlignment = raw.WarAlignment;
		p.warLevel = raw.WarLevel;
		p.warBossKills = raw.WarBossKills;
		p.hardlineReputation = raw.HardlineReputation;

		// Total kills = sum of all CategoryKills entries
		int totalKills = SumMap(raw.CategoryKills);
		int aiKills = 0;
		if (raw.CategoryKills && raw.CategoryKills.Contains("AI"))
			aiKills = raw.CategoryKills.Get("AI");

		int totalDeaths = SumMap(raw.CategoryDeaths);
		int suicides = 0;
		if (raw.CategoryDeaths && raw.CategoryDeaths.Contains("SelfInflicted"))
			suicides = raw.CategoryDeaths.Get("SelfInflicted");

		p.kills = totalKills;
		p.deaths = totalDeaths;
		p.aiKills = aiKills;
		p.totalDeaths = totalDeaths;
		p.suicides = suicides;

		// Longest shot = max of all CategoryLongestRanges entries
		p.longestShot = MaxMap(raw.CategoryLongestRanges);
		p.playtime = 0;

		// Extended stats
		p.shotsFired = raw.ShotsFired;
		p.shotsHit = raw.ShotsHit;
		p.headshots = raw.Headshots;
		p.distanceTravelled = raw.DistanceTravelled;
		p.distanceOnFoot = raw.DistanceOnFoot;
		p.distanceInVehicle = raw.DistanceInVehicle;

		// Serialize maps to JSON strings for WordPress payload (preserves all keys)
		p.categoryKillsJson = SerializeIntMap(raw.CategoryKills);
		p.categoryDeathsJson = SerializeIntMap(raw.CategoryDeaths);
		p.categoryLongestRangesJson = SerializeIntMap(raw.CategoryLongestRanges);

		return p;
	}

	protected static int SumMap(map<string, int> m)
	{
		int total;
		int idx;

		if (!m)
			return 0;

		total = 0;
		for (idx = 0; idx < m.Count(); idx++)
		{
			total = total + m.GetElement(idx);
		}
		return total;
	}

	protected static int MaxMap(map<string, int> m)
	{
		int max;
		int value;
		int idx;

		if (!m)
			return 0;

		max = 0;
		for (idx = 0; idx < m.Count(); idx++)
		{
			value = m.GetElement(idx);
			if (value > max)
				max = value;
		}
		return max;
	}

	protected static string SerializeIntMap(map<string, int> m)
	{
		int idx;
		PF_JsonBuilder b;

		if (!m || m.Count() == 0)
			return "{}";

		b = PF_JsonBuilder.Begin();
		for (idx = 0; idx < m.Count(); idx++)
		{
			b.AddInt(m.GetKey(idx), m.GetElement(idx));
		}
		return b.Build();
	}
}

// --- Ninjin Tracking Mod deserialization classes ---

class PF_NinjinCategoryKills
{
	int PlayersBased;
	int ZombiesBased;
	int AIBased;
	int AnimalsBased;

	void PF_NinjinCategoryKills()
	{
		PlayersBased = 0;
		ZombiesBased = 0;
		AIBased = 0;
		AnimalsBased = 0;
	}
}

class PF_NinjinCategoryDeaths
{
	int SelfInflicted;
	int PlayersBased;
	int ZombiesBased;
	int AnimalsBased;

	void PF_NinjinCategoryDeaths()
	{
		SelfInflicted = 0;
		PlayersBased = 0;
		ZombiesBased = 0;
		AnimalsBased = 0;
	}
}

class PF_NinjinCategoryRanges
{
	int PlayersBased;
	int ZombiesBased;
	int AIBased;
	int AnimalsBased;

	void PF_NinjinCategoryRanges()
	{
		PlayersBased = 0;
		ZombiesBased = 0;
		AIBased = 0;
		AnimalsBased = 0;
	}
}

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
	ref PF_NinjinCategoryKills CategoryKills;
	ref PF_NinjinCategoryDeaths CategoryDeaths;
	ref PF_NinjinCategoryRanges CategoryLongestRanges;

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
		CategoryKills = new PF_NinjinCategoryKills();
		CategoryDeaths = new PF_NinjinCategoryDeaths();
		CategoryLongestRanges = new PF_NinjinCategoryRanges();
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

		// Total kills = sum of all CategoryKills
		int totalKills = 0;
		int totalDeaths = 0;

		if (raw.CategoryKills)
		{
			totalKills = raw.CategoryKills.PlayersBased + raw.CategoryKills.ZombiesBased + raw.CategoryKills.AIBased + raw.CategoryKills.AnimalsBased;
			p.aiKills = raw.CategoryKills.AIBased;
		}

		if (raw.CategoryDeaths)
		{
			totalDeaths = raw.CategoryDeaths.SelfInflicted + raw.CategoryDeaths.PlayersBased + raw.CategoryDeaths.ZombiesBased + raw.CategoryDeaths.AnimalsBased;
		}

		p.kills = totalKills;
		p.deaths = totalDeaths;

		// Longest shot = max of all CategoryLongestRanges
		float maxRange = 0;
		if (raw.CategoryLongestRanges)
		{
			if (raw.CategoryLongestRanges.PlayersBased > maxRange)
				maxRange = raw.CategoryLongestRanges.PlayersBased;
			if (raw.CategoryLongestRanges.ZombiesBased > maxRange)
				maxRange = raw.CategoryLongestRanges.ZombiesBased;
			if (raw.CategoryLongestRanges.AIBased > maxRange)
				maxRange = raw.CategoryLongestRanges.AIBased;
			if (raw.CategoryLongestRanges.AnimalsBased > maxRange)
				maxRange = raw.CategoryLongestRanges.AnimalsBased;
		}
		p.longestShot = maxRange;
		p.playtime = 0;

		// Serialize category objects to JSON strings for WordPress payload
		p.categoryKillsJson = SerializeCategoryKills(raw.CategoryKills);
		p.categoryDeathsJson = SerializeCategoryDeaths(raw.CategoryDeaths);
		p.categoryLongestRangesJson = SerializeCategoryRanges(raw.CategoryLongestRanges);

		return p;
	}

	protected static string SerializeCategoryKills(PF_NinjinCategoryKills cat)
	{
		if (!cat)
			return "{}";

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.AddInt("PlayersBased", cat.PlayersBased);
		b.AddInt("ZombiesBased", cat.ZombiesBased);
		b.AddInt("AIBased", cat.AIBased);
		b.AddInt("AnimalsBased", cat.AnimalsBased);
		return b.Build();
	}

	protected static string SerializeCategoryDeaths(PF_NinjinCategoryDeaths cat)
	{
		if (!cat)
			return "{}";

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.AddInt("SelfInflicted", cat.SelfInflicted);
		b.AddInt("PlayersBased", cat.PlayersBased);
		b.AddInt("ZombiesBased", cat.ZombiesBased);
		b.AddInt("AnimalsBased", cat.AnimalsBased);
		return b.Build();
	}

	protected static string SerializeCategoryRanges(PF_NinjinCategoryRanges cat)
	{
		if (!cat)
			return "{}";

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.AddInt("PlayersBased", cat.PlayersBased);
		b.AddInt("ZombiesBased", cat.ZombiesBased);
		b.AddInt("AIBased", cat.AIBased);
		b.AddInt("AnimalsBased", cat.AnimalsBased);
		return b.Build();
	}
}

class PF_WP_PlayerData
{
	string playerID;
	string playerName;
	string odolozId;
	int pvePoints;
	int pvpPoints;
	int kills;
	int deaths;
	int aiKills;
	float longestShot;
	float playtime;
	int isOnline;
	string lastLoginDate;
	string warFaction;
	int warAlignment;
	int warLevel;
	int warBossKills;
	int hardlineReputation;
	string categoryKillsJson;
	string categoryDeathsJson;
	string categoryLongestRangesJson;

	// Ninjin extended stats
	int shotsFired;
	int shotsHit;
	int headshots;
	float distanceTravelled;
	float distanceOnFoot;
	float distanceInVehicle;
	int totalDeaths;
	int suicides;

	void PF_WP_PlayerData()
	{
		playerID = "";
		playerName = "";
		odolozId = "";
		pvePoints = 0;
		pvpPoints = 0;
		kills = 0;
		deaths = 0;
		aiKills = 0;
		longestShot = 0;
		playtime = 0;
		isOnline = 0;
		lastLoginDate = "";
		warFaction = "NEUTRAL";
		warAlignment = 0;
		warLevel = 0;
		warBossKills = 0;
		hardlineReputation = 0;
		categoryKillsJson = "{}";
		categoryDeathsJson = "{}";
		categoryLongestRangesJson = "{}";
		shotsFired = 0;
		shotsHit = 0;
		headshots = 0;
		distanceTravelled = 0.0;
		distanceOnFoot = 0.0;
		distanceInVehicle = 0.0;
		totalDeaths = 0;
		suicides = 0;
	}
}

class PF_WordPressPayload : PF_JsonPayload
{
	string apiKey;
	string generatedAt;
	int playerOnlineCounter;
	int totalPlayers;
	int globalEastPoints;
	int globalWestPoints;
	ref array<ref PF_WP_PlayerData> topPVEPlayers;
	ref array<ref PF_WP_PlayerData> topPVPPlayers;

	void PF_WordPressPayload()
	{
		playerOnlineCounter = 0;
		totalPlayers = 0;
		globalEastPoints = 0;
		globalWestPoints = 0;
		topPVEPlayers = new array<ref PF_WP_PlayerData>();
		topPVPPlayers = new array<ref PF_WP_PlayerData>();
	}

	override string Serialize()
	{
		PF_JsonBuilder builder = PF_JsonBuilder.Begin();
		builder.Add("apiKey", apiKey);
		builder.Add("generatedAt", generatedAt);
		builder.AddInt("playerOnlineCounter", playerOnlineCounter);
		builder.AddInt("totalPlayers", totalPlayers);
		builder.AddInt("globalEastPoints", globalEastPoints);
		builder.AddInt("globalWestPoints", globalWestPoints);

		string pvePlayers = SerializePlayerArray(topPVEPlayers);
		builder.AddRaw("topPVEPlayers", pvePlayers);

		string pvpPlayers = SerializePlayerArray(topPVPPlayers);
		builder.AddRaw("topPVPPlayers", pvpPlayers);

		return builder.Build();
	}

	protected string SerializePlayerArray(array<ref PF_WP_PlayerData> players)
	{
		string result = "[";
		for (int i = 0; i < players.Count(); i++)
		{
			if (i > 0)
				result += ",";

			PF_WP_PlayerData p = players[i];
			PF_JsonBuilder pb = PF_JsonBuilder.Begin();
			pb.Add("playerID", p.playerID);
			pb.Add("playerName", p.playerName);
			pb.Add("odolozId", p.odolozId);
			pb.AddInt("pvePoints", p.pvePoints);
			pb.AddInt("pvpPoints", p.pvpPoints);
			pb.AddInt("kills", p.kills);
			pb.AddInt("deaths", p.deaths);
			pb.AddInt("aiKills", p.aiKills);
			pb.AddFloat("longestShot", p.longestShot);
			pb.AddFloat("playtime", p.playtime);
			pb.AddInt("isOnline", p.isOnline);
			pb.Add("lastLoginDate", p.lastLoginDate);
			pb.Add("warFaction", p.warFaction);
			pb.AddInt("warAlignment", p.warAlignment);
			pb.AddInt("warLevel", p.warLevel);
			pb.AddInt("warBossKills", p.warBossKills);
			pb.AddInt("hardlineReputation", p.hardlineReputation);
			pb.AddInt("shotsFired", p.shotsFired);
			pb.AddInt("shotsHit", p.shotsHit);
			pb.AddInt("headshots", p.headshots);
			pb.AddFloat("distanceTravelled", p.distanceTravelled);
			pb.AddFloat("distanceOnFoot", p.distanceOnFoot);
			pb.AddFloat("distanceInVehicle", p.distanceInVehicle);
			pb.AddInt("totalDeaths", p.totalDeaths);
			pb.AddInt("suicides", p.suicides);
			pb.AddRaw("categoryKills", p.categoryKillsJson);
			pb.AddRaw("categoryDeaths", p.categoryDeathsJson);
			pb.AddRaw("categoryLongestRanges", p.categoryLongestRangesJson);
			result += pb.Build();
		}
		result += "]";
		return result;
	}
}

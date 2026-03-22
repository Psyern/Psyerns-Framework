class PF_WP_PlayerData
{
	string playerName;
	string odolozId;
	int kills;
	int deaths;
	int aiKills;
	float longestShot;
	float playtime;

	void PF_WP_PlayerData()
	{
		kills = 0;
		deaths = 0;
		aiKills = 0;
		longestShot = 0;
		playtime = 0;
	}
}

class PF_WordPressPayload : PF_JsonPayload
{
	string apiKey;
	string generatedAt;
	int playerOnlineCounter;
	int totalPlayers;
	ref array<ref PF_WP_PlayerData> topPVEPlayers;
	ref array<ref PF_WP_PlayerData> topPVPPlayers;

	void PF_WordPressPayload()
	{
		playerOnlineCounter = 0;
		totalPlayers = 0;
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
			pb.Add("playerName", p.playerName);
			pb.Add("odolozId", p.odolozId);
			pb.AddInt("kills", p.kills);
			pb.AddInt("deaths", p.deaths);
			pb.AddInt("aiKills", p.aiKills);
			pb.AddFloat("longestShot", p.longestShot);
			pb.AddFloat("playtime", p.playtime);
			result += pb.Build();
		}
		result += "]";
		return result;
	}
}

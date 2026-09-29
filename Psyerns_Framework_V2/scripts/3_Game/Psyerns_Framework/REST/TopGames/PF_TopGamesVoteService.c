ref PF_TopGamesVoteService g_PF_TopGamesVoteService;

PF_TopGamesVoteService GetPF_TopGamesVoteService()
{
	return g_PF_TopGamesVoteService;
}

class PF_TopGamesVoteService : PF_RestBase
{
	void PF_TopGamesVoteService(string baseUrl, string apiKey)
	{
		g_PF_TopGamesVoteService = this;
		RefreshConfig(baseUrl, apiKey);
		PsyCore_Log.Get("PF-REST").Info("TopGamesVoteService initialized");
	}

	void RefreshConfig(string baseUrl, string serverToken)
	{
		m_BaseUrl = baseUrl;
		m_ApiKey = serverToken;

		if (m_BaseUrl == "")
		{
			m_RestContext = null;
			return;
		}

		m_RestContext = m_Rest.GetRestContext(m_BaseUrl);
		if (m_RestContext)
		{
			m_RestContext.SetHeader("application/json");
		}
	}

	bool IsConfigured()
	{
		if (m_BaseUrl == "")
			return false;

		if (m_ApiKey == "")
			return false;

		return m_RestContext != null;
	}

	void GetPlayersRanking(RestCallback cb, string endpoint = "")
	{
		if (!IsConfigured())
		{
			PsyCore_Log.Get("PF-REST").Error("TopGamesVoteService::GetPlayersRanking - service not configured");
			return;
		}

		string rankingEndpoint = endpoint;
		if (rankingEndpoint == "")
		{
			rankingEndpoint = "/v1/servers/" + m_ApiKey + "/players-ranking";
		}

		rankingEndpoint = NormalizeEndpoint(rankingEndpoint);
		PsyCore_Log.Get("PF-REST").Info("GET " + PF_Logger.MaskSecrets(m_BaseUrl + rankingEndpoint));
		PF_RestCallback.PF_Retain(cb);
		m_RestContext.GET(cb, rankingEndpoint);
	}

	void ClaimSteamVote(string steam64Id, RestCallback cb)
	{
		if (!IsConfigured())
		{
			PsyCore_Log.Get("PF-REST").Error("TopGamesVoteService::ClaimSteamVote - service not configured");
			return;
		}

		if (steam64Id == "")
		{
			PsyCore_Log.Get("PF-REST").Error("TopGamesVoteService::ClaimSteamVote - empty steam64Id");
			return;
		}

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("server_token", m_ApiKey);
		args.Add("steam_id", steam64Id);
		args.Add("standard_http_code", "0");

		string endpoint = args.ToQuery("/v1/votes/claim-steam");
		PsyCore_Log.Get("PF-REST").Info("GET " + PF_Logger.MaskSecrets(m_BaseUrl + endpoint));
		PF_RestCallback.PF_Retain(cb);
		m_RestContext.GET(cb, endpoint);
	}

	void ClaimUsernameVote(string playerName, RestCallback cb)
	{
		if (!IsConfigured())
		{
			PsyCore_Log.Get("PF-REST").Error("TopGamesVoteService::ClaimUsernameVote - service not configured");
			return;
		}

		if (playerName == "")
		{
			PsyCore_Log.Get("PF-REST").Error("TopGamesVoteService::ClaimUsernameVote - empty playerName");
			return;
		}

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("server_token", m_ApiKey);
		args.Add("playername", playerName);

		string endpoint = args.ToQuery("/v1/votes/claim-username");
		PsyCore_Log.Get("PF-REST").Info("GET " + PF_Logger.MaskSecrets(m_BaseUrl + endpoint));
		PF_RestCallback.PF_Retain(cb);
		m_RestContext.GET(cb, endpoint);
	}

	protected string NormalizeEndpoint(string endpoint)
	{
		string normalizedEndpoint = endpoint;
		if (m_BaseUrl != "" && normalizedEndpoint.IndexOf(m_BaseUrl) == 0)
		{
			normalizedEndpoint.Replace(m_BaseUrl, "");
		}

		return normalizedEndpoint;
	}
}
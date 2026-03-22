class PF_WordPressApi : PF_WebApiBase
{
	protected string m_ApiKey;

	void PF_WordPressApi(string baseUrl, string apiKey)
	{
		m_ApiKey = apiKey;
	}

	void UploadLeaderboard(PF_WordPressPayload payload)
	{
		payload.apiKey = m_ApiKey;
		string data = payload.Serialize();

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("api_key", m_ApiKey);
		string endpoint = args.ToQuery("/upload");

		Print("[Psyerns Framework] Uploading leaderboard to WordPress...");
		Post(endpoint, data);
	}

	void Ping()
	{
		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("api_key", m_ApiKey);
		string endpoint = args.ToQuery("/ping");

		Get(endpoint);
	}
}

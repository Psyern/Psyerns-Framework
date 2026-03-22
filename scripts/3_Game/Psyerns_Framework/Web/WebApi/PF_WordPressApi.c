class PF_WordPressApi : PF_WebApiBase
{
	protected string m_ApiKey;

	void PF_WordPressApi(string baseUrl, string apiKey)
	{
		m_BaseUrl = baseUrl;
		m_ApiKey = apiKey;
		m_RestContext = m_Rest.GetRestContext(m_BaseUrl);
		m_RestContext.SetHeader("application/json");
	}

	void UploadLeaderboard(PF_WordPressPayload payload)
	{
		payload.apiKey = m_ApiKey;
		string data = payload.Serialize();

		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("api_key", m_ApiKey);
		string endpoint = args.ToQuery("/upload");

		PF_Logger.Log("Uploading leaderboard to WordPress: " + m_BaseUrl + endpoint);
		PF_Logger.Debug("WordPress payload size: " + data.Length().ToString() + " bytes");
		Post(endpoint, data);
	}

	void Ping()
	{
		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("api_key", m_ApiKey);
		string endpoint = args.ToQuery("/ping");

		PF_Logger.Debug("Pinging WordPress: " + m_BaseUrl + endpoint);
		Get(endpoint);
	}
}

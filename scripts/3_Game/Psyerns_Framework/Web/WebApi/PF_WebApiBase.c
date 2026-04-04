class PF_WebApiBase
{
	protected RestApi m_Rest;
	protected RestContext m_RestContext;
	protected string m_BaseUrl;

	void PF_WebApiBase()
	{
		m_Rest = GetRestApi();
		if (!m_Rest)
			m_Rest = CreateRestApi();
	}

	string GetBaseUrl()
	{
		return m_BaseUrl;
	}

	void Post(string endpoint, string data)
	{
		if (!m_RestContext)
		{
			PF_Logger.Error("PF_WebApiBase: RestContext is null");
			return;
		}
		PF_Logger.Debug("POST " + m_BaseUrl + endpoint + " (" + data.Length().ToString() + " bytes)");
		m_RestContext.POST(new PF_RestCallback(), endpoint, data);
	}

	void Get(string endpoint)
	{
		if (!m_RestContext)
		{
			PF_Logger.Error("PF_WebApiBase: RestContext is null");
			return;
		}
		PF_Logger.Debug("GET " + m_BaseUrl + endpoint);
		m_RestContext.GET(new PF_RestCallback(), endpoint);
	}
}

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

	void Post(string endpoint, string data, ref PF_RestCallback callback = null)
	{
		if (!callback)
			callback = new PF_RestCallback();

		PF_Logger.Debug("POST " + m_BaseUrl + endpoint + " (" + data.Length().ToString() + " bytes)");
		m_RestContext.POST(callback, endpoint, data);
	}

	void Get(string endpoint, ref PF_RestCallback callback = null)
	{
		if (!callback)
			callback = new PF_RestCallback();

		PF_Logger.Debug("GET " + m_BaseUrl + endpoint);
		m_RestContext.GET(callback, endpoint);
	}
}

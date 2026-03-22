class PF_WebApiBase
{
	protected RestApi m_Rest;
	protected RestContext m_RestContext;
	protected string m_BaseUrl;

	void PF_WebApiBase(string baseUrl)
	{
		m_BaseUrl = baseUrl;
		m_Rest = GetRestApi();
		if (!m_Rest)
			m_Rest = CreateRestApi();

		m_RestContext = m_Rest.GetRestContext(m_BaseUrl);
		m_RestContext.SetHeader("application/json");
	}

	string GetBaseUrl()
	{
		return m_BaseUrl;
	}

	void Post(string endpoint, string data, PF_RestCallback callback = null)
	{
		if (!callback)
			callback = new PF_RestCallback();

		m_RestContext.POST(callback, endpoint, data);
	}

	void Get(string endpoint, PF_RestCallback callback = null)
	{
		if (!callback)
			callback = new PF_RestCallback();

		m_RestContext.GET(callback, endpoint);
	}
}

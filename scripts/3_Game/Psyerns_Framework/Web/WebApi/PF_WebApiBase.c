class PF_WebApiBase
{
	protected RestApi m_Rest;
	protected RestContext m_RestContext;
	protected string m_BaseUrl;

	// Zentraler Zugriff auf die Engine-RestApi. Ein bedingungsloses
	// CreateRestApi() ersetzt die globale Instanz und macht alle bereits
	// gecachten RestContext-Pointer (auch die anderer Mods) zu Dangling
	// Pointers - RestApi/RestContext haben private Destruktoren, das Skript
	// besitzt sie nie.
	static RestApi PF_AcquireRestApi()
	{
		RestApi api = GetRestApi();
		if (!api)
			api = CreateRestApi();

		return api;
	}

	void PF_WebApiBase()
	{
		m_Rest = PF_AcquireRestApi();
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
		PF_RestCallback cb = new PF_RestCallback();
		PF_RestCallback.PF_Retain(cb);
		m_RestContext.POST(cb, endpoint, data);
	}

	void Get(string endpoint)
	{
		if (!m_RestContext)
		{
			PF_Logger.Error("PF_WebApiBase: RestContext is null");
			return;
		}
		PF_Logger.Debug("GET " + m_BaseUrl + endpoint);
		PF_RestCallback cb = new PF_RestCallback();
		PF_RestCallback.PF_Retain(cb);
		m_RestContext.GET(cb, endpoint);
	}
}

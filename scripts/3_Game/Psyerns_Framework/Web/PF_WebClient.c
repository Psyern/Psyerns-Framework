class PF_WebClient
{
	protected static ref PF_WebClient s_Instance;
	protected RestApi m_RestApi;
	protected ref map<string, RestContext> m_Contexts;

	void PF_WebClient()
	{
		m_Contexts = new map<string, RestContext>();
		m_RestApi = CreateRestApi();
	}

	static PF_WebClient GetInstance()
	{
		if (!s_Instance)
			s_Instance = new PF_WebClient();

		return s_Instance;
	}

	RestContext GetRestContext(string baseUrl)
	{
		if (m_Contexts.Contains(baseUrl))
			return m_Contexts.Get(baseUrl);

		RestContext ctx = m_RestApi.GetRestContext(baseUrl);
		ctx.SetHeader("application/json");
		m_Contexts.Insert(baseUrl, ctx);
		return ctx;
	}

	void Send(PF_WebRequest request)
	{
		RestContext ctx = GetRestContext(request.GetBaseUrl());
		ctx.SetHeader(request.GetHeader());

		ref PF_RestCallback callback = new PF_RestCallback();

		if (request.GetMethod() == 1)
			ctx.POST(callback, request.GetEndpoint(), request.GetBody());
		else
			ctx.GET(callback, request.GetEndpoint());
	}

	RestApi GetRestApi()
	{
		return m_RestApi;
	}
}

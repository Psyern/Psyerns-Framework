/**
 * PF_RestBase — Base class for all REST manager classes
 * Extends PF_WebApiBase with API key handling and [PF-REST] logging.
 *
 * IMPORTANT: DayZ RestContext.SetHeader() only supports Content-Type as header value.
 * Custom headers like Authorization or X-Api-Key are NOT supported by the engine.
 * API keys are therefore passed as query parameters (?api_key=...) instead.
 */
class PF_RestBase : PF_WebApiBase
{
	protected string m_ApiKey;

	void PF_RestBase(string baseUrl, string apiKey)
	{
		m_BaseUrl = baseUrl;
		m_ApiKey = apiKey;
		m_RestContext = m_Rest.GetRestContext(m_BaseUrl);
		m_RestContext.SetHeader("application/json");
		Print("[PF-REST] REST base initialized: " + m_BaseUrl);
	}

	// Builds endpoint path with api_key as query parameter
	protected string BuildEndpoint(string path)
	{
		PF_HttpArguments args = new PF_HttpArguments();
		args.Add("api_key", m_ApiKey);
		return args.ToQuery(path);
	}

	// Builds endpoint with additional custom query parameters
	protected string BuildEndpointArgs(string path, PF_HttpArguments extraArgs)
	{
		extraArgs.Add("api_key", m_ApiKey);
		return extraArgs.ToQuery(path);
	}

	// POST with custom callback for response handling
	void PostWithCallback(string path, string jsonData, RestCallback cb)
	{
		string endpoint = BuildEndpoint(path);
		Print("[PF-REST] POST " + m_BaseUrl + endpoint);
		m_RestContext.POST(cb, endpoint, jsonData);
	}

	// GET with custom callback for response handling
	void GetWithCallback(string path, RestCallback cb)
	{
		string endpoint = BuildEndpoint(path);
		Print("[PF-REST] GET " + m_BaseUrl + endpoint);
		m_RestContext.GET(cb, endpoint);
	}

	// GET with extra query parameters and custom callback
	void GetWithArgs(string path, PF_HttpArguments args, RestCallback cb)
	{
		string endpoint = BuildEndpointArgs(path, args);
		Print("[PF-REST] GET " + m_BaseUrl + endpoint);
		m_RestContext.GET(cb, endpoint);
	}

	// POST fire-and-forget (uses default PF_RestCallback)
	void PostJson(string path, string jsonData)
	{
		string endpoint = BuildEndpoint(path);
		Print("[PF-REST] POST " + m_BaseUrl + endpoint);
		Post(endpoint, jsonData);
	}

	// GET fire-and-forget (uses default PF_RestCallback)
	void GetJson(string path)
	{
		string endpoint = BuildEndpoint(path);
		Print("[PF-REST] GET " + m_BaseUrl + endpoint);
		Get(endpoint);
	}

	string GetApiKey()
	{
		return m_ApiKey;
	}
}

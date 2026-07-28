class PF_WebRequest
{
	protected string m_BaseUrl;
	protected string m_Endpoint;
	protected string m_Header;
	protected string m_Body;
	protected int m_Method; // 0 = GET, 1 = POST

	void PF_WebRequest()
	{
		m_Header = "application/json";
		m_Method = 1;
	}

	PF_WebRequest SetUrl(string url)
	{
		m_BaseUrl = url;
		return this;
	}

	PF_WebRequest SetEndpoint(string endpoint)
	{
		m_Endpoint = endpoint;
		return this;
	}

	PF_WebRequest SetHeader(string header)
	{
		m_Header = header;
		return this;
	}

	PF_WebRequest SetBody(string body)
	{
		m_Body = body;
		return this;
	}

	PF_WebRequest SetMethod(int method)
	{
		m_Method = method;
		return this;
	}

	PF_WebRequest Post()
	{
		m_Method = 1;
		return this;
	}

	PF_WebRequest Get()
	{
		m_Method = 0;
		return this;
	}

	string GetBaseUrl()
	{
		return m_BaseUrl;
	}

	string GetEndpoint()
	{
		return m_Endpoint;
	}

	string GetHeader()
	{
		return m_Header;
	}

	string GetBody()
	{
		return m_Body;
	}

	int GetMethod()
	{
		return m_Method;
	}
}

class PF_WebConfig
{
	bool EnableDebugLogging;
	int DefaultRetryCount;
	int QueueMaxSize;
	ref array<ref PF_WebEndpoint> Endpoints;

	[NonSerialized()]
	protected static ref PF_WebConfig s_Instance;

	void PF_WebConfig()
	{
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		Endpoints = new array<ref PF_WebEndpoint>();
	}

	static PF_WebConfig GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new PF_WebConfig();
			s_Instance.Load();
		}

		return s_Instance;
	}

	static string GetConfigPath()
	{
		return "$profile:Psyerns_Framework\\PsyernsFrameworkConfig.json";
	}

	void Load()
	{
		string path = GetConfigPath();

		if (FileExist(path))
		{
			JsonFileLoader<PF_WebConfig>.JsonLoadFile(path, this);
			Print("[Psyerns Framework] Config loaded from " + path);
		}
		else
		{
			CreateDefaults();
			Save();
			Print("[Psyerns Framework] Default config created at " + path);
		}
	}

	void Save()
	{
		string path = GetConfigPath();
		JsonFileLoader<PF_WebConfig>.JsonSaveFile(path, this);
	}

	void CreateDefaults()
	{
		EnableDebugLogging = false;
		DefaultRetryCount = 3;
		QueueMaxSize = 100;
		Endpoints.Clear();

		PF_WebEndpoint wp = new PF_WebEndpoint();
		wp.Name = "WordPress";
		wp.BaseUrl = "https://your-site.com/wp-json/psyern/v1";
		wp.ApiKey = "YOUR_API_KEY_HERE";
		wp.Enabled = false;
		wp.RateLimitMs = 5000;
		Endpoints.Insert(wp);

		PF_WebEndpoint discord = new PF_WebEndpoint();
		discord.Name = "Discord";
		discord.BaseUrl = "https://discord.com/api/webhooks";
		discord.ApiKey = "";
		discord.Enabled = false;
		discord.RateLimitMs = 1000;
		Endpoints.Insert(discord);
	}

	PF_WebEndpoint GetEndpoint(string name)
	{
		for (int i = 0; i < Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = Endpoints[i];
			string epName = ep.Name;
			epName.ToLower();
			string searchName = name;
			searchName.ToLower();

			if (epName == searchName)
				return ep;
		}

		return null;
	}

	bool IsEndpointEnabled(string name)
	{
		PF_WebEndpoint ep = GetEndpoint(name);
		if (!ep)
			return false;

		return ep.Enabled;
	}
}

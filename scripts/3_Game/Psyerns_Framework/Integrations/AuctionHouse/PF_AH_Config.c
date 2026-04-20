/**
 * PF_AH_Config — AuctionHouse integration configuration
 *
 * Persisted as a standalone JSON file sibling to PsyernsFrameworkConfig.json:
 *   $profile:DeadmansEcho\PsyernsFramework\PsyernsAuctionHouseConfig.json
 *
 * Kept separate from PF_WebConfig so existing installations are not touched.
 * Auto-creates defaults when the file is missing.
 */
class PF_AH_Config
{
	bool Enabled;
	string WpUrl;
	string ApiKey;
	int PushIntervalSeconds;
	int PollIntervalSeconds;
	string CurrencyMode;

	[NonSerialized()]
	protected static ref PF_AH_Config s_Instance;

	void PF_AH_Config()
	{
		Enabled = false;
		WpUrl = "https://your-site.com";
		ApiKey = "";
		PushIntervalSeconds = 30;
		PollIntervalSeconds = 10;
		CurrencyMode = "Expansion";
	}

	static string GetConfigPath()
	{
		return "$profile:DeadmansEcho\\PsyernsFramework\\PsyernsAuctionHouseConfig.json";
	}

	static string GetConfigDirectory()
	{
		return "$profile:DeadmansEcho\\PsyernsFramework";
	}

	static PF_AH_Config Load()
	{
		if (s_Instance)
			return s_Instance;

		s_Instance = new PF_AH_Config();
		string path = GetConfigPath();

		if (FileExist(path))
		{
			JsonFileLoader<PF_AH_Config>.JsonLoadFile(path, s_Instance);
			PF_Logger.Log("AH: Config loaded from " + path);
		}
		else
		{
			s_Instance.Save();
			PF_Logger.Log("AH: Default config created at " + path);
		}

		s_Instance.ValidateAndClamp();
		return s_Instance;
	}

	void Save()
	{
		string dir = GetConfigDirectory();
		if (!FileExist(dir))
			MakeDirectory(dir);

		string path = GetConfigPath();
		JsonFileLoader<PF_AH_Config>.JsonSaveFile(path, this);
	}

	static PF_AH_Config GetInstance()
	{
		if (!s_Instance)
			return Load();
		return s_Instance;
	}

	static void Reload()
	{
		s_Instance = null;
		Load();
	}

	protected void ValidateAndClamp()
	{
		if (PushIntervalSeconds < 10)
			PushIntervalSeconds = 10;
		if (PushIntervalSeconds > 3600)
			PushIntervalSeconds = 3600;

		if (PollIntervalSeconds < 3)
			PollIntervalSeconds = 3;
		if (PollIntervalSeconds > 300)
			PollIntervalSeconds = 300;

		if (CurrencyMode == "")
			CurrencyMode = "Expansion";
	}
}

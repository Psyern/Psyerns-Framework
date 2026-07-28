/**
 * PF_AH_BalanceReader — Reads currency balances from disk.
 *
 * Two modes:
 *   "Expansion" → enumerates $profile:ExpansionMod\ATM\*.json (ExpansionMarketATM_Data)
 *   "Internal"  → loads $profile:DME_AH\Data\PlayerData.json (DME_AH_PlayerData.Balances)
 *
 * Both modes degrade gracefully: missing files produce an empty array, not an error.
 */
class PF_AH_BalanceReader
{
	static const string EXPANSION_ATM_DIR = "$profile:ExpansionMod\\ATM";
	static const string DME_AH_PLAYER_DATA = "$profile:DME_AH\\Data\\PlayerData.json";

	static array<ref PF_AH_BalanceEntry> GetAllBalances(string currencyMode)
	{
		string modeLower = currencyMode;
		modeLower.ToLower();

		if (modeLower == "internal")
			return ReadInternalBalances();

		return ReadExpansionBalances();
	}

	protected static array<ref PF_AH_BalanceEntry> ReadExpansionBalances()
	{
		array<ref PF_AH_BalanceEntry> result = new array<ref PF_AH_BalanceEntry>();

		if (!FileExist(EXPANSION_ATM_DIR))
		{
			PF_Logger.Debug("AH: Expansion ATM directory not found: " + EXPANSION_ATM_DIR);
			return result;
		}

		string fileName;
		FileAttr fileAttr;
		string searchPattern = EXPANSION_ATM_DIR + "\\*.json";

		FindFileHandle handle = FindFile(searchPattern, fileName, fileAttr, FindFileFlags.ALL);
		if (!handle)
		{
			PF_Logger.Debug("AH: FindFile returned null for: " + searchPattern);
			return result;
		}

		int read = 0;
		int skipped = 0;
		bool hasFile = true;

		while (hasFile)
		{
			if (fileName != "" && fileName.IndexOf(".json") > 0)
			{
				string fullPath = EXPANSION_ATM_DIR + "\\" + fileName;
				PF_AH_ExpansionATMData atm = new PF_AH_ExpansionATMData();
				JsonFileLoader<PF_AH_ExpansionATMData>.JsonLoadFile(fullPath, atm);

				if (atm.PlayerID != "")
				{
					PF_AH_BalanceEntry entry = new PF_AH_BalanceEntry();
					entry.uid = atm.PlayerID;
					entry.source = "Expansion";
					entry.balance = atm.MoneyDeposited;
					result.Insert(entry);
					read++;
				}
				else
				{
					skipped++;
				}
			}

			hasFile = FindNextFile(handle, fileName, fileAttr);
		}

		CloseFindFile(handle);
		PF_Logger.Debug("AH: Expansion balances read=" + read.ToString() + " skipped=" + skipped.ToString());
		return result;
	}

	protected static array<ref PF_AH_BalanceEntry> ReadInternalBalances()
	{
		array<ref PF_AH_BalanceEntry> result = new array<ref PF_AH_BalanceEntry>();

#ifdef DME_AH_Scripts
		if (!FileExist(DME_AH_PLAYER_DATA))
		{
			PF_Logger.Debug("AH: DME_AH PlayerData not found: " + DME_AH_PLAYER_DATA);
			return result;
		}

		DME_AH_PlayerData data = new DME_AH_PlayerData();
		JsonFileLoader<DME_AH_PlayerData>.JsonLoadFile(DME_AH_PLAYER_DATA, data);

		if (!data.Balances)
		{
			PF_Logger.Debug("AH: Internal balances map is null");
			return result;
		}

		array<string> keys = data.Balances.GetKeyArray();

		for (int i = 0; i < keys.Count(); i++)
		{
			string uid = keys[i];
			int balance = data.Balances.Get(uid);

			PF_AH_BalanceEntry entry = new PF_AH_BalanceEntry();
			entry.uid = uid;
			entry.source = "Internal";
			entry.balance = balance;
			result.Insert(entry);
		}

		PF_Logger.Debug("AH: Internal balances read=" + result.Count().ToString());
#else
		PF_Logger.Log("AH: [WARN] Internal balance mode requires DME_Auction_House mod (not loaded).");
#endif
		return result;
	}
}

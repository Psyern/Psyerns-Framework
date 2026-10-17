/**
 * PF_AH_Types — shared data classes used by the AuctionHouse integration.
 *
 * All member names match the JSON schemas in WP-Plugin_Psyerns_AuctionHouse/README.md §7.
 */

class PF_AH_BalanceEntry
{
	string uid;
	string source;
	int balance;

	void PF_AH_BalanceEntry()
	{
		uid = "";
		source = "";
		balance = 0;
	}
}

class PF_AH_SyncState
{
	int lastUploadedTransactionTs;

	void PF_AH_SyncState()
	{
		lastUploadedTransactionTs = 0;
	}

	static string GetStatePath()
	{
		return "$profile:Psyerns_Framework\\AHState.json";
	}

	static PF_AH_SyncState Load()
	{
		PF_AH_SyncState state = new PF_AH_SyncState();
		string path = GetStatePath();
		string loadError;
		if (FileExist(path) && !PsyCore_Json<PF_AH_SyncState>.Load(path, state, loadError))
			PF_Logger.Error("AH: Sync state " + path + " could not be read: " + loadError);
		return state;
	}

	void Save()
	{
		PsyCore_Paths.EnsureDir("$profile:Psyerns_Framework");
		// same pretty JSON as before, no .bak
		string saveError;
		if (!PsyCore_Json<PF_AH_SyncState>.Save(GetStatePath(), this, saveError, false))
			PF_Logger.Error("AH: Sync state save failed: " + saveError);
	}
}

class PF_AH_PendingAction
{
	string action_uuid;
	string type;
	string player_uid;
	string listing_id;
	int amount;
	string created_at;

	void PF_AH_PendingAction()
	{
		action_uuid = "";
		type = "";
		player_uid = "";
		listing_id = "";
		amount = 0;
		created_at = "";
	}
}

class PF_AH_PendingActionArray
{
	ref array<ref PF_AH_PendingAction> actions;

	void PF_AH_PendingActionArray()
	{
		actions = new array<ref PF_AH_PendingAction>();
	}
}

/**
 * Local mirror of Expansion's ATM_Data JSON schema.
 * The real ExpansionMarketATM_Data class lives in 4_World and cannot be referenced here.
 * Field names MUST match the on-disk JSON exactly.
 */
class PF_AH_ExpansionATMData
{
	string PlayerID;
	int MoneyDeposited;

	void PF_AH_ExpansionATMData()
	{
		PlayerID = "";
		MoneyDeposited = 0;
	}
}

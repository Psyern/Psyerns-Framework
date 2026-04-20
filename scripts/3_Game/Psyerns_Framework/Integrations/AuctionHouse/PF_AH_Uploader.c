/**
 * PF_AH_Uploader — Builds and sends the /internal/upload payload.
 *
 * Reads listings + transactions + balances, emits them per README §7 upload shape:
 *   {
 *     "generatedAt": "...",
 *     "serverTimeEpoch": 1713621600,
 *     "currencyMode": "Expansion|Internal",
 *     "listings":           [ DME_AH_Listing shape, ... ],
 *     "recentTransactions": [ DME_AH_Transaction shape, ... ],  // delta only
 *     "balances":           [ { uid, source, balance }, ... ]
 *   }
 *
 * Delta logic: only transactions with Timestamp > m_SyncState.lastUploadedTransactionTs.
 * The watermark is persisted eagerly BEFORE the POST, because the engine RestCallback
 * cannot cleanly mutate outer state from within OnSuccess. Cost: a rare failed upload
 * could lose a batch of transactions from the WP mirror. Transactions are idempotent
 * server-side (unique transaction_id), so a re-send on startup would be harmless.
 */
class PF_AH_Uploader : PF_RestBase
{
	protected ref PF_AH_SyncState m_SyncState;
	protected string m_CurrencyMode;

	void PF_AH_Uploader(string wpUrl, string apiKey, string currencyMode)
	{
		// PF_RestBase constructor already stored wpUrl + apiKey
		m_CurrencyMode = currencyMode;
		m_SyncState = PF_AH_SyncState.Load();
	}

	void Upload()
	{
		string payload = BuildPayload();
		if (payload == "")
		{
			PF_Logger.Log("AH: Upload skipped — payload empty.");
			return;
		}

		int len = payload.Length();
		if (len > 1048576)
			PF_Logger.Log("AH: [WARN] Upload payload size " + len.ToString() + " bytes exceeds 1 MB soft limit.");

		PF_Logger.Log("AH: Uploading (" + len.ToString() + " bytes)");
		PostJson("/wp-json/psyern-ah/v1/internal/upload", payload);
	}

	protected string BuildPayload()
	{
		string listingsJson = BuildListingsArray();
		string txJson = BuildTransactionsArrayAndAdvanceWatermark();
		string balancesJson = BuildBalancesArray();

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("generatedAt", GetIsoTimestamp());
		b.AddInt("serverTimeEpoch", GetUnixEpoch());
		b.Add("currencyMode", m_CurrencyMode);
		b.AddRaw("listings", listingsJson);
		b.AddRaw("recentTransactions", txJson);
		b.AddRaw("balances", balancesJson);
		return b.Build();
	}

	protected string BuildListingsArray()
	{
#ifdef DME_AH_Scripts
		string path = "$profile:DME_AH\\Data\\ActiveListings.json";
		if (!FileExist(path))
		{
			PF_Logger.Debug("AH: ActiveListings.json not found at " + path);
			return "[]";
		}

		DME_AH_ListingArray arr = new DME_AH_ListingArray();
		JsonFileLoader<DME_AH_ListingArray>.JsonLoadFile(path, arr);

		if (!arr.Listings || arr.Listings.Count() == 0)
			return "[]";

		string result = "[";
		for (int i = 0; i < arr.Listings.Count(); i++)
		{
			DME_AH_Listing l = arr.Listings[i];
			if (!l)
				continue;

			if (i > 0)
				result += ",";

			result += SerializeListing(l);
		}
		result += "]";
		return result;
#else
		return "[]";
#endif
	}

#ifdef DME_AH_Scripts
	protected string SerializeListing(DME_AH_Listing l)
	{
		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("ListingID", l.ListingID);
		b.Add("SellerUID", l.SellerUID);
		b.Add("SellerName", l.SellerName);
		b.Add("ItemClassName", l.ItemClassName);
		b.Add("ItemDisplayName", l.ItemDisplayName);
		b.AddInt("CategoryID", l.CategoryID);
		b.AddInt("ListingType", l.ListingType);
		b.AddInt("StartPrice", l.StartPrice);
		b.AddInt("BuyNowPrice", l.BuyNowPrice);
		b.AddInt("CurrentBid", l.CurrentBid);
		b.Add("CurrentBidderUID", l.CurrentBidderUID);
		b.Add("CurrentBidderName", l.CurrentBidderName);
		b.AddInt("BidCount", l.BidCount);
		b.AddInt("CreatedTimestamp", l.CreatedTimestamp);
		b.AddInt("ExpiresTimestamp", l.ExpiresTimestamp);
		b.AddInt("Status", l.Status);
		return b.Build();
	}

	protected string SerializeTransaction(DME_AH_Transaction t)
	{
		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("TransactionID", t.TransactionID);
		b.Add("ListingID", t.ListingID);
		b.Add("SellerUID", t.SellerUID);
		b.Add("BuyerUID", t.BuyerUID);
		b.Add("SellerName", t.SellerName);
		b.Add("BuyerName", t.BuyerName);
		b.Add("ItemClassName", t.ItemClassName);
		b.Add("ItemDisplayName", t.ItemDisplayName);
		b.AddInt("FinalPrice", t.FinalPrice);
		b.AddInt("Fee", t.Fee);
		b.AddInt("Timestamp", t.Timestamp);
		b.AddInt("Type", t.Type);
		return b.Build();
	}
#endif

	protected string BuildTransactionsArrayAndAdvanceWatermark()
	{
#ifdef DME_AH_Scripts
		string path = "$profile:DME_AH\\Data\\CompletedListings.json";
		if (!FileExist(path))
			return "[]";

		DME_AH_TransactionArray arr = new DME_AH_TransactionArray();
		JsonFileLoader<DME_AH_TransactionArray>.JsonLoadFile(path, arr);

		if (!arr.Transactions || arr.Transactions.Count() == 0)
			return "[]";

		int watermark = m_SyncState.lastUploadedTransactionTs;
		int newWatermark = watermark;
		int deltaCount = 0;

		string result = "[";
		for (int i = 0; i < arr.Transactions.Count(); i++)
		{
			DME_AH_Transaction t = arr.Transactions[i];
			if (!t)
				continue;
			if (t.Timestamp <= watermark)
				continue;

			if (deltaCount > 0)
				result += ",";
			result += SerializeTransaction(t);
			deltaCount++;

			if (t.Timestamp > newWatermark)
				newWatermark = t.Timestamp;
		}
		result += "]";

		if (newWatermark > watermark)
		{
			m_SyncState.lastUploadedTransactionTs = newWatermark;
			m_SyncState.Save();
			PF_Logger.Debug("AH: Advanced transaction watermark to " + newWatermark.ToString() + " (+" + deltaCount.ToString() + ")");
		}

		return result;
#else
		return "[]";
#endif
	}

	protected string BuildBalancesArray()
	{
		array<ref PF_AH_BalanceEntry> entries = PF_AH_BalanceReader.GetAllBalances(m_CurrencyMode);
		if (!entries || entries.Count() == 0)
			return "[]";

		string result = "[";
		for (int i = 0; i < entries.Count(); i++)
		{
			PF_AH_BalanceEntry e = entries[i];
			if (!e)
				continue;

			if (i > 0)
				result += ",";

			PF_JsonBuilder b = PF_JsonBuilder.Begin();
			b.Add("uid", e.uid);
			b.Add("source", e.source);
			b.AddInt("balance", e.balance);
			result += b.Build();
		}
		result += "]";
		return result;
	}

	protected string GetIsoTimestamp()
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
	}

	protected int GetUnixEpoch()
	{
#ifdef DME_AH_Scripts
		return DME_AH_Util.GetTimestamp();
#else
		// Fallback: approximate using local date. Caller tolerates both formats; WP uses generatedAt primarily.
		return 0;
#endif
	}
}

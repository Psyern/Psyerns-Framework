// --- Singleton accessor ---
ref PF_KillFeedManager g_PF_KillFeedManager;

PF_KillFeedManager GetPF_KillFeedManager()
{
	return g_PF_KillFeedManager;
}

/**
 * PF_KillEvent — Serializable kill event data
 * Contains all information about a player kill for webhook delivery.
 */
class PF_KillEvent
{
	string killerSteamId;
	string killerName;
	string killerWeapon;
	string victimSteamId;
	string victimName;
	float posX;
	float posY;
	float posZ;
	float distance;
	string timestamp;

	void PF_KillEvent()
	{
		killerSteamId = "";
		killerName = "Unknown";
		killerWeapon = "Unknown";
		victimSteamId = "";
		victimName = "Unknown";
		posX = 0;
		posY = 0;
		posZ = 0;
		distance = 0;
		timestamp = "";
	}

	string ToJson()
	{
		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("killerSteamId", killerSteamId);
		b.Add("killerName", killerName);
		b.Add("killerWeapon", killerWeapon);
		b.Add("victimSteamId", victimSteamId);
		b.Add("victimName", victimName);
		b.AddFloat("posX", posX);
		b.AddFloat("posY", posY);
		b.AddFloat("posZ", posZ);
		b.AddFloat("distance", distance);
		b.Add("timestamp", timestamp);
		return b.Build();
	}
}

/**
 * PF_KillFeedManager — Sends kill events to all registered webhook URLs
 *
 * Usage:
 *   GetPF_KillFeedManager().OnPlayerKilled(victim, killer);
 *
 * Sends the PF_KillEvent JSON via POST to every URL in PF_RestConfig.WebhookUrls.
 * Each webhook gets its own RestApi/RestContext to handle different base URLs.
 */
class PF_KillFeedManager
{
	protected ref array<string> m_WebhookUrls;
	protected ref array<RestApi> m_RestApis;
	protected ref array<RestContext> m_Contexts;

	void PF_KillFeedManager(array<string> webhookUrls)
	{
		m_WebhookUrls = new array<string>();
		m_RestApis = new array<RestApi>();
		m_Contexts = new array<RestContext>();

		if (webhookUrls)
		{
			for (int i = 0; i < webhookUrls.Count(); i++)
			{
				string url = webhookUrls[i];
				if (url == "")
					continue;

				m_WebhookUrls.Insert(url);
				RestApi api = CreateRestApi();
				m_RestApis.Insert(api);
				RestContext ctx = api.GetRestContext(url);
				ctx.SetHeader("application/json");
				m_Contexts.Insert(ctx);
			}
		}

		Print("[PF-REST] KillFeedManager initialized with " + m_WebhookUrls.Count().ToString() + " webhook(s)");
	}

	/**
	 * Called when a player is killed — builds the event and sends to all webhooks
	 * @param victim  The killed player
	 * @param killer  The entity that caused the kill
	 */
	void OnPlayerKilled(PlayerBase victim, Object killer)
	{
		if (!victim)
			return;

		PF_KillEvent killEvent = new PF_KillEvent();

		// Victim data
		if (victim.GetIdentity())
		{
			killEvent.victimSteamId = victim.GetIdentity().GetPlainId();
			killEvent.victimName = victim.GetIdentity().GetName();
		}

		// Victim position
		vector victimPos = victim.GetPosition();
		killEvent.posX = victimPos[0];
		killEvent.posY = victimPos[1];
		killEvent.posZ = victimPos[2];

		// Killer data
		PlayerBase killerPlayer;
		if (Class.CastTo(killerPlayer, killer))
		{
			if (killerPlayer.GetIdentity())
			{
				killEvent.killerSteamId = killerPlayer.GetIdentity().GetPlainId();
				killEvent.killerName = killerPlayer.GetIdentity().GetName();
			}

			// Weapon from killer's hands
			EntityAI itemInHands;
			HumanInventory inv = killerPlayer.GetHumanInventory();
			if (inv)
				itemInHands = inv.GetEntityInHands();
			if (itemInHands)
				killEvent.killerWeapon = itemInHands.GetType();

			// Distance between killer and victim
			vector killerPos = killerPlayer.GetPosition();
			killEvent.distance = vector.Distance(killerPos, victimPos);
		}
		else if (killer)
		{
			killEvent.killerName = killer.GetType();
		}

		// Timestamp
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		killEvent.timestamp = year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";

		// Serialize and send to all webhooks
		string json = killEvent.ToJson();
		SendToWebhooks(json);

		PF_Logger.Log("KillFeed: " + killEvent.killerName + " killed " + killEvent.victimName + " with " + killEvent.killerWeapon + " at " + killEvent.distance.ToString() + "m");
	}

	protected void SendToWebhooks(string json)
	{
		for (int i = 0; i < m_WebhookUrls.Count(); i++)
		{
			PF_RestCallback cb = new PF_RestCallback();
			m_Contexts[i].POST(cb, "", json);
			Print("[PF-REST] KillFeed sent to: " + m_WebhookUrls[i]);
		}
	}
}

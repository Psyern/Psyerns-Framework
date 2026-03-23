// --- Singleton accessor ---
ref PF_DiscordIntegration g_PF_DiscordIntegration;

PF_DiscordIntegration GetPF_DiscordIntegration()
{
	return g_PF_DiscordIntegration;
}

/**
 * PF_DiscordIntegration — Event-based Discord webhook integration
 *
 * Wraps PF_DiscordWebhook with event type routing and automatic embed formatting.
 *
 * Supported event types:
 *   "player.login"   — Green embed, player connected
 *   "player.logout"  — Red embed, player disconnected
 *   "player.kill"    — Orange embed, kill event
 *   "player.loot"    — Blue embed, loot event
 *
 * Usage:
 *   GetPF_DiscordIntegration().Send("player.login", "PlayerName joined the server");
 */
class PF_DiscordIntegration
{
	protected ref PF_DiscordWebhook m_Webhook;
	protected ref map<string, int> m_EventColors;
	protected ref map<string, string> m_EventTitles;

	// Discord embed colors
	static const int COLOR_GREEN  = 3066993;   // #2ECC71
	static const int COLOR_RED    = 15158332;  // #E74C3C
	static const int COLOR_ORANGE = 15105570;  // #E67E22
	static const int COLOR_BLUE   = 3447003;   // #3498DB
	static const int COLOR_GRAY   = 9807270;   // #95A5A6

	void PF_DiscordIntegration(string webhookId, string webhookToken)
	{
		if (webhookId == "" || webhookToken == "")
		{
			Print("[PF-REST] [ERROR] DiscordIntegration — empty webhook ID or token");
			return;
		}

		m_Webhook = new PF_DiscordWebhook(webhookId, webhookToken);

		// Map event types to colors
		m_EventColors = new map<string, int>();
		m_EventColors.Set("player.login", COLOR_GREEN);
		m_EventColors.Set("player.logout", COLOR_RED);
		m_EventColors.Set("player.kill", COLOR_ORANGE);
		m_EventColors.Set("player.loot", COLOR_BLUE);

		// Map event types to titles
		m_EventTitles = new map<string, string>();
		m_EventTitles.Set("player.login", "Player Connected");
		m_EventTitles.Set("player.logout", "Player Disconnected");
		m_EventTitles.Set("player.kill", "Kill Event");
		m_EventTitles.Set("player.loot", "Loot Event");

		Print("[PF-REST] DiscordIntegration initialized");
	}

	/**
	 * Send a Discord webhook message for a specific event type
	 * @param eventType   One of: player.login, player.logout, player.kill, player.loot
	 * @param message     The message body / description
	 * @param embedColor  Optional: override the default color for this event type (0 = use default)
	 */
	void Send(string eventType, string message, int embedColor = 0)
	{
		if (!m_Webhook)
		{
			Print("[PF-REST] [ERROR] DiscordIntegration::Send — webhook not initialized");
			return;
		}

		// Determine color
		int color = COLOR_GRAY;
		if (embedColor != 0)
		{
			color = embedColor;
		}
		else if (m_EventColors.Contains(eventType))
		{
			color = m_EventColors.Get(eventType);
		}

		// Determine title
		string title = eventType;
		if (m_EventTitles.Contains(eventType))
			title = m_EventTitles.Get(eventType);

		// Build and send payload
		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";

		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetTitle(title);
		embed.SetDescription(message);
		embed.SetColor(color);
		embed.SetTimestamp(GetTimestamp());

		m_Webhook.Send(payload);
		Print("[PF-REST] Discord event sent: " + eventType + " — " + message);
	}

	/**
	 * Send a Discord message with custom embed fields
	 * @param eventType  Event type for color/title mapping
	 * @param message    Main description
	 * @param fields     Map of field name -> field value for embed fields
	 */
	void SendWithFields(string eventType, string message, map<string, string> fields)
	{
		if (!m_Webhook)
		{
			Print("[PF-REST] [ERROR] DiscordIntegration::SendWithFields — webhook not initialized");
			return;
		}

		int color = COLOR_GRAY;
		if (m_EventColors.Contains(eventType))
			color = m_EventColors.Get(eventType);

		string title = eventType;
		if (m_EventTitles.Contains(eventType))
			title = m_EventTitles.Get(eventType);

		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";

		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetTitle(title);
		embed.SetDescription(message);
		embed.SetColor(color);
		embed.SetTimestamp(GetTimestamp());

		if (fields)
		{
			for (int i = 0; i < fields.Count(); i++)
			{
				string fieldName = fields.GetKey(i);
				string fieldValue = fields.GetElement(i);
				embed.AddField(fieldName, fieldValue, true);
			}
		}

		m_Webhook.Send(payload);
		Print("[PF-REST] Discord event sent with fields: " + eventType);
	}

	// Returns ISO-style timestamp string
	protected string GetTimestamp()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
	}

	PF_DiscordWebhook GetWebhook()
	{
		return m_Webhook;
	}
}

class PF_DiscordWebhook : PF_WebApiBase
{
	protected string m_WebhookId;
	protected string m_WebhookToken;

	void PF_DiscordWebhook(string webhookId, string webhookToken)
	{
		m_WebhookId = webhookId;
		m_WebhookToken = webhookToken;
	}

	void Send(PF_DiscordPayload payload)
	{
		string endpoint = "/" + m_WebhookId + "/" + m_WebhookToken;
		string data = payload.Serialize();
		Print("[Psyerns Framework] Sending Discord webhook...");
		Post(endpoint, data);
	}

	void SendSimple(string title, string message, int color)
	{
		PF_DiscordPayload payload = new PF_DiscordPayload();
		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetTitle(title);
		embed.SetDescription(message);
		embed.SetColor(color);
		Send(payload);
	}
}

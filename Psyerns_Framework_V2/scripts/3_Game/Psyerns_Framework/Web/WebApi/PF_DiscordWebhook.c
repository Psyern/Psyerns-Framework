class PF_DiscordWebhook : PF_WebApiBase
{
	protected string m_WebhookId;
	protected string m_WebhookToken;

	void PF_DiscordWebhook(string webhook_id, string webhook_token)
	{
		m_WebhookId = webhook_id;
		m_WebhookToken = webhook_token;
		m_RestContext = m_Rest.GetRestContext(GetBaseUrl());
		m_RestContext.SetHeader("application/json");
	}

	override string GetBaseUrl()
	{
		return "https://discord.com/api/webhooks";
	}

	void Send(PF_DiscordPayload payload)
	{
		string endpoint = "/" + m_WebhookId + "/" + m_WebhookToken;
		string data = payload.Serialize();
		PF_Logger.Log("Sending Discord webhook to /" + m_WebhookId + "/***");
		PF_Logger.Debug("Discord payload size: " + data.Length().ToString() + " bytes");
		Post(endpoint, data);
	}

	void SendSimple(string title, string message, int color)
	{
		PF_Logger.Debug("SendSimple: title=" + title + " color=" + color.ToString());
		PF_DiscordPayload payload = new PF_DiscordPayload();
		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetTitle(title);
		embed.SetDescription(message);
		embed.SetColor(color);
		Send(payload);
	}
}

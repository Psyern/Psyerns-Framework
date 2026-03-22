modded class MissionServer
{
	protected ref PF_WebQueueProcessor m_PF_QueueProcessor;
	protected float m_PF_StartNotifyTimer;
	protected bool m_PF_StartNotifySent;

	override void OnInit()
	{
		super.OnInit();

		PF_WebConfig config = PF_WebConfig.GetInstance();

		PF_Logger.Init(config.EnableDebugLogging);
		PF_Logger.Log("Psyerns Framework v1.0.0 initializing...");
		PF_Logger.Log("Author: Psyern | Community: Deadmans Echo");

		PF_WebClient.GetInstance();

		m_PF_QueueProcessor = PF_WebQueueProcessor.GetInstance();
		m_PF_QueueProcessor.Start();

		m_PF_StartNotifyTimer = 0;
		m_PF_StartNotifySent = false;

		int enabledCount = 0;
		for (int i = 0; i < config.Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = config.Endpoints[i];
			if (ep.Enabled)
			{
				PF_Logger.Log("Endpoint enabled: " + ep.Name + " (" + ep.BaseUrl + ")");
				enabledCount++;
			}
		}

		if (enabledCount == 0)
			PF_Logger.Log("No endpoints enabled. Configure PsyernsFrameworkConfig.json to activate.");

		if (config.EnableServerStartNotification)
			PF_Logger.Log("Server start notification will be sent after " + config.ServerStartDelaySeconds.ToString() + "s delay");

		PF_Logger.Log("Framework initialized. Queue processor running.");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (m_PF_QueueProcessor)
			m_PF_QueueProcessor.OnUpdate(timeslice);

		if (!m_PF_StartNotifySent)
		{
			m_PF_StartNotifyTimer += timeslice;
			PF_WebConfig config = PF_WebConfig.GetInstance();

			if (config.EnableServerStartNotification && m_PF_StartNotifyTimer >= config.ServerStartDelaySeconds)
			{
				m_PF_StartNotifySent = true;
				SendServerStartNotification(config);
			}
		}
	}

	protected void SendServerStartNotification(PF_WebConfig config)
	{
		PF_WebEndpoint discordEp = config.GetEndpoint("Discord");
		if (!discordEp || !discordEp.Enabled)
		{
			PF_Logger.Debug("Server start notification skipped: Discord endpoint not configured or disabled");
			return;
		}

		if (discordEp.ApiKey == "")
		{
			PF_Logger.Error("Server start notification failed: Discord ApiKey (webhook_id/webhook_token) is empty");
			return;
		}

		string webhookId;
		string webhookToken;

		if (!ParseWebhookApiKey(discordEp.ApiKey, webhookId, webhookToken))
		{
			PF_Logger.Error("Server start notification failed: ApiKey must be in format 'webhook_id/webhook_token'");
			return;
		}

		PF_DiscordWebhook webhook = new PF_DiscordWebhook(webhookId, webhookToken);

		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";

		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetColor(3066993);
		embed.SetTitle("Server Online");
		embed.SetDescription("**" + config.ServerName + "** ist jetzt erreichbar!");

		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		string ts = year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
		embed.SetTimestamp(ts);

		embed.SetAuthor("Psyerns Framework");

		webhook.Send(payload);
		PF_Logger.Log("Server start notification sent for: " + config.ServerName);
	}

	protected bool ParseWebhookApiKey(string apiKey, out string webhookId, out string webhookToken)
	{
		webhookId = "";
		webhookToken = "";

		int slashPos = apiKey.IndexOf("/");
		if (slashPos <= 0 || slashPos >= apiKey.Length() - 1)
			return false;

		webhookId = apiKey.Substring(0, slashPos);
		webhookToken = apiKey.Substring(slashPos + 1, apiKey.Length() - slashPos - 1);
		return true;
	}

	override void OnMissionFinish()
	{
		if (m_PF_QueueProcessor)
			m_PF_QueueProcessor.Stop();

		PF_Logger.Log("Psyerns Framework shutdown.");
		super.OnMissionFinish();
	}
}

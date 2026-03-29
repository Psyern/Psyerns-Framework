modded class MissionServer
{
	protected ref PF_WebQueueProcessor m_PF_QueueProcessor;
	protected static ref PF_DiscordWebhook s_PF_PendingStartWebhook;
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

		GetRPCManager().AddRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_REQUEST, this, SingleplayerExecutionType.Server);

		PF_ServerNotifications.Init();
		PF_ServerNotifications.CheckModUpdates();
	}

	void PF_ReloadRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Server || !sender)
			return;

		string playerName = sender.GetName();
		string playerGUID = sender.GetId();

		PF_Logger.Log("Config reload request from: " + playerName + " (" + playerGUID + ")");

		PF_WebConfig config = PF_WebConfig.GetInstance();
		if (!config.IsAdmin(playerGUID))
		{
			PF_Logger.Log("Reload denied — not an admin: " + playerName);
			Param2<bool, string> deny = new Param2<bool, string>(false, "Not authorized");
			GetRPCManager().SendRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, deny, true, sender);
			return;
		}

		PF_WebConfig.Reload();
		PF_Logger.Init(PF_WebConfig.GetInstance().EnableDebugLogging);

		Param2<bool, string> ok = new Param2<bool, string>(true, "Config reloaded!");
		GetRPCManager().SendRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, ok, true, sender);
		PF_Logger.Log("Config reloaded by admin: " + playerName);
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

		PF_ServerNotifications.OnUpdate(timeslice);
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
		s_PF_PendingStartWebhook = webhook;

		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";
		if (config.DiscordAvatarUrl != "")
			payload.avatar_url = config.DiscordAvatarUrl;

		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetColor(3066993);
		embed.SetTitle("Server Online");
		embed.SetDescription("**" + config.ServerName + "** ist jetzt erreichbar!");

		// Endpoint Status Fields
		string wpDot = GetEndpointStatusDot(config, "WordPress");
		string lbDot = GetEndpointStatusDot(config, "Leaderboard");
		string statusText = wpDot + " WordPress\n" + lbDot + " Leaderboard";

		if (config.EnableServerStatus)
			statusText = statusText + "\n:green_circle: Server Status";

		if (config.EnableKillFeed)
			statusText = statusText + "\n:green_circle: KillFeed";

		if (config.EnableDiscordEvents)
			statusText = statusText + "\n:green_circle: Discord Events";

		embed.AddField("Verbindungen", statusText, false);

		embed.SetAuthor("Psyerns Framework");

		webhook.Send(payload);
		PF_Logger.Log("Server start notification sent for: " + config.ServerName);
	}

	protected string GetEndpointStatusDot(PF_WebConfig config, string name)
	{
		PF_WebEndpoint ep = config.GetEndpoint(name);
		if (!ep)
			return ":red_circle:";

		if (!ep.Enabled)
			return ":red_circle:";

		return ":green_circle:";
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
		super.OnMissionFinish();

		PF_ServerNotifications.SendServerStopNotification();

		if (m_PF_QueueProcessor)
			m_PF_QueueProcessor.Stop();

		PF_Logger.Log("Psyerns Framework shutdown.");
	}
}

/**
 * PF_ServerNotifications — Central class for all server webhook notifications
 *
 * Handles: Server Stop, Heartbeat, Mod Update Detection
 * Server Lock/Unlock: Not implementable — DayZ provides no script-level
 * lock/unlock event or API. BattlEye manages this externally.
 */
class PF_ServerNotifications
{
	protected static float s_HeartbeatTimer;
	protected static float s_HeartbeatInterval;
	protected static float s_ServerStartTime;

	/**
	 * Initialize notifications. Called from PF_MissionInit.OnInit().
	 */
	static void Init()
	{
		s_HeartbeatTimer = 0;
		s_ServerStartTime = GetGame().GetTickTime();
		PF_WebConfig config = PF_WebConfig.GetInstance();
		s_HeartbeatInterval = config.HeartbeatIntervalSeconds;
		if (s_HeartbeatInterval < 10)
			s_HeartbeatInterval = 60;
		PF_Logger.Debug("ServerNotifications initialized. Heartbeat interval: " + s_HeartbeatInterval.ToString() + "s");
	}

	/**
	 * Tick heartbeat timer. Called from MissionServer.OnUpdate().
	 */
	static void OnUpdate(float timeslice)
	{
		PF_WebConfig config = PF_WebConfig.GetInstance();
		if (!config.EnableHeartbeat)
			return;

		s_HeartbeatTimer = s_HeartbeatTimer + timeslice;
		if (s_HeartbeatTimer >= s_HeartbeatInterval)
		{
			s_HeartbeatTimer = 0;
			SendHeartbeat();
		}
	}

	/**
	 * Send server stop notification to Discord. Called from OnMissionFinish().
	 */
	static void SendServerStopNotification()
	{
		PF_WebConfig config = PF_WebConfig.GetInstance();
		if (!config.EnableServerStopNotification)
			return;

		string webhookId;
		string webhookToken;
		if (!GetDiscordWebhook(webhookId, webhookToken))
			return;

		float uptimeSeconds = GetGame().GetTickTime() - s_ServerStartTime;
		int uptimeMin = (int)(uptimeSeconds / 60.0);
		int uptimeH = uptimeMin / 60;
		int uptimeM = uptimeMin % 60;
		string uptimeStr = uptimeH.ToString() + "h " + uptimeM.ToString() + "m";

		PF_DiscordWebhook webhook = new PF_DiscordWebhook(webhookId, webhookToken);
		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";
		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetColor(15158332);
		embed.SetTitle("Server Offline");
		embed.SetDescription("**" + config.ServerName + "** is shutting down.");
		embed.AddField("Uptime", uptimeStr, true);
		embed.SetTimestamp(GetTimestamp());
		embed.SetAuthor("Psyerns Framework");
		webhook.Send(payload);
		PF_Logger.Log("Server stop notification sent");
	}

	/**
	 * Send heartbeat POST to WordPress/webhook endpoint.
	 */
	static void SendHeartbeat()
	{
		PF_WebConfig config = PF_WebConfig.GetInstance();
		PF_WebEndpoint wpEp = config.GetEndpoint("WordPress");
		if (!wpEp || !wpEp.Enabled)
		{
			PF_Logger.Debug("Heartbeat skipped: WordPress endpoint not enabled");
			return;
		}

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		int playerCount = players.Count();

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("serverName", config.ServerName);
		b.Add("timestamp", GetTimestamp());
		b.AddInt("playerCount", playerCount);
		string json = b.Build();

		PF_RestBase restBase = new PF_RestBase(wpEp.BaseUrl, wpEp.ApiKey);
		restBase.PostJson("/server/heartbeat", json);
		PF_Logger.Debug("Heartbeat sent: " + playerCount.ToString() + " players");
	}

	/**
	 * Detect mod changes since last start. Called once from OnInit().
	 */
	static void CheckModUpdates()
	{
		PF_WebConfig config = PF_WebConfig.GetInstance();
		if (!config.EnableModUpdateNotification)
			return;

		string webhookId;
		string webhookToken;
		if (!GetDiscordWebhook(webhookId, webhookToken))
			return;

		string modsDir = "$profile:DeadmansEcho\\PsyernsFramework";
		string modsFile = modsDir + "\\last_mods.txt";
		string currentMods = GetCurrentModList();

		if (currentMods == "")
		{
			PF_Logger.Debug("ModUpdate: No mods detected");
			return;
		}

		string previousMods = "";
		if (FileExist(modsFile))
		{
			FileHandle fh = OpenFile(modsFile, FileMode.READ);
			if (fh)
			{
				string line;
				while (FGets(fh, line) >= 0)
				{
					previousMods = previousMods + line;
				}
				CloseFile(fh);
			}
		}

		if (previousMods != "" && previousMods != currentMods)
		{
			PF_DiscordWebhook webhook = new PF_DiscordWebhook(webhookId, webhookToken);
			PF_DiscordPayload payload = new PF_DiscordPayload();
			payload.username = "Psyerns Framework";
			PF_DiscordEmbed embed = payload.CreateEmbed();
			embed.SetColor(3447003);
			embed.SetTitle("Mod Configuration Changed");
			embed.SetDescription("Server mods have been updated since last start.");
			embed.SetTimestamp(GetTimestamp());
			embed.SetAuthor("Psyerns Framework");
			webhook.Send(payload);
			PF_Logger.Log("Mod update notification sent");
		}
		else
		{
			PF_Logger.Debug("ModUpdate: No changes detected");
		}

		FileHandle fw = OpenFile(modsFile, FileMode.WRITE);
		if (fw)
		{
			FPrintln(fw, currentMods);
			CloseFile(fw);
		}
	}

	protected static string GetCurrentModList()
	{
		string mods = "";
		int count = GetGame().ConfigGetChildrenCount("CfgMods");
		for (int i = 0; i < count; i++)
		{
			string name;
			GetGame().ConfigGetChildName("CfgMods", i, name);
			if (name != "")
				mods = mods + name + ";";
		}
		return mods;
	}

	protected static bool GetDiscordWebhook(out string webhookId, out string webhookToken)
	{
		webhookId = "";
		webhookToken = "";
		PF_WebConfig config = PF_WebConfig.GetInstance();
		PF_WebEndpoint discordEp = config.GetEndpoint("Discord");
		if (!discordEp || !discordEp.Enabled || discordEp.ApiKey == "")
			return false;

		int slashPos = discordEp.ApiKey.IndexOf("/");
		if (slashPos <= 0 || slashPos >= discordEp.ApiKey.Length() - 1)
			return false;

		webhookId = discordEp.ApiKey.Substring(0, slashPos);
		webhookToken = discordEp.ApiKey.Substring(slashPos + 1, discordEp.ApiKey.Length() - slashPos - 1);
		return true;
	}

	protected static string GetTimestamp()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
	}
}

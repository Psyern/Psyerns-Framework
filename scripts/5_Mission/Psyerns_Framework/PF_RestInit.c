/**
 * PF_RestInit — Mission-level initialization for all REST API subsystems
 *
 * This modded MissionServer stacks on top of PF_MissionInit (which handles
 * the queue processor and server start notification). It initializes:
 *   - PF_WhitelistManager (if enabled)
 *   - PF_PlayerLookup (if enabled)
 *   - PF_ServerStatus (if enabled, ticked via OnUpdate)
 *   - PF_KillFeedManager (if enabled)
 *   - PF_DiscordIntegration (if enabled)
 *   - PF_AlertSystem (if enabled)
 *
 * All settings come from the unified PsyernsFrameworkConfig.json via PF_RestConfig proxy.
 */
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		if (!GetGame().IsDedicatedServer())
			return;

		PF_RestConfig restCfg = PF_RestConfig.GetInstance();
		string baseUrl = restCfg.GetBaseUrl();
		string apiKey = restCfg.GetApiKey();

		PF_Logger.Log("Initializing REST subsystems...");

		int enabledCount = 0;

		if (restCfg.IsWhitelistEnabled())
		{
			g_PF_WhitelistManager = new PF_WhitelistManager(baseUrl, apiKey);
			enabledCount++;
		}

		if (restCfg.IsPlayerLookupEnabled())
		{
			g_PF_PlayerLookup = new PF_PlayerLookup(baseUrl, apiKey);
			enabledCount++;
		}

		if (restCfg.IsServerStatusEnabled())
		{
			g_PF_ServerStatus = new PF_ServerStatus(baseUrl, apiKey, restCfg.GetServerStatusInterval());
			enabledCount++;
		}

		if (restCfg.IsKillFeedEnabled())
		{
			g_PF_KillFeedManager = new PF_KillFeedManager(restCfg.GetWebhookUrls());
			enabledCount++;
		}

		if (restCfg.IsDiscordEventsEnabled())
		{
			g_PF_DiscordIntegration = new PF_DiscordIntegration(restCfg.GetDiscordWebhookId(), restCfg.GetDiscordWebhookToken());
			enabledCount++;
		}

		if (restCfg.IsAlertSystemEnabled())
		{
			g_PF_AlertSystem = new PF_AlertSystem(restCfg.GetAlertRules());
			enabledCount++;
		}

		PF_Logger.Log("REST initialization complete. " + enabledCount.ToString() + " feature(s) enabled.");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (g_PF_ServerStatus)
			g_PF_ServerStatus.OnUpdate(timeslice);
	}

	override void OnMissionFinish()
	{
		g_PF_WhitelistManager = null;
		g_PF_PlayerLookup = null;
		g_PF_ServerStatus = null;
		g_PF_KillFeedManager = null;
		g_PF_DiscordIntegration = null;
		g_PF_AlertSystem = null;

		PF_Logger.Log("REST subsystems shut down.");
		super.OnMissionFinish();
	}
}

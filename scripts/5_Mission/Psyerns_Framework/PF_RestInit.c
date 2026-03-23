/**
 * PF_RestInit — Mission-level initialization for all REST API subsystems
 *
 * This modded MissionServer stacks on top of PF_MissionInit (which handles
 * the queue processor and server start notification). It initializes:
 *   - PF_RestConfig (loaded from JSON)
 *   - PF_WhitelistManager (if enabled)
 *   - PF_PlayerLookup (if enabled)
 *   - PF_ServerStatus (if enabled, ticked via OnUpdate)
 *   - PF_KillFeedManager (if enabled)
 *   - PF_DiscordIntegration (if enabled)
 *   - PF_AlertSystem (if enabled)
 */
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		if (!GetGame().IsDedicatedServer())
			return;

		PF_RestConfig restCfg = PF_RestConfig.GetInstance();
		Print("[PF-REST] Initializing REST subsystems...");

		int enabledCount = 0;

		// Feature 1: Whitelist API
		if (restCfg.EnableWhitelist)
		{
			g_PF_WhitelistManager = new PF_WhitelistManager(restCfg.BaseUrl, restCfg.ApiKey);
			enabledCount++;
		}

		// Feature 2: Player Lookup
		if (restCfg.EnablePlayerLookup)
		{
			g_PF_PlayerLookup = new PF_PlayerLookup(restCfg.BaseUrl, restCfg.ApiKey);
			enabledCount++;
		}

		// Feature 3: Server Status
		if (restCfg.EnableServerStatus)
		{
			g_PF_ServerStatus = new PF_ServerStatus(restCfg.BaseUrl, restCfg.ApiKey, restCfg.ServerStatusIntervalSeconds);
			enabledCount++;
		}

		// Feature 4: Kill Feed
		if (restCfg.EnableKillFeed)
		{
			g_PF_KillFeedManager = new PF_KillFeedManager(restCfg.WebhookUrls);
			enabledCount++;
		}

		// Feature 5: Discord Integration
		if (restCfg.EnableDiscordEvents)
		{
			g_PF_DiscordIntegration = new PF_DiscordIntegration(restCfg.DiscordWebhookId, restCfg.DiscordWebhookToken);
			enabledCount++;
		}

		// Feature 6: Alert System
		if (restCfg.EnableAlertSystem)
		{
			g_PF_AlertSystem = new PF_AlertSystem(restCfg.AlertRules);
			enabledCount++;
		}

		Print("[PF-REST] REST initialization complete. " + enabledCount.ToString() + " feature(s) enabled.");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		// Tick server status timer
		if (g_PF_ServerStatus)
			g_PF_ServerStatus.OnUpdate(timeslice);
	}

	override void OnMissionFinish()
	{
		// Cleanup singleton refs
		g_PF_WhitelistManager = null;
		g_PF_PlayerLookup = null;
		g_PF_ServerStatus = null;
		g_PF_KillFeedManager = null;
		g_PF_DiscordIntegration = null;
		g_PF_AlertSystem = null;

		Print("[PF-REST] REST subsystems shut down.");
		super.OnMissionFinish();
	}
}

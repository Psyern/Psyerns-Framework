/**
 * PF_KillFeedHook — Hooks into PlayerBase.EEKilled() to capture kill events
 *
 * When a player dies, this forwards the event to PF_KillFeedManager (if enabled)
 * and to PF_DiscordIntegration (if enabled) and PF_AlertSystem (if enabled).
 *
 * Note: Boss kills are handled by DME-WAR (#ifdef DME_War) directly via
 * DMEW_CheckBossKillWebhook → PF_DiscordIntegration. This hook skips
 * AI-killed-player events when DME_War is loaded to avoid duplicates.
 */
modded class PlayerBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!g_Game || !g_Game.IsDedicatedServer())
			return;

		string victimName = "Unknown";
		string killerName = "Unknown";
		bool killerIsPlayer = false;
		bool killerIsAI = false;

		if (GetIdentity())
			victimName = GetIdentity().GetName();

		PlayerBase killerPlayer;
		if (Class.CastTo(killerPlayer, killer) && killerPlayer.GetIdentity())
		{
			killerName = killerPlayer.GetIdentity().GetName();
			killerIsPlayer = true;
		}
		else if (killer)
		{
			killerName = killer.GetType();
			killerIsAI = true;
		}

		// Kill Feed webhook
		PF_KillFeedManager killFeed = GetPF_KillFeedManager();
		if (killFeed)
		{
			killFeed.OnPlayerKilled(this, killer);
		}

		// Discord notification
		PF_DiscordIntegration discord = GetPF_DiscordIntegration();
		if (discord)
		{
			#ifdef DME_War
			if (killerIsAI)
			{
				PF_Logger.Debug("Kill webhook skipped (AI kill handled by DME-WAR): " + killerName + " -> " + victimName);
			}
			else
			{
				discord.Send("player.kill", "**" + killerName + "** killed **" + victimName + "**");
			}
			#else
			string msg;
			if (killerIsPlayer)
				msg = "**" + killerName + "** killed **" + victimName + "**";
			else
				msg = "**" + victimName + "** was killed by **" + killerName + "**";
			discord.Send("player.kill", msg);
			#endif
		}

		// Alert system trigger
		PF_AlertSystem alerts = GetPF_AlertSystem();
		if (alerts)
		{
			alerts.CheckTrigger("kill", this, GetPosition());
		}
	}
}

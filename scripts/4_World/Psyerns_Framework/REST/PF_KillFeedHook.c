/**
 * PF_KillFeedHook — Hooks into PlayerBase.EEKilled() to capture kill events
 *
 * When a player dies, this forwards the event to PF_KillFeedManager (if enabled)
 * and to PF_DiscordIntegration (if enabled) and PF_AlertSystem (if enabled).
 *
 * Uses modded PlayerBase instead of MissionServer.OnPlayerKilled since
 * vanilla MissionServer does not provide a kill callback.
 */
modded class PlayerBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (!GetGame().IsDedicatedServer())
			return;

		// Kill Feed webhook
		PF_KillFeedManager killFeed = GetPF_KillFeedManager();
		if (killFeed)
		{
			killFeed.OnPlayerKilled(this, killer);
		}

		// Discord notification for kill events
		PF_DiscordIntegration discord = GetPF_DiscordIntegration();
		if (discord)
		{
			string victimName = "Unknown";
			string killerName = "Unknown";

			if (GetIdentity())
				victimName = GetIdentity().GetName();

			PlayerBase killerPlayer;
			if (Class.CastTo(killerPlayer, killer) && killerPlayer.GetIdentity())
				killerName = killerPlayer.GetIdentity().GetName();
			else if (killer)
				killerName = killer.GetType();

			discord.Send("player.kill", "**" + killerName + "** killed **" + victimName + "**");
		}

		// Alert system trigger
		PF_AlertSystem alerts = GetPF_AlertSystem();
		if (alerts)
		{
			alerts.CheckTrigger("kill", this, GetPosition());
		}
	}
}

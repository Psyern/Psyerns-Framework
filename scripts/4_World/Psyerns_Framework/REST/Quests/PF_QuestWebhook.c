#ifdef EXPANSIONMODQUESTS
/**
 * PF_QuestWebhook — Quest completion webhook for Expansion Quests
 *
 * Hooks into Expansion's MissionBaseWorld.Expansion_OnQuestCompletion() callback.
 * Sends a Discord embed when a player completes (turns in) a quest.
 */
modded class MissionBaseWorld
{
	void Expansion_OnQuestCompletion(ExpansionQuest quest)
	{

		if (!GetGame().IsDedicatedServer())
			return;

		PF_WebConfig config = PF_WebConfig.GetInstance();
		if (!config.EnableQuestNotifications)
			return;

		if (!quest)
			return;

		ExpansionQuestConfig questConfig = quest.GetQuestConfig();
		if (!questConfig)
			return;

		string playerName = "Unknown";
		string playerUID = quest.GetPlayerUID();
		PlayerBase player = PlayerBase.GetPlayerByUID(playerUID);
		if (player && player.GetIdentity())
			playerName = player.GetIdentity().GetName();

		string questTitle = questConfig.GetTitle();
		int questId = questConfig.GetID();

		PF_Logger.Log("Quest completed: " + questTitle + " by " + playerName);

		PF_WebEndpoint discordEp = config.GetEndpoint("Discord");
		if (!discordEp || !discordEp.Enabled || discordEp.ApiKey == "")
			return;

		string webhookId;
		string webhookToken;
		int slashPos = discordEp.ApiKey.IndexOf("/");
		if (slashPos <= 0 || slashPos >= discordEp.ApiKey.Length() - 1)
			return;

		webhookId = discordEp.ApiKey.Substring(0, slashPos);
		webhookToken = discordEp.ApiKey.Substring(slashPos + 1, discordEp.ApiKey.Length() - slashPos - 1);

		PF_DiscordWebhook webhook = new PF_DiscordWebhook(webhookId, webhookToken);
		PF_DiscordPayload payload = new PF_DiscordPayload();
		payload.username = "Psyerns Framework";

		PF_DiscordEmbed embed = payload.CreateEmbed();
		embed.SetColor(10181046);
		embed.SetTitle("Quest Completed");
		embed.SetDescription("**" + playerName + "** completed **" + questTitle + "**");
		embed.AddField("Quest ID", questId.ToString(), true);
		embed.AddField("Player", playerName, true);

		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		string ts = year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
		embed.SetTimestamp(ts);
		embed.SetAuthor("Psyerns Framework");

		webhook.Send(payload);
		PF_Logger.Log("Quest completion webhook sent for: " + questTitle);
	}
}
#endif

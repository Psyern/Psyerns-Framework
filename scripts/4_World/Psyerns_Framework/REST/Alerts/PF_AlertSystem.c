// --- Singleton accessor ---
ref PF_AlertSystem g_PF_AlertSystem;

PF_AlertSystem GetPF_AlertSystem()
{
	return g_PF_AlertSystem;
}

/**
 * PF_AlertRule — Runtime representation of an alert rule
 * Built from PF_AlertRuleConfig during initialization.
 */
class PF_AlertRule
{
	string triggerType;   // zone_enter, kill, loot
	float radius;
	vector position;
	string webhookUrl;
	string messageTemplate;

	// Webhook infrastructure for this rule
	protected RestApi m_RestApi;
	protected RestContext m_RestCtx;

	void PF_AlertRule(PF_AlertRuleConfig cfg)
	{
		triggerType = cfg.TriggerType;
		radius = cfg.Radius;
		position = Vector(cfg.PosX, cfg.PosY, cfg.PosZ);
		webhookUrl = cfg.WebhookUrl;
		messageTemplate = cfg.MessageTemplate;

		if (webhookUrl != "")
		{
			m_RestApi = CreateRestApi();
			m_RestCtx = m_RestApi.GetRestContext(webhookUrl);
			m_RestCtx.SetHeader("application/json");
		}
	}

	/**
	 * Check if a trigger event at the given position falls within this rule's radius
	 */
	bool IsInRange(vector pos)
	{
		if (radius <= 0)
			return true;

		return vector.Distance(position, pos) <= radius;
	}

	/**
	 * Build the alert message from the template, replacing placeholders
	 * Supported placeholders: {playerName}, {posX}, {posY}, {posZ}, {triggerType}
	 */
	string BuildMessage(string playerName, vector pos)
	{
		string msg = messageTemplate;
		msg.Replace("{playerName}", playerName);
		msg.Replace("{posX}", pos[0].ToString());
		msg.Replace("{posY}", pos[1].ToString());
		msg.Replace("{posZ}", pos[2].ToString());
		msg.Replace("{triggerType}", triggerType);
		return msg;
	}

	/**
	 * Fire this rule's webhook with the alert data
	 */
	void Fire(string playerName, vector pos)
	{
		if (!m_RestCtx)
		{
			Print("[PF-REST] [ERROR] AlertRule::Fire — no RestContext (webhookUrl empty?)");
			return;
		}

		string message = BuildMessage(playerName, pos);

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("triggerType", triggerType);
		b.Add("playerName", playerName);
		b.AddFloat("posX", pos[0]);
		b.AddFloat("posY", pos[1]);
		b.AddFloat("posZ", pos[2]);
		b.Add("message", message);
		string json = b.Build();

		PF_RestCallback cb = new PF_RestCallback();
		m_RestCtx.POST(cb, "", json);
		Print("[PF-REST] Alert fired: " + triggerType + " — " + message);
	}
}

/**
 * PF_AlertSystem — Manages alert rules and evaluates triggers
 *
 * Rules are loaded from PF_RestConfig.AlertRules on initialization.
 *
 * Usage:
 *   GetPF_AlertSystem().CheckTrigger("zone_enter", player, playerPos);
 *   GetPF_AlertSystem().CheckTrigger("kill", player, killPos);
 *   GetPF_AlertSystem().CheckTrigger("loot", player, lootPos);
 *
 * For each matching rule (triggerType matches AND position within radius),
 * the alert fires its webhook with the populated message template.
 */
class PF_AlertSystem
{
	protected ref array<ref PF_AlertRule> m_Rules;

	void PF_AlertSystem(array<ref PF_AlertRuleConfig> ruleConfigs)
	{
		m_Rules = new array<ref PF_AlertRule>();

		if (ruleConfigs)
		{
			for (int i = 0; i < ruleConfigs.Count(); i++)
			{
				PF_AlertRuleConfig cfg = ruleConfigs[i];
				if (!cfg || cfg.TriggerType == "" || cfg.WebhookUrl == "")
					continue;

				PF_AlertRule rule = new PF_AlertRule(cfg);
				m_Rules.Insert(rule);
			}
		}

		Print("[PF-REST] AlertSystem initialized with " + m_Rules.Count().ToString() + " rule(s)");
	}

	/**
	 * Evaluate all rules for a given trigger event
	 * @param triggerType  One of: zone_enter, kill, loot
	 * @param player       The player involved in the event
	 * @param pos          World position of the event
	 */
	void CheckTrigger(string triggerType, PlayerBase player, vector pos)
	{
		if (!player)
			return;

		string playerName = "Unknown";
		if (player.GetIdentity())
			playerName = player.GetIdentity().GetName();

		for (int i = 0; i < m_Rules.Count(); i++)
		{
			PF_AlertRule rule = m_Rules[i];
			if (!rule)
				continue;

			if (rule.triggerType != triggerType)
				continue;

			if (!rule.IsInRange(pos))
				continue;

			rule.Fire(playerName, pos);
		}
	}

	/**
	 * Get the total number of loaded rules
	 */
	int GetRuleCount()
	{
		return m_Rules.Count();
	}

	/**
	 * Get rules matching a specific trigger type
	 */
	int GetRuleCountForType(string triggerType)
	{
		int count = 0;
		for (int i = 0; i < m_Rules.Count(); i++)
		{
			if (m_Rules[i].triggerType == triggerType)
				count++;
		}
		return count;
	}
}

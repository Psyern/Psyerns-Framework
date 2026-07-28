/**
 * PF_AH_Sync — AuctionHouse integration orchestrator.
 *
 * Lifecycle:
 *   Init()     — loads config, wires up uploader/poller/executor if Enabled
 *   OnUpdate() — ticks two independent timers (push + poll) using the same
 *                timeslice pattern as PF_LeaderboardExport / PF_ServerStatus
 *   Shutdown() — clears references so GC can reclaim
 *
 * Server-only: this module is instantiated only from PF_RestInit's modded
 * MissionServer.OnInit(), which is already gated on g_Game.IsDedicatedServer().
 *
 * Disabled gracefully when:
 *   - Config.Enabled == false
 *   - Config.WpUrl == "" or default placeholder
 *   - Config.ApiKey == ""
 */
class PF_AH_Sync
{
	protected static ref PF_AH_Sync s_Instance;

	protected ref PF_AH_Config m_Config;
	protected ref PF_AH_Uploader m_Uploader;
	protected ref PF_AH_PendingPoller m_Poller;
	protected ref PF_AH_ActionExecutor m_Executor;

	protected float m_PushTimer;
	protected float m_PollTimer;
	protected bool m_Active;

	static PF_AH_Sync GetInstance()
	{
		if (!s_Instance)
			s_Instance = new PF_AH_Sync();
		return s_Instance;
	}

	void PF_AH_Sync()
	{
		m_PushTimer = 0;
		m_PollTimer = 0;
		m_Active = false;
	}

	void Init()
	{
		if (!g_Game || !g_Game.IsDedicatedServer())
			return;

		m_Config = PF_AH_Config.Load();

		if (!m_Config.Enabled)
		{
			PF_Logger.Log("AH: Integration disabled via config.");
			return;
		}

		if (m_Config.WpUrl == "" || m_Config.WpUrl == "https://your-site.com")
		{
			PF_Logger.Log("AH: [WARN] WpUrl not configured — integration will not start.");
			return;
		}

		if (m_Config.ApiKey == "")
		{
			PF_Logger.Log("AH: [WARN] ApiKey is empty — integration will not start.");
			return;
		}

		m_Executor = new PF_AH_ActionExecutor();
		m_Uploader = new PF_AH_Uploader(m_Config.WpUrl, m_Config.ApiKey, m_Config.CurrencyMode);
		m_Poller = new PF_AH_PendingPoller(m_Config.WpUrl, m_Config.ApiKey, m_Executor);

		m_PushTimer = 0;
		m_PollTimer = 0;
		m_Active = true;

		PF_Logger.Log("AH: Integration started (push=" + m_Config.PushIntervalSeconds.ToString() + "s, poll=" + m_Config.PollIntervalSeconds.ToString() + "s, currency=" + m_Config.CurrencyMode + ")");
	}

	void OnUpdate(float timeslice)
	{
		if (!m_Active)
			return;

		m_PushTimer += timeslice;
		if (m_PushTimer >= m_Config.PushIntervalSeconds)
		{
			m_PushTimer = 0;
			if (m_Uploader)
				m_Uploader.Upload();
		}

		m_PollTimer += timeslice;
		if (m_PollTimer >= m_Config.PollIntervalSeconds)
		{
			m_PollTimer = 0;
			if (m_Poller)
				m_Poller.Poll();
		}
	}

	void Shutdown()
	{
		m_Active = false;
		m_Uploader = null;
		m_Poller = null;
		m_Executor = null;
		PF_Logger.Log("AH: Integration shut down.");
	}

	bool IsActive()
	{
		return m_Active;
	}
}

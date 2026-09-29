modded class MissionBase
{
	// Client RPC handler (Psyerns Core router); null on the dedicated server.
	protected ref PF_ReloadRpc m_PF_ReloadRpcClient;

	override void OnInit()
	{
		super.OnInit();

		if (!g_Game || !g_Game.IsDedicatedServer())
		{
			m_PF_ReloadRpcClient = new PF_ReloadRpc(false);
		}
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (g_Game && g_Game.IsDedicatedServer())
			return;

		UAInput reloadInput = GetUApi().GetInputByName("PF_ReloadConfig");
		if (reloadInput && reloadInput.LocalPress())
		{
			PF_ReloadRpc.SendReloadRequest();
			if (g_Game && g_Game.GetMission())
				g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Psyerns Framework: Reload requested...", ""));
		}
	}
}

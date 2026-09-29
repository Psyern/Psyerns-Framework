/**
 * PF_ReloadRpc - config reload request/response via the Psyerns Core RPC router.
 *
 * Namespace and function names are unchanged from the former CF RPC channel:
 *   PF_RPC_CHANNEL ("Psyerns_Framework") / PF_RPC_RELOAD_REQUEST   client -> server, no payload
 *   PF_RPC_CHANNEL ("Psyerns_Framework") / PF_RPC_RELOAD_RESPONSE  server -> client, Param2<bool, string>
 *
 * One instance lives on the server (MissionServer) and one on the client (MissionBase on non-dedicated
 * machines); each registers only the handler of its side. The router holds handlers weakly, so the
 * owning mission keeps the instance alive as a ref member.
 */
class PF_ReloadRpc : Managed
{
	void PF_ReloadRpc(bool isServer)
	{
		if (isServer)
			PsyCore_RPC.Register(PF_RPC_CHANNEL, PF_RPC_RELOAD_REQUEST, this, "OnReloadRequest", PsyCore_RpcFlags.SERVER);
		else
			PsyCore_RPC.Register(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, this, "OnReloadResponse", PsyCore_RpcFlags.CLIENT);
	}

	// ---- server -------------------------------------------------------------------------

	// Answered for admins and non-admins alike (the client shows the result in chat).
	void OnReloadRequest(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (!sender)
			return;

		string playerName = sender.GetName();
		string playerGUID = sender.GetId();

		PF_Logger.Log("Config reload request from: " + playerName + " (" + playerGUID + ")");

		if (!PsyCore_Admin.IsAdmin(sender))
		{
			PF_Logger.Log("Reload denied — not an admin: " + playerName);
			Param2<bool, string> deny = new Param2<bool, string>(false, "Not authorized");
			PsyCore_RPC.SendParam(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, deny, true, sender);
			return;
		}

		PF_WebConfig.Reload();
		PF_Logger.Init(PF_WebConfig.GetInstance().EnableDebugLogging);

		Param2<bool, string> ok = new Param2<bool, string>(true, "Config reloaded!");
		PsyCore_RPC.SendParam(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, ok, true, sender);
		PF_Logger.Log("Config reloaded by admin: " + playerName);
	}

	// ---- client -------------------------------------------------------------------------

	void OnReloadResponse(ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		Param2<bool, string> data;
		if (!ctx.Read(data) || !data)
			return;

		bool success = data.param1;
		string message = data.param2;

		string prefix = "Psyerns Framework: ";
		if (g_Game && g_Game.GetMission())
		{
			if (success)
				g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", prefix + message, ""));
			else
				g_Game.GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", prefix + "ERROR: " + message, ""));
		}
	}

	// Client -> server (no payload).
	static void SendReloadRequest()
	{
		PsyCore_RPC.SendEmpty(PF_RPC_CHANNEL, PF_RPC_RELOAD_REQUEST);
	}
}

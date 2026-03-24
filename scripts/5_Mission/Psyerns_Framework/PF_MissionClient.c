modded class MissionBase
{
	override void OnInit()
	{
		super.OnInit();

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			GetRPCManager().AddRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, this, SingleplayerExecutionType.Client);
		}
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (!GetGame().IsClient() && GetGame().IsMultiplayer())
			return;

		UAInput reloadInput = GetUApi().GetInputByName("PF_ReloadConfig");
		if (reloadInput && reloadInput.LocalPress())
		{
			GetRPCManager().SendRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_REQUEST, null, true);
			GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", "Psyerns Framework: Reload requested...", ""));
		}
	}

	void PF_ReloadResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
	{
		if (type != CallType.Client)
			return;

		Param2<bool, string> data;
		if (!ctx.Read(data))
			return;

		bool success = data.param1;
		string message = data.param2;

		string prefix = "Psyerns Framework: ";
		if (success)
			GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", prefix + message, ""));
		else
			GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(CCDirect, "", prefix + "ERROR: " + message, ""));
	}
}

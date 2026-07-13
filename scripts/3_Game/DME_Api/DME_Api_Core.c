/**
 * Psyerns_Framework — DME_Api
 *
 * Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)
 *   https://github.com/daemonforge/DayZ-UniveralApi
 * Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo
 * Modified by Deadmans Echo, 2026.   [AGPL §5(a) change notice]
 *
 * This file is part of Psyerns_Framework and is licensed under the
 * GNU Affero General Public License v3.0. See LICENSE in the repo root.
 * SPDX-License-Identifier: AGPL-3.0-only
 */
class DME_Api_Core extends Managed {

	//Getter function for the Database Endpoint using either DME_API_OBJECT_DB or DME_API_PLAYER_DB
	// "DME_API_PLAYER_DB" is only accessable on client for the player info being requested
	// "DME_API_OBJECT_DB" all clients can access all data.
	DME_Api_DBEndpoint db(int collection = DME_API_OBJECT_DB){
		if (collection == DME_API_OBJECT_DB){
			if (!m_ObjectEndPoint){
				m_ObjectEndPoint = new DME_Api_DBEndpoint("Object");
			}
			return m_ObjectEndPoint;
		}
		if (collection == DME_API_PLAYER_DB){
			if (!m_PlayerEndPoint){
				m_PlayerEndPoint = new DME_Api_DBEndpoint("Player");
			}
			return m_PlayerEndPoint;
		}
		return NULL;
	}

	//Getter function for the Discord Endpoint
	DME_Api_DSEndpoint ds(){
		if (!m_DME_Api_DSEndpoint){
			m_DME_Api_DSEndpoint = new DME_Api_DSEndpoint;
		}
		return m_DME_Api_DSEndpoint;
	}

	//Getter function for the Globals Endpoint
	DME_Api_DBGlobalEndpoint globals(){
		if (!m_DME_Api_DBGlobalEndpoint){
			m_DME_Api_DBGlobalEndpoint = new DME_Api_DBGlobalEndpoint;
		}
		return m_DME_Api_DBGlobalEndpoint;
	}

	//Getter function for the API Endpoint
	DME_Api_APIEndpoint api(){
		if (!m_DME_Api_APIEndpoint){
			m_DME_Api_APIEndpoint = new DME_Api_APIEndpoint;
		}
		return m_DME_Api_APIEndpoint;
	}

	//Request a call to be canceled
	void RequestCallCancel(int cid){
		m_CanceledCalls.Insert(cid);
	}

	//A super simple Post Interface to help people
	static int Post(string url)
	{
		RestContext ctx = RestCore().GetRestContext(url);
		ctx.SetHeader("application/json");
		ctx.POST(new DME_Api_SilentCallBack, "", "{}");
		return 0;
	}

	//A super simple Post Interface to help people
	static int Post(string url, string jsonString, RestCallback UCBX = NULL, string contentType = "application/json")
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx = RestCore().GetRestContext(url);
		ctx.SetHeader(contentType);
		ctx.POST(UCBX, "", jsonString);
		return 0;
	}

	//A super simple Post Interface to help people
	static int Post(string url, string jsonString, DME_Api_CallbackBase cb, string contentType = "application/json")
	{
		int cid = DME_Api().CallId();
		if (cb){
			RestContext ctx = RestCore().GetRestContext(url);
			ctx.SetHeader(contentType);
			ctx.POST(new DME_Api_DBNestedCallBack(cb,cid), "", jsonString);
			return cid;
		}
		return -1;
	}

	//A super simple Get Interface to help people
	static int Get(string url)
	{
		RestContext ctx =  RestCore().GetRestContext(url);
		ctx.GET(new DME_Api_SilentCallBack, "");
		return 0;
	}
	//A super simple Get Interface to help people
	static int Get(string url, RestCallback UCBX)
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx =  RestCore().GetRestContext(url);
		ctx.GET(UCBX , "");
		return 0;
	}
	//A super simple Get Interface to help people
	static int Get(string url, DME_Api_CallbackBase cb)
	{
		int cid = DME_Api().CallId();
		if (cb){
			RestContext ctx =  RestCore().GetRestContext(url);
			ctx.GET(new DME_Api_DBNestedCallBack(cb,cid), "");
			return cid;
		}
		return -1;
	}

	//Will return true if the discord endpoint is configured (this doesn't mean its configured correctly though :p)
	bool IsDiscordEnabled(){
		return m_DME_Api_DiscordEnabled;
	}

	//Will return true if the Translate Endpoint is configured
	bool IsTranslateEnabled(){
		return m_DME_Api_TranslateEnabled;
	}

	//Returns True if the status check has come back and everything is okay
	bool IsOnline(){
		return m_DME_Api_Online;
	}

	//Returns current Version Offset 0 Version Matches exactly
	// -1 or 1 off by a patch this is not a problem and won't cause any major issues
	// -2 or 2 off by a Minor Version this may cause some endpoints to not work or features to be missing
	// -3 or 3 off by a Major Version most likely the mod will not work at all!
	int VersionOffset(){
		return m_DME_Api_VersionOffset;
	}

	//Checks to see if the Random Numbers are below half and add's more
	void CheckAndRenewQRandom(){
		if (Math.QRandomRemaining()<= 2000){
			GetQRandomNumbers();
		}
	}

	//Returns Current Version of the Mod
	static string GetVersion(){
		return DME_API_VERSION;
	}

	//Simple function for finding a player based on their GUID
	static DayZPlayer FindPlayer(string GUID){
		if (!g_Game) return NULL;
		if (g_Game.IsDedicatedServer()){
			array<Man> players = new array<Man>;
			g_Game.GetPlayers( players );
			for (int i = 0; i < players.Count(); i++){
				DayZPlayer player = DayZPlayer.Cast(players.Get(i));
				if (player.GetIdentity() && player.GetIdentity().GetId() == GUID ){
					return player;
				}
			}
		}
		return NULL;
	}

	//Simple function for finding a player based on their identity
	static DayZPlayer FindPlayerByIdentity(PlayerIdentity identity) {
		if (!identity)
			return NULL;
		if (!g_Game) return NULL;

		int highBits;
		int lowBits;
		g_Game.GetPlayerNetworkIDByIdentityID(identity.GetPlayerId(), lowBits, highBits);
		return DayZPlayer.Cast(g_Game.GetObjectByNetworkId(lowBits, highBits));
	}


	//Stuff that you don't need to worry about :P

	protected bool m_IsServer = false;

	protected int m_CallId = 0;
	protected int m_AuthRetries = 0;

	protected bool m_DME_Api_Online = false;
	protected int m_DME_Api_VersionOffset = 0;
	protected bool m_DME_Api_DiscordEnabled = false;
	protected bool m_DME_Api_TranslateEnabled = false;

	protected bool DME_API_Init = false;
	protected ref DME_Api_AuthToken m_authToken;

	protected ref DME_Api_Rest m_DME_Api_Rest;

	protected ref DME_Api_DiscordRest m_DME_Api_DiscordRest;
	protected ref DME_Api_DSEndpoint m_DME_Api_DSEndpoint;
	protected ref DME_Api_DBGlobalEndpoint m_DME_Api_DBGlobalEndpoint;

	protected ref DME_Api_DiscordUser dsUser;

	protected ref map<string, string> PlayerAuths = new map<string, string>;

	protected ref DME_Api_DBEndpoint m_PlayerEndPoint;

	protected ref DME_Api_DBEndpoint m_ObjectEndPoint;
	//Can't Do Globals due to how globals work

	protected ref DME_Api_APIEndpoint m_DME_Api_APIEndpoint;

	protected ref TIntSet m_CanceledCalls = new TIntSet;

	protected int LastRandomNumberRequestCall = -1;


	protected static RestApi RestCore()
	{
		RestApi clCore = GetRestApi();
		if (!clCore) {
			clCore = CreateRestApi();
			clCore.SetOption(ERestOption.ERESTOPTION_READOPERATION, 15);
		}
		return clCore;
	}

	string GetAuthToken(){
		if (m_authToken && !m_IsServer){
			return m_authToken.GetAuthToken();
		} else if (m_IsServer && DME_Api_GetConfig().ServerAuth != ""){
			return DME_Api_GetConfig().ServerAuth;
		}
		return "null";
	}

	bool HasValidAuth(){
		return (GetAuthToken() != "null" && GetAuthToken() != "error" && GetAuthToken() != "ERROR" && GetAuthToken() != "" );
	}


	//OLD RestCallBack Endpoints use if you want to use RestCallBack Classes instead of Function Based
	DME_Api_Rest Rest(){
		if (!m_DME_Api_Rest){
			m_DME_Api_Rest = new DME_Api_Rest;
		}
		return m_DME_Api_Rest;
	}

	DME_Api_DiscordRest Discord(){
		if (!m_DME_Api_DiscordRest){
			m_DME_Api_DiscordRest = new DME_Api_DiscordRest;
		}
		return m_DME_Api_DiscordRest;
	}



	void ~DME_Api_Core(){
		if (m_IsServer && DME_API_Init && g_Game){
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.Remove(this.CheckAndRenewQRandom); }
		}
	}

	void Init(){
		if (!DME_API_Init){
			Print("[DME_Api] First Init");
			DME_API_Init = true;
			if (g_Game){
				m_IsServer = g_Game.IsDedicatedServer();
			}
			if (m_IsServer){
				Print("[DME_Api] Detected Server");
			}
			GetRPCManager().AddRPC( "DME_Api", "RPC_DME_Api_Config", this, SingleplayerExecutionType.Both );
			GetRPCManager().AddRPC( "DME_Api", "RPCRequestQnAConfig", this, SingleplayerExecutionType.Both );
			GetRPCManager().AddRPC( "DME_Api", "RPCRequestAuthToken", this, SingleplayerExecutionType.Both );
			GetRPCManager().AddRPC( "DME_Api", "RPCRequestRetry", this, SingleplayerExecutionType.Both );
			if (m_IsServer && g_Game){
				DME_Api().api().Status(this, "CBStatusCheck");
				CheckAndRenewQRandom();
				ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
				if (queue){ queue.CallLater(this.CheckAndRenewQRandom, 10 * 60 * 1000, true); }
			}
		}
	}

	protected void RPC_DME_Api_Config( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if (!g_Game) return;
		Print("[DME_Api] Received DME_Api Config");
		Param2<DME_Api_AuthToken, DME_Api_Config> data;
		if ( !ctx.Read( data ) ) return;
		m_AuthRetries = 0;
		m_authToken = data.param1;
		m_DME_Api_Config = data.param2;
		ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
		if (queue){ queue.Call(this.OnTokenReceived); }
	}

	protected void OnTokenReceived(){
		if (!g_Game) return;
		DME_Api().api().Status(this, "CBStatusCheck");
		if (m_DME_Api_Config.QnAEnabled){
			GetRPCManager().SendRPC("DME_Api", "RPCRequestQnAConfig", new Param1<DME_Api_QnAMakerServerAnswers>(NULL), true);
		}
		DME_Api().ds().GetUser(GetDayZGame().GetSteamId(), GetDayZGame(), "CBCacheDiscordInfo");
		g_Game.GameScript.CallFunction(g_Game.GetMission(), "DME_Api_ReadyTokenReceived", NULL, NULL);
		CheckAndRenewQRandom();
		Print("[DME_Api] OnTokenReceived Proccessed");
	}


	void RPCRequestRetry( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target ) {
		if (!g_Game) return;
		if (!g_Game.IsDedicatedServer() && ++m_AuthRetries <= 20){
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.CallLater(this.RequestAuthToken, m_AuthRetries * 2200, false, true); }
		}
	}

	void RequestAuthToken(bool first = false){
		if (!m_IsServer){
			GetRPCManager().SendRPC("DME_Api", "RPCRequestAuthToken", new Param1<bool>(first), true);
		}
	}

	void PreparePlayerAuth(string guid){
		this.Rest().GetAuth(guid);
	}

	void AddPlayerAuth(string guid, string auth){
		if (!PlayerAuths){PlayerAuths = new map<string, string>;}
		//Print("[DME_Api] Adding PlayerAuth for " + guid + " to cache");
		PlayerAuths.Set(guid,auth); //Set Auth incase a request comes in.

		DayZPlayer player; //If renewing or if player is availbe send to player
		if (Class.CastTo(player, FindPlayer(guid)) && player.GetIdentity() ){
			SendAuthToken(player.GetIdentity(), auth);
		}
	}

	bool GetPlayerAuth(string guid, out string auth){
		if (PlayerAuths && PlayerAuths.Contains(guid)){
			auth = PlayerAuths.Get(guid);
			return true;
		}
		Print("[DME_Api] Failed to find Player Auth for " + guid);
		return false;
	}

	protected void RPCRequestQnAConfig( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if (!g_Game) return;
		Param1<DME_Api_QnAMakerServerAnswers> data;
		if ( !ctx.Read( data ) ) return;
		if (!g_Game.IsDedicatedServer() && !PlayerIdentity.Cast(sender) && data.param1){
			m_QnAMakerServerAnswers = data.param1;
		} else if (g_Game.IsDedicatedServer() && PlayerIdentity.Cast(sender) && data.param1 == NULL){
			GetRPCManager().SendRPC("DME_Api", "RPCRequestQnAConfig", new Param1<DME_Api_QnAMakerServerAnswers>(m_QnAMakerServerAnswers), true, sender);
		}
	}

	protected void RPCRequestAuthToken( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		Param1<bool> data;
		if ( !ctx.Read( data ) ) return;
		PlayerIdentity identity = PlayerIdentity.Cast(sender);
		if (m_IsServer && identity){
			string authtoken = "";
			if (DME_Api_GetConfig().ServerAuth != "" && DME_Api_GetConfig().ServerAuth != "null" ){
				if (data.param1 && GetPlayerAuth(identity.GetId(), authtoken)){
					//Print("[DME_Api] RPCRequestAuthToken Sending Cached Token ");
					SendAuthToken(identity, authtoken);
				} else if (FindPlayer(identity.GetId())){
					//Print("[DME_Api] RPCRequestAuthToken  Renewing Auth Token" );
					PreparePlayerAuth(identity.GetId());
				}  else {
					Print("[DME_Api] RPCRequestAuthToken Requesting client retry." );
					GetRPCManager().SendRPC("DME_Api", "RPCRequestRetry", new Param1<bool>(true), true, identity);
				}
			} else {
				Error("[DME_Api] Server Auth is empty or null");
			}
		}
	}

	protected void SendAuthToken(PlayerIdentity idenitity, string auth){
		if (idenitity && auth != ""){
			Print("[DME_Api] Sending PlayerAuth Token to " + idenitity.GetId());
			DME_Api_Config clientConfig = new DME_Api_Config;
			clientConfig.ConfigVersion = DME_Api_GetConfig().ConfigVersion;
			clientConfig.ServerURL = DME_Api_GetConfig().ServerURL;
			clientConfig.ServerID = DME_Api_GetConfig().ServerID;
			clientConfig.ServerAuth = "null";
			clientConfig.QnAEnabled = DME_Api_GetConfig().QnAEnabled;
			clientConfig.EnableBuiltinLogging = DME_Api_GetConfig().EnableBuiltinLogging;
			clientConfig.PromptDiscordOnConnect = DME_Api_GetConfig().PromptDiscordOnConnect;
			DME_Api_AuthToken clientToken = new DME_Api_AuthToken;
			clientToken.GUID = idenitity.GetId();
			clientToken.AUTH = auth;
			GetRPCManager().SendRPC("DME_Api", "RPC_DME_Api_Config", new Param2<DME_Api_AuthToken, DME_Api_Config>(clientToken, clientConfig), true, idenitity);
		} else {
			Print("[DME_Api] [DME_Api_AuthCallBack] ERROR ");
			if (idenitity){
				DME_Api().AuthError(idenitity.GetId());
			}
		}
	}

	void AuthError(string guid){
		if (!g_Game) return;
		Print("[DME_Api] Auth Error for " + guid);
		//If Auth Token Failed just try again in 3 minutes
		if (guid != "" && IsOnline()){
			ScriptCallQueue queueAuth = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queueAuth){ queueAuth.CallLater(Rest().GetAuth, 180 * 1000, false, guid); }
		}
		if (!m_IsServer && !IsOnline()){
			DME_Api().api().Status(this, "CBStatusCheck");
			ScriptCallQueue queueStatus = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queueStatus){ queueStatus.CallLater(this.AuthError, 300 * 1000, false, guid); }
		}
	}

	static void DiscordMessage(string webhookUrl, string message, string botName = "", string botAvatarUrl = ""){
		DME_Api_DiscordObject discordMessage = new DME_Api_DiscordObject;
		discordMessage.content = message;
		discordMessage.username = botName;
		discordMessage.avatar_url = botAvatarUrl;
		Post(webhookUrl, discordMessage.ToJson());
	}

	static void DiscordObject(string webhookUrl, DME_Api_DiscordObject discordObject){
		Post(webhookUrl, discordObject.ToJson());
	}


	static string ErrorToString(int ErrorCode){
		switch ( ErrorCode )
		{
			case ERestResultState.EREST_EMPTY:
				return "EREST_EMPTY";
			case ERestResultState.EREST_PENDING:
				return "EREST_PENDING";
			case ERestResultState.EREST_FEEDING:
				return "EREST_FEEDING";
			case ERestResultState.EREST_SUCCESS:
				return "EREST_SUCCESS";
			case ERestResultState.EREST_ERROR:
				return "EREST_ERROR";
			case ERestResultState.EREST_ERROR_CLIENTERROR:
				return "EREST_ERROR_CLIENTERROR";
			case ERestResultState.EREST_ERROR_SERVERERROR:
				return "EREST_ERROR_SERVERERROR";
			case ERestResultState.EREST_ERROR_APPERROR:
				return "EREST_ERROR_APPERROR";
			case ERestResultState.EREST_ERROR_TIMEOUT:
				return "EREST_ERROR_TIMEOUT";
			case ERestResultState.EREST_ERROR_NOTIMPLEMENTED:
				return "EREST_ERROR_NOTIMPLEMENTED";
			case ERestResultState.EREST_ERROR_UNKNOWN:
				return "EREST_ERROR_UNKNOWN";
		}
		return "UNDEFINED_ERROR";
	}

	int CallId(){
		return ++m_CallId;
	}


	bool IsCallCanceled(int cid){
		return (m_CanceledCalls.Find(cid) != -1);
	}

	protected void GetQRandomNumbers(){
		if (LastRandomNumberRequestCall != -1){
			return;
		}
		LastRandomNumberRequestCall = api().RandomNumbersFull(-1, this, "CBRandomNumber");
	}

	protected void CBRandomNumber(int cid, int status, string oid, DME_Api_RandomNumberResponse data){
		LastRandomNumberRequestCall = -1;
		if (status == DME_API_SUCCESS && data){
			Math.AddQRandomNumber(data.Numbers);
			Math.Randomize(Math.QRandom()); //Randomize the Vanilla Randomization a bit more.
			return;
		}
		Print("[DME_Api] Failed to update the Q Random Numbers");
	}

	protected void CBStatusCheck(int cid, int status, string oid, DME_Api_Status data){
		if (status == DME_API_SUCCESS && data){
			if (data.Error == "noerror"){
				m_DME_Api_Online = true;
				Print("[DME_Api] WebService Online Version: " + data.Version + " Mod Version: " + DME_API_VERSION);
			}
			if (data.Error == "noauth"){
				m_DME_Api_Online = false;
				Print("[DME_Api] Auth Key is not vaild");
			}
			if (data.Error == "noerror" && data.Discord == "Enabled"){
				m_DME_Api_DiscordEnabled = true;
			}
			if (data.Error == "noerror" && data.Translate == "Enabled"){
				m_DME_Api_TranslateEnabled = true;
			}
			m_DME_Api_VersionOffset = data.CheckVersion(DME_API_VERSION);
			if (m_DME_Api_VersionOffset > 2){
				Error2("DME_Api WebService Needs Update", "[DME_Api] Webservice is outdated and should be updated right away | WebService Version: " + data.Version + " Mod Version: " + DME_API_VERSION);
				return;
			}
			if (m_DME_Api_VersionOffset > 1){
				Error("[DME_Api] Webservice is outdated and should be updated right away");
				return;
			}
			if (m_DME_Api_VersionOffset > 0){
				Print("[DME_Api] You may want to check for new versions of the DME_Api WebService");
				return;
			}
			if (m_DME_Api_VersionOffset < -2){
				Error2("DME_Api Mod Needs Update", "[DME_Api] DME_Api Mod is outdated and should be updated right away | WebService Version: " + data.Version + " Mod Version: " + DME_API_VERSION);
				return;
			}
			if (m_DME_Api_VersionOffset < -1){
				Print("[DME_Api] DME_Api Mod maybe outdated and should be updated right away");
				return;
			}
			return;
		} else if (status == DME_API_ERROR){
			Error2("DME_Api", "[DME_Api] Something went wrong communicating with the webservice check to make sure it is installed correctly and the mongodb service is running correctly! URL: " + DME_Api_GetConfig().GetBaseURL());
			m_DME_Api_Online = false;
		}  else if (status == DME_API_TIMEOUT){
			Error2("DME_Api", "[DME_Api] Webservice is offline or unreachable! URL: " + DME_Api_GetConfig().GetBaseURL());
			m_DME_Api_Online = false;
		} else {
			Error2("DME_Api", "[DME_Api] Error with WebService! Status: " + status + " URL: " + DME_Api_GetConfig().GetBaseURL());
			m_DME_Api_Online = false;
		}
	}

};

static ref DME_Api_Core g_DME_Api_Core;

static DME_Api_Core DME_Api()
{
	if ( !g_DME_Api_Core )
	{
		g_DME_Api_Core = new DME_Api_Core;
		g_DME_Api_Core.Init();
	}

	return g_DME_Api_Core;
};

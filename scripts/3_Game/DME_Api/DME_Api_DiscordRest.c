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
class DME_Api_DiscordRest extends Managed {	

	protected static RestApi Api()
	{
		RestApi clCore = GetRestApi();
		if (!clCore)
		{
			clCore = CreateRestApi();
			clCore.SetOption(ERestOption.ERESTOPTION_READOPERATION, 15);
		}
		return clCore;
	}
	
	static string Link(string PlainId = ""){
		if (!g_Game) return "";
		if (PlainId == "" && !g_Game.IsDedicatedServer()){
			return BaseUrl() + "Discord/" +  GetDayZGame().GetSteamId();
		}
		return BaseUrl() + "Discord/" + PlainId;
	}
	
	protected static void Post(string url, string jsonString = "{}", RestCallback UCBX = NULL)
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx =  Api().GetRestContext(url);
		ctx.SetHeader(DME_Api().GetAuthToken());
		ctx.POST(UCBX , "", jsonString);
	}
	
	protected static void Get(string url, RestCallback UCBX = NULL)
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx =  Api().GetRestContext(url);
		ctx.GET(UCBX , "");
	}
	
	protected static string PostNow(string url, string jsonString = "{}")
	{
		RestContext ctx =  Api().GetRestContext(url);
		ctx.SetHeader(DME_Api().GetAuthToken());
		// WARNING: blocking/synchronous REST call (thread-blocking). Prefer the async variant.
		return ctx.POST_now("", jsonString);
	}

	
	protected static string BaseUrl(){
		return DME_Api_GetConfig().ServerURL;
	}
	
	static void AddRole(string GUID, string RoleId, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}

		string url = BaseUrl() + "Discord/AddRole/" + GUID;
		
		DME_Api_DiscordRoleReq roleReq = new DME_Api_DiscordRoleReq(RoleId);
		
		string jsonString = roleReq.ToJson();
		if (jsonString){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Discord] Error Adding Role (" + RoleId + ") To " + GUID);
		}
	}
	
	static void RemoveRole(string GUID, string RoleId, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Discord/RemoveRole/" + GUID;
		
		DME_Api_DiscordRoleReq roleReq = new DME_Api_DiscordRoleReq(RoleId);
		
		string jsonString = roleReq.ToJson();
		
		if (jsonString){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Discord] Error Removing Role (" + RoleId + ") To " + GUID);
		}
	}
	
	
	static void GetUser(string GUID, RestCallback UCBX) {
		string url = BaseUrl() + "Discord/Get/" + GUID;
		
		Post(url,"{}",UCBX);
	}
	
	static void GetUserWithPlainId(string plainId, RestCallback UCBX) {
		// NOTE: dead route in service 1.3.2 (returns 501); kept for reference
		string url = BaseUrl() + "Discord/GetWithPlainId/" + plainId;
		
		Post(url,"{}",UCBX);
	}
	
	
	static void CheckDiscord(string PlainId, RestCallback UCBX,  string baseUrl = ""){		
		if (baseUrl == ""){
			baseUrl = BaseUrl();
		}
		string url = baseUrl + "Discord/Check/" + PlainId;
		
		Post(url,"{}",UCBX);
	}
	
	
	
	
	static void ChannelCreate(string Name, RestCallback UCBX, DME_Api_ChannelOptions Options = NULL) {
		
		DME_Api_CreateChannelObject obj = new DME_Api_CreateChannelObject(Name, DME_Api_ChannelCreateOptions.Cast(Options));
		
		if (obj){
			string url = BaseUrl() + "Discord/Channel/Create";
			
			Post(url,obj.ToJson(),UCBX);		
		}
	}
	
	static void ChannelDelete(string id, string reason,  RestCallback UCBX = NULL){

		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		DME_Api_UpdateChannelObject obj = new DME_Api_UpdateChannelObject(reason, NULL);
		
		if (obj){
			string url = BaseUrl() + "Discord/Channel/Delete/" + id;
			
			Post(url,obj.ToJson(),UCBX);		
		}
	}
	
	
	static void ChannelEdit(string id, string reason, DME_Api_ChannelUpdateOptions options, RestCallback UCBX = NULL){

		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		DME_Api_UpdateChannelObject obj = new DME_Api_UpdateChannelObject(reason, DME_Api_ChannelUpdateOptions.Cast(options));
		
		if (obj){
			string url = BaseUrl() + "Discord/Channel/Edit/" + id;
			
			Post(url,obj.ToJson(),UCBX);		
		}
	}
	
	
	static void ChannelSend(string id, string message, RestCallback UCBX = NULL){

		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		DME_Api_DiscordBasicMessage obj = new DME_Api_DiscordBasicMessage(message);
		
		if (obj){
			string url = BaseUrl() + "Discord/Channel/Send/" + id;
			
			Post(url,obj.ToJson(),UCBX);		
		}
	}
	
	
	static void ChannelSendEmbed(string id, DME_Api_DiscordEmbed message, RestCallback UCBX = NULL){

		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
				
		if (message){
			string url = BaseUrl() + "Discord/Channel/Send/" + id;
			
			Post(url,message.ToJson(),UCBX);		
		}
	}
	
	
	
	
	static void ChannelMessages(string id,  RestCallback UCBX, DME_Api_DiscordChannelFilter filter = NULL,  string auth = ""){
	
		if (!filter){
			filter = new DME_Api_DiscordChannelFilter();
		}
		
		string url = BaseUrl() + "Discord/Channel/Messages/" + id;
		
		if (filter && UCBX){
			Post(url,filter.ToJson(),UCBX);	
		} else if (UCBX) {
			Post(url, "{}",UCBX);
		}
	}
	
	// !!!!!WARNING!!!!! 
	
	// ALL OF THE FOLLOWING FUCNTIONS ARE THREAD BLOCKING ONLY RUN in Secondary Thread!
	
	
	
	
	
	
	
	
	
	
	// !!!!!WARNING!!!!!
	// THE FOLLOWING FUCNTION IS THREAD BLOCKING ONLY RUN in secondary Thread!
	static DME_Api_DiscordUser GetUserNow(string GUID, bool ReturnError = false) {

		string url = BaseUrl() + "Discord/Get/" + GUID;
		
		string Result = PostNow(url,"{}");
		
		
		JsonSerializer js = new JsonSerializer();
		string error;
		
		DME_Api_DiscordUser user;
		
		js.ReadFromString(user, Result, error);
		
		if (error != ""){
			Print("[DME_Api] [GetUserNow] Error: " + error);
		}
		if (user && (user.Status == "Success" || ReturnError)){
			return user;
		} else if (!user && ReturnError){
			user = new DME_Api_DiscordUser;
			user.Status = "Error";
			user.Error = "Error Fetching Data";
			return user;
		}
		
		return NULL;
		
	}
	
	
	// !!!!!WARNING!!!!!
	// THE FOLLOWING FUCNTION IS THREAD BLOCKING ONLY RUN in secondary Thread!
	static DME_Api_DiscordUser GetUserWithPlainIdNow(string plainId, bool ReturnError = false) {

		string url = BaseUrl() + "Discord/GetWithPlainId/" + plainId;
		
		string Result = PostNow(url,"{}");
		Print(Result);
		JsonSerializer js = new JsonSerializer();
		string error;
		
		DME_Api_DiscordUser user;
		
		Print("[DME_Api] [GetUserWithPlainIdNow] Read from Sting");
		js.ReadFromString(user, Result, error);
		
		if (error != ""){
			Print("[DME_Api] [GetUserWithPlainIdNow] Error: " + error);
		}
		if (user && (user.Status == "Success" || ReturnError)){
			Print("[DME_Api] [GetUserWithPlainIdNow] Returning User");
			return user;
		} else if (!user && ReturnError){
			Print("[DME_Api] [GetUserWithPlainIdNow] Returning Error");
			user = new DME_Api_DiscordUser;
			user.Status = "Error";
			user.Error = "Error Fetching Data";
			return user;
		}
		Print("[DME_Api] [GetUserWithPlainIdNow] Returning NULL");
		
		return NULL;		
	}	
	
}


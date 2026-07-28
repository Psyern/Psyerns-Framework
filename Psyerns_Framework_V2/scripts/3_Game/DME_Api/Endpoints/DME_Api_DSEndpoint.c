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
class DME_Api_DSEndpoint extends DME_Api_BaseEndpoint
{	
	

	override protected string EndpointBaseUrl(){
		return DME_Api_GetConfig().GetBaseURL() + "Discord/";
	}
	
	//Returns a link for the player based on the players steam id so they can connect there discord to there steam account
	string Link(string PlainId = ""){
		if (!g_Game) return "";
		if (PlainId == "" && !g_Game.IsDedicatedServer()){
			return EndpointBaseUrl() + GetDayZGame().GetSteamId();
		}
		return EndpointBaseUrl() + PlainId;
	}
		
	//Add's a role to a user's connected discord
	int AddRole(string GUID, string RoleId, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false) {
		if (GUID == "" || RoleId == ""){
			Error2("[DME_Api] Error Adding Role from User","GUID and RoleId must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		}  else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordUser>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
			
		string url = "AddRole/" + GUID;
		
		DME_Api_DiscordRoleReq roleReq = new DME_Api_DiscordRoleReq(RoleId);
		
		Post(url,roleReq.ToJson(),DBCBX);
		
		return cid;
	}
	
	//Removes a role from a user's connected discord
	int RemoveRole(string GUID, string RoleId, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false) {
		if (GUID == "" || RoleId == ""){
			Error2("[DME_Api] Error Removing Role from User","GUID and RoleId must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		}  else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordUser>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "RemoveRole/" + GUID;
		
		DME_Api_DiscordRoleReq roleReq = new DME_Api_DiscordRoleReq(RoleId);
		
		Post(url,roleReq.ToJson(),DBCBX);
		
		return cid;
	}
	
	//Sends a DM to a user's discord retuns `DME_Api_StatusObject`
	int UserSend(string GUID, string message,  Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false){
		if (GUID == "" || message == ""){
			Error2("[DME_Api] Error Sending DM to User","GUID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		}  else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Send/" + GUID;
		
		DME_Api_DiscordBasicMessage obj = new DME_Api_DiscordBasicMessage(message);
		Post(url,obj.ToJson(),DBCBX);	
		return cid;	
	}

	//Return's a User's `DME_Api_DiscordUser` Object 
	int GetUser(string GUID, Class cbInstance, string cbFunction, bool ReturnString = false) {
		if (GUID == ""){
			Error2("[DME_Api] Error Getting Users Object","GUID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordUser>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Get/" + GUID;
		
		Post(url,"{}",DBCBX);
		return cid;
	}	
	
	//Return's a User's currently connected channel `DME_Api_DiscordStatusObject` Object 
	int GetUsersChannel(string GUID, Class cbInstance, string cbFunction, bool ReturnString = false) {
		if (GUID == ""){
			Error2("[DME_Api] Error Getting Users Channel","GUID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "GetChannel/" + GUID;
		
		Post(url,"{}",DBCBX);
		return cid;
	}	
	
	int MoveTo(string GUID, string ChannelId, Class cbInstance = NULL , string cbFunction = "", bool ReturnString = false) {
		if (GUID == "" || ChannelId == ""){
			Error2("[DME_Api] Error moving user","GUID and ChannelId must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Move/" + GUID + "/" + ChannelId;
		
		Post(url, "{}", DBCBX);
		return cid;
	}
	
	int KickUser(string GUID, string Reason = "", Class cbInstance = NULL , string cbFunction = "", bool ReturnString = false) {
		if (GUID == ""){
			Error2("[DME_Api] Error kicking user","GUID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Kick/" + GUID;
		DME_Api_TextObject txtObj = new DME_Api_TextObject(Reason);
		
		Post(url, txtObj.ToJson(), DBCBX);
		return cid;
	}
	
	int MuteUser(string GUID, bool ToMute, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false) {
		if (GUID == ""){
			Error2("[DME_Api] Error Muteing user","GUID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Mute/" + GUID;
		
		DME_Api_DiscordMute muteObject = new DME_Api_DiscordMute(ToMute);
		
		Post(url, muteObject.ToJson(), DBCBX);
		
		return cid;
	}		
	
	int SetNickname(string GUID, string Nickname, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false) {
		if (GUID == "" || Nickname == ""){
			Error2("[DME_Api] Error Setting Nickname","GUID and Nickname must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, GUID);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, GUID), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "SetNickname/" + GUID;
		
		DME_Api_DiscordNickname nickObject = new DME_Api_DiscordNickname(Nickname);
		
		Post(url, nickObject.ToJson(), DBCBX);
		
		return cid;
	}	
	
	int ChannelCreate(string Name, DME_Api_ChannelOptions Options = NULL, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false) {
		if ( Name == "" ){
			Error2("[DME_Api] Error Creating channel","Channel ID must be valid string");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, Name);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, Name), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_CreateChannelObject obj = new DME_Api_CreateChannelObject(Name, DME_Api_ChannelCreateOptions.Cast(Options));
		
		string url = "Channel/Create";
			
		Post(url,obj.ToJson(),DBCBX);	
		
		return cid;	
	}
	
	
	int ChannelDelete(string id, string reason, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false){
		if (id == "" || reason == ""){
			Error2("[DME_Api] Error Deleting channel","Both Channel ID and reason must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, id);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, id), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_UpdateChannelObject obj = new DME_Api_UpdateChannelObject(reason, NULL);
		
		string url = "Channel/Delete/" + id;
		
		Post(url,obj.ToJson(),DBCBX);
		return cid;	
	}
	
	int ChannelEdit(string id, string reason, DME_Api_ChannelUpdateOptions options, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false){
		if (id == "" || reason == ""){
			Error2("[DME_Api] Error Editing channel","Both Channel ID and reason must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, id);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, id), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_UpdateChannelObject obj = new DME_Api_UpdateChannelObject(reason, DME_Api_ChannelUpdateOptions.Cast(options));
		
		string url = "Channel/Edit/" + id;
			
		Post(url,obj.ToJson(),DBCBX);	
		return cid;		
	}
	
	int ChannelSend(string id, string message, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false){
		if (id == "" || message == ""){
			Error2("[DME_Api] Error Sending message to channel","Both Channel ID and message must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, id);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, id), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_DiscordBasicMessage obj = new DME_Api_DiscordBasicMessage(message);
		
		string url = "Channel/Send/" + id;
			
		Post(url,obj.ToJson(),DBCBX);		
		return cid;	
	}
	
	
	int ChannelSendEmbed(string id, DME_Api_DiscordEmbed message, Class cbInstance = NULL, string cbFunction = "", bool ReturnString = false){
		if (id == "" || message != NULL){
			Error2("[DME_Api] Error Sending Embed to channel","Both Channel ID and message must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, id);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordStatusObject>(cbInstance, cbFunction, id), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Channel/Send/" + id;
			
		Post(url,message.ToJson(),DBCBX);	
		return cid;	
	}
	
	
	int ChannelMessages(string id,  Class cbInstance, string cbFunction, DME_Api_DiscordChannelFilter filter = NULL, bool ReturnString = false){
		if (id == ""){
			Error2("[DME_Api] Error Getting messages from channel","Channel ID must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		if (!filter){
			filter = new DME_Api_DiscordChannelFilter();
		}
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, id);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordMessagesResponse>(cbInstance, cbFunction, id), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "Channel/Messages/" + id;
		
		Post(url,filter.ToJson(),DBCBX);	
		return cid;	
	}
	
	//A way to check if a player's discord is set up before they connect to the server and get an authkey
	int CheckRoleDiscord(string PlainId, string RoleId, Class cbInstance, string cbFunction,  string baseUrl = "", bool ReturnString = false){		
		int cid = DME_Api().CallId();
		if (baseUrl == ""){
			baseUrl = DME_Api_GetConfig().GetBaseURL();
		}
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, PlainId);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, PlainId), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = baseUrl + "Discord/CheckRole/" + PlainId + "/" + RoleId;
		
		DME_Api().Post(url,"{}",DBCBX);
		
		return cid;
	}
	
	//A way to check if a player's discord is set up before they connect to the server and get an authkey
	int CheckDiscord(string PlainId, Class cbInstance, string cbFunction,  string baseUrl = "", bool ReturnString = false){		
		int cid = DME_Api().CallId();
		if (baseUrl == ""){
			baseUrl = DME_Api_GetConfig().GetBaseURL();
		}
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, PlainId);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_StatusObject>(cbInstance, cbFunction, PlainId), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = baseUrl + "Discord/Check/" + PlainId;
		
		DME_Api().Post(url,"{}",DBCBX);
		
		return cid;
	}
	
	
	
	//Obsolete GetUser accepts both plainid or GUID
	/*int GetUserWithPlainId(string plainId, Class cbInstance, string cbFunction) {
		int cid = DME_Api().CallId();
		autoptr RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, plainId);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "GetWithPlainId/" + plainId;
		if (plainId && plainId != ""){
			Post(url,"{}",DBCBX);
		}
		return cid;
	}*/
	
	//Obsolete GetUserObj accepts both plainid or GUID
	/*int GetUserObjWithPlainId(string plainId, Class cbInstance, string cbFunction) {
		int cid = DME_Api().CallId();
		autoptr RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_DiscordUser>(cbInstance, cbFunction, plainId), cid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string url = "GetWithPlainId/" + plainId;
		if (plainId && plainId != ""){
			Post(url,"{}",DBCBX);
		}
		return cid;
	}*/
	
	
}
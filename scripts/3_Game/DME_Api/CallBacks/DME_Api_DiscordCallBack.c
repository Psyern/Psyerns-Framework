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
class DME_Api_DiscordCallBack: RestCallback
{
	
	override void OnError(int errorCode) {
		Print("[DME_Api] [DME_Api_DiscordCallBack] Failed errorCode: " + errorCode);
	};
	
	override void OnTimeout() {
		Print("[DME_Api] [DME_Api_DiscordCallBack] Failed errorCode: Timeout");
	};
	
	override void OnSuccess(string data, int dataSize) {
		DME_Api_DiscordUser user;
		
		JsonSerializer js = new JsonSerializer();
		string error;
		js.ReadFromString(user, data, error);
		if (error != ""){
			Print("[DME_Api] [DME_Api_DiscordCallBack] Error: " + error);
		}
		if (user.Status && user.Status == "Success" && user.id && user.id != "0"){
			OnDiscordUserReceived(DME_Api_DiscordUser.Cast(user));
		} else if (user.Status && (user.Status == "NotFound" || user.Status ==  "NotSetup")){
			OnDiscordUserNotFound(DME_Api_DiscordUser.Cast(user));
			
		} else {
			OnDiscordUserError(DME_Api_DiscordUser.Cast(user));
		}
	};
	
	void OnDiscordUserReceived(DME_Api_DiscordUser user){
		//Do Stuff Here
		Print("[DME_Api] [DME_Api_DiscordCallBack] Success: " + user.id );
		
	}
	
	void OnDiscordUserNotFound(DME_Api_DiscordUser user){
		//Do Stuff Here
		Print("[DME_Api] [DME_Api_DiscordCallBack] User not found");
	}
	
	void OnDiscordUserError(DME_Api_DiscordUser user){
		//Do Stuff Here
		Print("[DME_Api] [DME_Api_DiscordCallBack] Error: " + user.Error);
		
	}
}

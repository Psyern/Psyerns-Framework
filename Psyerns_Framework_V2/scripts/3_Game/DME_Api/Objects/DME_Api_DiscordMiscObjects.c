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
class DME_Api_DiscordRoleReq extends DME_Api_Object_Base{
	string Role;
	void DME_Api_DiscordRoleReq(string role){
		Role = role;
	}
	
	override string ToJson(){
		return DME_Api_JSONHandler<DME_Api_DiscordRoleReq>.ToString(this);
	}
}

class DME_Api_DiscordStatusObject extends DME_Api_StatusObject {
	
	string oid;

}


class DME_Api_DiscordMute extends DME_Api_Object_Base{
	
	bool State = true;
	
	void DME_Api_DiscordMute(bool state){
		State = state;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordMute>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordNickname extends DME_Api_Object_Base{
	
	string Nickname = "";
	
	void DME_Api_DiscordNickname(string nickname){
		Nickname = nickname;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordNickname>.ToString(this);
		return jsonString;
	}
}
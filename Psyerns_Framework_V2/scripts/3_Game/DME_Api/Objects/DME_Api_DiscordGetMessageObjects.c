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
class DME_Api_DiscordChannelFilter extends DME_Api_Object_Base {
	
	int Limit = -1;
	string Before = "";
	string After = "";
	
	void DME_Api_DiscordChannelFilter(int limit = -1, string before = "", string after = ""){
		Limit = limit;
		Before = before;
		After = after;
	}
	
	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_DiscordChannelFilter>.JsonMakeData(this);
		return jsonString;
	}
	
}



class DME_Api_DiscordMessagesResponse extends DME_Api_StatusObject {
	
	ref array<ref DME_Api_DiscordMessage> Messages;
	
	
	array<ref DME_Api_DiscordMessage> GetMessages(){
		return Messages;
	}
}
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
class DME_Api_DiscordUser extends DME_Api_StatusObject{
		
	string id;
	string Username;
	
	string Discriminator;
	string Avatar;
	
	ref TStringArray Roles;
	
	string VoiceChannel;
	
	bool HasRole(string roleid){
		if (!Roles) return false;
		return (Roles.Find(roleid) != -1);
	}
	
}
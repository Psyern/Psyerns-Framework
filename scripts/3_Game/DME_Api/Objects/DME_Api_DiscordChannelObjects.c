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
class DME_Api_CreateChannelObject extends DME_Api_Object_Base{

	string Name = "new-channel";
	
	ref DME_Api_ChannelCreateOptions Options;
	
	
	void DME_Api_CreateChannelObject(string name, DME_Api_ChannelCreateOptions options = NULL){
		Name = name;
		if (!options){
			Options = new DME_Api_ChannelCreateOptions("Created Via DayZ");
		} else {
			Options = DME_Api_ChannelCreateOptions.Cast(options);
		}
	}
	
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_CreateChannelObject>.ToString(this);
		return jsonString;
	}

}

class DME_Api_UpdateChannelObject extends DME_Api_Object_Base{
	
	string Reason = "";
	ref DME_Api_ChannelUpdateOptions Options;
	
	void DME_Api_UpdateChannelObject(string reason, DME_Api_ChannelUpdateOptions options){
		Reason = reason;
		Options = DME_Api_ChannelUpdateOptions.Cast(options);
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_UpdateChannelObject>.ToString(this);
		return jsonString;
	}
}


class DME_Api_ChannelUpdateOptions extends DME_Api_ChannelOptions {
	string name;
	
	void DME_Api_ChannelOptions(string Reason, string Name, string Topic = ""){
		name = Name;
		reason =  Reason;
		topic = Topic;
	}
	
}

class DME_Api_ChannelCreateOptions extends DME_Api_ChannelOptions{
	string type;

	void DME_Api_ChannelCreateOptions(string Reason, string Type = "text", string Topic = ""){
		type = Type;
	}
	
}

class DME_Api_ChannelOptions extends Managed {
	string reason = "Created Via DayZ";
	string topic;
	bool nsfw;
	string parent;
	ref array<ref DME_Api_ChannelPermissions> permissionOverwrites;
	int position = -1;
	int rateLimitPerUser = -1;
	
	
	void AddPerm(string id, string perm, bool isAllow = true){
		if (!permissionOverwrites){
			permissionOverwrites = new array<ref DME_Api_ChannelPermissions>;
		}
		bool added = false;
		for (int i = 0; i < permissionOverwrites.Count(); i++){
			if (permissionOverwrites.Get(i) && permissionOverwrites.Get(i).id == id){
				if (isAllow){
					added = true;
					permissionOverwrites.Get(i).allow.Insert(perm);
				} else {
					added = true;
					permissionOverwrites.Get(i).deny.Insert(perm);
				}
			}
		} 
		if (!added){
			if (isAllow){
				permissionOverwrites.Insert(new DME_Api_ChannelPermissions(id, { perm }, NULL));
			} else {
				permissionOverwrites.Insert(new DME_Api_ChannelPermissions(id, NULL, { perm }));
			}
		}
	}
	
	void SetPerms(string id, TStringArray perms, bool isAllow = true){
		if (!permissionOverwrites){
			permissionOverwrites = new array<ref DME_Api_ChannelPermissions>;
		}
		bool added = false;
		for (int i = 0; i < permissionOverwrites.Count(); i++){
			if (permissionOverwrites.Get(i) && permissionOverwrites.Get(i).id == id){
				if (isAllow){
					added =true;
					permissionOverwrites.Get(i).allow = perms;
				} else {
					added =true;
					permissionOverwrites.Get(i).deny = perms;
				}
			}
		} 
		
		if (!added){
			if (isAllow){
				permissionOverwrites.Insert(new DME_Api_ChannelPermissions(id, perms, NULL));
			} else {
				permissionOverwrites.Insert(new DME_Api_ChannelPermissions(id, NULL, perms));
			}
		}
		
	}
}

class DME_Api_ChannelPermissions extends Managed{

	string id;
	ref TStringArray allow;
	ref TStringArray deny;
	
	void DME_Api_ChannelPermissions(string Id, TStringArray Allow, TStringArray Deny){
		id = Id;
		allow = Allow;
		deny = Deny;
	}
}
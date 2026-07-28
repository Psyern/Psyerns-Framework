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
class DME_Api_DiscordBasicMessage extends DME_Api_Object_Base {
	
	string Message= "";
	
	void DME_Api_DiscordBasicMessage(string message){
		Message = message;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordBasicMessage>.ToString(this);
		return jsonString;
	}
	
}

class DME_Api_DiscordObject extends DME_Api_Object_Base {
	string username = "";
	string avatar_url = "";
	string content = "";
	ref array<ref DME_Api_DiscordEmbed> embeds = new array<ref DME_Api_DiscordEmbed>;
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordObject>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordMessage extends DME_Api_Object_Base {
	string id; //Message Id
	string AuthorId; // Discord ID of the player
	string AuthorGUID; // if player has discord connected in database this will be there GUID
	string Content; // Text content of the message
	string ChannelId; //Channel id for the message
	string RepliedTo; //Message id of the message if this message is a reply
	ref DME_Api_DiscordEmbed Embed; //If the message has an Embed this will be the embed object
	int TimeStamp; //Time stamp of the message
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordMessage>.ToString(this);
		return jsonString;
	}
}


class DME_Api_DiscordEmbed extends DME_Api_Object_Base{
	 
	ref DME_Api_DiscordAuthor author;
	string title = "";
	string url = "";
	string description = "";
	int color = 0;
	ref array<ref DME_Api_DiscordField> embeds = new array<ref DME_Api_DiscordField>;
	ref DME_Api_DiscordImage thumbnail;
	ref DME_Api_DiscordImage image;
	ref DME_Api_DiscordFooter footer;
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordEmbed>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordAuthor extends DME_Api_Object_Base {
	string name = "";
    string url = "";
    string icon_url= "";
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordAuthor>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordField extends DME_Api_Object_Base {
	string name = "";
	string value = "";
	bool inline = false;
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordField>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordImage extends DME_Api_Object_Base {
	
	string url = "";
	int height;
	int width;
	
	void DME_Api_DiscordImage(string value){
		url = value;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordImage>.ToString(this);
		return jsonString;
	}
}

class DME_Api_DiscordFooter extends DME_Api_Object_Base{
	
	string text = "";
	string icon_url = "";
	
	void DME_Api_DiscordFooter(string txt, string url){
		text = txt;
		icon_url = url;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DiscordFooter>.ToString(this);
		return jsonString;
	}
}

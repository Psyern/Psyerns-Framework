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
static const string DME_API_VERSION = "1.3.2";

static const int DME_API_SUCCESS = 200;
static const int DME_API_EMPTY = 204; //Means response was empty or query result returned no results
static const int DME_API_NOTSETUP = 424; //Used for discord requests only right now.
static const int DME_API_TIMEOUT = 408;
static const int DME_API_CLIENTERROR = 400;
static const int DME_API_SERVERERROR = 500;
static const int DME_API_ERROR = 418;
static const int DME_API_JSONERROR = 406;
static const int DME_API_NOTFOUND = 404;
static const int DME_API_TOOEARLY = 425;
static const int DME_API_UNAUTHORIZED = 401;


static const int DME_API_DBSUCCESS = 200;
static const int DME_API_DBEMPTY = 204;
static const int DME_API_DBTIMEOUT = 408;
static const int DME_API_DBSERVERERROR = 500;


static const int DME_API_DBUNAUTHORIZED = 401;
static const int DME_API_DBERROR = 418;
static const int DME_API_DBTOOEARLY = 425;


static const int DME_API_PLAYER_DB = 100;
static const int DME_API_OBJECT_DB = 101;

// Native RPC id for the remote-weapon fix resync (was a magic number 155494166 in Weapon_Base)
static const int DME_API_RPC_WEAPON_FIX = 155494166;


class DME_Api_UpdateOpts {
	static string SET = "set"; // `set` to set the value of an element
	static string PULL = "pull"; // `pull` to pull a value out of an array
	static string PUSH = "push"; // `push` to push a value into an array
	static string UNSET = "unset";// `unset` to remove an element from the database
	static string MUL = "mul"; // `mul` to mulitply an element by the value in the database
	static string RENAME = "rename";// `rename` to rename an element in the database
	static string PULLALL = "pullAll";// `pullAll` to empty an array
}


class DME_Api_DSPerms {
	
	static string ADD_REACTIONS = "ADD_REACTIONS"; // (add new reactions to messages)
	static string VIEW_AUDIT_LOG = "VIEW_AUDIT_LOG";
	static string PRIORITY_SPEAKER = "PRIORITY_SPEAKER";
	static string STREAM = "STREAM";
	static string VIEW_CHANNEL = "VIEW_CHANNEL";
	static string SEND_MESSAGES = "SEND_MESSAGES";
	static string SEND_TTS_MESSAGES = "SEND_TTS_MESSAGES";
	static string MANAGE_MESSAGES = "MANAGE_MESSAGES"; // (delete messages and reactions)
	static string EMBED_LINKS = "EMBED_LINKS"; // (links posted will have a preview embedded)
	static string ATTACH_FILES = "ATTACH_FILES"; 
	static string READ_MESSAGE_HISTORY = "READ_MESSAGE_HISTORY"; // (view messages that were posted prior to opening Discord)
	static string MENTION_EVERYONE = "MENTION_EVERYONE";
	static string USE_EXTERNAL_EMOJIS = "USE_EXTERNAL_EMOJIS"; // (use emojis from different guilds)
	static string CONNECT = "CONNECT"; // (connect to a voice channel)
	static string USE_VAD = "USE_VAD"; // (use voice activity detection)
	static string SPEAK = "SPEAK"; // (speak in a voice channel)
	static string CREATE_INSTANT_INVITE = "CREATE_INSTANT_INVITE"; // (create invitations to the guild)
	
	
	//Since there is no functions to manage the discord these permission are kinda usless but keeping them encase something changes in the future
	static string ADMINISTRATOR = "ADMINISTRATOR";// (implicitly has all permissions, and bypasses all channel overwrites)
	static string KICK_MEMBERS = "KICK_MEMBERS";
	static string BAN_MEMBERS = "BAN_MEMBERS";
	static string MANAGE_CHANNELS = "MANAGE_CHANNELS"; //(edit and reorder channels)
	static string MANAGE_GUILD = "MANAGE_GUILD"; //  (edit the guild information, region, etc.)
	static string VIEW_GUILD_INSIGHTS = "VIEW_GUILD_INSIGHTS";
	static string MUTE_MEMBERS = "MUTE_MEMBERS"; //(mute members across all voice channels)
	static string DEAFEN_MEMBERS = "DEAFEN_MEMBERS"; //(deafen members across all voice channels)
	static string MOVE_MEMBERS = "MOVE_MEMBERS"; //(move members between voice channels)
	static string CHANGE_NICKNAME = "CHANGE_NICKNAME";
	static string MANAGE_NICKNAMES = "MANAGE_NICKNAMES"; //(change other members' nicknames)
	static string MANAGE_ROLES = "MANAGE_ROLES";
	static string MANAGE_WEBHOOKS = "MANAGE_WEBHOOKS";
	static string MANAGE_EMOJIS = "MANAGE_EMOJIS";	
}
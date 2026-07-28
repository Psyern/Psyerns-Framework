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
 *
 * Config moved to the Psyerns_Framework profile folder with a one-time read
 * fallback from the legacy UniversalApi path. JSON keys are unchanged (wire).
 */
class DME_Api_Config extends Managed {

	protected static string ConfigDIR = "$profile:DeadmansEcho\\PsyernsFramework";
	protected static string ConfigPATH = ConfigDIR + "\\DME_Api.json";
	protected static string LegacyConfigPATH = "$profile:UApi\\UniversalApi.json";
	string ConfigVersion = "1";
	string ServerURL = "";
	string ServerID = "";
	string ServerAuth = "";
	bool QnAEnabled = false;
	int EnableBuiltinLogging = 0;
	int PromptDiscordOnConnect = 0;

	void Load(){
		if (!g_Game) return;
		if (!g_Game.IsDedicatedServer()){
			return;
		}
		bool loaded = false;
		if (FileExist(ConfigPATH)){ //If config exist load File
			JsonFileLoader<DME_Api_Config>.JsonLoadFile(ConfigPATH, this);
			loaded = true;
		} else if (FileExist(LegacyConfigPATH)){ //Auto-migrate the legacy UniversalApi config (leave the old file for rollback)
			JsonFileLoader<DME_Api_Config>.JsonLoadFile(LegacyConfigPATH, this);
			MakeDirectory("$profile:DeadmansEcho");
			MakeDirectory(ConfigDIR);
			Save();
			Print("[DME_Api] Migrated legacy UApi config to " + ConfigPATH);
			loaded = true;
		}

		if (loaded){
			if (ServerURL != ""){
				int lastIndex = ServerURL.Length() - 1;
				if ( ServerURL.Substring(lastIndex,1) != "/"){ //correct URL
					ServerURL = ServerURL + "/";
					Save();
				}
				if (QnAEnabled){
					DME_Api_GetQnAMaker();
				}
			}
			if (ConfigVersion != "1"){
				ConfigVersion = "1";
				PromptDiscordOnConnect = 0;
				Save();
			}
		} else { //File does not exist create file
			MakeDirectory("$profile:DeadmansEcho");
			MakeDirectory(ConfigDIR);
			Save();
		}
	}

	string GetBaseURL(){
		return ServerURL;
	}

	string GetAuth(){
		if (g_Game && g_Game.IsDedicatedServer()){
			return ServerAuth;
		}
		return "ERROR";
	}

	void Save(){
		JsonFileLoader<DME_Api_Config>.JsonSaveFile(ConfigPATH, this);
	}


}

ref DME_Api_Config m_DME_Api_Config;

//Helper function to return Config
static DME_Api_Config DME_Api_GetConfig()
{
	if ( g_Game && g_Game.IsDedicatedServer()){
		if (!m_DME_Api_Config)
		{
			m_DME_Api_Config = new DME_Api_Config;
			m_DME_Api_Config.Load();
		}
	}
	return m_DME_Api_Config;
};

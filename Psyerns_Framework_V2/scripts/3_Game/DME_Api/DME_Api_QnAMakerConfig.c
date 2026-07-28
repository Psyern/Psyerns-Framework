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
class DME_Api_QnAMakerServerAnswers extends Managed
{
	protected static string ConfigDIR = "$profile:DeadmansEcho\\PsyernsFramework";
	protected static string ConfigPATH = ConfigDIR + "\\QnAMakerServerAnswers.json";
	protected bool UseNotifcationsMod = false;
	string BotName = "Bot";
	ref array<ref DME_Api_QnAMakerServerAnswer> ServerSpecificAnswers = new array<ref DME_Api_QnAMakerServerAnswer>;
	
	void Load(){
		if (!g_Game) return;
		if (g_Game.IsDedicatedServer()){
			if (FileExist(ConfigPATH)){ //If config exist load File
			    JsonFileLoader<DME_Api_QnAMakerServerAnswers>.JsonLoadFile(ConfigPATH, this);
			}else{ //File does not exist create FileMode.
				ServerSpecificAnswers.Insert(new DME_Api_QnAMakerServerAnswer("#SERVERNAME#", "US1"));
				Save();
			}
		}
	}
	
	void ProcessAnswer(string answer){
		string response = answer;
		for (int i = 0; i < ServerSpecificAnswers.Count(); i++){
			response.Replace(ServerSpecificAnswers.Get(i).ResponseCode, ServerSpecificAnswers.Get(i).Response); 
		}
		SendRespone(response);
	}
	
	void Save(){
			JsonFileLoader<DME_Api_QnAMakerServerAnswers>.JsonSaveFile(ConfigPATH, this);
	}
	
	void SendRespone(string text){
		if (!g_Game) return;
		if (!UseNotifcationsMod){
			g_Game.Chat(BotName + ": " + text, "colorImportant");
		} else {
			
			#ifdef NOTIFICATIONS 
				float nTime = 5;
				int strlen = text.Length();
				if (strlen > 640){
					nTime = 70;
				} else if (strlen > 400){
					nTime = 50;
				} else if (strlen > 240){
					nTime = 35;
				} else if (strlen > 120){
					nTime = 25;
				} else if (strlen > 60){
					nTime = 15;
				}
				NotificationSystem.SimpleNoticiation(text, BotName, "Psyerns_Framework/images/Bot.edds", ARGB(230, 142, 180, 230), nTime, NULL);
			#endif
		}
	}
	
}


class DME_Api_QnAMakerServerAnswer {
	string ResponseCode = "";
	string Response = "";
	void DME_Api_QnAMakerServerAnswer( string code, string response){
		ResponseCode = code;
		Response = response;
	}
}


ref DME_Api_QnAMakerServerAnswers m_QnAMakerServerAnswers;

//Helper function to return Config
static DME_Api_QnAMakerServerAnswers DME_Api_GetQnAMaker()
{
	if (!g_Game) return NULL;
	if ( g_Game.IsDedicatedServer()){
		if (!m_QnAMakerServerAnswers)
		{
			m_QnAMakerServerAnswers = new DME_Api_QnAMakerServerAnswers;
			m_QnAMakerServerAnswers.Load();
		}
	}
	return m_QnAMakerServerAnswers;
};
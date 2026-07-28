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
modded class DayZGame extends CGame
{
	protected ref DME_Api_DiscordUser m_DME_Api_DiscordUser;

	DME_Api_DiscordUser DiscordUser(){
		if (IsDedicatedServer()){
			Error2("[DME_Api] DiscordInfo", "Can't get discord info from server you must request it from the api directly only client caches");
			return NULL;
		}
		return m_DME_Api_DiscordUser;
	}
	
	
	protected void CBCacheDiscordInfo(int cid, int status, string oid, DME_Api_DiscordUser data){
		if (!IsDedicatedServer() && status == DME_API_SUCCESS){
			if (Class.CastTo(m_DME_Api_DiscordUser, data)){
				Print("[DME_Api] Discord is set up and cached " + m_DME_Api_DiscordUser.Username + "#" +  m_DME_Api_DiscordUser.Discriminator);
			}
		}
		if (!IsDedicatedServer() && status == DME_API_NOTSETUP && DME_Api_GetConfig().PromptDiscordOnConnect >= 1){
			OpenURL(DME_Api().ds().Link());
		}
	}
	
	protected void CBQnAChatMessageSilent(int cid, int status, string oid, DME_Api_QnAAnswer data){
		if (status == DME_API_SUCCESS && data){
			DME_Api_QnAMakerServerAnswers qnaMaker = DME_Api_GetQnAMaker();
			if (!qnaMaker) return;
			if (data.get() != "null" && data.get() != "error" &&  data.get() != "ERROR" &&  data.get() != ""){
				qnaMaker.ProcessAnswer(data.get());
			} else {
				qnaMaker.SendRespone("Sorry couldn't find the an answer to your question? Try rephrasing it or asking a real person");
			}
		}
	}
	
	protected void CBQnAChatMessage(int cid, int status, string oid, DME_Api_QnAAnswer data){
		if (status == DME_API_SUCCESS && data){
			DME_Api_QnAMakerServerAnswers qnaMaker = DME_Api_GetQnAMaker();
			if (!qnaMaker) return;
			if (data.get() != "null" && data.get() != "error" &&  data.get() != "ERROR" &&  data.get() != ""){
				qnaMaker.ProcessAnswer(data.get());
			}
		}
	}
	
	//Client side function to get the steam id
	string GetSteamId(){
		DayZPlayer player;
		if (GetUserManager() && GetUserManager().GetTitleInitiator()){
			return GetUserManager().GetTitleInitiator().GetUid();
		} else if (!IsDedicatedServer() && Class.CastTo(player, GetPlayer()) && player.GetIdentity() && player.GetIdentity().GetPlainId() != "" ){
			return player.GetIdentity().GetPlainId();
		} 
		return "";
	}
}
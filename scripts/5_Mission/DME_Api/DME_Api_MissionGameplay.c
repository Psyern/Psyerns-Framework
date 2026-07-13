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
 * Stacks alongside the PF_ modded MissionGameplay (Fall F).
 */
modded class MissionGameplay
{

	override void OnMissionStart(){
		super.OnMissionStart();
		m_DME_Api_Initialized = false;
		DME_Api().RequestAuthToken(true);
		//Token expires in 46.5 minutes, tokens renew every 21-23 Minutes ensuring that if the API is down at the time of the renewal token will work till next retry
		int TokenRefreshRate = Math.RandomInt(1260,1380) * 1000; //Uses a 2 minutes random to prevent lots of renewals happening at exact same time after server restarts
		if (g_Game){
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.CallLater(this.RequestNewAuthToken, TokenRefreshRate, false); }
		}
	}

	override void OnMissionFinish(){
		super.OnMissionFinish();
		if (g_Game){
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.Remove(this.RequestNewAuthToken); }
		}
	}


	override void DME_Api_Ready(){
		//You requests for after the AuthToken Is received
		super.DME_Api_Ready();
	}


	void RequestNewAuthToken(){
		if (!g_Game) return;
		if (!g_Game.IsDedicatedServer()){
			DME_Api().RequestAuthToken();
			int TokenRefreshRate = Math.QRandomInt(1260,1380) * 1000;
			//Token expires in 46.5 minutes, tokens renew every 21-23 Minutes ensuring that if the API is down at the time of the renewal token will work till next retry
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.CallLater(this.RequestNewAuthToken, TokenRefreshRate, false); }
		}
	}


}

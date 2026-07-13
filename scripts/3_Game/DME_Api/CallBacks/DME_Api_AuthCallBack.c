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
class DME_Api_AuthCallBack : RestCallback
{
	protected int m_TryCount = 0;
	protected string m_GUID = "";
	
	void DME_Api_AuthCallBack(string guid = ""){
		m_GUID = guid;
	}
	
	override void OnError(int errorCode) {
		Print("[DME_Api] [DME_Api_AuthCallBack] Auth of a Player Failed errorCode: " + DME_Api().ErrorToString(errorCode));
		if (m_GUID != ""){
			DME_Api().AuthError(m_GUID);
		}
	};
	override void OnTimeout() {
		Print("[DME_Api] [DME_Api_AuthCallBack] Auth of a Player Failed errorCode: Timeout");
		if (m_GUID != ""){
			DME_Api().AuthError(m_GUID);
		}
	};
	
	override void OnSuccess(string data, int dataSize) {
		
		//Print("[DME_Api] [DME_Api_AuthCallBack] Auth of a Player Success data: " + data);
		DME_Api_AuthToken authToken;
		
		JsonSerializer js = new JsonSerializer();
		string error;
		js.ReadFromString(authToken, data, error);
		if (error != ""){
			Print("[DME_Api] [DME_Api_AuthCallBack] Error: " + error);
		}
		if (!authToken){
			Print("[DME_Api] [DME_Api_AuthCallBack] Error: Failed to parse auth token, aborting");
			return;
		}
		if (authToken.GUID == m_GUID && authToken.AUTH != "ERROR"){
			Print("[DME_Api] [DME_Api_AuthCallBack] Auth of a Player Success data: GUID " + authToken.GUID);
			DME_Api().AddPlayerAuth(authToken.GUID, authToken.AUTH);
		} else {
			if (m_GUID != ""){
				DME_Api().AuthError(m_GUID);
			}
		}
	};
	
};

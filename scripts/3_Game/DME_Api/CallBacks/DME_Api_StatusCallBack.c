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
class DME_Api_StatusCallBack : DME_Api_DBCallBack
{
	
	override void OnError(int errorCode) {
		if (!g_Game) return;
		if (Instance && Function != ""){
			g_Game.GameScript.CallFunctionParams(Instance, Function, NULL, new Param4<int, int, string, DME_Api_StatusObject>(CallId, DME_API_ERROR, OID, NULL));
		}
	};
	
	override void OnTimeout() {
		if (!g_Game) return;
		if (Instance && Function != ""){
			g_Game.GameScript.CallFunctionParams(Instance, Function, NULL, new Param4<int, int, string, DME_Api_StatusObject>(CallId, DME_API_TIMEOUT, OID, NULL));
		}
	};
	
	override void OnSuccess(string data, int dataSize) {
		if (!g_Game) return;
		if (Instance && Function != ""){
			
			DME_Api_StatusObject obj;
			
			JsonSerializer js = new JsonSerializer();
			string error;
			js.ReadFromString(obj, data, error);
			if (error != ""){
				Print("[DME_Api] [DME_Api_StatusCallBack] Error: " + error);
			}
			if (obj && obj.Status && (obj.Status == "Success" || obj.Status == "Ok") ){ //Will eventually Phase out "Ok"			
				g_Game.GameScript.CallFunctionParams(Instance, Function, NULL, new Param4<int, int, string, DME_Api_StatusObject>(CallId, DME_API_SUCCESS, OID, DME_Api_StatusObject.Cast(obj)));
				return;
			} 
			if (obj.Status && (obj.Status == "NotFound" || obj.Status ==  "NotSetup")){
				g_Game.GameScript.CallFunctionParams(Instance, Function, NULL, new Param4<int, int, string, DME_Api_StatusObject>(CallId, DME_API_NOTFOUND, OID, DME_Api_StatusObject.Cast(obj)));
				return;
			}
			g_Game.GameScript.CallFunctionParams(Instance, Function, NULL, new Param4<int, int, string, DME_Api_StatusObject>(CallId, DME_API_ERROR, OID, DME_Api_StatusObject.Cast(obj)));
		}
	};
};
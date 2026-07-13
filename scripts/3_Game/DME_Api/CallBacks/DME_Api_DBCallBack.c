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
class DME_Api_DBCallBack : RestCallback
{
	protected Class Instance;
	protected string Function;
	protected string OID;
	protected int CallId;

	
	protected Class GetInstance(){
		return Instance;
	}
	
	void DME_Api_DBCallBack(Class instance, string function, int id, string oid){
		Instance = instance;
		Function = function;
		CallId = id;
		OID = oid;
	}
	
	override void OnError(int errorCode) {
		if (!g_Game) return;
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnError " + DME_Api().ErrorToString(errorCode));
			return;
		}
		int rstatus = DME_API_SERVERERROR;
		if (errorCode == ERestResultState.EREST_ERROR_CLIENTERROR){
			rstatus = DME_API_CLIENTERROR;
		}
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, string>(CallId, rstatus, OID, "{}"));
		}
	};
	
	override void OnTimeout() {
		if (!g_Game) return;
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnTimeout");
			return;
		}
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, string>(CallId, DME_API_TIMEOUT, OID, "{}"));
		}
	};
	
	override void OnSuccess(string data, int dataSize) {
		if (!g_Game) return;
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnSuccess");
			return;
		}
		int rstatus = DME_API_SUCCESS;
		if (data == "{}" || data == "" || data == "{ }"){
			rstatus = DME_API_EMPTY;
		}
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, string>(CallId, rstatus, OID, data));
		}
	};
};

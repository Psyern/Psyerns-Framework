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
//This meathod has to be used for the template class to work, you can't have a template class that exends RestCallback


class DME_Api_Callback<Class T> extends DME_Api_CallbackBase{
	
	override void OnError(int errorCode, int cid) {
		if (!g_Game) return;
		if (GetInstance() && Function != "") {
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, T>(cid, errorCode, OID, NULL));
		}
	}
	
	override void OnSuccess(string jsonData, int cid) {
		if (!g_Game) return;
		if (GetInstance() && Function != ""){
			T obj;
			if (DME_Api_JSONHandler<T>.FromString(jsonData, obj)){
				int rstatus = DME_API_SUCCESS;
				DME_Api_StatusObject sobj;
				if (Class.CastTo(sobj, obj)){
					switch (sobj.Status) {
						case "NotFound":
							rstatus = DME_API_NOTFOUND;
							break;
						case "NoResults":
							rstatus = DME_API_EMPTY;
							break;
						case "Error":
							rstatus = DME_API_ERROR;
							break;
						case "NoPerms":
							rstatus = DME_API_UNAUTHORIZED;
							break;
						case "NoAuth":
							rstatus = DME_API_UNAUTHORIZED;
							break;
						case "InvalidAuth":
							rstatus = DME_API_UNAUTHORIZED;
							break;
						case "NotSetup":
							rstatus = DME_API_NOTSETUP;
							break;
					}
				}
				g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, T>(cid, rstatus, OID, obj));
			} else {
				g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, T>(cid, DME_API_JSONERROR, OID, NULL));
			}
		}
	}
}

//Allows you to load the json to a defined object
class DME_Api_CallbackLoader<Class T> extends DME_Api_CallbackBase {

	ref T obj;
	
	void SetObject(T object){
		obj = object;
	}
	
	override void OnError(int errorCode, int cid) {
		if (!g_Game) return;
		if (GetInstance() && Function != "") {
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, T>(cid, errorCode, OID, NULL));
		}
	}
	
	override void OnSuccess(string jsonData, int cid) {
		if (!g_Game) return;
		int rstatus = DME_API_JSONERROR;
		if (DME_Api_JSONHandler<T>.FromString(jsonData, obj)){
			rstatus = DME_API_SUCCESS;
			DME_Api_StatusObject sobj;
			if (Class.CastTo(sobj, obj)){
				switch (sobj.Status) {
					case "NotFound":
						rstatus = DME_API_NOTFOUND;
						break;
					case "NoResults":
						rstatus = DME_API_EMPTY;
						break;
					case "Error":
						rstatus = DME_API_ERROR;
						break;
					case "NoPerms":
						rstatus = DME_API_UNAUTHORIZED;
						break;
					case "NoAuth":
						rstatus = DME_API_UNAUTHORIZED;
						break;
					case "InvalidAuth":
						rstatus = DME_API_UNAUTHORIZED;
						break;
					case "NotSetup":
						rstatus = DME_API_NOTSETUP;
						break;
				}
			}
		}
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, T>(cid, rstatus, OID, obj));
		}
	}
}

class DME_Api_JSONCallback extends DME_Api_CallbackBase {
	
	override void OnError(int errorCode, int cid) {
		if (!g_Game) return;
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, string>(cid, errorCode, OID, "{}"));
		}
	}
		
	override void OnSuccess(string jsonData, int cid) {
		if (!g_Game) return;
		if (GetInstance() && Function != ""){
			g_Game.GameScript.CallFunctionParams(GetInstance(), Function, NULL, new Param4<int, int, string, string>(cid, DME_API_SUCCESS, OID, jsonData));
		}
	}
}



class DME_Api_CallbackBase extends Managed{

	protected Class Instance;
	protected string Function;
	protected string OID;

	
	protected Class GetInstance(){
		return Instance;
	}
	
	void DME_Api_CallbackBase(Class instance, string function, string oid = ""){
		Instance = instance;
		Function = function;
		OID = oid;
	}
	
	//So I can set it automaticly to save on coding for other devs
	void SetOID(string oid){
		if (OID == "" && oid != ""){
			OID = oid;
		}
	}
	
	void OnError(int errorCode, int cid) {
		Error2("[DME_Api] Callback Error", "Error calling back OnError, not set up correctly CallId: " + cid);
	}
		
	void OnSuccess(string jsonData, int cid) {
		Error2("[DME_Api] Callback Error", "Error calling back OnSuccess, not set up correctly CallId: " + cid);
	}
}

class DME_Api_DBNestedCallBack : RestCallback
{
	protected int CallId;
	protected ref DME_Api_CallbackBase m_CB;

	
	protected DME_Api_CallbackBase GetCB(){
		return m_CB;
	}
	
	void DME_Api_DBNestedCallBack(DME_Api_CallbackBase cb, int callId){
		m_CB = cb;
		CallId = callId;
	}
	
	override void OnError(int errorCode) {
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnError " + DME_Api().ErrorToString(errorCode));
			return;
		}
		int rstatus = DME_API_SERVERERROR;
		if (errorCode == ERestResultState.EREST_ERROR_CLIENTERROR){
			rstatus = DME_API_CLIENTERROR;
		}
		GetCB().OnError(rstatus, CallId);
	};
	
	override void OnTimeout() {
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnTimeout");
			return;
		}
		
		GetCB().OnError(DME_API_TIMEOUT, CallId);
	};
	
	override void OnSuccess(string data, int dataSize) {
		if (DME_Api().IsCallCanceled(CallId)){
			Print("[DME_Api] Call " + CallId + " not called as it was requested to be canceled - OnSuccess");
			return;
		}
		if (dataSize <= 0 || data == "{}" || data == "" || data == "{ }"){
			GetCB().OnError(DME_API_EMPTY, CallId);
			return;
		}
		GetCB().OnSuccess(data, CallId);
	};
};

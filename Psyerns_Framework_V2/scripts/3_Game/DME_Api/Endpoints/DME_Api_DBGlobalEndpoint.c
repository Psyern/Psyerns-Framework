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
class DME_Api_DBGlobalEndpoint extends DME_Api_BaseEndpoint {
	
	override protected string EndpointBaseUrl(){
		return DME_Api_GetConfig().GetBaseURL() + "Globals/";
	}
	
	int Save(string mod, string jsonString) {	
		int cid = DME_Api().CallId();	
		string endpoint = "/Save/" + mod;
		if (mod && jsonString){
			Post(endpoint,jsonString, new DME_Api_SilentCallBack());
		} else {
			Print("[DME_Api] [Api] Error Saving " + endpoint + " Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	
	int Save(string mod, string jsonString, Class cbInstance, string cbFunction) {	
		int cid = DME_Api().CallId();	
		string endpoint = "/Save/" + mod;		
		if (mod && jsonString){
			Post(endpoint,jsonString, new DME_Api_DBCallBack(cbInstance, cbFunction, cid, mod));
		} else {
			Print("[DME_Api] [Api] Error Saving " + endpoint + " Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	int Save(string mod, string jsonString, DME_Api_CallbackBase cb) {	
		int cid = DME_Api().CallId();	
		string endpoint = "/Save/" + mod;

		if (mod && jsonString && cb){
			cb.SetOID(mod); //Only sets if not set
			Post(endpoint,jsonString, new DME_Api_DBNestedCallBack(cb, cid));
		} else {
			Print("[DME_Api] [Api] Error Saving " + endpoint + " Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	
	int Load(string mod, Class cbInstance, string cbFunction, string jsonString = "{}") {		
		int cid = DME_Api().CallId();
		string endpoint = "/Load/" + mod;

		if (mod && jsonString){
			Post(endpoint,jsonString,new DME_Api_DBCallBack(cbInstance, cbFunction, cid, mod));
		} else {
			Print("[DME_Api] [Api] Error Loading Player Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	
	int Load(string mod, DME_Api_CallbackBase cb, string jsonString = "{}") {		
		int cid = DME_Api().CallId();
		string endpoint = "/Load/" + mod;
		if (mod && cb && jsonString){
			cb.SetOID(mod); //Only sets if not set
			Post(endpoint,jsonString, new DME_Api_DBNestedCallBack(cb, cid));
		} else {
			Print("[DME_Api] [Api] Error Loading Player Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	
	int Increment(string mod, string element, float value = 1){
		return Transaction(mod, element, value);
	}
	
	int Transaction(string mod, string element, float value) {
		int cid = DME_Api().CallId();
		string endpoint = "/Transaction/" + mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && mod){
			Post(endpoint,transaction.ToJson(),new DME_Api_SilentCallBack());
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
			cid = -1;
		}
		return cid;
	}
	
	int Transaction(string mod, string element, float value, Class cbInstance, string cbFunction) {
		int cid = DME_Api().CallId();
		string endpoint = "/Transaction/" + mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && mod){
			Post(endpoint,transaction.ToJson(), new DME_Api_DBCallBack(cbInstance, cbFunction, cid, mod));
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
			cid = -1;
		}
		return cid;
	}
	
	int Transaction(string mod, string element, float value, DME_Api_CallbackBase cb) {
		int cid = DME_Api().CallId();
		string endpoint = "/Transaction/" + mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && mod){
			cb.SetOID(mod); //Only sets if not set
			Post(endpoint,transaction.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
			cid = -1;
		}
		return cid;
	}
	
		
	int Update(string mod, string element, string value, string operation = DME_Api_UpdateOpts.SET, Class cbInstance = NULL, string cbFunction = "") {	
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, mod);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string endpoint = "/Update/" + mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		if ( element && updatedata && DBCBX){
			Post(endpoint, updatedata.ToJson(), DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
			cid = -1;
		}
		return cid;
	}

}

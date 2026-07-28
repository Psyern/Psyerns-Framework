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
class DME_Api_DBEndpoint extends DME_Api_BaseEndpoint {
		
	protected string m_Collection = "Object";
	
	void DME_Api_DBEndpoint(string collection){
		m_Collection = collection;
	}
	
	override protected string EndpointBaseUrl(){
		return DME_Api_GetConfig().GetBaseURL() + m_Collection + "/";
	}
	
	int Save(string mod, string oid, string jsonString) {	
		if (mod == "" || oid == "" || jsonString == ""){
			Error2("[DME_Api] Error on DB Save","OID, jsonString and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();	
		string endpoint = "Save/" + oid + "/" + mod;
		
		Post(endpoint,jsonString,new DME_Api_SilentCallBack());
		
		return cid;
	}
	
	int Save(string mod, string oid, string jsonString, Class cbInstance, string cbFunction) {	
		if (mod == "" || oid == "" || jsonString == ""){
			Error2("[DME_Api] Error on DB Save","OID, jsonString and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();	
		string endpoint = "Save/" + oid + "/" + mod;

		Post(endpoint,jsonString, new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		
		return cid;
	}
	
	int Save(string mod, string oid, string jsonString, DME_Api_CallbackBase cb) {	
		if (mod == "" || oid == "" || jsonString == "" || !cb){
			Error2("[DME_Api] Error on DB Save","OID and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();	
		string endpoint = "Save/" + oid + "/" + mod;
		
		cb.SetOID(oid); //Only sets if not set
		
		Post(endpoint,jsonString, new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int Load(string mod, string oid, DME_Api_CallbackBase cb, string jsonString = "{}") {		
		if (mod == "" || oid == "" || jsonString == "" || !cb){
			Error2("[DME_Api] Error on DB Load","OID and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		string endpoint = "Load/" + oid + "/" + mod;
		
		cb.SetOID(oid); //Only sets if not set
		
		Post(endpoint,jsonString,new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int Load(string mod, string oid, Class cbInstance, string cbFunction, string jsonString = "{}") {		
		if (mod == "" || oid == "" || jsonString == ""){
			Error2("[DME_Api] Error on DB Load","OID, jsonString and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		string endpoint = "Load/" + oid + "/" + mod;
		
		Post(endpoint,jsonString, new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		
		return cid;
	}
	
	
	int Query(string mod, DME_Api_QueryBase query, DME_Api_CallbackBase cb) {
		if (mod == "" || !query || !cb){
			Error2("[DME_Api] Error on DB Query","Mod, query and callback must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		string endpoint = "Query/" + mod;
				
		if ( query && mod && cb){
			cb.SetOID(mod); //Only sets if not set
			Post(endpoint,query.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		} else {
			Print("[DME_Api] [Api] Error Querying " +  mod);
			cid = -1;
		}
		return cid;
	}
	
	int Query(string mod, DME_Api_QueryBase query, Class cbInstance, string cbFunction) {
		if ( mod == "" || !query ){
			Error2("[DME_Api] Error on DB Query","Mod and query must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		string endpoint = "Query/" + mod;
				
		Post(endpoint,query.ToJson(),new DME_Api_DBCallBack(cbInstance, cbFunction, cid, ""));
		
		return cid;
	}
	
	int Increment(string mod, string oid, string element, float value = 1){
		if (mod == "" || oid == "" || element == ""){
			Error2("[DME_Api] Error on DB Incerment","OID and Mod must be valid strings");
			return -1;
		}
		return Transaction(mod, oid, element, value);
	}
	
	int Transaction(string mod, string oid, string element, float value) {
		if (mod == "" || oid == "" || element == ""){
			Error2("[DME_Api] Error on DB Transaction","OID, element and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
				
		string endpoint = "Transaction/" + oid   + "/" + mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		Post(endpoint,transaction.ToJson(), new DME_Api_SilentCallBack());
		
		return cid;
	}
	
	int Transaction(string mod, string oid, string element, float value, DME_Api_CallbackBase cb) {
		if (mod == "" || oid == "" || element == "" || !cb){
			Error2("[DME_Api] Error on DB Transaction","OID, element, callback and Mod must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Transaction/" + oid   + "/" + mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		cb.SetOID(oid); //Only sets if not set
			
		Post(endpoint,transaction.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int Transaction(string mod, string oid, string element, float value, float min, float max, DME_Api_CallbackBase cb) {
		if (mod == "" || oid == ""  || element == "" || !cb){
			Error2("[DME_Api] Error on DB Transaction","OID, element, callback and Mod must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Transaction/" + oid   + "/" + mod;
		
		DME_Api_ValidatedTransaction transaction = new DME_Api_ValidatedTransaction(element, value, min, max);
		
		cb.SetOID(oid); //Only sets if not set
		
		Post(endpoint,transaction.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int Transaction(string mod, string oid, string element, float value, Class cbInstance, string cbFunction) {
		if (mod == "" || element == "" || oid == ""){
			Error2("[DME_Api] Error on DB Transaction","OID, element and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Transaction/" + oid   + "/"+ mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		Post(endpoint,transaction.ToJson(), new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		
		return cid;
	}
	
	int Transaction(string mod, string oid, string element, float value, float min, float max, Class cbInstance, string cbFunction) {
		if (mod == "" || oid == "" || element == ""){
			Error2("[DME_Api] Error on DB Transaction","OID, element, and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Transaction/" + oid   + "/"+ mod;
		
		DME_Api_ValidatedTransaction transaction = new DME_Api_ValidatedTransaction(element, value, min, max);
		
		Post(endpoint, transaction.ToJson(), new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		
		return cid;
	}
	
	int Update(string mod, string oid, string element, string value, string operation = DME_Api_UpdateOpts.SET) {	
		if (mod == "" || oid == "" || element == "" || operation == ""){
			Error2("[DME_Api] Error on DB Update","OID, Element, operation, and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Update/" + oid   + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		Post(endpoint, updatedata.ToJson(), new DME_Api_SilentCallBack());
		
		return cid;
	}
		
	int Update(string mod, string oid, string element, string value, string operation, DME_Api_CallbackBase cb) {	
		if (mod == "" || oid == "" || element == "" || operation == "" || !cb){
			Error2("[DME_Api] Error on DB Update","OID, callback, operation, Element and Mod must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();

		string endpoint = "Update/" + oid   + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		cb.SetOID(oid); //Only sets if not set
		Post(endpoint, updatedata.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int Update(string mod, string oid, string element, string value, string operation, Class cbInstance, string cbFunction) {	
		if (mod == "" || oid == "" || element == "" || operation == ""){
			Error2("[DME_Api] Error on DB Update","OID, Element, operation, and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string endpoint = "Update/" + oid   + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		Post(endpoint, updatedata.ToJson(), DBCBX);
		
		return cid;
	}
	
	
	int QueryUpdate(DME_Api_QueryBase query, string mod, string element, string value, string operation = DME_Api_UpdateOpts.SET) {	
		if (!query || mod == "" || element == "" || operation == ""){
			Error2("[DME_Api] Error on DB Update","OID, Element, operation, and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		
		string endpoint = "Query/Update/" + mod;
		
		DME_Api_DBQueryUpdate updatedata = new DME_Api_DBQueryUpdate(query, element, value, operation);
		
		Post(endpoint, updatedata.ToJson(), new DME_Api_SilentCallBack());
		
		return cid;
	}
	
	int QueryUpdate(DME_Api_QueryBase query, string mod, string element, string value, string operation, DME_Api_CallbackBase cb) {	
		if (!query || mod == "" || element == "" || operation == "" || !cb){
			Error2("[DME_Api] Error on DB Update","OID, callback, operation, Element and Mod must be valid");
			return -1;
		}
		int cid = DME_Api().CallId();

		string endpoint = "Query/Update/" + mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		cb.SetOID(mod); //Only sets if not set
		Post(endpoint, updatedata.ToJson(), new DME_Api_DBNestedCallBack(cb, cid));
		
		return cid;
	}
	
	int QueryUpdate(DME_Api_QueryBase query, string mod, string element, string value, string operation, Class cbInstance, string cbFunction) {
		if (!query || mod == "" || element == "" || operation == ""){
			Error2("[DME_Api] Error on DB Update","OID, Element, operation, and Mod must be valid strings");
			return -1;
		}
		int cid = DME_Api().CallId();
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, mod);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		string endpoint = "Query/Update/" + mod;
		
		DME_Api_DBQueryUpdate updatedata = new DME_Api_DBQueryUpdate(query,element, value, operation);
		
		Post(endpoint, updatedata.ToJson(), DBCBX);
		
		return cid;
		
	}
	
	
	//Only Works on Player Data	
	int PublicSave(string mod, string oid, string jsonString, Class cbInstance = NULL, string cbFunction = "") {	
		if (m_Collection != "Player") return -1;
		int cid = DME_Api().CallId();	
		string endpoint = "PublicSave/" + oid + "/" + mod;
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		if (jsonString){
			Post(endpoint,jsonString,DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Saving " + endpoint + " Data for " + mod);
			cid = -1;
		}
		return cid;
	}
	
	int PublicLoad(string mod, string oid, Class cbInstance, string cbFunction, string jsonString = "{}", string baseUrl = "") {		
		if (m_Collection != "Player") return -1;
		int cid = DME_Api().CallId();
		string endpoint = "PublicLoad/" + oid + "/" + mod;
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		if ( baseUrl != "" && DBCBX ){
			string url = baseUrl + m_Collection + "/" + endpoint;
			//Print("[DME_Api] Public Load with custom Base: " + url);
			DME_Api().Post(url,jsonString,DBCBX);
		} else if (DBCBX){
			Post(endpoint,jsonString,DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Loading Player Data for " + mod);
			cid = -1;
		}
		return cid;
	}


}

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
class DME_Api_Rest extends Managed
{		
	protected static RestApi Api()
	{
		RestApi clCore = GetRestApi();
		if (!clCore)
		{
			clCore = CreateRestApi();
			clCore.SetOption(ERestOption.ERESTOPTION_READOPERATION, 15);
		}
		return clCore;
	}
	
	protected static void Post(string url, string jsonString = "{}", RestCallback UCBX = NULL)
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx =  Api().GetRestContext(url);
		ctx.SetHeader(DME_Api().GetAuthToken());
		ctx.POST(UCBX , "", jsonString);
	}
	
	protected static void Get(string url, RestCallback UCBX = NULL)
	{
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		RestContext ctx =  Api().GetRestContext(url);
		ctx.GET(UCBX , "");
	}
	
	protected static string BaseUrl(){
		return DME_Api_GetConfig().ServerURL;
	}
	
	static void GetAuth( string guid ){
		string url = BaseUrl() + "GetAuth/" + guid;
		
		Post(url, "{}", new DME_Api_AuthCallBack(guid));
	}
	
	static void PlayerSave(string mod, string guid, string jsonString, RestCallback UCBX = NULL) {	
		string url = BaseUrl() + "Player/Save/" + guid + "/" + mod;
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		if (jsonString){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Saving Player Data for " + mod);
		}
	}
	
	static void PlayerLoad(string mod, string guid,  RestCallback UCBX, string jsonString = "{}") {
		string url = BaseUrl() + "Player/Load/" + guid + "/" + mod;
		
		if (UCBX){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Loading Player Data for " + mod);
		}
	}
	
	static void PlayerQuery(string mod, DME_Api_QueryBase query, RestCallback UCBX) {
		string url = BaseUrl() + "Player/Query/" + mod;
		
		if ( query && UCBX){
			Post(url,query.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Querying " +  mod);
		}
	}
	
	static void PlayerIncrement(string mod, string guid, string element, float value = 1){
		PlayerTransaction(mod, guid, element, value, NULL);
	}
	
	static void PlayerTransaction(string mod, string guid, string element, float value = 1, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_TransactionCallBack;
		}
				
		string url = BaseUrl() + "Player/Transaction/" + guid   + "/"+ mod;
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && UCBX){
			Post(url,transaction.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void PlayerUpdate(string mod, string guid, string element, string value, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
				
		string url = BaseUrl() + "Player/Update/" + guid   + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void PlayerUpdateAdv(string mod, string guid, string element, string value, string operation, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Player/Update/" + guid   + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	static void GlobalsSave(string mod, string jsonString, RestCallback UCBX = NULL) {

		string url = BaseUrl() + "Globals/Save/" + mod;
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		if (jsonString){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Saving Globals Data for " + mod);
		}
	}
	
	static void GlobalsLoad(string mod, RestCallback UCBX, string jsonString = "{}") {

		string url = BaseUrl() + "Globals/Load/" + mod;


		if (UCBX){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Loading Globals Data for " + mod);
		}
	}
	
	
	static void GlobalsIncrement(string mod, string element, float value = 1){
		GlobalsTransaction(mod, element, value, NULL);
	}
	
	static void GlobalsTransaction(string mod, string element, float value = 1, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_TransactionCallBack;
		}

		string url = BaseUrl() + "Globals/Transaction/" + mod;

		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && UCBX){
			Post(url,transaction.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void GlobalsUpdate(string mod, string element, string value, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}

		string url = BaseUrl() + "Globals/Update/" + mod;

		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void GlobalsUpdateAdv(string mod, string element, string value, string operation, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}

		string url = BaseUrl() + "Globals/Update/" + mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	
	//Saving or loading an object with the ObjectId of "NewObject" will generate an Object ID for you, this Object ID will be returned
	//in the ObjectId var of the Class so make sure your Class has the varible ObjectId if you plan on using this feature
	static void ObjectSave(string mod, string objectId, string jsonString, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}

		string url = BaseUrl() + "Object/Save/" + objectId + "/" +  mod;
		
		if (jsonString){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Saving Object " + objectId + " Data for " + mod);
		}
	}
	
	static void ObjectLoad(string mod, string objectId, RestCallback UCBX, string jsonString = "{}") {
		
		string url = BaseUrl() + "Object/Load/" +  objectId + "/" + mod;
		
		if (UCBX){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Loading Object (" + objectId + ") Data for " + mod);
		}
	}
	
	static void ObjectQuery(string mod, DME_Api_QueryBase query, RestCallback UCBX) {
		
		string url = BaseUrl() + "Object/Query/" + mod;
		
		if ( query && UCBX){
			Post(url,query.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Querying " +  mod);
		}
	}
	
	static void ObjectIncrement(string mod, string objectId, string element, float value = 1){
		ObjectTransaction(mod, objectId, element, value, NULL);
	}
	
	static void ObjectTransaction(string mod, string objectId, string element, float value = 1, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_TransactionCallBack;
		}
		
		string url = BaseUrl() + "Object/Transaction/" + objectId + "/"+ mod;
		
		
		DME_Api_Transaction transaction = new DME_Api_Transaction(element, value);
		
		if ( element && transaction && UCBX){
			Post(url,transaction.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void ObjectUpdate(string mod, string guid, string element, string value, RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Object/Update/" + guid  + "/"+ mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	//String Values must be wrapped with Quotes example string newValue = "\"NewValue\""
	static void ObjectUpdateAdv(string mod, string guid, string element, string value, string operation = "set", RestCallback UCBX = NULL) {
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Object/Update/" + guid  + "/" + mod;
		
		DME_Api_UpdateData updatedata = new DME_Api_UpdateData(element, value, operation);
		
		if ( element && updatedata && UCBX){
			Post(url,updatedata.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Transaction " +  mod);
		}
	}
	
	static void Request(DME_Api_Forwarder data, RestCallback UCBX = NULL){
				
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Forward";
		
		if ( data && UCBX){
			Post(url,data.ToJson(),UCBX);
		} else {
			Print("[DME_Api] [Api] Error Fowarding ");
		}
	}

	static void Log(string jsonString, RestCallback UCBX = NULL){
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Logger/One/" + DME_Api_GetConfig().ServerID;
		
		if ( jsonString && UCBX){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Fowarding ");
		}
	}
	
	//JsonFileLoader<array<autoptr LogObject>>.JsonMakeData(AnArrayOfYourObjects);
	static void LogBulk(string jsonString, RestCallback UCBX = NULL){
		
		if (!UCBX){
			UCBX = new DME_Api_SilentCallBack;
		}
		
		string url = BaseUrl() + "Logger/Many/" + DME_Api_GetConfig().ServerID;
		if (jsonString && UCBX){
			Post(url,jsonString,UCBX);
		} else {
			Print("[DME_Api] [Api] Error Fowarding ");
		}
	}	
	
};
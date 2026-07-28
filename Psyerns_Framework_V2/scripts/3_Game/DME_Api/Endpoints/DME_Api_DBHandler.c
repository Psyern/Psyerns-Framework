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
/* 
	Template DB Handler
	This is the newest Method in which modders can interact with the DME_Api's MongoDB Endpoints providing access to modders to be
	able to build mods that utilize the DME_Api Webservice.
	
	view full documentation https://github.com/daemonforge/DayZ-UniveralApi/wiki/Developer-Reference

	static autoptr DME_Api_DBHandler<myClass> m_MyModHandler = new DME_Api_DBHandler<myClass>("MyMod", DME_API_PLAYER_DB); //DME_API_PLAYER_DB or DME_API_OBJECT_DB
	
	m_MyModHandler.Save("GUID", myObject); //returns Call ID
	m_MyModHandler.Load("GUID", player, "MyCallBackFunction"); //returns Call ID


	
*/

class DME_Api_DBHandler<Class T> extends DME_Api_DBHandlerBase{

	/*
	Load and Save
	
	CALLBACK FUNCTION EXAMPLE
	protected void MyCallBackFunction(int cid, int status, string guid, myClass data) {
		if ( status == DME_API_SUCCESS ){
			//Do something with data
		}
	}*/
	
	
	override int Save(string oid, Class object) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, object) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			return DME_Api().db(Database).Save(Mod, oid, jsonString);
		}
		Error2("[DME_Api] DB HANDLER Save", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	override int Save(string oid, Class object, Class cbInstance, string cbFunction) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, object) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			return DME_Api().db(Database).Save(Mod, oid, jsonString, new DME_Api_Callback<T>(cbInstance, cbFunction));
		}
		Error2("[DME_Api] DB HANDLER Save", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	
	
	
	
	override int Load(string oid, Class cbInstance, string cbFunction) {
		return DME_Api().db(Database).Load(Mod,oid, new DME_Api_Callback<T>(cbInstance, cbFunction), "{}");
	}
	override int Load(string oid, Class cbInstance, string cbFunction, string defaultJson) {
		return DME_Api().db(Database).Load(Mod,oid, new DME_Api_Callback<T>(cbInstance, cbFunction), defaultJson);
	}
	override int Load(string oid, Class cbInstance, string cbFunction, Class inObject) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, inObject) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			DME_Api_CallbackLoader<T> cb = new DME_Api_CallbackLoader<T>(cbInstance, cbFunction);
			cb.SetObject(obj);
			return DME_Api().db(Database).Load(Mod, oid, cb, jsonString);
		} 
		Error2("[DME_Api] DB HANDLER Load", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	
	
	
	/*
	
	Query
	
	
	Query Will return a DME_Api_QueryResult with your Class Callback function for this would be
	
	//CALLBACK FUNCTION EXAMPLE
	protected void MyCallBackFunction(int cid, int status, string guid, DME_Api_QueryResult<myClass> data) {
		if ( status == DME_API_SUCCESS ){
			//Do something with data
			array<autoptr myClass> results = data.GetResults();
		} else if (status == DME_API_EMPTY){ // no results

		}
	}
	
	Query Example "{ \"MyVar\": 3 }" would return all objects that have MyVar = 3
	Query Example "{ \"MyVar\": { \"$gte\": 5} }" would return all objects that have MyVar = 5 or higher
	Query Example "{ \"MyTStrringArray\": "somevalue" }" would return all objects that have a value of 'somevalue' inside the array
	Query Example "{ \"MySubObject.SubVar\": 3 }" would return all objects that have a SubVar = 3 inside a sub object this also works if the sub object was an array
	
	
	*/
	override int Query(DME_Api_QueryBase query, Class cbInstance, string cbFunction) {
		return DME_Api().db(Database).Query(Mod,query,new DME_Api_Callback<DME_Api_QueryResult<T>>(cbInstance, cbFunction));
	}
	override int Query(string query, Class cbInstance, string cbFunction) {
		return DME_Api().db(Database).Query(Mod, new DME_Api_DBQuery(query),new DME_Api_Callback<DME_Api_QueryResult<T>>(cbInstance, cbFunction));
	}
}




//just to be able to manage them in like an array or map?
class DME_Api_DBHandlerBase extends Managed {
	
	string Mod = "";
	int Database = DME_API_PLAYER_DB;
	
	void DME_Api_DBHandlerBase(string mod, int database = DME_API_PLAYER_DB){
		Mod = mod;
		Database = database;
	}
	
	int Save(string oid, Class object) {
		Error2("[DME_Api] DME_Api_DBHandlerBase SAVE","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Save(string oid, Class object, Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_DBHandlerBase SAVE","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(string oid, Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_DBHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(string oid, Class cbInstance, string cbFunction, string defaultJson) {
		Error2("[DME_Api] DME_Api_DBHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(string oid, Class cbInstance, string cbFunction, Class inObject){
		Error2("[DME_Api] DME_Api_DBHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	
	/*
	
		LoadJson Returns string instead of object
	
		CALLBACK FUNCTION EXAMPLE
		protected void MyCallBackFunction(int cid, int status, string guid, string data) {
			if ( status == DME_API_SUCCESS ){
				//Do something with data you use 
				autoptr myClass obj;
				if (DME_Api_JSONHandler<myClass>.GetString(obj, data)){
					
				}
			}
		}
	*/
	int LoadJson(string oid, Class cbInstance, string cbFunction, string defaultJson = "{}") {
		return DME_Api().db(Database).Load(Mod, oid, cbInstance, cbFunction, defaultJson);
	}
	
	int Increment(string oid, string element, float value = 1){
		return Transaction(oid, element, value);
	}
	
	/*
		Transactions
	
	
		Updates a sub value inside the object in the database then returns the new value only works with floats or ints
		Sub objects can be used with dot notation aka MySubObject.SubObjectVar
		Will return status of DME_API_SUCCESS if operations was successful
	*/
	int Transaction(string oid, string element, float value) {
		return DME_Api().db(Database).Transaction(Mod,oid,element,value);
	}
	int Transaction(string oid, string element, float value, Class cbInstance, string cbFunction) {
		return DME_Api().db(Database).Transaction(Mod, oid, element, value, new DME_Api_Callback<DME_Api_TransactionResponse>(cbInstance, cbFunction));
	}
	int Transaction(string oid, string element, float value, float min, float max, Class cbInstance, string cbFunction) {
		return DME_Api().db(Database).Transaction(Mod, oid, element, value, min, max, new DME_Api_Callback<DME_Api_TransactionResponse>(cbInstance, cbFunction));
	}
	
	
	/*
	Update
	
		Updates a sub value inside the object in the database, can also use other operations
		https://github.com/daemonforge/DayZ-UniveralApi/blob/master/_UniversalApi/scripts/1_Core/Constants.c#L30
	
		Values can be in JSON format to update or push elements into arrays
		
		Sub objects can be used with dot notation aka MySubObject.SubObjectVar
		will return status of DME_API_SUCCESS if operations was successful
	*/
	int Update(string oid, string element, string value, string operation = DME_Api_UpdateOpts.SET) {
		return DME_Api().db(Database).Update(Mod, oid, element, value, operation);
	}
	int Update(string oid, string element, string value, string operation, Class cbInstance, string cbFunction) {	
		return DME_Api().db(Database).Update(Mod, oid, element, value, operation, new DME_Api_Callback<DME_Api_UpdateResponse>(cbInstance, cbFunction) );
	}
	
	
	int QueryUpdate(DME_Api_QueryBase query, string element, string value, string operation = DME_Api_UpdateOpts.SET) {
		return DME_Api().db(Database).QueryUpdate(query, Mod, element, value, operation);
	}
	int QueryUpdate(DME_Api_QueryBase query, string element, string value, string operation, Class cbInstance, string cbFunction) {	
		return DME_Api().db(Database).QueryUpdate(query, Mod, element, value, operation, new DME_Api_Callback<DME_Api_QueryUpdateResponse>(cbInstance, cbFunction) );
	}
	
	
	int Query(DME_Api_QueryBase query, Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_DBHandlerBase QUERY","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	int Query(string query, Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_DBHandlerBase QUERY","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	
	/* 
		Call Cancel
		
		This allows you to cancel a call back to prevent access violations 
	*/
	void Cancel(int cid){
		DME_Api().RequestCallCancel(cid);
	}
}

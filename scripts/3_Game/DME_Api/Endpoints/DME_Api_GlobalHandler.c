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
	Template Global Handler
	This is the newest Method in which modders can interact with the DME_Api's MongoDB Endpoints providing access to modders to be
	able to build mods that utilize the DME_Api Webservice.
	
	view full documentation https://github.com/daemonforge/DayZ-UniveralApi/wiki/Developer-Reference

	static autoptr DME_Api_GlobalHandler<myClass> m_MyModHandler = new DME_Api_GlobalHandler<myClass>("MyMod");
	
	m_MyModHandler.Save(myObject); //returns Call ID
	m_MyModHandler.Load(player, "MyCallBackFunction"); //returns Call ID


	
*/

class DME_Api_GlobalHandler<Class T> extends DME_Api_GlobalHandlerBase{

	/*
	Load and Save
	
	CALLBACK FUNCTION EXAMPLE
	protected void MyCallBackFunction(int cid, int status, string mod, myClass data) {
		if ( status == DME_API_SUCCESS ){
			//Do something with data
		}
	}*/
		
	override int Save(Class object) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, object) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			return DME_Api().globals().Save(Mod,jsonString);
		}
		Error2("[DME_Api] DB HANDLER Save", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	override int Save(Class object, Class cbInstance, string cbFunction) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, object) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			return DME_Api().globals().Save(Mod, jsonString, new DME_Api_Callback<T>(cbInstance, cbFunction));
		}
		Error2("[DME_Api] DB HANDLER Save", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	
	
	
	override int Load(Class cbInstance, string cbFunction) {
		return DME_Api().globals().Load(Mod,new DME_Api_Callback<T>(cbInstance, cbFunction), "{}");
	}
	override int Load(Class cbInstance, string cbFunction, string defaultJson) {
		return DME_Api().globals().Load(Mod,new DME_Api_Callback<T>(cbInstance, cbFunction), defaultJson);
	}
	override int Load(Class cbInstance, string cbFunction, Class inObject) {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, inObject) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			DME_Api_CallbackLoader<T> cb = new DME_Api_CallbackLoader<T>(cbInstance, cbFunction);
			cb.SetObject(obj);
			return DME_Api().globals().Load(Mod, cb, jsonString);
		} 
		Error2("[DME_Api] DB HANDLER Load", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
	override int LoadSelf(Class cbInstance, string cbFunction = "") {
		string jsonString = "{}";
		T obj; //Might not need Casting here but using it anyways
		if (Class.CastTo(obj, cbInstance) && DME_Api_JSONHandler<T>.GetString(obj, jsonString)) {
			DME_Api_CallbackLoader<T> cb = new DME_Api_CallbackLoader<T>(cbInstance, cbFunction);
			cb.SetObject(obj);
			return DME_Api().globals().Load(Mod, cb, jsonString);
		} 
		Error2("[DME_Api] DB HANDLER LoadSelf", "Error convertering to JSON or casting make sure you are passing the right class type");
		return -1;
	}
}


//just to be able to manage them in like an array or map?
class DME_Api_GlobalHandlerBase extends Managed {
	
	string Mod = "";
	
	void DME_Api_GlobalHandlerBase(string mod){
		Mod = mod;
	}
	
	int Save(Class object) {
		Error2("[DME_Api] DME_Api_GlobalHandlerBase SAVE","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Save(Class object, Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_GlobalHandlerBase SAVE","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(Class cbInstance, string cbFunction) {
		Error2("[DME_Api] DME_Api_GlobalHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(Class cbInstance, string cbFunction, string defaultJson) {
		Error2("[DME_Api] DME_Api_GlobalHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	
	int Load(Class cbInstance, string cbFunction, Class inObject){
		Error2("[DME_Api] DME_Api_GlobalHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
		return -1;
	}
	int LoadSelf(Class cbInstance, string cbFunction = ""){
		Error2("[DME_Api] DME_Api_GlobalHandlerBase LOAD","Incorrect Ussage class is not type of DME_Api_DBHandler<T>");
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
	int LoadJson(Class cbInstance, string cbFunction, string defaultJson = "{}") {
		return DME_Api().globals().Load(Mod, cbInstance, cbFunction, defaultJson);
	}
	
	int Increment(string element, float value = 1){
		return Transaction(element, value);
	}
	
	/*
		Transactions
	
	
		Updates a sub value inside the object in the database then returns the new value only works with floats or ints
		Sub objects can be used with dot notation aka MySubObject.SubObjectVar
		Will return status of DME_API_SUCCESS if operations was successful
	*/
	int Transaction(string element, float value) {
		return DME_Api().globals().Transaction(Mod,element,value);
	}
	int Transaction(string element, float value, Class cbInstance, string cbFunction) {
		return DME_Api().globals().Transaction(Mod, element, value, new DME_Api_Callback<DME_Api_TransactionResponse>(cbInstance, cbFunction));
	}
	//int Transaction(string element, float value, float min, float max, Class cbInstance, string cbFunction) {
	//	return DME_Api().globals().Transaction(Mod, element, value, min, max, new DME_Api_Callback<DME_Api_TransactionResponse>(cbInstance, cbFunction));
	//}
	
	
	/*
	Update
	
		Updates a sub value inside the object in the database, can also use other operations
		https://github.com/daemonforge/DayZ-UniveralApi/blob/master/_UniversalApi/scripts/1_Core/Constants.c#L30
	
		Values can be in JSON format to update or push elements into arrays
		
		Sub objects can be used with dot notation aka MySubObject.SubObjectVar
		will return status of DME_API_SUCCESS if operations was successful
	*/
	int Update(string element, string value, string operation = DME_Api_UpdateOpts.SET) {
		return DME_Api().globals().Update(Mod, element, value, operation);
	}
	int Update(string element, string value, string operation, Class cbInstance, string cbFunction) {	
		return DME_Api().globals().Update(Mod, element, value, operation, new DME_Api_Callback<DME_Api_UpdateResponse>(cbInstance, cbFunction) );
	}
	
	
	/* 
		Call Cancel
		
		This allows you to cancel a call back to prevent access violations 
	*/
	static void Cancel(int cid){
		DME_Api().RequestCallCancel(cid);
	}
}

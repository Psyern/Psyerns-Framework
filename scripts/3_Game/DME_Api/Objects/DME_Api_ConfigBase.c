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
Config Base Class

   This is just a template on how you could build your config classes you can inherit this 
but at the end of the day this is just a template to help modders newer to API something 
to start from

*/
class DME_Api_ConfigBase : RestCallback {
	protected bool m_DataReceived = false;
	
	
	void Load(){
		m_DataReceived = false;
		SetDefaults();//Set the Defaults so that way, when you load if this its the server Requesting the data it will create it based on the defaults
		
		/*
		Global Configs
		DME_Api().Rest().GlobalsLoad("MODNAME", this, this.ToJson());
		*/
	}
	
	void Load( string ID){
		m_DataReceived = false;
		SetDefaults();//Set the Defaults so that way, when you load if this its the server Requesting the data it will create it based on the defaults
		
		/*
		Player Configs
		DME_Api().Rest().PlayerLoad("MODNAME", ID, this, this.ToJson());
		
		Item Configs / Party Configs / anything that could have an ID Number
		DME_Api().Rest().ItemLoad("MODNAME", ID, this, this.ToJson());
		*/
	}
	

	
	void Save(){
		/* 
		if (g_Game.IsServer()){	//By Default the API is configure to only allow save operations from the server AUTH
		
			Global Configs
			DME_Api().Rest().GlobalsSave("MODNAME", this.ToJson());
		
			Player Configs
			DME_Api().Rest().PlayerSave("MODNAME", PlayerGUID, this.ToJson());
		
			Item Configs / Party Configs / anything that could have an ID Number
			DME_Api().Rest().ItemSave("MODNAME", ItemId, this.ToJson());
		
		}
		*/
	}
	
	void SetDefaults(){
		/*
	
		  This is to set the defaults for the mod before requesting a load so that way 
		if it doesn't exsit the API will create the file
	
		*/
	}
	
	string ToJson(){
		// Override and Replace with your class Name
		string jsonString = DME_Api_JSONHandler<DME_Api_ConfigBase>.ToString(this);
		Print("[DME_Api] Error You didn't override ToJson: " + jsonString); 
		return jsonString;
	}
	
	
	
	
	void SetDataReceived(bool dataReceived = true){
		m_DataReceived = dataReceived;
	}
	
	bool DataReceived(){
		return m_DataReceived;
	}
	
	void OnDataReceive(){
		SetDataReceived();
		/*
		if(ModVersion != CurrentVersion){
			DoSome Code Upgrade
		
			Save(); //Resave the upgrade Version Back to the server
		}
		*/
	}
	
	
	// This is called by the API System on the successfull response from the API
	override void OnSuccess(string data, int dataSize) {
		JsonFileLoader<DME_Api_ConfigBase>.JsonLoadData(data, this);
		if (this){
			OnDataReceive();
		} else {
			Print("[DME_Api] CallBack Failed errorCode: Invalid Data");
		}
	};
	
	
		
	// This Are Called by the API System on errors from the API System
	override void OnError(int errorCode) {
		Print("[DME_Api] CallBack Failed errorCode: " + DME_Api().ErrorToString(errorCode));		
	};
	
	override void OnTimeout() {
		Print("[DME_Api] CallBack Failed errorCode: Timeout");
	};
}
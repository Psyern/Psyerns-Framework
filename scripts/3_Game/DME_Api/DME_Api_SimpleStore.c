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
class DME_Api_SimpleValueStore extends Managed {
	string Value = "";
	void DME_Api_SimpleValueStore(string value){
		Value = value;
	}
	string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_SimpleValueStore>.ToString(this);
		return jsonString;
	}
	
	static string StoreValue(string value){
		DME_Api_SimpleValueStore obj = new DME_Api_SimpleValueStore(value);
		return obj.ToJson();
	}
	
	static string GetValue(string json){
		DME_Api_SimpleValueStore obj;
		if (DME_Api_JSONHandler<DME_Api_SimpleValueStore>.FromString(json, obj)){
			return obj.Value;
		}
		return "";
	}
	
	static string GenerateJson(string data){
		return StoreValue(data);
	}
	
	static string UseJson(string jsonData){
		return GetValue(jsonData);
	}
}
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
class DME_Api_Forwarder extends Managed{

	string URL = "";
    ref array<ref DME_Api_Headers> Headers = new array<ref DME_Api_Headers>;
    string Method = "post";
    string Body = "";
    string ReturnValue = "";
    int ReturnValueArrayIndex = -1;
	
	void DME_Api_Forwarder( string url, string body = "{}", array<ref DME_Api_Headers> headers = NULL ){
		URL = url;
		if (headers == NULL){
			Headers.Insert(new DME_Api_Headers("Content-Type", "application/json"));
		}
		Body = body;
	}
	
	void AddHeader(string key, string value){
		Headers.Insert(new DME_Api_Headers(key, value));
	} 
	
	string ToJson(){
		return DME_Api_JSONHandler<DME_Api_Forwarder>.ToString(this);
	}
}

class DME_Api_Headers{
	string Key;
	string Value;
	
	void DME_Api_Headers(string key, string value){
		Key = key;
		Value = value;
	}
	
}
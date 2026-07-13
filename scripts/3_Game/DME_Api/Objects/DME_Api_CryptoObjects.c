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
class DME_Api_CryptoRequest extends DME_Api_Object_Base {
	
	ref TStringArray From = new TStringArray;
	
	void DME_Api_CryptoRequest(TStringArray from){
		From.Copy(from);
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_CryptoRequest>.ToString(this);
		return jsonString;
	}
}

class DME_Api_CryptoConvertRequest extends DME_Api_Object_Base {
	
	float Value = 0;
	
	void DME_Api_CryptoConvertRequest(float value){
		Value = value;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_CryptoConvertRequest>.ToString(this);
		return jsonString;
	}
}

class DME_Api_CryptoConvertResult extends DME_Api_StatusObject{
	float Value;
	
	float Get(){
		return Value;
	}
} 

class DME_Api_CryptoResults extends DME_Api_StatusObject{
	ref map<string,float> Values;
	map<string,float> Get(){
		map<string,float> rValue = new  map<string,float>;
		rValue.Copy(Values);
		return rValue;
	}
}
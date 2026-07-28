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
class DME_Api_Transaction extends DME_Api_Object_Base {
	string Element;
	float Value;
	
	void DME_Api_Transaction(string element, float value){
		Element = element;
		Value = value;
	}
	
	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_Transaction>.JsonMakeData(this);
		return jsonString;
	}
	
};

class DME_Api_ValidatedTransaction extends DME_Api_Object_Base {
	string Element;
	float Value;
	float Min;
	float Max;
	
	void DME_Api_ValidatedTransaction(string element, float value, float min, float max){
		Element = element;
		Value = value;
		Min = min;
		Max = max;
	}
	
	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_ValidatedTransaction>.JsonMakeData(this);
		return jsonString;
	}
	
};

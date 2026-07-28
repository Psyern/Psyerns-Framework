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
class DME_Api_UpdateData extends DME_Api_Object_Base {
	string Element;
	string Operation = DME_Api_UpdateOpts.SET; // set | push | pull | unset | mul | rename | pullAll
	string Value;
	
	void DME_Api_UpdateData(string element, string value, string operation = DME_Api_UpdateOpts.SET){
		Element = element;
		Value = value;
		Operation = operation;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_UpdateData>.ToString(this);
		return jsonString;
	}
	
};
class DME_Api_QueryUpdateResponse extends DME_Api_StatusObject {
	string Element;
	string Mod;
	int Count;
}

class DME_Api_UpdateResponse extends DME_Api_TransactionResponse {
	float Value;
}

class DME_Api_TransactionResponse extends DME_Api_StatusObject {
	string ID;
	string Element;
	string Mod;
}
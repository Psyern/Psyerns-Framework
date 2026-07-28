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
class DME_Api_TextObject extends DME_Api_Object_Base{
	
	string Text = "";
	
	void DME_Api_TextObject(string text){
		Text = text;
	}
	
	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_TextObject>.JsonMakeData(this);
		return jsonString;
	}
}

class DME_Api_ToxicityResponse extends DME_Api_StatusObject {
	
	float IdentityAttack;
	float Insult;
	float Obscene;
	float SevereToxicity;
	float SexualExplicit;
	float Threat;
	float Toxicity;
	
}

class DME_Api_RandomNumberResponse extends DME_Api_StatusObject {
	
	ref TIntArray Numbers;
	
}
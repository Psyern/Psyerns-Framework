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
class DME_Api_StatusObject extends Managed {
	
    string Status =  "Pending"; //Success or Error
	string Error =  "Not an Error Object";
	
}


class DME_Api_Status extends DME_Api_StatusObject {
	
    string Version =  "0.0.0";
	string Discord =  "Disabled";
	string Translate =  "Disabled";
	ref TStringArray Wit;
	ref TStringArray QnA;
	ref TStringArray LUIS;
	
	int CheckVersion(string version){
		if (version == Version){
			return 0;
		}
		TStringArray ModVerMap = {};
		TStringArray ApiVerMap = {};
		version.Split(".", ModVerMap);
		Version.Split(".", ApiVerMap);
		int ModMajor = ModVerMap.Get(0).ToInt();
		int ModMinor = ModVerMap.Get(1).ToInt();
		int ModPatch = ModVerMap.Get(2).ToInt();
		int ApiMajor = ApiVerMap.Get(0).ToInt();
		int ApiMinor = ApiVerMap.Get(1).ToInt();
		int ApiPatch = ApiVerMap.Get(2).ToInt();
		if (ModMajor > ApiMajor){
			return 3;
		} 
		if (ModMajor < ApiMajor){
			return -3;
		}
		if (ModMinor > ApiMinor){
			return 2;
		} 
		if (ModMinor < ApiMinor){
			return -2;
		} 
		if (ModPatch > ApiPatch){
			return 1;
		}
		if (ModPatch < ApiPatch){
			return -1;
		}
		return 0;
	}
	
}

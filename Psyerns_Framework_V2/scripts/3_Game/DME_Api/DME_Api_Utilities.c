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
static string GetLogPlayerPosArray(array<ref DME_Api_LogPlayerPos> thePlayerlist){
	return JsonFileLoader<array<ref DME_Api_LogPlayerPos>>.JsonMakeData(thePlayerlist);
}


class DME_Api_Util extends Managed {
	
	//Client side function to get the steam id
	static string GetSteamId(){
		if (!g_Game) return "";
		DayZPlayer player;
		if (g_Game && g_Game.GetUserManager() && g_Game.GetUserManager().GetTitleInitiator()){
			return g_Game.GetUserManager().GetTitleInitiator().GetUid();
		} else if (g_Game && !g_Game.IsDedicatedServer() && Class.CastTo(player, g_Game.GetPlayer()) && player.GetIdentity() && player.GetIdentity().GetPlainId() != "" ){
			return player.GetIdentity().GetPlainId();
		} 
		return "";
	}
	
	//Return an array of file names for all the files in the specified directory
	static TStringArray FindFilesInDirectory(string directory)  { 
		TStringArray fileList = new TStringArray;
		
		string		fileName;
		int		fileAttr;
		int		flags;
		//Add \ to directory path and add search parameter (*)
		string pathpattern = directory + "\\*";
		
		//Search for files in file directory
		FindFileHandle fileHandler = FindFile(pathpattern, fileName, fileAttr, flags);
		
		bool found = true;
	    while ( found ) {//while there are files loop through looking for more
		    if ( fileAttr ) {
		        	//If file exsit add to array
		        	fileList.Insert(fileName);
		    }
			found = FindNextFile(fileHandler, fileName, fileAttr);
	    }
		return fileList; 
	};
	
	static string GetRandomId(int number){
		DME_Api().CheckAndRenewQRandom();
		TStringArray Chars = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","0","1","2","3","4","5","6","7","8","9"};
		string id = "";
		for (int i = 0; i <= number; i++){
			int idx = Math.QRandomInt(0,(Chars.Count() - 1));
			id = id + Chars.Get(idx);
		}
		return id;
	}
	
	//Simple function for finding a player based on their GUID
	static DayZPlayer FindPlayer(string GUID){
		if (!g_Game) return NULL;
		if (g_Game.IsDedicatedServer()){
			array<Man> players = new array<Man>;
			g_Game.GetPlayers( players );
			for (int i = 0; i < players.Count(); i++){
				DayZPlayer player = DayZPlayer.Cast(players.Get(i));
				if (player.GetIdentity() && player.GetIdentity().GetId() == GUID ){
					return player;
				}
			}
		}
		return NULL;
	}
	
	//Simple function for finding a player based on their identity
	static DayZPlayer FindPlayerByIdentity(PlayerIdentity identity) {
		if (!g_Game) return NULL;
		if (!identity)
			return NULL;

		int highBits;
		int lowBits;
		g_Game.GetPlayerNetworkIDByIdentityID(identity.GetPlayerId(), lowBits, highBits);
		return DayZPlayer.Cast(g_Game.GetObjectByNetworkId(lowBits, highBits));
	}
	
	static void SendNotificationEx(string Header, string Text, PlayerIdentity player, string Icon = "Psyerns_Framework/images/info.edds") {
		if (!g_Game) return;
		if (g_Game.IsDedicatedServer()){
			NotificationSystem.SendNotificationToPlayerIdentityExtended(player, 5, Header, Text, Icon );
		} else {
			NotificationSystem.AddNotificationExtended(5, Header, Text, Icon);
		}
	}
	
	static void SendNotification(string Header, string Text, PlayerIdentity player, string Icon = "Psyerns_Framework/images/info.edds") {
		if (!player) return;
		SendNotificationEx(Header,Text,player,Icon);
	}
	
	
	static string ConvertIntToNiceString(int DollarAmount){
		string prefix = "";
		string NiceString = "";
		if (DollarAmount < 0){
			prefix = "-";
		}
		string OrginalString = Math.AbsInt(DollarAmount).ToString();
		if (OrginalString.Length() <= 3){
			return prefix + OrginalString;
		} 
		int StrLen = OrginalString.Length() - 3;
		string StrSelection = OrginalString.Substring(StrLen,3);
		NiceString = StrSelection;
		while (StrLen > 3){
			StrLen = StrLen - 3;
			StrSelection = OrginalString.Substring(StrLen,3);
			NiceString = StrSelection + "," + NiceString;
		}
		StrSelection = OrginalString.Substring(0,StrLen);
		NiceString = StrSelection + "," + NiceString;
		return prefix + NiceString;
	}
	
	static string RestErrorToString(int ErrorCode){
		switch ( ErrorCode )
		{
			case ERestResultState.EREST_EMPTY:
				return "EREST_EMPTY";
			case ERestResultState.EREST_PENDING:
				return "EREST_PENDING";
			case ERestResultState.EREST_FEEDING:
				return "EREST_FEEDING";
			case ERestResultState.EREST_SUCCESS:
				return "EREST_SUCCESS";
			case ERestResultState.EREST_ERROR:
				return "EREST_ERROR";
			case ERestResultState.EREST_ERROR_CLIENTERROR:
				return "EREST_ERROR_CLIENTERROR";
			case ERestResultState.EREST_ERROR_SERVERERROR:
				return "EREST_ERROR_SERVERERROR";
			case ERestResultState.EREST_ERROR_APPERROR:
				return "EREST_ERROR_APPERROR";
			case ERestResultState.EREST_ERROR_TIMEOUT:
				return "EREST_ERROR_TIMEOUT";
			case ERestResultState.EREST_ERROR_NOTIMPLEMENTED:
				return "EREST_ERROR_NOTIMPLEMENTED";
			case ERestResultState.EREST_ERROR_UNKNOWN:
				return "EREST_ERROR_UNKNOWN";
		}
		return "UNDEFINED_ERROR";
	}
	
	static string GetDateStamp() {
		int yr;
		int mth;
		int day;
		GetYearMonthDay(yr, mth, day);
		string sday = day.ToString();
		if (sday.Length() == 1){
			sday = "0" + sday;
		}
		
		string smth = mth.ToString();
		if (smth.Length() == 1){
			smth = "0" + mth.ToString();
		}
		
		return yr.ToString() + "-" + smth + "-" + sday;
	}
	static string GetTimeStamp() {
		int hr;
		int min;
		int sec;
		GetHourMinuteSecond(hr, min, sec);
		
		string ssec = sec.ToString();
		if (ssec.Length() == 1){
			ssec = "0" + ssec;
		}
		string smin = min.ToString();
		if (smin.Length() == 1){
			smin = "0" + smin;
		}
		string shr = hr.ToString();
		if (shr.Length() == 1) {
			shr = "0" + shr;
		}
		return  shr + ":" + smin + ":" + ssec;
	}
	
	protected static int UnixStartYear = 1970;
	protected static int DaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	protected static bool IsLeapYear(int year){
		if (year % 4 == 0) {
	        if (year % 100 == 0) {
	            if (year % 400 == 0) {
	                return true;
	            }
	    		return false;
			}
	        return true;
	    }
	    return false;
	}
	
	//Get Days since JAN 01 1970
	static int GetDateInt() {
		int yr;
		int mth;
		int day;
		GetYearMonthDay(yr, mth, day);
		int count = day;
		for (int i = 0; i < (mth - 1); i++){
			count = count + DaysInMonth[i];
			if (IsLeapYear(yr) && i == 1){
				count++;
			}
		}
		count = count + Math.Floor((yr - UnixStartYear) * 365.25);
		return count;
	}
	
	//Get Days since JAN 01 1970
	static int GetUTCDateInt() {
		int yr;
		int mth;
		int day;
		GetYearMonthDayUTC(yr, mth, day);
		int count = day;
		for (int i = 0; i < (mth - 1); i++){
			count = count + DaysInMonth[i];
			if (IsLeapYear(yr) && i == 1){
				count++;
			}
		}
		count = count + Math.Floor((yr - UnixStartYear) * 365.25);
		return count;
	}
	
	//Gets the current unix time stamp for the current server timezone
	//Due to int.MAX will break on Tue Jan 19 2038 03:14:07
	static int GetUnixInt() {
		int hr;
		int min;
		int sec;
		GetHourMinuteSecond(hr, min, sec);
		return (GetDateInt() * 86400) + (hr * 3600) + (min * 60) + sec;
	}
	
	// Gets the current unix time stamp at UTC
	//Due to int.MAX will break on Tue Jan 19 2038 03:14:07
	static int GetUTCUnixInt() {
		int hr;
		int min;
		int sec;
		GetHourMinuteSecondUTC(hr, min, sec);
		return (GetUTCDateInt() * 86400) + (hr * 3600) + (min * 60) + sec;
	}
	
	
	
	static bool GetConfigInt(string type, string varible, out int value){
		if (!g_Game) return false;
		
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetInt(  CFG_MAGAZINESPATH  + " " + type + " " + varible);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH  + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetInt(  CFG_WEAPONSPATH  + " " + type + " " + varible);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetInt( CFG_VEHICLESPATH + " " + type + " " + varible );
			return true;
		}
		return false;
	}
	static bool GetConfigFloat(string type, string varible, out float value){
		if (!g_Game) return false;
		
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetFloat(  CFG_MAGAZINESPATH  + " " + type + " " + varible);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetFloat( CFG_WEAPONSPATH + " " + type + " " + varible );
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			value = g_Game.ConfigGetFloat( CFG_VEHICLESPATH + " " + type + " " + varible );
			return true;
		}
		return false;
	}
	static bool GetConfigString(string type, string varible, out string value){
		if (!g_Game) return false;
		
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			return g_Game.ConfigGetText(  CFG_MAGAZINESPATH  + " " + type + " " + varible,value);
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH  + " " + type + " " + varible ) ){
			return g_Game.ConfigGetText(  CFG_WEAPONSPATH  + " " + type + " " + varible,value);
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			return g_Game.ConfigGetText( CFG_VEHICLESPATH + " " + type + " " + varible,value);
		}
		return false;
	}
	static bool GetConfigTStringArray(string type, string varible, out TStringArray value){
		if (!g_Game) return false;
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetTextArray(  CFG_MAGAZINESPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetTextArray(  CFG_WEAPONSPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			g_Game.ConfigGetTextArray( CFG_VEHICLESPATH + " " + type + " " + varible, value);
			return true;
		}
		return false;
	}
	static bool GetConfigTFloatArray(string type, string varible, out TFloatArray value){
		if (!g_Game) return false;
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetFloatArray(  CFG_MAGAZINESPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetFloatArray(  CFG_WEAPONSPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			g_Game.ConfigGetFloatArray( CFG_VEHICLESPATH + " " + type + " " + varible, value);
			return true;
		}
		return false;
	}
	static bool GetConfigTIntArray(string type, string varible, out TIntArray value){
		if (!g_Game) return false;
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetIntArray(  CFG_MAGAZINESPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_WEAPONSPATH  + " " + type + " " + varible ) ){
			g_Game.ConfigGetIntArray(  CFG_WEAPONSPATH  + " " + type + " " + varible, value);
			return true;
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + type + " " + varible ) ){
			g_Game.ConfigGetIntArray( CFG_VEHICLESPATH + " " + type + " " + varible, value);
			return true;
		}
		return false;
	}
	
	
}
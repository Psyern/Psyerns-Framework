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
class DME_Api_LogBase{
	string Log = "";
	string GUID = "";
	vector Position;
	
	
	void DME_Api_LogBase(string log, string playerGUID = "", vector pos = vector.Zero){
		Log = log;
		GUID = playerGUID;
		if (pos != vector.Zero){
			Position = pos;
		}
	}
	
	string ToJson(){
		return JsonFileLoader<DME_Api_LogBase>.JsonMakeData(this);
	}
	
}

class DME_Api_LogMisc{
	string Log = "";
	string Action = "";
	string Item = "";
	string Target = "";
	string GUID = "";
	
	vector Position;
	
	
	void DME_Api_LogMisc(string log, string playerGUID = "", vector pos = vector.Zero, string action = "", string item = "", string target = ""){
		Log = log;
		GUID = playerGUID;
		if (pos != vector.Zero){
			Position = pos;
		}
		Item = item;
		Action = action;
		Target = target;
	}
	
	string ToJson(){
		return JsonFileLoader<DME_Api_LogMisc>.JsonMakeData(this);
	}
	
}

class DME_Api_LogPlayerPos {
	string Log = "PlayerPos";
	
	string GUID = "";
	
	vector Position;
	float Speed;
	
	bool InTransport;
	
	void DME_Api_LogPlayerPos(string playerGUID, vector pos = vector.Zero, float speed = 0, bool inTransport = false) {
		GUID = playerGUID;
		if (pos != vector.Zero){
			Position = pos;
		}
		Speed = speed;
		InTransport = inTransport;
	}
	
	string ToJson(){
		return JsonFileLoader<DME_Api_LogPlayerPos>.JsonMakeData(this);
	}
	
}


class DME_Api_LogKilled{
	
	string Log = "PlayerKilled";
	
	string GUID = "";
	vector Position;
	float Distance;
	
	string KilledBy = "";
	string KilledByGUID = "";
	vector KilledByPosition;
	
	float StatWater;
	float StatEnergy;
	int BleedingSources;
	
	void DME_Api_LogKilled(string playerGUID = "", vector pos = vector.Zero, string killedBy = "", vector killedByPos = vector.Zero){
		GUID = playerGUID;
		if (pos != vector.Zero){
			Position = pos;
		}
		KilledBy = killedBy;
		if (killedByPos != vector.Zero){
			KilledByPosition = killedByPos;
		}
		if (killedByPos != vector.Zero && pos != vector.Zero){
			Distance = vector.Distance(pos, killedByPos);
		}
	}

	void AddStats(float statWater, float statEnergy, int bleedingSources = -1){
		
		StatWater = statWater;
		StatEnergy = statEnergy;
		if (bleedingSources != -1){
			BleedingSources = bleedingSources;
		}
	}
	
	void ByPlayer(string killedByGUID){
		KilledByGUID = killedByGUID;
	}
	
	string ToJson(){
		return JsonFileLoader<DME_Api_LogKilled>.JsonMakeData(this);
	}
	
}
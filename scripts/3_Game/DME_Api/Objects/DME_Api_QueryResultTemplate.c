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
class DME_Api_QueryResult<Class T> : DME_Api_StatusObject {
	
	ref array<ref T> Results;
	int Count;
	
	
	static DME_Api_QueryResult<T> CreateFrom(string  stringData){
		DME_Api_QueryResult<T> returnval;
		if (DME_Api_JSONHandler<DME_Api_QueryResult<T>>.FromString( stringData, returnval)){
			return returnval;
		} 
		Error("[DME_Api] Failed to create Query Results");
		return NULL;
	}
	
	bool FromJson(string stringData) {
		return DME_Api_JSONHandler<DME_Api_QueryResult<T>>.FromString( stringData, this);
	}

	array<ref T> GetResults(){
		return Results;
	}
}

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
class DME_Api_QueryBase extends Managed{
	
	/*
		
	
	*/
	
	string Query = "{}"; //A mongo DB Query https://docs.mongodb.com/manual/reference/operator/meta/query/
	
	string OrderBy = "{}"; //The OrderBy for the Query https://docs.mongodb.com/manual/reference/operator/meta/orderby/
	
	string ReturnObject = ""; //This if you want to return a specific value within the document
	
	int MaxResults = -1;  //Max Number of Results to return, Note: dayz seems to crash at over 30mb in some of my tests
	
	bool FixQuery = false;  //This will correct queries to match the api's save structure in mongodb 
	
	
	string ToJson(){
		return JsonFileLoader<DME_Api_QueryBase>.JsonMakeData(this);
	}
	
}

class DME_Api_QueryObject extends DME_Api_QueryBase {	
	
	
	void DME_Api_QueryObject(string query = "{}", string orderBy = "{}", int maxResults = -1, string returnObject = ""){
		
		Query = query;
		OrderBy = orderBy;
		ReturnObject = returnObject;
		MaxResults = maxResults;
		FixQuery = false;
	}
	
	
}

class DME_Api_DBQuery extends DME_Api_QueryBase{
		
	void DME_Api_DBQuery(string query = "{}", string orderBy = "{}", bool fixQuery = true, int maxResults = -1, string returnObject = ""){
		Query = query;
		OrderBy = orderBy;
		FixQuery = fixQuery;
		ReturnObject = returnObject;
		MaxResults = maxResults;
	}
	
}

class DME_Api_DBQueryUpdate extends DME_Api_Object_Base {
	string Element;
	string Operation = DME_Api_UpdateOpts.SET; // set | push | pull | unset | mul | rename | pullAll
	string Value;
	ref DME_Api_QueryBase Query;
	
	
	void DME_Api_DBQueryUpdate(DME_Api_QueryBase query, string element, string value, string operation = DME_Api_UpdateOpts.SET){
		Element = element;
		Value = value;
		Operation = operation;
		Query = query;
	}
	
	override string ToJson(){
		string jsonString = DME_Api_JSONHandler<DME_Api_DBQueryUpdate>.ToString(this);
		return jsonString;
	}
	
}

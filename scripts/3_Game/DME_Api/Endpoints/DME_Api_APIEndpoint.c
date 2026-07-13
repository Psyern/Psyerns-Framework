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
class DME_Api_APIEndpoint extends DME_Api_BaseEndpoint {
	
	
	//Uses the QnA Endpoint to send requests returns DME_Api_QnAAnswer
	int QnA(string Question,  string Key, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "QnA/" + Key;
				
		DME_Api_QuestionRequest questionObj = new DME_Api_QuestionRequest(Question);
		string jsonString = questionObj.ToJson();
		
		if (Question && jsonString && ReturnString){
			Post(endpoint, jsonString, new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		} else if (Question && jsonString){
			Post(endpoint, jsonString, new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_QnAAnswer>(cbInstance, cbFunction, oid), cid));
		} else {
			Print("[DME_Api] [Api] Error QnA K:" +  Key + " Q: " + Question );
			cid = -1;
		}
		return cid;	
	}
		
	//Helper function for returning the question to chat
	int ChatQnA(string Question, bool Slient){
		if (Slient){
			return QnA(Question, "", GetDayZGame(), "CBQnAChatMessageSilent");	
		}
		return QnA(Question, "", GetDayZGame(), "CBQnAChatMessage");	
	}
	
	
	//Sends request to get text translated returns a `DME_Api_TranslationResponse` object
	int Translate(string Text, string To, string From, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Translate";
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_TranslationResponse>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_TranslationRequest translationReq = new DME_Api_TranslationRequest(Text, {To}, From);
		
		if ( translationReq && Text && To && DBCBX){
			Post(endpoint,translationReq.ToJson(),DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Translate " +  Text);
			cid = -1;
		}
		return cid;
		
	}
	
	//Sends request to get text translated returns a `DME_Api_TranslationResponse` object
	int Translate(string Text, TStringArray To, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Translate";
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_TranslationResponse>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_TranslationRequest translationReq = new DME_Api_TranslationRequest(Text, To);
		
		if ( translationReq && Text && To && DBCBX){
			Post(endpoint,translationReq.ToJson(),DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Translate " +  Text);
			cid = -1;
		}
		return cid;
		
	}
		
	
	//Runs a wit query to the key specificed must be configured server side
	int Wit(string Text, string Key, Class cbInstance, string cbFunction, string oid = ""){
		int cid = DME_Api().CallId();
		string endpoint = "Wit/" + Key;
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_QuestionRequest questionreq = new DME_Api_QuestionRequest(Text);
		string jsonString = questionreq.ToJson();
		
		if ( jsonString && Text && Text != "" && Key && Key != "" && DBCBX){
			Post(endpoint,jsonString,DBCBX);
		} else {
			Print("[DME_Api] [Api] Error Wit K:" +  Key + " T:" + Text);
			cid = -1;
		}
		return cid;
	}
	
	//Sends a query to Microsoft's LUIS
	int LUIS(string Text, string Key, Class cbInstance, string cbFunction, string oid = ""){
		int cid = DME_Api().CallId();
		string endpoint = "LUIS/" + Key;
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		DME_Api_QuestionRequest questionreq = new DME_Api_QuestionRequest(Text);
		string jsonString = questionreq.ToJson();
		
		if ( jsonString && Text && Text != "" && Key && Key != "" && DBCBX){
			Post(endpoint,jsonString,DBCBX);
		} else {
			Print("[DME_Api] [Api] Error LUIS K:" +  Key + " T:" + Text);
			cid = -1;
		}
		return cid;
	}
	
	
	
	//Replacing ServerQuery Runs a Steam Query for a server returning a `DME_Api_ServerStatus` object
	int SteamQuery(string ip, string queryPort, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "ServerQuery/Status/" + ip + "/" + queryPort;
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_ServerStatus>(cbInstance, cbFunction, oid), cid);
		}
		
		if (  ip && ip != "" && queryPort && queryPort != "" && DBCBX){
			Post(endpoint,"{}",DBCBX);
		} else {
			Print("[DME_Api] [Api] Error ServerQuery IP:" +  ip + " Port:" + queryPort);
			cid = -1;
		}
		return cid;
	}
	
	//To Be removed
	//Runs a Steam Query for a server returning a `DME_Api_ServerStatus` object
	int ServerQuery(string ip, string queryPort, Class cbInstance, string cbFunction, string oid = ""){
		int cid = DME_Api().CallId();
		string endpoint = "ServerQuery/Status/" + ip + "/" + queryPort;
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else {
			DBCBX = new DME_Api_SilentCallBack();
		}
		
		
		if (  ip && ip != "" && queryPort && queryPort != "" && DBCBX){
			Post(endpoint,"{}",DBCBX);
		} else {
			Print("[DME_Api] [Api] Error ServerQuery IP:" +  ip + " Port:" + queryPort);
			cid = -1;
		}
		return cid;
	}
	
	//Runs a Steam Query for a server returning a `DME_Api_ServerStatus` object
	int ServerQueryObj(string ip, string queryPort, Class cbInstance, string cbFunction, string oid = ""){
		int cid = DME_Api().CallId();
		string endpoint = "ServerQuery/Status/" + ip + "/" + queryPort;
		
		if (  ip && ip != "" && queryPort && queryPort != "" ){
			Post(endpoint,"{}",new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_ServerStatus>(cbInstance, cbFunction, oid), cid));
		} else {
			Print("[DME_Api] [Api] Error ServerQuery IP:" +  ip + " Port:" + queryPort);
			cid = -1;
		}
		return cid;
	}
	
	//Sends text for Toxicity Check returns a `DME_Api_ToxicityResponse` object
	int Toxicity(string text, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Toxicity";
		
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_ToxicityResponse>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_QuestionRequest questionreq = new DME_Api_QuestionRequest(text);
		
		if ( text && text != "" && questionreq && DBCBX){
			Post(endpoint, questionreq.ToJson(), DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Toxicity ", "Text:" +  text + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Get a array of random numbers from 0 - 65535 returns `DME_Api_RandomNumberResponse`
	int RandomNumbers(int count, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Random";
		if (count == -1){
			count = 2048;
		}
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != ""){
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_RandomNumberResponse>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_RandomNumberRequest randomreq = new DME_Api_RandomNumberRequest(count);
		
		if (  count > 0 && count <= 2048 && randomreq && DBCBX){
			Post(endpoint, randomreq.ToJson(), DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Random", "Count: " +  count + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Gets an array of random number from  -2147483647 to 2147483647 returns `DME_Api_RandomNumberResponse`
	int RandomNumbersFull(int count, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Random/Full";
		if (count == -1){
			count = 4096;
		}
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_RandomNumberResponse>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_RandomNumberRequest randomreq = new DME_Api_RandomNumberRequest(count);
		
		if (  count > 0 && count <= 4096 && randomreq && DBCBX){
			Post(endpoint, randomreq.ToJson(), DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Random", "Count: " +  count + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Gets the value of the set value amount market prices for Crypto currencys `DME_Api_CryptoConvertResult`
	int CryptoPrice(string from, string to, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Crypto/Price/" + from + "/" + to;
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_CryptoConvertResult>(cbInstance, cbFunction, oid), cid);
		}
		
		if ( from && to && DBCBX){
			Post(endpoint, "{}", DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Crypto Price", "From: " +  from + " To: " +  to + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Gets the value of the set value amount market prices for Crypto currencys `DME_Api_CryptoConvertResult`
	int CryptoConvert(string from, string to, float value, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Crypto/Convert/" + from + "/" + to;
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_CryptoConvertResult>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_CryptoConvertRequest req = new DME_Api_CryptoConvertRequest(value);
		
		if ( from && to && value > 0 && DBCBX){
			Post(endpoint, req.ToJson(), DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Crypto Convert", "From: " +  from + " To: " +  to + " Value: " + value + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Gets a map of live market prices for Crypto currencys `DME_Api_CryptoResults`
	int Crypto(TStringArray from, string to, Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		string endpoint = "Crypto/" + to;
		RestCallback DBCBX;
		if (cbInstance && cbFunction != "" && ReturnString){
			DBCBX = new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid);
		} else if (cbInstance && cbFunction != "") {
			DBCBX = new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_CryptoResults>(cbInstance, cbFunction, oid), cid);
		}
		
		DME_Api_CryptoRequest req = new DME_Api_CryptoRequest(from);
		
		if ( from && from.Count() > 0 && to && DBCBX){
			Post(endpoint, req.ToJson(), DBCBX);
		} else {
			Error2("[DME_Api] [Api] Error Crypto", "From: " +  from.Count() + " To: " +  to + " CID:" + cid);
			cid = -1;
		}
		return cid;
	}
	
	//Request a status check from the api so you can get version number and such returns a `DME_Api_Status` object
	int Status(Class cbInstance, string cbFunction, string oid = "", bool ReturnString = false){
		int cid = DME_Api().CallId();
		if (ReturnString){	
			Post("Status", "{}", new DME_Api_DBCallBack(cbInstance, cbFunction, cid, oid));
		} else {
			Post("Status", "{}",  new DME_Api_DBNestedCallBack(new DME_Api_Callback<DME_Api_Status>(cbInstance, cbFunction, oid), cid));
		}
		return cid;
	}
}
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
class DME_Api_TranslationResponse extends DME_Api_StatusObject {

	ref array<ref DME_Api_Translation> Translations;
	string Detected = "";
}

class DME_Api_Translation extends Managed {
	string text;
	string to;
}

class DME_Api_TranslationRequest extends DME_Api_Object_Base{

	string Text = "";
	ref TStringArray To = {"en"};
	string From = "";


	
	void DME_Api_TranslationRequest(string text, TStringArray to, string from = "auto"){
		Text = text;
		if (to){
			To = to;
		}
		From = from;
	}
	
	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_TranslationRequest>.JsonMakeData(this);
		return jsonString;
	}
}
/*  *** Default Supported Languages ***
    +-----------------------+----------+
    | Language              | Code     |
    +-----------------------+----------+
    | Arabic                | ar       |
    +-----------------------+----------+
    | Chinese   Simplified  | zh       |
    +-----------------------+----------+
    | English               | en       |
    +-----------------------+----------+
    | French                | fr       |
    +-----------------------+----------+
    | German                | de       |
    +-----------------------+----------+
    | Hindi                 | hi       |
    +-----------------------+----------+
    | Indonesian            | id       |
    +-----------------------+----------+
    | Irish                 | ga       |
    +-----------------------+----------+
    | Italian               | it       |
    +-----------------------+----------+
    | Japanese              | ja       |
    +-----------------------+----------+
    | Korean                | ko       |
    +-----------------------+----------+
    | Polish                | pl       |
    +-----------------------+----------+
    | Portuguese            | pt       |
    +-----------------------+----------+
    | Russian               | ru       |
    +-----------------------+----------+
    | Spanish               | es       |
    +-----------------------+----------+
    | Turkish               | tr       |
    +-----------------------+----------+
    | Vietnamese            | vi       | 
    +-----------------------+----------+
*/

/*  *** Supported Languages Microsoft *** 
    Both Ways (Auto Detected)
    +-----------------------+----------+
    | Language              | Code     |
    +-----------------------+----------+
    | Afrikaans             | af       |
    +-----------------------+----------+
    | Albanian              | sq       |
    +-----------------------+----------+
    | Arabic                | ar       |
    +-----------------------+----------+
    | Bulgarian             | bg       |
    +-----------------------+----------+
    | Catalan               | ca       |
    +-----------------------+----------+
    | Chinese   Simplified  | zh-Hans  |
    +-----------------------+----------+
    | Chinese   Traditional | zh-Hant  |
    +-----------------------+----------+
    | Croatian              | hr       |
    +-----------------------+----------+
    | Czech                 | cs       |
    +-----------------------+----------+
    | Danish                | da       |
    +-----------------------+----------+
    | Dutch                 | nl       |
    +-----------------------+----------+
    | English               | en       |
    +-----------------------+----------+
    | Estonian              | et       |
    +-----------------------+----------+
    | Finnish               | fi       |
    +-----------------------+----------+
    | French                | fr       |
    +-----------------------+----------+
    | German                | de       |
    +-----------------------+----------+
    | Greek                 | el       |
    +-----------------------+----------+
    | Gujarati              | gu       |
    +-----------------------+----------+
    | Haitian   Creole      | ht       |
    +-----------------------+----------+
    | Hebrew                | he       |
    +-----------------------+----------+
    | Hindi                 | hi       |
    +-----------------------+----------+
    | Hungarian             | hu       |
    +-----------------------+----------+
    | Icelandic             | is       |
    +-----------------------+----------+
    | Indonesian            | id       |
    +-----------------------+----------+
    | Inuktitut             | iu       |
    +-----------------------+----------+
    | Irish                 | ga       |
    +-----------------------+----------+
    | Italian               | it       |
    +-----------------------+----------+
    | Japanese              | ja       |
    +-----------------------+----------+
    | Klingon               | tlh-Latn |
    +-----------------------+----------+
    | Korean                | ko       |
    +-----------------------+----------+
    | Kurdish   (Central)   | ku-Arab  |
    +-----------------------+----------+
    | Latvian               | lv       |
    +-----------------------+----------+
    | Lithuanian            | lt       |
    +-----------------------+----------+
    | Malay                 | ms       |
    +-----------------------+----------+
    | Maltese               | mt       |
    +-----------------------+----------+
    | Norwegian             | nb       |
    +-----------------------+----------+
    | Pashto                | ps       |
    +-----------------------+----------+
    | Persian               | fa       |
    +-----------------------+----------+
    | Polish                | pl       |
    +-----------------------+----------+
    | Portuguese            | pt       |
    +-----------------------+----------+
    | Romanian              | ro       |
    +-----------------------+----------+
    | Russian               | ru       |
    +-----------------------+----------+
    | Serbian   (Cyrillic)  | sr-Cyrl  |
    +-----------------------+----------+
    | Serbian   (Latin)     | sr-Latn  |
    +-----------------------+----------+
    | Slovak                | sk       |
    +-----------------------+----------+
    | Slovenian             | sl       |
    +-----------------------+----------+
    | Spanish               | es       |
    +-----------------------+----------+
    | Swahili               | sw       |
    +-----------------------+----------+
    | Swedish               | sv       |
    +-----------------------+----------+
    | Tahitian              | ty       |
    +-----------------------+----------+
    | Thai                  | th       |
    +-----------------------+----------+
    | Turkish               | tr       |
    +-----------------------+----------+
    | Ukrainian             | uk       |
    +-----------------------+----------+
    | Urdu                  | ur       |
    +-----------------------+----------+
    | Vietnamese            | vi       |
    +-----------------------+----------+
    | Welsh                 | cy       |
    +-----------------------+----------+
    | Yucatec   Maya        | yua      |
    +-----------------------+----------+

Full List: 
  https://docs.microsoft.com/en-us/azure/cognitive-services/translator/language-support
*/
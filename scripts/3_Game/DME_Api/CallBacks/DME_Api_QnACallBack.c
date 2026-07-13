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
class DME_Api_QnACallBack : RestCallback
{
	
	protected bool m_AlwaysAnswer = false;
	
	void SetAlwaysAnswer(bool alwaysAnswer = true){
		m_AlwaysAnswer = alwaysAnswer;
	}
	
	override void OnError(int errorCode) {
		if (m_AlwaysAnswer){
			DME_Api_GetQnAMaker().SendRespone("Sorry Something went wrong try asking again later");
		}
	};
	
	override void OnTimeout() {
		if (m_AlwaysAnswer){
			DME_Api_GetQnAMaker().SendRespone("Sorry Something went wrong try asking again later");
		}
	};
	override void OnSuccess(string data, int dataSize) {
		DME_Api_QnAAnswer AnswerObj;
		JsonFileLoader<DME_Api_QnAAnswer>.JsonLoadData(data, AnswerObj);
		if (AnswerObj.get() != "null" && AnswerObj.get() != "error" &&  AnswerObj.get() != "ERROR" &&  AnswerObj.get() != ""){
			DME_Api_GetQnAMaker().ProcessAnswer(AnswerObj.get());
		} else if (m_AlwaysAnswer) {
			DME_Api_GetQnAMaker().SendRespone("Sorry couldn't find the an answer to your question? Try rephrasing it or asking a real person");
		}
	};
};
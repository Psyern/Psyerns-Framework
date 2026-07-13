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
 *
 * bug #5 fix: DME_Api_GetConfig() is NULL on the client until the config RPC
 * arrives; guard it before dereferencing QnAEnabled.
 */
modded class ChatInputMenu
{
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		DME_Api_Config cfg = DME_Api_GetConfig();
		if (cfg && cfg.QnAEnabled){
			if (!finished) return super.OnChange(w, x, y, finished);
			string question = m_edit_box.GetText();
			if (question.Length() > 3){
				int lastIndex = question.Length() - 1;
				bool silentQuestion = (question.Substring(0,1) == "?" );
				bool generalQuestion = (question.Substring(lastIndex,1) == "?" );
				if (question != "" && (silentQuestion || generalQuestion))	{
					string rdyQuestion = question;
					Print("Question: '" + question + "' Set to API");
					if (silentQuestion){
						rdyQuestion = question.Substring(1,lastIndex);
					}
					DME_Api().api().ChatQnA(rdyQuestion, silentQuestion);
					if (silentQuestion){
						m_close_timer.Run(0.1, this, "Close");
						return true;
					}
				}
			}
		}
		return super.OnChange(w, x, y, finished);
	}
};

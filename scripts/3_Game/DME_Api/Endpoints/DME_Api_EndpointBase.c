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
class DME_Api_BaseEndpoint extends Managed {
	
	protected RestContext m_Context;
	
	protected string EndpointBaseUrl(){
		return DME_Api_GetConfig().GetBaseURL();
	}

	protected string AuthToken(){
		return DME_Api().GetAuthToken();
	}
	
	
	protected RestContext Api()
	{
		RestApi clCore = GetRestApi();
		if (!clCore)
		{
			clCore = CreateRestApi();
			clCore.SetOption(ERestOption.ERESTOPTION_READOPERATION, 30);
		}
		m_Context = clCore.GetRestContext(EndpointBaseUrl());
		m_Context.SetHeader(AuthToken());
		return m_Context;
	}
	
	protected void Post(string route, string jsonString, RestCallback UCBX)
	{
		//Print(EndpointBaseUrl() + route);
		Api().POST(UCBX, route, jsonString);
	}
	
	void UpdateAuthToken(){
		if (m_Context){
			m_Context.SetHeader(AuthToken());
		}
	}
	
}

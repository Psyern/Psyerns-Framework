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
modded class MissionBaseWorld
{
	protected bool m_DME_Api_Initialized = false;
	
	bool DME_Api_IsInitialized(){
		return m_DME_Api_Initialized;
	}
	
	void DME_Api_ReadyTokenReceived(){
		//Print("[DME_Api] MissionBaseWorld - DME_Api_ReadyTokenReceived");
		if (!DME_Api_IsInitialized()){
			m_DME_Api_Initialized = true;
			this.DME_Api_Ready();
		}
	}
	
	void DME_Api_Ready(){
		//Print("[DME_Api] MissionBaseWorld - DME_Api_Ready");
	
	}

}
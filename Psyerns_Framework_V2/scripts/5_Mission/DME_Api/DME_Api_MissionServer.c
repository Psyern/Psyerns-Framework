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
 * Stacks alongside the PF_ modded MissionServer layers (Fall F).
 */
modded class MissionServer
{
	void MissionServer()
	{
		DME_Api();
		if (g_Game){
			ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queue){ queue.Call(this.DME_Api_Ready); }
		}
	}

	override void OnClientPrepareEvent(PlayerIdentity identity, out bool useDB, out vector pos, out float yaw, out int preloadTimeout)
	{
		super.OnClientPrepareEvent(identity, useDB, pos, yaw, preloadTimeout);
		if (identity){
			//Print("[DME_Api] On Prepare - GUID: " + identity.GetId() );
			DME_Api().PreparePlayerAuth(identity.GetId());
		}
	}

	override void DME_Api_Ready(){
		//You requests for after the AuthToken Is received for server side code
		super.DME_Api_Ready();
	}
}

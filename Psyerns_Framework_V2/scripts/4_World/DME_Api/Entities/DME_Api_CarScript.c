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
 * Cross-PBO merge (Fall D): the empty _UAPIBase hook declarations merged with
 * the _UniversalApi implementations into one modded class. The hooks are now
 * declared here directly, so no override/super is used.
 */
modded class CarScript {

	void OnDME_ApiSave(DME_Api_EntityStore data) {
		data.Write("m_FuelAmmount", m_FuelAmmount);
		data.Write("m_CoolantAmmount", m_CoolantAmmount);
		data.Write("m_OilAmmount", m_OilAmmount);
		data.Write("m_BrakeAmmount", m_BrakeAmmount);

		m_EngineHealth = GetHealth01("Engine", "");
		data.Write("m_EngineHealth", m_EngineHealth);
		m_FuelTankHealth = GetHealth01("FuelTank", "");
		data.Write("m_FuelTankHealth", m_FuelTankHealth);
	}

	void OnDME_ApiLoad(DME_Api_EntityStore data){
		data.Read("m_FuelAmmount", m_FuelAmmount);
		data.Read("m_CoolantAmmount", m_CoolantAmmount);
		data.Read("m_OilAmmount", m_OilAmmount);
		data.Read("m_BrakeAmmount", m_BrakeAmmount);

		if (data.Read("m_EngineHealth", m_EngineHealth)){
			SetHealth01("Engine", "", m_EngineHealth);
		}
		if (data.Read("m_FuelTankHealth", m_FuelTankHealth)){
			SetHealth01("FuelTank", "", m_FuelTankHealth);
		}
	}

}

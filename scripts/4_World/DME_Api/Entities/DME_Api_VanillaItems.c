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
modded class TransmitterBase extends ItemTransmitter{
	override void OnDME_ApiSave(DME_Api_EntityStore data){
		super.OnDME_ApiSave(data);
		
		data.Write("ChannelIndex", GetTunedFrequencyIndex());
	}
	
	override void OnDME_ApiLoad(DME_Api_EntityStore data){
		super.OnDME_ApiLoad(data);
		
		SetFrequencyByIndex(data.GetInt("ChannelIndex"));
	}
}

modded class Edible_Base extends ItemBase
{

	override void OnDME_ApiSave(DME_Api_EntityStore data){
		super.OnDME_ApiSave(data);
		
		data.Write("m_DecayTimer", m_DecayTimer );
		data.Write("m_LastDecayStage",  m_LastDecayStage );
		if (GetFoodStage()){
			data.Write("m_FoodStage", GetFoodStage().GetFoodStageType());
		}
	}
	
	override void OnDME_ApiLoad(DME_Api_EntityStore data){
		super.OnDME_ApiLoad(data);
		if (!data.Read("m_DecayTimer", m_DecayTimer )){
			m_DecayTimer = 0.0;
		}
		if (!data.Read("m_LastDecayStage", m_LastDecayStage )){
			m_LastDecayStage = FoodStageType.NONE;
		}
		int foodStageType;
		if (data.Read("m_FoodStage",foodStageType) && GetFoodStage()){
			GetFoodStage().ChangeFoodStage(foodStageType);
		}
	}
	
}
	
modded class BloodContainerBase extends ItemBase
{	
	override void OnDME_ApiSave(DME_Api_EntityStore data){
		super.OnDME_ApiSave(data);
		
		int IsBloodTypeVisible = GetBloodTypeVisible();
		data.Write("m_IsBloodTypeVisible",  IsBloodTypeVisible );
	}
	
	override void OnDME_ApiLoad(DME_Api_EntityStore data){
		super.OnDME_ApiLoad(data);
		
		int IsBloodTypeVisible = 0;
		if (data.Read("m_IsBloodTypeVisible", IsBloodTypeVisible ) && IsBloodTypeVisible == 1){
			SetBloodTypeVisible(true);
			SetSynchDirty();
		}
		
	}
}
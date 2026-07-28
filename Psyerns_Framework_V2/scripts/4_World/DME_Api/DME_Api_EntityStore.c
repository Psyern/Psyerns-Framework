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
 * Cross-PBO merge (Fall A): base data/API from _UAPIBase ItemStore.c textually
 * merged with the _UniversalApi ItemStore.c implementations (SaveEntity/Create/
 * CreateAtPos/LoadEntity). Destructor + delete removed (GC), autoptr->ref,
 * pre-existing logic bugs fixed (muzzle restore, null parent, null m_MagAmmo).
 */
class DME_Api_EntityStore extends DME_Api_Object_Base {

	string m_Type = "";
	int m_pid1;
	int m_pid2;
	int m_pid3;
	int m_pid4;
	float m_Health = -1;
	float m_Quantity;
	float m_Wet;
	float m_Tempature;
	float m_Energy;
	int m_LiquidType;
	int m_Slot;
	int m_Idx;
	int m_Row;
	int m_Col;
	bool m_Flip;
	bool m_IsInHands;
	bool m_IsOn;
	int m_QuickBarSlot;
	int m_Agents;
	int m_Cleanness;
	protected ref array<ref DME_Api_ZoneHealthData> m_HealthZones;

	protected ref array<ref DME_Api_EntityStore> m_Cargo;

	bool m_IsMagazine;
	ref array<ref DME_Api_AmmoData> m_MagAmmo;
	bool m_IsWeapon;
	bool m_IsVehicle;
	ref array<int> m_FireModes;
	ref DME_Api_AmmoData m_ChamberedRound;

	protected ref array<ref DME_Api_MetaData> m_MetaData;

	void DME_Api_EntityStore(EntityAI item = NULL){
		if (!item) return;
		SaveEntity(item, true);
	}

	void SaveEntity(notnull EntityAI item, bool recursive = true ){
		m_Type = item.GetType();
		item.GetPersistentID(m_pid1, m_pid2, m_pid3, m_pid4); //Just for testing but maybe someone will find this usefull
		m_Health = item.GetHealth("", "");
		array<EntityAI> items = new array<EntityAI>;
		int i = 0;
		InventoryLocation il = new InventoryLocation;
		if (item.GetInventory().GetCurrentInventoryLocation(il)){
			m_Slot = il.GetSlot();
			m_Idx = il.GetIdx();
			m_Row = il.GetRow();
			m_Col = il.GetCol();
			m_Flip = il.GetFlip();
		}
		if (recursive){
			item.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
			if (items && items.Count() > 0){
				for (i = 0; i < items.Count(); i++){
					EntityAI child_item = EntityAI.Cast(items.Get(i));
					if (!m_Cargo){m_Cargo = new array<ref DME_Api_EntityStore>;}
					if (child_item && ( item.GetInventory().HasEntityInCargo(child_item) || item.GetInventory().HasAttachment(child_item) ) ){
						DME_Api_EntityStore crg_itemstore = new DME_Api_EntityStore(child_item);
						m_Cargo.Insert(crg_itemstore);
					} else {
						break;
					}
				}
			}
		}
		PlayerBase HoldingPlayer;
		if (Class.CastTo(HoldingPlayer, item.GetHierarchyRootPlayer())){
			m_IsInHands = (HoldingPlayer.GetHumanInventory().GetEntityInHands() == item);
			m_QuickBarSlot = HoldingPlayer.FindQuickBarEntityIndex(item);
		}
		m_IsMagazine = item.IsMagazine() && !item.IsAmmoPile();
		m_IsWeapon = item.IsWeapon();

		ItemBase itemB;
		if (Class.CastTo(itemB, item)){
			if (itemB.HasQuantity()){
				m_Quantity = itemB.GetQuantity();
			}
			m_Wet = itemB.GetWet();
			m_Tempature = itemB.GetTemperature();
			m_Energy = itemB.GetEnergy();
			if (itemB.GetCompEM()){
				m_IsOn = itemB.GetCompEM().IsSwitchedOn();
			}
			m_LiquidType = itemB.GetLiquidType();
			m_Agents = itemB.GetAgents();
			m_Cleanness = itemB.m_Cleanness;
			itemB.OnDME_ApiSave(this);
		}
		Magazine_Base mag;
		float dmg;
		string cartType;
		if (m_IsMagazine && Class.CastTo(mag, item)){
			m_Quantity = mag.GetAmmoCount();
			for (i = 0; i < mag.GetAmmoCount(); i++){
				dmg = -1;
				cartType = "";
				if (mag.GetCartridgeAtIndex(i, dmg, cartType) && cartType != "" && dmg >= 0){
					if (!m_MagAmmo){ m_MagAmmo = new array<ref DME_Api_AmmoData>;}
					m_MagAmmo.Insert(new DME_Api_AmmoData(i, dmg, cartType));
				}
			}
		} else if (item.IsAmmoPile() && Class.CastTo(mag, item)){
			m_Quantity = mag.GetAmmoCount();
		}
		Weapon_Base weap;
		if (m_IsWeapon && Class.CastTo(weap, item)){
			int currentMuzzle = weap.GetCurrentMuzzle();
			m_Quantity = weap.GetTotalCartridgeCount(currentMuzzle);
			Write("m_IsJammed",  weap.IsJammed());
			Write("m_CurrentMuzzle", currentMuzzle);
			Write("m_Zeroing", weap.GetStepZeroing(weap.GetCurrentMuzzle()));
			Write("m_Zoom", weap.GetZoom());
			for (i = 0; i < weap.GetTotalCartridgeCount(currentMuzzle); i++){
				dmg = -1;
				cartType = "";
				if (weap.GetInternalMagazineCartridgeInfo(currentMuzzle, i, dmg, cartType) && cartType != "" && dmg >= 0){
					if (!m_MagAmmo){ m_MagAmmo = new array<ref DME_Api_AmmoData>;}
					m_MagAmmo.Insert(new DME_Api_AmmoData(i, dmg, cartType));
				}
			}
			if (!weap.IsChamberEmpty(currentMuzzle)){
				dmg = -1;
				cartType = "";
				if (weap.GetCartridgeInfo(currentMuzzle, dmg, cartType) && cartType != "" && dmg >= 0 ){
					m_ChamberedRound = new DME_Api_AmmoData(-1, dmg,cartType);
				}
			}
			if (!m_FireModes){m_FireModes = new array<int>;}
			for (i = 0; i < weap.GetMuzzleCount(); ++i){
				m_FireModes.Insert(weap.GetCurrentMode(i));
			}
		}

		// Damage System
		DamageZoneMap zones = new DamageZoneMap;
		DamageSystem.GetDamageZoneMap(item,zones);
		for( i = 0; i < zones.Count(); i++ ){
			string zone = zones.GetKey(i);
			SaveZoneHealth(zone, item.GetHealth(zone, ""));
		}

		CarScript vehicle;
		if (Class.CastTo(vehicle,item)){
			m_IsVehicle = true;
			vehicle.OnDME_ApiSave(this);
		}
	}

	EntityAI Create(EntityAI parent = NULL, bool RestoreOrginalLocation = true){
		if (!g_Game) return NULL;
		EntityAI item;
		if (parent == NULL){
			item = EntityAI.Cast(g_Game.CreateObject(m_Type, "0 0 0"));
		} else if (m_Slot == -1) {
			item = EntityAI.Cast(parent.GetInventory().CreateEntityInCargoEx(m_Type, m_Idx, m_Row, m_Col, m_Flip));
		} else if (m_IsInHands){
			PlayerBase player = PlayerBase.Cast(parent.GetHierarchyRootPlayer());
			if ( player ) {
			 	item = EntityAI.Cast(player.GetHumanInventory().CreateInHands(m_Type));
			}
		} else {
			item = EntityAI.Cast(parent.GetInventory().CreateAttachmentEx(m_Type, m_Slot));
		}
		if (!item && parent){
			item = EntityAI.Cast(g_Game.CreateObject(m_Type, parent.GetPosition()));
		}
		if (!item){
			Print("[DME_Api] [ERROR] Couldn't create item " + m_Type);
			return NULL;
		}
		LoadEntity(item);
		return item;
	}

	EntityAI CreateAtPos(vector Pos, vector Ori = "0 0 0"){
		if (!g_Game) return NULL;
		EntityAI item;
		item = EntityAI.Cast(g_Game.CreateObject(m_Type, Pos));
		if (!item){
			Print("[DME_Api] [ERROR] Couldn't create item " + m_Type);
			return NULL;
		}
		item.SetPosition(Pos);
		item.SetOrientation(Ori);
		LoadEntity(item);
		return item;
	}

	void LoadEntity(EntityAI item){
		if (!g_Game) return;
		int i;
		item.SetHealth("", "", m_Health);
		ItemBase itemB;

		Weapon_Base weap;
		if (m_IsWeapon && Class.CastTo(weap, item)){
			int currentMuzzle = GetInt("m_CurrentMuzzle");
			if (currentMuzzle < weap.GetMuzzleCount() && currentMuzzle >= 0){
				weap.SetCurrentMuzzle(currentMuzzle);
			}
		}
		if (m_Cargo && m_Cargo.Count() > 0){
			for(i = 0; i < m_Cargo.Count(); i++){
				if (m_Cargo.Get(i) && m_Cargo.Get(i).m_IsMagazine && m_IsWeapon && weap){ //Is a mag in a weapon
					Magazine_Base child_mag = Magazine_Base.Cast(m_Cargo.Get(i).Create(item));
					if (weap && child_mag){
						weap.AttachMagazine(weap.GetCurrentMuzzle(), child_mag);
					}
				} else {
					m_Cargo.Get(i).Create(item);
				}
			}
		}
		if (Class.CastTo(itemB, item)){
			if (itemB.HasQuantity() && !itemB.IsMagazine()){
				itemB.SetQuantity(m_Quantity);
			}
			itemB.SetWet(m_Wet);
			itemB.SetTemperature(m_Tempature);
			itemB.SetLiquidType(m_LiquidType);
			if (itemB.GetCompEM()){
				itemB.GetCompEM().SetEnergy(m_Energy);
				if (m_IsOn){
					itemB.GetCompEM().SwitchOn();
				}
			}
			itemB.RemoveAllAgents();//Removes any default agents then add the needed ones.
			itemB.TransferAgents(m_Agents);
			itemB.SetCleanness(m_Cleanness);
			itemB.OnDME_ApiLoad(this);
		}
		PlayerBase HoldingPlayer;
		if (Class.CastTo(HoldingPlayer, item.GetHierarchyRootPlayer())){
			if (m_QuickBarSlot >= 0){
				Print("[DME_Api] SetQuickBarEntityShortcut " + m_Type + " to " + m_QuickBarSlot);
				HoldingPlayer.SetQuickBarEntityShortcut(item, m_QuickBarSlot);
			}
		}
		Magazine_Base mag;
		float dmg;
		string cartType;
		int count;
		if (m_IsMagazine && Class.CastTo(mag, item)){
			count = m_Quantity;
			mag.ServerSetAmmoCount(count);
			if (m_MagAmmo){
				for (i = 0; i < mag.GetAmmoCount(); i++){
					if (i >= m_MagAmmo.Count()){break;}
					if (m_MagAmmo.Get(i) && m_MagAmmo.Get(i).dmg() >= 0 && m_MagAmmo.Get(i).cartTypeName() != "" && m_MagAmmo.Get(i).cartIndex() == i){
						mag.SetCartridgeAtIndex(m_MagAmmo.Get(i).cartIndex(), m_MagAmmo.Get(i).dmg(), m_MagAmmo.Get(i).cartTypeName());
					}
				}
			}
		} else if (item.IsAmmoPile() && Class.CastTo(mag, item)){
			count = m_Quantity;
			mag.ServerSetAmmoCount(count);
		}

		CarScript vehicle;
		if (m_IsVehicle && Class.CastTo(vehicle,item)){
			vehicle.OnDME_ApiLoad(this);
			ScriptCallQueue queueSync = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
			if (queueSync){ queueSync.Call(vehicle.Synchronize); }
		}

		// Damage System
		DamageZoneMap zones = new DamageZoneMap;
		DamageSystem.GetDamageZoneMap(item, zones);
		for( i = 0; i < zones.Count(); i++ ){
			string zone = zones.GetKey(i);
			float health;
			if (ReadZoneHealth(zone, health)){
				item.SetHealth(zone, "", health);
			}
		}

		item.SetSynchDirty();
		ScriptCallQueue queueLoad = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
		if (queueLoad){ queueLoad.Call(item.AfterStoreLoad); }
	}


	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_EntityStore>.JsonMakeData(this);
		return jsonString;
	}

	bool IsValid(){
		return m_Type != "" && m_Health >= 0;
	}

	bool Write(string var, bool data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		m_MetaData.Insert(new DME_Api_MetaData(var, data.ToString()));
		return true;
	}
	bool Write(string var, int data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		m_MetaData.Insert(new DME_Api_MetaData(var, data.ToString()));
		return true;
	}
	bool Write(string var, float data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		m_MetaData.Insert(new DME_Api_MetaData(var, data.ToString()));
		return true;
	}
	bool Write(string var, vector data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		m_MetaData.Insert(new DME_Api_MetaData(var, data.ToString()));
		return true;
	}
	bool Write(string var, TStringArray data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		for (int ii = 0; ii < data.Count(); ii++){
			m_MetaData.Insert(new DME_Api_MetaData(var, data.Get(ii)));
		}
		return true;
	}
	bool Write(string var, TIntArray data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		for (int ii = 0; ii < data.Count(); ii++){
			m_MetaData.Insert(new DME_Api_MetaData(var, data.Get(ii).ToString()));
		}
		return true;
	}
	bool Write(string var, TBoolArray data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		for (int ii = 0; ii < data.Count(); ii++){
			m_MetaData.Insert(new DME_Api_MetaData(var, data.Get(ii).ToString()));
		}
		return true;
	}
	bool Write(string var, TFloatArray data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		for (int ii = 0; ii < data.Count(); ii++){
			m_MetaData.Insert(new DME_Api_MetaData(var, data.Get(ii).ToString()));
		}
		return true;
	}
	bool Write(string var, TVectorArray data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		for (int ii = 0; ii < data.Count(); ii++){
			m_MetaData.Insert(new DME_Api_MetaData(var, data.Get(ii).ToString()));
		}
		return true;
	}
	bool Write(string var, string data){
		if (!m_MetaData) { m_MetaData = new array<ref DME_Api_MetaData>;}
		m_MetaData.Insert(new DME_Api_MetaData(var, data));
		return true;
	}
	bool Write(string var, Class data){
		Error("[DME_Api] Trying to save undefined data class to " + var + " for " + m_Type + " try converting to a string before saving");
		return false;
	}


	bool Read(string var, out bool data){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data = m_MetaData.Get(i).ReadInt();
				return true;
			}
		}
		return false;
	}
	bool Read(string var, out int data){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data = m_MetaData.Get(i).ReadInt();
				return true;
			}
		}
		return false;
	}
	bool Read(string var, out float data){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data = m_MetaData.Get(i).ReadFloat();
				return true;
			}
		}
		return false;
	}
	bool Read(string var, out vector data){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data = m_MetaData.Get(i).ReadVector();
				return true;
			}
		}
		return false;
	}
	bool Read(string var, out TStringArray data){
		bool found = false;
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data.Insert(m_MetaData.Get(i).ReadString());
				found = true;
			}
		}
		return found;
	}
	bool Read(string var, out TIntArray data){
		bool found = false;
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data.Insert(m_MetaData.Get(i).ReadInt());
				found = true;
			}
		}
		return found;
	}
	bool Read(string var, out TFloatArray data){
		bool found = false;
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data.Insert(m_MetaData.Get(i).ReadFloat());
				found = true;
			}
		}
		return found;
	}
	bool Read(string var, out TBoolArray data){
		bool found = false;
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				bool value = (m_MetaData.Get(i).ReadInt());
				data.Insert(value);
				found = true;
			}
		}
		return found;
	}
	bool Read(string var, out TVectorArray data){
		bool found = false;
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data.Insert(m_MetaData.Get(i).ReadVector());
				found = true;
			}
		}
		return found;
	}
	bool Read(string var, out string data){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){
				data = m_MetaData.Get(i).ReadString();
				return true;
			}
		}
		return false;
	}
	bool Read(string var, out Class data){
		Error("[DME_Api] Trying to read undefined data class for " + var + " for " + m_Type + " try converting to a string before saving");
		return false;
	}

	int GetInt(string var){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){return m_MetaData.Get(i).ReadInt();}
		}
		return 0;
	}
	float GetFloat(string var){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){ return m_MetaData.Get(i).ReadFloat(); }
		}
		return 0;
	}
	vector GetVector(string var){
		for(int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){ return m_MetaData.Get(i).ReadVector(); }
		}
		return Vector(0,0,0);
	}
	string GetString(string var){
		for (int i = 0; i < m_MetaData.Count(); i++){
			if (m_MetaData.Get(i) && m_MetaData.Get(i).Is(var)){ return m_MetaData.Get(i).ReadString(); }
		}
		return "";
	}


	bool SaveZoneHealth(string zone, float health){
		if (!m_HealthZones){m_HealthZones = new array<ref DME_Api_ZoneHealthData>;}
		m_HealthZones.Insert(new DME_Api_ZoneHealthData(zone, health));
		return true;
	}
	bool ReadZoneHealth(string zone, out float health){
		for (int i = 0; i < m_HealthZones.Count(); i++){
			if (m_HealthZones.Get(i) && m_HealthZones.Get(i).Is(zone)){
				health = m_HealthZones.Get(i).Health();
				return true;
			}
		}
		return false;
	}
}

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
modded class Weapon_Base extends Weapon {
	
	override void OnDME_ApiSave(DME_Api_EntityStore data){
		super.OnDME_ApiSave(data);
		
	}
	
	override void OnDME_ApiLoad(DME_Api_EntityStore data){
		int i;
		super.OnDME_ApiLoad(data);
		if (!g_Game) return;
		Print("===========================================================================================================");
		Print("===========================================================================================================");
		Print("[DME_Api] [INFO] Validating and Repairing the Weapon Unless this is just before a crash this was not the cause");
		Print("-----------------------------------------------------------------------------------------------------------");
			ValidateAndRepair();
			int dummy_version = int.MAX;
			PlayerBase parentPlayer = PlayerBase.Cast(GetHierarchyRootPlayer());
			if (!parentPlayer)
				dummy_version -= 1;
			ScriptReadWriteContext ctxdata = new ScriptReadWriteContext;
			OnStoreSave(ctxdata.GetWriteContext());
			OnStoreLoad(ctxdata.GetReadContext(), dummy_version);
			/*g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(weap.ValidateAndRepair,100,false);*/
			for (int mi = 0; mi < GetMuzzleCount(); ++mi)
			{
				if (data.m_ChamberedRound){
					Print("[DME_Api] Pushing Round to Chamber");
					PushCartridgeToChamber(mi, data.m_ChamberedRound.dmg(), data.m_ChamberedRound.cartTypeName());
				}
				for (i = 0; i < data.m_MagAmmo.Count(); i++){
					if (i > data.m_Quantity) {break;}
					PushCartridgeToInternalMagazine( mi, data.m_MagAmmo.Get(i).dmg(), data.m_MagAmmo.Get(i).cartTypeName());
				}
			}
			SetStepZeroing(GetCurrentMuzzle(), data.GetInt("m_Zeroing"));
			SetZoom(data.GetFloat("m_Zoom"));
			DryFire(GetCurrentMuzzle());
		Print("===========================================================================================================");
		Print("===========================================================================================================");
			if (data.GetInt("m_IsJammed") == 1){
				Print("Setting SetJammed");
				SetJammed(true);
			}
			if (data.m_FireModes){
				for (i = 0; i < data.m_FireModes.Count(); ++i)
				{
					SetCurrentMode(i, data.m_FireModes.Get(i));
				}
			}
		ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
		if (queue){ queue.Call(this.SendUApiWeaponAfterLoadClient, data.m_QuickBarSlot); }
	}

	void SendUApiWeaponAfterLoadClient(int quickBarSlot){
		if (!g_Game) return;
		g_Game.RemoteObjectTreeDelete(this);
		g_Game.RemoteObjectTreeCreate(this);
		ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
		if (queue){ queue.Call(DME_Api_FixRemote, quickBarSlot); }
	}
	
	void DME_Api_FixRemote(int quickBarSlot){
		//array<EntityAI> items = new array<EntityAI>;
		/*GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
			if (items && items.Count() > 0){
				for (int i = 0; i < items.Count(); i++){
					EntityAI child_item = EntityAI.Cast(items.Get(i));
					if (child_item ){
						g_Game.RemoteObjectCreate(child_item);
					}
				}
			}*/
		if (quickBarSlot >= 0){
			PlayerBase HoldingPlayer;
			if (Class.CastTo(HoldingPlayer, GetHierarchyRootPlayer())){
				HoldingPlayer.SetQuickBarEntityShortcut(this, quickBarSlot);
			}
		}
		RPCSingleParam(DME_API_RPC_WEAPON_FIX, new Param1<bool>( true ), true, NULL);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		if (!g_Game) return;
		if (rpc_type == DME_API_RPC_WEAPON_FIX && !g_Game.IsDedicatedServer()) {
			Param1<bool> data;
			if (ctx.Read(data))	{
				Print("[DME_Api] OnRPC" + GetType());
				if (data.param1 && !g_Game.IsDedicatedServer()){
					DME_Api_WeaponAfterLoadClient();
				}
			}
		}
	}
	
	void DME_Api_WeaponAfterLoadClient(){
		if (!g_Game) return;
		int i;
		//if (!data){return;}
		if (g_Game.IsDedicatedServer() || !g_Game.IsMultiplayer()){return;}
		Print("===========================================================================================================");
		Print("===========================================================================================================");
		Print("[DME_Api] [INFO] Validating and Repairing the Weapon Unless this is just before a crash this was not the cause");
		Print("-----------------------------------------------------------------------------------------------------------");
			ValidateAndRepair();
			/*int dummy_version = int.MAX;
			PlayerBase parentPlayer = PlayerBase.Cast(GetHierarchyRootPlayer());
			if (!parentPlayer)
				dummy_version -= 1;
			ScriptReadWriteContext ctx = new ScriptReadWriteContext;
			OnStoreSave(ctx.GetWriteContext());
			OnStoreLoad(ctx.GetReadContext(), dummy_version);
			for (int mi = 0; mi < GetMuzzleCount(); ++mi)
			{
				if (data.m_ChamberedRound){
					Print("[DME_Api] Pushing Round to Chamber");
					PushCartridgeToChamber(mi,  data.m_ChamberedRound.dmg(),  data.m_ChamberedRound.cartTypeName());
				}
				for (i = 0; i < data.m_MagAmmo.Count(); i++){
					if (i > data.m_Quantity) {break;}
					PushCartridgeToInternalMagazine( mi,  data.m_MagAmmo.Get(i).dmg(),  data.m_MagAmmo.Get(i).cartTypeName());
				}
			}
		//g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(this.AfterStoreLoad);
		SetStepZeroing(GetCurrentMuzzle(), data.GetInt("Vanilla", "m_Zeroing"));
		SetZoom(data.GetFloat("Vanilla", "m_Zoom"));*/
		Print("===========================================================================================================");
		Print("===========================================================================================================");
			/*if (data.GetInt("Vanilla", "m_IsJammed") == 1){
				Print("Setting SetJammed");
				SetJammed(true);
			}
			if (data.m_FireModes){
				for (i = 0; i < data.m_FireModes.Count(); ++i)
				{
					SetCurrentMode(i, data.m_FireModes.Get(i));
				}
			}*/
	}
	
}
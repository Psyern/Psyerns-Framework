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
 * Cross-PBO merge (Fall E): item/quantity helpers from _UAPIBase PlayerBase.c
 * merged with the currency API from _UniversalApi PlayerBase.c. The redundant
 * GetQuickBarEntityIndex wrapper was dropped (callers use vanilla
 * FindQuickBarEntityIndex). Fixes: extra paren in UMaxQuantity, double semicolon
 * in UGetItemCount, g_Game null-checks on Config/CreateObjectEx/ObjectDelete.
 */
modded class PlayerBase {

	//Return How many Items it faild to create in the Inventory
	int UCreateItemInInventory(string itemType, int amount = 1)
	{
		array<EntityAI> itemsArray = new array<EntityAI>;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		string itemTypeLower = itemType;
		itemTypeLower.ToLower();
		ItemBase item;
		Ammunition_Base ammoItem;
		int currentAmount = amount;
		bool hasQuantity = ((UMaxQuantity(itemType) > 0) || UHasQuantity(itemType));
		if (hasQuantity){
			for (int i = 0; i < itemsArray.Count(); i++){
				if (currentAmount <= 0){
					this.UpdateInventoryMenu(); // RPC-Call needed?
					return 0;
				}
				Class.CastTo(item, itemsArray.Get(i));
				string itemPlayerType = "";
				if (item){
					if (item.IsRuined()){
						continue;
					}
					itemPlayerType = item.GetType();
					itemPlayerType.ToLower();
					if (itemTypeLower == itemPlayerType && !item.IsFullQuantity() && !item.IsMagazine()){
						currentAmount = item.UAddQuantity(currentAmount);
					}
				}

				Class.CastTo(ammoItem, itemsArray.Get(i));
				if (ammoItem){
					if (ammoItem.IsRuined()){
						continue;
					}
					itemPlayerType = ammoItem.GetType();
					itemPlayerType.ToLower();
					if (itemTypeLower == itemPlayerType && ammoItem.IsAmmoPile()){
						currentAmount = ammoItem.UAddQuantity(currentAmount);
					}
				}
			}
		}
		bool stoploop = false;
		int MaxLoop = 5000;
		//any leftover or new stacks
		while (currentAmount > 0 && !stoploop && MaxLoop > 0){
			MaxLoop--;
			ItemBase newItem = ItemBase.Cast(this.GetInventory().CreateInInventory(itemType));
			if (!newItem){
				stoploop = true; //To stop the loop from running away since it couldn't create an item
				for (int j = 0; j < itemsArray.Count(); j++){
					Class.CastTo(item, itemsArray.Get(j));
					if (item){
						newItem = ItemBase.Cast(item.GetInventory().CreateInInventory(itemType)); //CreateEntityInCargo
						if (newItem){
							stoploop = false; //Item was created so we don't need to stop the loop anymore
							break;
						}
					}
				}
			}

			Magazine newMagItem = Magazine.Cast(newItem);
			Ammunition_Base newammoItem = Ammunition_Base.Cast(newItem);
			if (newMagItem && !newammoItem)	{
				int SetAmount = currentAmount;
				if (newMagItem.GetQuantityMax() <= currentAmount){
					SetAmount = currentAmount;
					currentAmount = 0;
				} else {
					SetAmount = newMagItem.GetQuantityMax();
					currentAmount = currentAmount - SetAmount;
				}
				newMagItem.ServerSetAmmoCount(SetAmount);
			} else if (hasQuantity){
				if (newammoItem){
					currentAmount = newammoItem.USetQuantity(currentAmount);
				}
				ItemBase newItemBase;
				if (Class.CastTo(newItemBase, newItem)){
					currentAmount = newItemBase.USetQuantity(currentAmount);
				}
			} else { //It created just one of the item
				currentAmount--;
			}
		}
		return currentAmount;
	}

	void UCreateItemGround(string Type, int Amount = 1){
		if (!g_Game) return;
		int AmountToSpawn = Amount;
		bool HasQuantity = ((UMaxQuantity(Type) > 0) || UHasQuantity(Type));
		int MaxQuanity = UMaxQuantity(Type);
		int StacksRequired = AmountToSpawn;
		if (MaxQuanity != 0){
			StacksRequired = Math.Ceil( AmountToSpawn /  MaxQuanity);
		}
		for (int i = 0; i <= StacksRequired; i++){
			if (AmountToSpawn > 0){
				ItemBase newItem = ItemBase.Cast(g_Game.CreateObjectEx(Type, GetPosition(), ECE_PLACE_ON_SURFACE));
				if (newItem && HasQuantity){
					AmountToSpawn = newItem.USetQuantity(AmountToSpawn);
				}
			}
		}
	}

	int UCurrentQuantity(ItemBase money){
		ItemBase moneyItem = ItemBase.Cast(money);
		if (!moneyItem){
			return 0;
		}
		if (UMaxQuantity(moneyItem.GetType()) == 0){
			return 1;
		}
		if ( moneyItem.IsMagazine() ){
			Magazine mag = Magazine.Cast(moneyItem);
			if (mag){
				return mag.GetAmmoCount();
			}
		}
		return moneyItem.GetQuantity();
	}

	int UMaxQuantity(string Type)
	{
		if (!g_Game) return 0;
		if ( g_Game.ConfigIsExisting(  CFG_MAGAZINESPATH  + " " + Type + " count" ) ){
			return g_Game.ConfigGetInt(  CFG_MAGAZINESPATH  + " " + Type + " count" );
		}
		if ( g_Game.ConfigIsExisting(  CFG_VEHICLESPATH + " " + Type + " varQuantityMax" ) ){
			return g_Game.ConfigGetInt( CFG_VEHICLESPATH + " " + Type + " varQuantityMax" );
		}
		return 0;
	}

	bool USetMoneyAmount(ItemBase item, int amount)
	{
		ItemBase money = ItemBase.Cast(item);
		if (!money){
			return false;
		}
		if ( money.IsMagazine() ){
			Magazine mag = Magazine.Cast(money);
			if (mag){
				mag.ServerSetAmmoCount(amount);
				return true;
			}
		}
		else{
			money.SetQuantity(amount);
			return true;
		}
		return false;
	}

	bool UHasQuantity(string type)
	{
		if (!g_Game) return false;
		string path = CFG_MAGAZINESPATH  + " " + type + " count";
		if (g_Game.ConfigIsExisting(path)){
			if (g_Game.ConfigGetInt(path) > 0){
				return true;
			}
		}
		path = CFG_VEHICLESPATH  + " " + type + " quantityBar";
		if (g_Game.ConfigIsExisting(path))   {
			return g_Game.ConfigGetInt(path) == 1;
		}

		return false;
	}

	bool UCanAcceptCurrency(string key, ItemBase item){
		return !item.IsRuined() || DME_Api_Currency.GetCurrency(key).CanUseRuined();
	}


	int UGetPlayerBalance(string key){
		int PlayerBalance = 0;
		if (!DME_Api_Currency.GetCurrency(key) || DME_Api_Currency.GetCurrency(key).Count() < 1){
			DME_Api_Currency.UDebug();
			DME_Api_Log.Err("Currency key: " + key + " is not configured");
			return 0;
		}
		array<EntityAI> inventory = new array<EntityAI>;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);

		ItemBase item;
		for (int i = 0; i < inventory.Count(); i++){
			if (Class.CastTo(item, inventory.Get(i))){
				for (int j = 0; j < DME_Api_Currency.GetCurrency(key).Count(); j++){
					if (!DME_Api_Currency.GetCurrency(key).Get(j)){
						DME_Api_Log.Err("Currency key: " + key + " idx " + j + " is NULL");
						DME_Api_Currency.UDebug();
						break;
					}
					if (item.GetType() == DME_Api_Currency.GetCurrency(key).Get(j).TypeClass() && UCanAcceptCurrency(key, item)){
						PlayerBalance += UCurrentQuantity(item) * DME_Api_Currency.GetCurrency(key).Get(j).Value();
					}
				}
			}
		}
		return PlayerBalance;
	}


	int UAddMoney(string key, int Amount){
		if (Amount <= 0){
			return 2;
		}
		int Return = 0;
		int AmountToAdd = Amount;
		bool NoError = true;
		int PlayerBalance = UGetPlayerBalance(key);
		int OptimalPlayerBalance = PlayerBalance + AmountToAdd;

		DME_Api_CurrencyValue MoneyValue = DME_Api_Currency.GetCurrency(key).GetHighestDenomination(AmountToAdd);
		int MaxLoop = 3000;
		while (MoneyValue && AmountToAdd >= DME_Api_Currency.GetLowestDenominationValue(key) && NoError && MaxLoop > 0){
			MaxLoop--;
			int AmountToSpawn = DME_Api_Currency.GetAmount(MoneyValue,AmountToAdd);
			if (AmountToSpawn == 0){
				NoError = false;
			}

			int AmountLeft = UCreateItemInInventory(MoneyValue.TypeClass(), AmountToSpawn);
			if (AmountLeft > 0){
				Return = 1;
				UCreateItemGround(MoneyValue.TypeClass(), AmountLeft);
			}

			int AmmountAdded = MoneyValue.Value() * AmountToSpawn;

			AmountToAdd = AmountToAdd - AmmountAdded;

			DME_Api_CurrencyValue NewMoneyValue = DME_Api_Currency.GetCurrency(key).GetHighestDenomination(AmountToAdd);
			if (NewMoneyValue && NewMoneyValue != MoneyValue){
				MoneyValue = NewMoneyValue;
			} else {
				NoError = false;
			}
		}
		return Return;
	}


	int URemoveMoney(string key, int Amount){
		if (Amount <= 0){
			return 2;
		}
		int Return = 0;
		int AmountToRemove = Amount;
		bool NoError = true;
		for (int i = 0; i < DME_Api_Currency.GetCurrency(key).Count(); i++){
			AmountToRemove =  URemoveMoneyInventory(key, DME_Api_Currency.GetCurrency(key).Get(i), AmountToRemove);
		}
		if (AmountToRemove >= DME_Api_Currency.GetLowestDenominationValue(key)){ // Now to delete a larger bill and make change
			for (int j = DME_Api_Currency.GetLastIndex(key); j >= 0; j--){
				int NewAmountToRemove =  URemoveMoneyInventory(key, DME_Api_Currency.GetCurrency(key).Get(j), DME_Api_Currency.GetCurrency(key).Get(j).Value());
				if (NewAmountToRemove == 0){
					int AmountToAddBack = DME_Api_Currency.GetCurrency(key).Get(j).Value() - AmountToRemove;
					Return = UAddMoney(key, AmountToAddBack);
				}
			}
		}
		return Return;
	}

	//Return how much left still to remove
	int URemoveItemFromInventory(string removeItemType, float Amount = 1 ){
		if (!g_Game) return Amount;
		int AmountToRemove = Amount;
		if (AmountToRemove > 0){
			array<EntityAI> itemsArray = new array<EntityAI>;
			this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
			for (int i = 0; i < itemsArray.Count(); i++){
				ItemBase item = ItemBase.Cast(itemsArray.Get(i));
				if ( item ){
					string ItemType = item.GetType();
					ItemType.ToLower();
					string RemoveItemType = removeItemType;
					RemoveItemType.ToLower();
					if (ItemType == RemoveItemType){
						int CurQuantity = item.GetQuantity();
						int AmountRemoved = 0;
						if (!item.HasQuantity()){
							CurQuantity = 1;
						}
						if (AmountToRemove < CurQuantity){
							AmountRemoved = AmountToRemove;
							item.USetQuantity(CurQuantity - AmountToRemove);
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount - AmountRemoved;
						} else if (AmountToRemove == CurQuantity){
							AmountRemoved = AmountToRemove;
							g_Game.ObjectDelete(item);
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount - AmountRemoved;
						} else {
							AmountRemoved = CurQuantity;
							AmountToRemove = AmountToRemove - CurQuantity;
							g_Game.ObjectDelete(item);
							Amount = Amount - AmountRemoved;
						}
						if (AmountToRemove <= 0){
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount;
						}
					}
				}
			}
		}
		this.UpdateInventoryMenu(); // RPC-Call needed?
		return Amount;
	}

	int UGetItemCount(string itemType, bool CountRuined = true){
		int PlayerBalance = 0;
		array<EntityAI> inventory = new array<EntityAI>;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);

		ItemBase item;
		for (int i = 0; i < inventory.Count(); i++){
			if (Class.CastTo(item, inventory.Get(i))){
				if (item.GetType() == itemType && ( !item.IsRuined() || CountRuined)){
					PlayerBalance += UCurrentQuantity(item);
				}
			}
		}
		return PlayerBalance;
	}

	//Return how much left still to remove
	float URemoveMoneyInventory(string key, DME_Api_CurrencyValue MoneyValue, float Amount ){
		if (!g_Game) return Amount;
		int AmountToRemove = DME_Api_Currency.GetAmount(MoneyValue, Amount);
		if (AmountToRemove > 0){
			array<EntityAI> itemsArray = new array<EntityAI>;
			this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
			for (int i = 0; i < itemsArray.Count(); i++){
				ItemBase item = ItemBase.Cast(itemsArray.Get(i));
				if (item && UCanAcceptCurrency(key, item)){
					string ItemType = item.GetType();
					ItemType.ToLower();
					string MoneyType = MoneyValue.TypeClass();
					MoneyType.ToLower();
					if (ItemType == MoneyType){
						int CurQuantity = item.GetQuantity();
						int AmountRemoved = 0;
						if (!item.HasQuantity()){
							CurQuantity = 1;
						}
						if (AmountToRemove < CurQuantity){
							AmountRemoved = MoneyValue.Value() * AmountToRemove;
							item.USetQuantity(CurQuantity - AmountToRemove);
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount - AmountRemoved;
						} else if (AmountToRemove == CurQuantity){
							AmountRemoved = MoneyValue.Value() * AmountToRemove;
							g_Game.ObjectDelete(item);
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount - AmountRemoved;
						} else {
							AmountRemoved = MoneyValue.Value() * CurQuantity;
							AmountToRemove = AmountToRemove - CurQuantity;
							g_Game.ObjectDelete(item);
							Amount = Amount - AmountRemoved;
						}
						if (AmountToRemove <= 0){
							this.UpdateInventoryMenu(); // RPC-Call needed?
							return Amount;
						}
					}
				}
			}
		}
		this.UpdateInventoryMenu(); // RPC-Call needed?
		return Amount;
	}

}

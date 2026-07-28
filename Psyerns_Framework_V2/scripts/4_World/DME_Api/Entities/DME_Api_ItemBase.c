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
 * Cross-PBO merge (Fall C): base leaf-hooks + quantity helpers from _UAPIBase
 * ItemBase.c. The _UniversalApi no-op override was dropped (single modded class).
 */
modded class ItemBase {

	void OnDME_ApiSave(DME_Api_EntityStore data){

	}

	void OnDME_ApiLoad(DME_Api_EntityStore data){

	}

	//returns remaining
	int UAddQuantity(float amount) {
		if (!IsMagazine()) {
			int remainingQty = GetQuantityMax() - GetQuantity();
			if (remainingQty == 0){
				return amount;
			}
			if ( amount >= remainingQty ) {
				AddQuantity(remainingQty);
				return amount - remainingQty;
			} else {
				AddQuantity(amount);
				return 0;
			}
		}
		return amount;
	}

	//returns remaining
	int USetQuantity(float amount) {
		if (!IsMagazine()) {
			int maxQty = GetQuantityMax();
			if ( amount >= maxQty ) {
				SetQuantity(maxQty);
				return amount - maxQty;
			} else {
				SetQuantity(amount);
				return 0;
			}
		}
		return amount;
	}

	bool UHasQuantityBar() {
		return this.ConfigGetBool("quantityBar");
	}
}

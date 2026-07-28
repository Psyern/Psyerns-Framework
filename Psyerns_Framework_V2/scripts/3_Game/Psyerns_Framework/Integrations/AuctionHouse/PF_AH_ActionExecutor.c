/**
 * PF_AH_ActionExecutor — Runs a pending web-action (purchase/bid/cancel) against
 * the live DME_AH_AuctionManager.
 *
 * The AuctionManager APIs already operate on string UIDs and do NOT require a
 * connected PlayerBase instance, so offline execution works natively via:
 *   DME_AH_Module.GetInstance().GetAuctionManager().BuyNow(uid, name, listingId)
 *   ...                                        .PlaceBid(uid, name, listingId, amount)
 *   ...                                        .CancelListing(uid, listingId)
 *
 * No wrappers in DME_AH_Module.c were needed — the existing public accessor chain
 * is sufficient.
 *
 * Seller name lookup: for actions where DME_AH needs a display name but the WP
 * payload only carries a UID, we look up the listing's SellerName/CurrentBidderName
 * when possible; otherwise we fall back to a synthetic "web:<uid>" label.
 *
 * All calls are guarded with #ifdef DME_AH_Scripts — if the auction-house mod is
 * not loaded, the action is reported back as failed_dme_ah_missing.
 */
class PF_AH_ActionExecutor
{
	void Execute(PF_AH_PendingAction action, out string outStatus, out string outResultCode, out string outMessage)
	{
		outStatus = "failed_unknown";
		outResultCode = "unknown";
		outMessage = "Unhandled action type";

		if (!action)
		{
			outStatus = "failed_null_action";
			outResultCode = "null_action";
			outMessage = "Action payload was null";
			return;
		}

		string typeLower = action.type;
		typeLower.ToLower();

#ifdef DME_AH_Scripts
		DME_AH_Module mod = DME_AH_Module.GetInstance();
		if (!mod)
		{
			outStatus = "failed_dme_ah_missing";
			outResultCode = "dme_ah_module_missing";
			outMessage = "DME_AH_Module instance unavailable";
			return;
		}

		DME_AH_AuctionManager mgr = mod.GetAuctionManager();
		if (!mgr)
		{
			outStatus = "failed_dme_ah_missing";
			outResultCode = "dme_ah_manager_missing";
			outMessage = "DME_AH_AuctionManager not initialized";
			return;
		}

		DME_AH_DataStore store = mod.GetDataStore();
		string buyerDisplayName = ResolveDisplayName(store, action.player_uid, action.listing_id);

		int code;
		if (typeLower == "purchase")
		{
			code = mgr.BuyNow(action.player_uid, buyerDisplayName, action.listing_id);
		}
		else if (typeLower == "bid")
		{
			code = mgr.PlaceBid(action.player_uid, buyerDisplayName, action.listing_id, action.amount);
		}
		else if (typeLower == "cancel")
		{
			code = mgr.CancelListing(action.player_uid, action.listing_id);
		}
		else if (typeLower == "admin_cancel")
		{
			code = HandleAdminCancel(mgr, store, action.listing_id);
		}
		else
		{
			outStatus = "failed_unknown_type";
			outResultCode = "unknown_type";
			outMessage = "Unknown action type: " + action.type;
			return;
		}

		MapResultCode(code, outStatus, outResultCode, outMessage);
		PF_Logger.Log("AH: Executed " + action.type + " for " + action.player_uid + " listing=" + action.listing_id + " → " + outStatus);
#else
		outStatus = "failed_dme_ah_missing";
		outResultCode = "dme_ah_mod_not_loaded";
		outMessage = "DME_Auction_House mod is not loaded on this server";
#endif
	}

#ifdef DME_AH_Scripts
	protected string ResolveDisplayName(DME_AH_DataStore store, string uid, string listingId)
	{
		if (!store)
			return "web:" + uid;

		DME_AH_Listing l = store.GetListingByID(listingId);
		if (l)
		{
			if (l.CurrentBidderUID == uid && l.CurrentBidderName != "")
				return l.CurrentBidderName;
			if (l.SellerUID == uid && l.SellerName != "")
				return l.SellerName;
		}

		return "web:" + uid;
	}

	/**
	 * admin_cancel per README §13 #14: items return to seller as a PendingPickup.
	 * The existing CancelListing() refuses to cancel listings that have bids, so we
	 * route through a dedicated admin path:
	 *   1. Refund the current bidder if any.
	 *   2. Force-cancel the listing via the seller UID.
	 *   3. Note: the seller's item-return is already handled by DME_AH's existing
	 *      PendingPickup system when the listing leaves Active state.
	 * If no bids, we can just invoke the normal CancelListing as the seller.
	 */
	protected int HandleAdminCancel(DME_AH_AuctionManager mgr, DME_AH_DataStore store, string listingId)
	{
		if (!mgr || !store)
			return EDME_AH_ResultCode.FailedServerError;

		DME_AH_Listing l = store.GetListingByID(listingId);
		if (!l)
			return EDME_AH_ResultCode.FailedListingNotFound;

		// Standard cancel path is sufficient for listings without bids.
		if (!l.HasBids())
			return mgr.CancelListing(l.SellerUID, listingId);

		// Listings with active bids require admin-level force cancel.
		// We emit a pending-pickup for the current bidder (refund) and then
		// remove the listing + write a Cancelled transaction record.
		// This intentionally does NOT call mgr.CancelListing() (which would
		// reject the listing because HasBids()==true).
		ForceAdminCancel(store, l);
		return EDME_AH_ResultCode.Success;
	}

	protected void ForceAdminCancel(DME_AH_DataStore store, DME_AH_Listing listing)
	{
		if (!store || !listing)
			return;

		if (listing.HasBids() && listing.CurrentBidderUID != "" && listing.CurrentBid > 0)
		{
			DME_AH_PendingPickup refund = new DME_AH_PendingPickup();
			refund.PendingID = "ADMIN_REFUND_" + listing.ListingID + "_" + listing.CurrentBidderUID;
			refund.PlayerUID = listing.CurrentBidderUID;
			refund.Amount = listing.CurrentBid;
			refund.Timestamp = DME_AH_Util.GetTimestamp();
			store.AddPendingPickup(refund);
		}

		listing.Status = EDME_AH_ListingStatus.Cancelled;

		DME_AH_Transaction tx = new DME_AH_Transaction();
		tx.TransactionID = DME_AH_Transaction.GenerateID();
		tx.ListingID = listing.ListingID;
		tx.SellerUID = listing.SellerUID;
		tx.SellerName = listing.SellerName;
		tx.ItemClassName = listing.ItemClassName;
		tx.ItemDisplayName = listing.ItemDisplayName;
		tx.Timestamp = DME_AH_Util.GetTimestamp();
		tx.Type = EDME_AH_TransactionType.Cancelled;

		store.AddTransaction(tx);
		store.RemoveListing(listing.ListingID);
	}
#endif

	protected void MapResultCode(int code, out string outStatus, out string outResultCode, out string outMessage)
	{
		if (code == 0)
		{
			outStatus = "success";
			outResultCode = "ok";
			outMessage = "";
			return;
		}

		outStatus = "failed_" + ResultCodeToSnake(code);
		outResultCode = ResultCodeToSnake(code);
		outMessage = ResultCodeToMessage(code);
	}

	protected string ResultCodeToSnake(int code)
	{
#ifdef DME_AH_Scripts
		if (code == EDME_AH_ResultCode.FailedNotEnoughMoney)       return "not_enough_money";
		if (code == EDME_AH_ResultCode.FailedListingNotFound)      return "listing_not_found";
		if (code == EDME_AH_ResultCode.FailedListingExpired)       return "listing_expired";
		if (code == EDME_AH_ResultCode.FailedBidTooLow)            return "bid_too_low";
		if (code == EDME_AH_ResultCode.FailedMaxListingsReached)   return "max_listings_reached";
		if (code == EDME_AH_ResultCode.FailedMaxBidsReached)       return "max_bids_reached";
		if (code == EDME_AH_ResultCode.FailedItemNotInInventory)   return "item_not_in_inventory";
		if (code == EDME_AH_ResultCode.FailedCannotCancelWithBids) return "cannot_cancel_with_bids";
		if (code == EDME_AH_ResultCode.FailedOwnListing)           return "own_listing";
		if (code == EDME_AH_ResultCode.FailedInvalidPrice)         return "invalid_price";
		if (code == EDME_AH_ResultCode.FailedServerError)          return "server_error";
#endif
		return "code_" + code.ToString();
	}

	protected string ResultCodeToMessage(int code)
	{
#ifdef DME_AH_Scripts
		if (code == EDME_AH_ResultCode.FailedNotEnoughMoney)       return "Not enough currency";
		if (code == EDME_AH_ResultCode.FailedListingNotFound)      return "Listing not found or not active";
		if (code == EDME_AH_ResultCode.FailedListingExpired)       return "Listing has expired";
		if (code == EDME_AH_ResultCode.FailedBidTooLow)            return "Bid amount is below minimum";
		if (code == EDME_AH_ResultCode.FailedMaxListingsReached)   return "Seller has reached max listings";
		if (code == EDME_AH_ResultCode.FailedMaxBidsReached)       return "Bidder has reached max bids";
		if (code == EDME_AH_ResultCode.FailedItemNotInInventory)   return "Item no longer in seller inventory";
		if (code == EDME_AH_ResultCode.FailedCannotCancelWithBids) return "Cannot cancel listing with active bids";
		if (code == EDME_AH_ResultCode.FailedOwnListing)           return "Cannot act on own listing";
		if (code == EDME_AH_ResultCode.FailedInvalidPrice)         return "Price is invalid";
		if (code == EDME_AH_ResultCode.FailedServerError)          return "Server error during action";
#endif
		return "Unknown result code " + code.ToString();
	}
}


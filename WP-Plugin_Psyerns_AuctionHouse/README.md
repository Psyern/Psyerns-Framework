# Psyerns AuctionHouse — WordPress Plugin

Bridge between the DayZ mod **DME_Auction_House** and a WordPress site. Players see active auctions, history and statistics in the browser and, once logged in via Steam, can **buy and bid directly from the web**.

> **Status:** Design phase. This document is the living spec during planning. Once the spec is approved, it becomes an implementation plan in `docs/superpowers/specs/`.

---

## 1. Project Identity

| Field | Value |
|---|---|
| Plugin Name | Psyerns AuctionHouse |
| Plugin Slug | `psyerns-auctionhouse` |
| Text Domain | `psyerns-auctionhouse` |
| REST Namespace | `psyern-ah/v1` |
| Class Prefix | `Psyern_AH_` |
| DB Table Prefix | `{wp_prefix}psyern_ah_` |
| Author | Psyern / Deadmans Echo |
| License | MIT |
| WP min | 5.8 |
| PHP min | 7.4 |
| Path | `C:\Users\Administrator\Desktop\Psyerns_Framework\WP-Plugin_Psyerns_AuctionHouse` |

---

## 2. Data Sources (DayZ Server Side)

### DME_Auction_House Mod
Path: `C:\Users\Administrator\Desktop\DME_Auction_House`

JSON storage (on server): `$profile:DME_AH\Data\`

| File | Content | Class |
|---|---|---|
| `ActiveListings.json` | All active listings | `DME_AH_ListingArray` → `array<DME_AH_Listing>` |
| `CompletedListings.json` | Transaction history | `DME_AH_TransactionArray` → `array<DME_AH_Transaction>` |
| `PlayerData.json` | Internal balances + pending pickups | `DME_AH_PlayerData` |

Config: `$profile:DME_AH\Config\Settings.json`, `Categories.json`, `NPCs.json`

### Listing Data Structure (excerpt)
```
DME_AH_Listing {
    string  ListingID;                 // e.g. "1712233412_84592"
    string  SellerUID, SellerName;
    string  ItemClassName, ItemDisplayName;
    int     CategoryID;
    int     ListingType;               // 0=BuyNow, 1=Auction, 2=AuctionWithBuyNow
    int     StartPrice, BuyNowPrice;
    int     CurrentBid, BidCount;
    string  CurrentBidderUID, CurrentBidderName;
    int     CreatedTimestamp;
    int     ExpiresTimestamp;
    int     Status;                    // 0=Active, 1=Sold, 2=Expired, 3=Cancelled
}
```

### Expansion ATM (for Internal Currency mirror, optional)
Path: `$profile:ExpansionMod\ATM\{PlayerUID}.json`
```
ExpansionMarketATM_Data { string PlayerID; int MoneyDeposited; }
```

### Currency Modes of the Mod
- `Expansion` — Expansion wallet integration
- `Item` — physical money item in inventory (e.g. `MoneyRuble100`)
- `Internal` — mod's own balance in `PlayerData.json` (the only one that works for web purchases without a running DayZ client)

Web purchases/bids work with **Expansion** (ATM balance is read) and **Internal**. **Not** with `Item`.

---

## 3. Decisions from Brainstorming

| # | Question | Decision |
|---|---|---|
| 1 | Data transport Mod → WP | **A** — HTTP push via Psyerns_Framework |
| 2 | Scope / phasing | **A** — View + BuyNow + Bidding all in one go |
| 3 | Steam login | **B** — Implement Steam OpenID inside the plugin |
| 4 | Mod code organization | **C** — As a module inside `Psyerns_Framework` (`PF_AH_Sync`) |
| 5 | Naming | **C** — Classes `Psyern_AH_*`, REST `psyern-ah/v1`, DB `psyern_ah_*` |
| 6 | Theming | **B** — Framework plugin as soft dependency, reuse its themes, fallback CSS in plugin |
| 7 | Shortcodes | **B** — 5 individual shortcodes (marketplace, listing, my, history, stats) |
| 8 | Filter/sort | **B** — Standard (category, type, price range, search + sort + pagination 20/p.) |
| 9 | Admin panel | **B** — Standard (API key, listings viewer, history, balance viewer, pending-actions log, settings) |
| 10 | Public visibility | **A** — Everything publicly visible (buying/bidding only with login) |

---

## 4. Architecture & Data Flow

```
┌──────────────────────────────────────────────┐
│  DayZ Server                                 │
│  ┌─────────────────────┐  ┌────────────────┐ │
│  │ DME_Auction_House   │  │ Expansion      │ │
│  │ $profile:DME_AH\    │  │ $profile:      │ │
│  │   ActiveListings    │  │   ExpansionMod\│ │
│  │   CompletedListings │  │   ATM\{UID}.json│ │
│  └────────┬────────────┘  └────────┬────────┘ │
│           └──────┬─────────────────┘          │
│                  ▼                            │
│  ┌──────────────────────────────────────────┐ │
│  │ Psyerns_Framework / Integrations / AH   │ │
│  │ PF_AH_Sync                               │ │
│  │  • Timer 30s: PUSH /upload               │ │
│  │  • Timer 10s: GET /internal/pending      │ │
│  │  • on action: execute via DME_AH_Module  │ │
│  │  • on result: PATCH /internal/pending/ID │ │
│  └──────────────────┬───────────────────────┘ │
└─────────────────────┼─────────────────────────┘
                      │  HTTPS + API-Key Header
                      ▼
┌──────────────────────────────────────────────┐
│  WordPress Server                            │
│  Plugin: psyerns-auctionhouse                │
│   REST psyern-ah/v1                          │
│   DB: psyern_ah_listings, _transactions,     │
│       _balances, _pending_actions, _users    │
│   Shortcodes: 5                              │
└──────────────────────────────────────────────┘
                      ▲
                      │ Browser
              ┌───────┴───────┐
              │ Player / Web  │
              └───────────────┘
```

### Core Principles
1. **Mod = Single Source of Truth** for balances and listing status. The website is always a mirror.
2. **Two transport directions**:
   - **Push** (Mod → WP, 30 s): upload listings, transactions, balances
   - **Poll** (Mod → WP, 10 s): fetch open web actions, execute, report result
3. **Asynchronous command pattern**: Web purchases/bids are not synchronous calls but jobs with a state machine:
   ```
   queued → dispatched → executing → (success | failed_{reason})
   ```
4. **Atomic balance reservation**: When placing a bid, the amount is reserved mod-side (deducted + marked as `reserved`). On outbid: returned. On auction win: finally deducted. Prevents double-spending.

### Trade-offs
- A web action shows its result after the next mod poll (max 10 s, avg 5 s) — fine for an AH.
- DayZ server offline → website is read-only, new jobs stay `queued` until the server is back.

---

## 5. Component Overview

### WordPress plugin (this directory)
- **Database layer** (`class-psyern-ah-database.php`) — 5 tables
- **REST API** (`class-psyern-ah-api.php`) — 3 areas: `/public/*`, `/user/*` (auth), `/internal/*` (API key)
- **Steam OpenID** (`class-psyern-ah-steam-auth.php`) — redirect, callback, UID ↔ WP user mapping
- **Auth** (`class-psyern-ah-auth.php`) — API key validation for mod endpoints
- **Data services** (`class-psyern-ah-listings.php`, `-transactions.php`, `-balances.php`, `-pending-actions.php`)
- **Shortcodes** (`class-psyern-ah-shortcodes.php`) — 5 shortcodes
- **Frontend assets** — JS for filter/AJAX + fallback CSS, uses framework themes via soft dependency
- **Admin** (`class-psyern-ah-admin.php`) — settings page + viewer tabs

### DayZ mod (inside `Psyerns_Framework`)
New folder: `scripts/3_Game/Psyerns_Framework/Integrations/AuctionHouse/`
- `PF_AH_Sync.c` — main orchestrator, timer logic
- `PF_AH_Uploader.c` — builds payloads, POST via `PF_WebClient`
- `PF_AH_PendingPoller.c` — GET /internal/pending, handoff to executor
- `PF_AH_ActionExecutor.c` — executes purchase/bid/cancel via `DME_AH_AuctionManager`, reports result
- `PF_AH_BalanceReader.c` — reads Expansion ATM files / DME_AH PlayerData
- `PF_AH_Config.c` — WP URL + API key + intervals (in `PsyernsFrameworkConfig.json` as block `AuctionHouse`)

---

## 6. Database Schema (Plan)

Tables (all prefixed `{wp_prefix}psyern_ah_`):

### `listings` — mirror of active listings
```sql
id               BIGINT UNSIGNED PK AUTO_INCREMENT
listing_id       VARCHAR(64) UNIQUE KEY         -- generated by the mod
seller_uid       VARCHAR(32)     INDEX
seller_name      VARCHAR(128)
item_class       VARCHAR(128)    INDEX
item_display     VARCHAR(255)
category_id      SMALLINT        INDEX
listing_type     TINYINT          -- 0/1/2
start_price      BIGINT
buy_now_price    BIGINT
current_bid      BIGINT
current_bidder_uid   VARCHAR(32)
current_bidder_name  VARCHAR(128)
bid_count        INT
created_ts       BIGINT
expires_ts       BIGINT           INDEX
status           TINYINT          INDEX  -- 0 Active
last_sync        DATETIME
```

### `transactions` — completed purchases
```sql
id               BIGINT UNSIGNED PK
transaction_id   VARCHAR(64) UNIQUE
listing_id       VARCHAR(64)     INDEX
seller_uid, seller_name, buyer_uid, buyer_name
item_class, item_display
final_price      BIGINT
fee              BIGINT
type             TINYINT          -- 0 BuyNow, 1 AuctionWon, 2 Expired, 3 Cancelled
timestamp        BIGINT           INDEX
```

### `balances` — balance mirror per player + currency source
```sql
id               BIGINT UNSIGNED PK
player_uid       VARCHAR(32)      INDEX
currency_source  VARCHAR(16)      -- "Expansion" | "Internal"
balance          BIGINT
updated_at       DATETIME
UNIQUE KEY (player_uid, currency_source)
```

### `pending_actions` — web jobs (purchase/bid/cancel)
```sql
id               BIGINT UNSIGNED PK
action_uuid      VARCHAR(36) UNIQUE      -- idempotency key
action_type      VARCHAR(16)             -- "purchase" | "bid" | "cancel"
player_uid       VARCHAR(32)    INDEX
listing_id       VARCHAR(64)    INDEX
amount           BIGINT                  -- bid: offer; purchase: BuyNowPrice snapshot
nonce            VARCHAR(64)
status           VARCHAR(16)    INDEX    -- queued/dispatched/executing/success/failed_*
result_code      VARCHAR(32)
result_message   TEXT
created_at       DATETIME
dispatched_at    DATETIME
completed_at     DATETIME
```

### `users` — mapping WordPress user ↔ Steam UID
```sql
id               BIGINT UNSIGNED PK
wp_user_id       BIGINT UNSIGNED UNIQUE
steam_uid        VARCHAR(32) UNIQUE
steam_name       VARCHAR(128)
avatar_url       VARCHAR(512)
linked_at        DATETIME
last_login       DATETIME
```

---

## 7. REST API (Plan)

| Method | Route | Auth | Purpose |
|---|---|---|---|
| GET  | `/public/listings` | none | Marketplace (pagination, filter, sort) |
| GET  | `/public/listings/{id}` | none | Detail |
| GET  | `/public/history` | none | Recent transactions |
| GET  | `/public/stats` | none | Top sellers, popular items, avg prices |
| GET  | `/public/price-history` | none | Price time series per `item_class` (query: `item_class`, `period=24h\|7d\|30d\|all`) |
| GET  | `/public/categories` | none | Category list |
| GET  | `/auth/steam/login` | none | Redirect to Steam OpenID |
| GET  | `/auth/steam/callback` | none | OpenID callback, set WP login |
| POST | `/auth/logout` | session | Logout |
| GET  | `/user/me` | session | Own info (UID, balance mirror, open jobs) |
| GET  | `/user/listings` | session | Own listings |
| GET  | `/user/bids` | session | Own bids |
| POST | `/user/purchase` | session + nonce | Create BuyNow job |
| POST | `/user/bid` | session + nonce | Create bid job |
| POST | `/user/cancel` | session + nonce | Cancel own listing |
| POST | `/internal/upload` | API key | Mod uploads listings/transactions/balances |
| GET  | `/internal/pending` | API key | Mod fetches open jobs (with dispatch lock) |
| PATCH| `/internal/pending/{uuid}` | API key | Mod reports result |
| GET  | `/internal/ping` | API key | Health check |

### Payload examples

**POST `/internal/upload`** (Mod → WP):
```json
{
  "generatedAt": "2026-04-20T14:00:00Z",
  "serverTimeEpoch": 1713621600,
  "currencyMode": "Expansion",
  "listings": [ { /* DME_AH_Listing shape */ } ],
  "recentTransactions": [ { /* DME_AH_Transaction shape, new since last sync */ } ],
  "balances": [ { "uid": "...", "source": "Expansion", "balance": 12340 } ]
}
```

**POST `/user/purchase`** (Web → WP):
```json
{
  "nonce": "wp-nonce-token",
  "listing_id": "1712233412_84592",
  "expected_price": 500
}
```
→ `{ "action_uuid": "...", "status": "queued" }`

**GET `/internal/pending`** (Mod → WP) — returns up to N open jobs and atomically sets them to `dispatched`:
```json
{
  "actions": [
    { "action_uuid": "...", "type": "purchase", "player_uid": "...",
      "listing_id": "...", "amount": 500, "created_at": "..." }
  ]
}
```

---

## 8. Shortcodes (Frontend)

| Shortcode | Purpose |
|---|---|
| `[psyerns_auctionhouse_marketplace theme="stalker" per_page="20"]` | Marketplace with filter, sort, pagination |
| `[psyerns_auctionhouse_listing id="..."]` | Single detail, Buy/Bid buttons, bid history |
| `[psyerns_auctionhouse_my]` | Logged-in user: balance, listings, bids, open jobs |
| `[psyerns_auctionhouse_history limit="50"]` | Recently sold items |
| `[psyerns_auctionhouse_stats]` | Top sellers, popular items, avg prices, **"Price Trends" tab with item dropdown + chart** |
| `[psyerns_auctionhouse_price_chart item_class="..." period="30d" height="300"]` | Standalone price chart for an `item_class`. Parameters: `item_class` (required), `period=24h\|7d\|30d\|all` (default `30d`), `height` in px |

**Marketplace filter** (via AJAX without page reload):
- Category (dropdown from `/public/categories`)
- Listing type (radio: All / BuyNow / Auction)
- Price range (min/max inputs)
- Search (item name, 300 ms debounce)
- Sort (dropdown: price ↑↓, time left ↑↓, newest, most bids)
- Pagination (20 per page)

---

## 9. Admin Panel

Menu: **Settings → Psyerns AuctionHouse** (tab interface mirroring the existing framework plugin).

| Tab | Content |
|---|---|
| Settings | Generate/rotate API key, polling intervals, currency format (e.g. `{amount} €`), public-visibility toggles |
| Listings | Table of all active listings (sort, search), admin-cancel button → creates `admin_cancel` pending action |
| History | Transactions with filters (date, player, item) |
| Balances | Read-only balance mirror of all players |
| Pending | Log of open/completed web jobs with status code + error message |
| Tools | "Force Re-Sync Request" (sets flag, next push is full sync), tables reset (dangerous, confirmed) |

---

## 10. Security Principles

- **API key** (Bearer) for all `/internal/*` routes — randomly generated, rotatable in admin
- **WP nonces** on all `/user/*` POST routes (CSRF)
- **Rate limit** per session on `/user/purchase` and `/user/bid` (e.g. max 10 per minute)
- **Idempotency** via `action_uuid` — mod can safely re-fetch the same job without double execution
- **Expected-price check**: web sends expected price, mod validates; if the listing changed in the meantime → `failed_price_mismatch`
- **Balance validation** server-side (in mod) — client sees only a mirror, cannot manipulate
- **Double-spend protection** via mod-side reservation on bid/purchase
- **Steam OpenID signature verification** on callback (no fake-UID injection)

---

## 11. Planned Directory Structure

```
WP-Plugin_Psyerns_AuctionHouse/
├── psyerns-auctionhouse.php              # Bootstrap, plugin header, hooks
├── uninstall.php                          # Drop tables + options
├── README.md                              # ← this document
├── readme.txt                             # WordPress.org format
├── includes/
│   ├── class-psyern-ah-database.php       # Table setup via dbDelta
│   ├── class-psyern-ah-auth.php           # API key validation
│   ├── class-psyern-ah-steam-auth.php     # OpenID login
│   ├── class-psyern-ah-api.php            # REST route registry
│   ├── class-psyern-ah-listings.php       # CRUD + filter listings
│   ├── class-psyern-ah-transactions.php   # CRUD + history
│   ├── class-psyern-ah-balances.php       # Balance mirror
│   ├── class-psyern-ah-pending-actions.php# Job state machine
│   ├── class-psyern-ah-stats.php          # Aggregation queries (incl. price time series)
│   └── class-psyern-ah-theme.php          # Framework theme detection/fallback
├── admin/
│   ├── class-psyern-ah-admin.php          # Menu + Settings API
│   ├── css/psyern-ah-admin.css
│   ├── js/psyern-ah-admin-tabs.js
│   └── views/
│       ├── settings-page.php
│       ├── listings-page.php
│       ├── history-page.php
│       ├── balances-page.php
│       └── pending-page.php
├── public/
│   ├── class-psyern-ah-shortcodes.php
│   ├── css/psyern-ah-public.css           # Fallback when framework is not active
│   ├── js/psyern-ah-marketplace.js        # AJAX filter, sort, pagination
│   ├── js/psyern-ah-listing.js            # Buy/Bid buttons, live countdown
│   ├── js/psyern-ah-price-chart.js        # Chart.js wrapper for price history
│   ├── vendor/chart.min.js                # Chart.js (local, no CDN dependency)
│   └── templates/
│       ├── marketplace.php
│       ├── listing-detail.php
│       ├── listing-card.php
│       ├── my.php
│       ├── history.php
│       ├── stats.php
│       └── price-chart.php
└── languages/
    └── psyerns-auctionhouse.pot
```

---

## 12. Polling Intervals (defaults, admin-configurable)

| Direction | Interval | Content |
|---|---|---|
| Mod → WP (push) | 30 s | Full upload (listings, transactions delta, balances) |
| Mod → WP (poll) | 10 s | Fetch open pending actions |
| Web → WP (browser refresh) | 15 s | Auto-refresh marketplace / own open jobs via fetch |

---

## 13. Additional Decisions (from clarification round)

| # | Question | Decision |
|---|---|---|
| 11 | Outbid notification | **No** — no emails. Only in-site badge/list under `[psyerns_auctionhouse_my]` (status column shows "Outbid" / "Leading" / "Won"). |
| 12 | Item icons | **Remote URLs** — from an item-map JSON on WordPress (admin-editable). No asset copies in the plugin. |
| 13 | Statistics periods | **All: 24h + 7d + 30d + all-time** — tab switching on `[psyerns_auctionhouse_stats]`. |
| 14 | Admin cancel | **Items back to seller** — `admin_cancel` pending action creates a pending pickup for the seller mod-side. |
| 15 | Sync mode | **Full sync** — mod sends all active listings on every push. WP upsert-replaces. Transactions are delta only (since `last_transaction_ts`). |
| 16 | Price chart | **C** — on listing detail automatically + stats tab "Price Trends" with item dropdown + free-form shortcode `[psyerns_auctionhouse_price_chart]`. Rendered via Chart.js. Periods: 24h/7d/30d/all. Metrics: avg price (line) + min/max (band) + sale count (bars). |

## 14. Open Technical Details

**Resolved in v1:**
- [x] Push payload size — no pagination/gzip in v1; soft-warn log if payload > 1 MB (realistic 20-60 KB at 50-200 listings). Add later if actual problems arise.
- [x] Item-map JSON — finalized schema: `{ version:1, default_icon_url, items: { "<item_class>": { display_name?, icon_url, rarity?, category_hint? } } }`. `rarity` controls card border color (common/uncommon/rare/epic/legendary).

**Deferred to v2:**
- [ ] **Bid history table.** `handle_get_user_bids` currently shows only listings where the user is still `current_bidder_uid` — when a user is outbid and a third party outbids again, the listing drops from the list. Real bid-participation feeds need either a new `psyern_ah_bid_history` table or a mod-side delta upload of bids.
- [ ] **Consolidate Steam-UID helper.** Phase 2 agents (listings, pending-actions) each implemented a private `get_current_steam_uid()`. Lift into `Psyern_AH_Auth::get_current_steam_uid()`.

---

## 15. Next Steps

1. ✅ Brainstorming complete
2. ⏳ Spec review with user
3. ⏳ Write design doc in `docs/superpowers/specs/2026-04-20-psyerns-auctionhouse-design.md` + commit
4. ⏳ Hand over to `writing-plans` skill for implementation plan
5. ⏳ Build: WP plugin + `PF_AH_Sync` in the framework

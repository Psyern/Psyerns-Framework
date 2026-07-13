# Phase 2+3 — Node Web Service Modernization Report

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework
**Date:** 2026-07-10 · **Agent:** Node-Webservice-Merge (Phase 2+3)
**Source (read-only):** `DeamonForge\DayZ-UniveralApi-1.3.2\DayZWebService\` (34 files, DesktopManager ignored)
**Target (written):** `Psyerns_Framework\Psyerns_Framework\DME_Api_WebService\`
**Status:** Code + deps modernized in place. **No `npm install` run, no lockfile generated, no deployment.**
Every modified `.js` file syntax-checked with `node --check` (Node v24) — all pass.

---

## 0. Summary

The WebService was copied to `DME_Api_WebService/` and modernized in place. The HTTP
**wire contract is untouched** (see §5). Work covers: dependency uplift, discord.js
13→14 (~30 callsites), mongodb 4→6 with a single shared pooled client (31 per-request
clients eliminated), jsonwebtoken 8→9 with the dead-callback auth bug fixed, node-fetch
→ native fetch, express-rate-limit 6→7, gamedig 4→5, greenlock removal, TensorFlow behind
a feature flag, and the 5 mandatory security fixes.

---

## a) Dependency matrix implemented (old → new)

`package.json` rewritten. `name` → `dme-api-service`, `version` stays **`1.3.2`**
(version handshake), `license` → `AGPL-3.0-only`, added `engines.node >= 22`.

| Package | Old | New | Notes |
|---|---|---|---|
| discord.js | ^13.7.0 | **^14.16.0** | full v14 rewrite, §b |
| mongodb | ^4.6.0 | **^6.12.0** | shared client, §c |
| jsonwebtoken | ^8.5.1 | **^9.0.2** | sync verify + `algorithms` pin, §d/S4 |
| express | ^4.18.1 | **^4.21.2** | drop-in |
| express-rate-limit | ^6.4.0 | **^7.4.0** | `onLimitReached`→`handler`, headers explicit |
| gamedig | ^4.0.4 | **^5.1.4** | `GameDig.query` |
| ejs | ^3.1.8 | **^3.1.10** | CVE-2024-33883 |
| ejs-lint | ^1.2.1 | **^1.2.2** | **kept as runtime dep** (required at runtime in discordConnector.js, not dev-only) |
| log4js | ^6.3.0 | **^6.9.1** | minor |
| node-wit | ^6.2.1 | **^6.6.0** | minor |
| @tensorflow/tfjs | ^3.11.0 | **^4.22.0** | moved to `optionalDependencies` (feature-flagged) |
| @tensorflow-models/toxicity | ^1.2.2 | ^1.2.2 | moved to `optionalDependencies` |
| nodemon (dev) | ^2.0.14 | **^3.1.9** | dev |
| **Removed** | | | |
| @tensorflow/tfjs-core, tfjs-converter | ^3.11.0 | — | pulled transitively by tfjs |
| body-parser | ^1.20.0 | — | replaced by built-in `express.json()` |
| crypto (npm stub) | ^1.0.1 | — | shadowed Node builtin — removed |
| https (npm stub) | ^1.0.0 | — | removed |
| greenlock-express | ^4.0.3 | — | TLS moved to reverse proxy, §8 |
| node-fetch | ^2.6.7 | — | native global `fetch` (Node ≥18) |
| saslprep | ^1.0.3 | — | mongodb 6 bundles its own |
| websocket | ^1.0.34 | — | not `require`d anywhere (verified) |
| pkg (dev) | ^5.5.2 | — | archived; plain `node app.js` run (the `pkg` block + `bin` field removed; `bin.js` kept as a harmless stub) |

Version choice note: caret floors are set conservatively (e.g. `^14.16.0`, `^6.12.0`) so
`npm install` resolves to the latest in‑major that exists at install time — matching the
audit's intended targets (14.26.x / 6.18.x) without risking an install failure on a
pinned version that may not yet be published.

## b) discord.js 13 → 14 — all callsites (discordConnector.js)

All done. 8 breaking categories:

1. **Intents** — `new Intents()`/`Intents.FLAGS.*` → `GatewayIntentBits` array (same 8 intents; no new privileged intents added, so existing bot logins keep working).
2. **Events** — `'ready'`→`Events.ClientReady`; `'disconnect'` (removed in v14)→`Events.ShardDisconnect`; `'error'`/`'warn'`→`Events.Error`/`Events.Warn`.
3. **Permissions** (3 sites: Move, Channel/Send, Channel/Messages) — string names → `PermissionFlagsBits.ViewChannel/Connect/SendMessages/ReadMessageHistory`.
4. **Voice casing** — all `player.voice.channelID` → `channelId` (10 occurrences).
5. **Channel create** — `create(Name, options)` → `create({ name, type, ... })` via `BuildCreateOptions()`, which maps the wire `type` string (default `"text"`) to `ChannelType` and converts `permissionOverwrites`.
6. **Embeds** — `{ embed: RawData }` → `{ embeds: [RawData] }`; embed JSON **field names passed through unchanged** (wire-critical).
7. **Collection.array()** — `(...).array()` → `[...(...).values()]` (this endpoint was already broken on v13).
8. **Message casing / errors** — `reference.messageID`→`messageId`; brittle `DiscordAPIError: Cannot send messages...` string compare → `e.code === 50007`.
   Also fixed: `channel.edit(options, reason)` → v14 single-object `edit({ ...options, reason })` via `BuildEditOptions()`.

**Permission-name interop:** the wire sends v13 SCREAMING_SNAKE names (`DSPerms`, e.g.
`VIEW_CHANNEL`). Added a converter (SCREAMING_SNAKE → PascalCase → `PermissionFlagsBits`)
with aliases for the 3 that don't map 1:1: `UseVad→UseVAD`, `SendTtsMessages→SendTTSMessages`,
`ManageEmojis→ManageGuildExpressions`. So the client keeps sending the old names unchanged.
`player._roles` left as-is (still populated in v14; returns the API role array).

**Open (needs live bot):** the embed passthrough and channel-create option/permission
mapping must be smoke-tested against a real bot + guild (privileged intents enabled).

## c) mongodb 4 → 6 — shared pooled client confirmed

- New `db.js` exports one **shared, lazily-connected, pooled** `MongoClient` per process (`getDb()`).
- **All 31 per-request `new MongoClient(...)` instances across 12 files were removed** and replaced by `await getDb()`; every per-request `client.close()` removed (closing the shared pool would break subsequent requests). `finally { … return X }` idioms preserved where load-bearing (GetDiscordObj / GetGUIDFromDiscordId).
- **All `useUnifiedTopology` removed** (no-op in the v4+ driver).
- Dead `result.ops[0]` read removed (Object.js).
- No callback→Promise migration was needed — the DB layer was already `async/await`. `InsertOne`/`updateOne` result shapes (`acknowledged`, `matchedCount`, `upsertedCount`, `insertedId`, `insertedCount`) are unchanged in v6. Verified no `new MongoClient`/`useUnifiedTopology`/`mongo.close` references remain except the single shared client in `db.js`.

## d) Security findings — status

| # | Finding | Status |
|---|---|---|
| **S1** | TLS private key shipped in `defaultkeys.json` | **FIXED.** Key + cert stripped (empty placeholders + `_README`). `app.js` no longer requires the file. No embedded-key HTTPS fallback: serves plain HTTP behind a proxy, or direct HTTPS only from real `Certificate`/`CertificateKey` files on disk. |
| **S2** | JWT signing secret via `Math.random`, and = ServerAuth bearer | **FIXED (CSPRNG)** + **PARTIAL decouple.** `utils.makeAuthToken` (first-run `ServerAuth`) now uses `crypto.randomBytes`. Optional decoupling added: `GetSigningAuth()` uses a dedicated `JwtSecret` config key if set, else falls back to `ServerAuth`. **Residual risk:** default still signs with `ServerAuth` (the audit's own migration guidance §2.4.5 keeps this so rolling updates don't invalidate live tokens). Operators can set `JwtSecret` to fully decouple with zero game-side change. |
| **S3** | NoSQL / operator injection | **FIXED.** (a) `/Query` + `/Query/Update`: parsed client queries run through `SanitizeQuery()` which strips `$where`/`$function`/`$accumulator` (server-side JS / DoS) while preserving normal operators ($gt/$in/$exists/$regex/$expr) so query semantics are intact. (b) All string-concatenated update/insert docs (player.js ×5, Object.js, globals.js transaction, Transaction.js ×2, Query.js) replaced with programmatic docs: computed field-path keys + native-typed values via `CoerceUpdateValue()` (reproduces the original type coercion) — no more building JSON strings from `mod`/`element`/`GUID`/`Value`. |
| **S5** | Rate-limit / whitelist trust spoofable IP headers | **FIXED.** `app.set('trust proxy', config.TrustProxy)` (default `false`). All limiters + the log ClientId now use `GetClientIP(req)` = `req.ip` (honours trust proxy) instead of raw `CF-Connecting-IP`/`x-forwarded-for`. express-rate-limit centralized in one v7 factory (`GenerateLimiter`); `onLimitReached`→`handler`; `standardHeaders`/`legacyHeaders` set explicitly. |
| **SSRF /Forward** | arbitrary client URL fetch | **FIXED.** `IsForwardUrlAllowed()`: if `ForwardAllowList` (new key) is non-empty, host must match (exact/suffix); otherwise loopback/private/link-local/CGNAT/metadata (169.254.169.254) targets are blocked; only http/https allowed. **Residual:** no DNS resolution, so a public hostname that resolves to an internal IP (DNS-rebinding) is not caught — use `ForwardAllowList` for strict control. |

Other (documented, not in the mandatory 5): debug=2 secret logging left as-is (operator-only
troubleshooting flag; unchanged behavior); `AllowClientWrite` broad-write behavior unchanged.

**Bonus crash fix (conscious, not silent):** `globals.js runUpdate` upstream referenced an
undefined `CheckPlayerAuth`/`GUID`. The wire audit assumed "Express→500", but this async
handler is **not awaited**, so on Node 22 the ReferenceError is an *unhandled rejection* that
crashes the worker (DoS). The documented contract is "Globals/Update is server-auth only" —
now enforced explicitly (`if (CheckServerAuth(auth))`): server path unchanged, non-server auth
gets a clean 401 instead of crashing.

## e) Interop — untouched (confirmed)

- **Routes/mounts:** all unchanged, incl. the intentional typo alias `/Gobals`, `/QnA`↔`/QnAMaker`, and the `Reqested bad URL` 501 fallback text. (Also fixed a latent cross-platform bug: `app.js` required `./serverQuery` but the file is `serverquery.js` — case-insensitive on Windows, would fail on Linux. Now `./serverquery`.)
- **Auth transport:** `auth-key` header + `ExtractAuthKey` Content-Type hack unchanged. JWT `expiresIn: 2800` unchanged. `ServerAuth` string-or-array + array[0]=signing-key unchanged.
- **MongoDB:** collections `Players`/`Objects`/`Globals`/`Logs`/`QnAMaker`, DB default `DayZ`, field names (`GUID`/`AUTH`/`Mod`/`Data`/`ObjectId`/`Public`), the `/Status` write to `Globals` `Mod:"UniversalApiStatus"`, and `NormalizeToGUID`/`RemoveBadProperties` — all unchanged.
- **config.json keys:** all preserved incl. the `LetsEncypt` typo. New keys are **additive only** (`JwtSecret`, `TrustProxy`, `ForwardAllowList`, `Toxicity.Enabled`).
- **Version string:** `package.json` version stays `1.3.2`; `global.APIVERSION`/`/Status` `Version` field unchanged. Response field names (`Status`, `Error`, `Version`, lowercase `answer`/`score`, …) unchanged.

## f) Open points / residual risk

1. **`npm install` is required** before running (deps changed heavily; no `node_modules`, no lockfile committed). For Toxicity: `npm install --include=optional`.
2. **Live verification needed** (as the audit anticipated): discord.js 14 bot flows (roles/voice/DM/channel create+edit/messages/embeds) against a real guild; gamedig 5 `/ServerQuery` result shape (`state.raw.tags` for DayZ keywords, `requestRules`) against a live DayZ server; full EnScript-client regression across all routes + Mongo.
3. **S2 residual:** default JWT signing = `ServerAuth` (set `JwtSecret` to fully decouple).
4. **SSRF residual:** DNS-rebinding not covered without `ForwardAllowList`.
5. **TensorFlow:** off by default (`Toxicity.Enabled=false` → `/Toxicity` returns 501). Model is loaded once and cached when enabled (was reloaded every request).
6. **`CheckForNewVersion` default → false** in sample-config (the GitHub poll targets daemonforge's repo and would log false "out of date" warnings). `global.NEWVERSIONDOWNLOAD` still points at daemonforge (left as source attribution).
7. **AGPL headers** added to every modified `.js`. The WebService's own `LICENSE` file (GPL-3.0 upstream) vs. the AGPL-3.0-only relicense is a **relicense-phase handoff** (Baseline §4), not touched here.
8. **cluster:** kept. With `cpuCount>1`, each worker gets its own pooled client (correct); the master still runs `CheckIndexes` once.

## g) Files affected

**36 files** in `DME_Api_WebService/` after the copy:
- **2 new:** `db.js` (shared Mongo client), `README.md` (reverse-proxy TLS + install + config keys).
- **24 JS modified:** app, utils, AuthChecker, Auth, configLoader, player, Object, globals, Query, Transaction, logger, Status, serverquery, apiFowarder, toxicityConnector, QnAMaker, witConnector, luisConnector, TranslateConnector, crypto, TrueRandom, discordConnector, log, bin.
- **3 JSON modified:** package.json, sample-config.json, defaultkeys.json (key stripped).
- **7 unchanged:** templates/{discordError,discordLogin,discordSuccess}.ejs, templates/defaultTemplates.json, public/favicon.ico, .gitignore, LICENSE.

---

*Generated by the Node-Webservice-Merge agent (Phase 2+3), 2026-07-10. No `npm install`, no deployment performed.*

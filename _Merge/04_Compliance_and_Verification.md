# Phase 4 — Verification & AGPL Compliance Gate

Independent orchestrator verification of the UniversalApi → Psyerns_Framework merge
(2026-07-10). Not the sub-agents' self-reports — every item below was re-checked
against the actual files in `Psyerns_Framework/`.

## Gate result: PASS (with documented residuals for the human checkpoint)

| # | Check | Result |
|---|---|---|
| 1 | `LICENSE` = AGPL-3.0 | ✅ AGPL-3.0-**only** full text (678 lines); MIT preserved as `LICENSE.MIT.bak`. `NOTICE` present. |
| 1 | daemonforge copyright retained in incorporated files | ✅ 63 EnScript files + modified `.js`/`.ejs`/`.php` carry the daemonforge attribution + AGPL §5 change note. |
| 1 | README states origin + AGPL + §13 | ✅ License & Attribution section added; AGPL badge. |
| 2 | No bundled AGPL-incompatible component | ✅ Only GPL fund is `psyerns-mods` = `GPL-2.0+` (or-later → GPLv3 → compatible). WebService `package.json` license = `AGPL-3.0-only`. No stray GPL-3.0 full-text LICENSE remains (the daemonforge WebService LICENSE was reconciled to a component notice). |
| 2 | GPLv2-only anywhere | ✅ None. |
| 2 | WP-plugin carve-outs | ✅ 3 explicit `LICENSE` files (Leaderboard MIT, AuctionHouse MIT, psyerns-mods GPL-2.0-or-later). |
| 2 | Binary-without-source excluded | ✅ `UApiMass.pbo` not incorporated (AGPL §6); documented in NOTICE. |
| 3 | No DME_Api class collides with DME-WAR `DME_` classes | ✅ 100 `DME_Api_` classes vs 321 DME-WAR `DME_` classes → **0 collisions** (sub-namespace verified). |
| 3 | No duplicate class definition in the merged tree | ✅ 158 non-modded class defs across PF_ + DME_Api → **all unique** (no compile-killer). |
| 4 | Rename completeness (EnScript) | ✅ 0 stray `UApi`/`UniversalApi` symbols in live EnScript code; remaining hits are intentional interop strings (`UniversalApiStatus`, routes) and comments. |
| 5 | EnScript forbidden-list sweep | ✅ 0 real violations across 63 `.c` (GetGame/IsServer/IsClient/autoptr/delete/multi-decl/ref-param). Every grep hit verified to be inside `/* */` doc/obsolete blocks. Brace balance 63/63. |
| 6 | Node/PHP sanity | ✅ Node `node --check` sample 6/6 (agent: 25/25); PHP `php -l` clean on all touched (agent: 104/104). `npm install` NOT run (see residuals). |
| — | config.cpp loadable | ✅ `engineScriptModule` (Common + 1_Core) added, Common in every module — per original `_UAPIBase` pattern. Braces 17/17. Runtime `.edds` (info/Bot/warning) placed in `images/`. |

## What was built

- **EnScript:** 63 `.c` in `scripts/{1_Core,Common,3_Game,4_World,5_Mission}/DME_Api/`, DayZ 1.29-modernized, `DME_Api_` namespace, 9 syntax defects + 10 pre-existing bugs fixed. ScriptExecutor (RCE) and UApiMass.pbo excluded.
- **Node:** `DME_Api_WebService/` (25 `.js` + `db.js` + README), discord.js 14, mongodb 6 shared client, JWT 9 + algorithm pin, node-fetch→native fetch, greenlock removed, TensorFlow optional. 5 security findings fixed (2 documented residuals). Wire contract untouched.
- **PHP/Web:** 3 PHP-8 security fixes (path-traversal, hash_equals, deploy-guard), EJS PII exfil to `api.daemonforge.dev` removed, water.css inlined, §13 source footer, 3 WP carve-out LICENSEs.
- **Relicense:** LICENSE→AGPL-3.0-only (SPDX-consistent across the tree), NOTICE, README.

## Residuals — for HITL Checkpoint 3 (before public release)

1. **`npm install` + lockfile** must be run once (`--include=optional` for the Toxicity feature); no lockfile committed. Then `npm audit`.
2. **Live verification** not performed here: DayZ server script compile (full mod set), discord.js-14 bot flows, gamedig-5 result shape, EnScript token round-trip, config auto-migration, EntityStore save/load.
3. **Security residuals (documented, not blocking merge but note before publish):** default JWT still signs with ServerAuth secret (kept per rolling-update guidance — decouple via `JwtSecret` when convenient); `/Forward` DNS-rebinding not covered by the allowlist.
4. **§13 source link** in the EJS footer currently points at `deadmans-echo.de`; replace with the canonical public source-repo URL at publish. Unify plugin-header domains (`deadmansecho.com` vs `deadmans-echo.de`).
5. **favicon.ico** (daemonforge) not replaced — supply a Psyerns asset.
6. **MapLink is NOT part of this workflow.** DME_Map-Link depends on daemonforge's *UniversalFramework* (`UF()`), a different product than the incorporated UniversalApi. Re-targeting MapLink onto the merged framework is a separate, behavior-affecting architecture decision (incl. the missing `Settings().Register` in UApi 1.3.2) — do not treat it as a mechanical rename.
7. **No git commit/push performed** (per workflow). HITL Checkpoint 3 sign-off required before release.

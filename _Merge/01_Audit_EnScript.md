# Phase 1 — Audit 1A: EnScript (UniversalApi → Psyerns_Framework)

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework (AGPL-Relicense per Phase 0)
**Agent:** Audit-Agent 1A (EnScript) · **Datum:** 2026-07-10 · **Modus:** nur Analyse, keine Code-Änderungen
**Quelle:** `C:\Users\Administrator\Desktop\Psyerns_Framework\DeamonForge\DayZ-UniveralApi-1.3.2\` — `_UniversalApi/scripts/` (56 .c) + `_UAPIBase/scripts/` (14 .c + 3 Nicht-Script). `DayZ-UniveralApi-stable` ignoriert (nur 2 Gegenchecks §1.11).
**Ziel:** `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\scripts\` (46 Dateien, 5.566 LOC, alle `PF_`)
**Vanilla-Referenz:** `C:\Users\Administrator\Desktop\Mod Repositories\scripts - 1.29`
**CF-Referenz:** `C:\Users\Administrator\Desktop\Mod Repositories\DayZ-CommunityFramework-production`

---

## 0. Executive Summary

| Kategorie | Anzahl | Schwere |
|---|---:|---|
| `GetGame()` (→ `g_Game` + Nullcheck) | **135** (Zeilen, davon 2 in Kommentaren) | Pflicht-Migration |
| `IsServer()`/`IsClient()` (→ `IsDedicatedServer()`-Logik) | **28 Zeilen / 30 Vorkommen** | Pflicht-Migration |
| `delete`-Statements | **5** (1 Datei) | Pflicht-Fix |
| Mehrfach-Deklarationen (`int a, b, c;`) | **11** | Pflicht-Fix |
| `ref` auf Parametern | **39 Signaturen** | Pflicht-Fix |
| `autoptr` gesamt (Locals/Params/Members gemischt) | **194** | Policy-Fix (§1.6) |
| Fehlendes `override` | **3** (PluginAdminLog) | Pflicht-Fix |
| super-Reihenfolge/-Gating abweichend | **3 Muster** | prüfen/dokumentieren |
| Member-Shadowing / `m_`-Locals | **4** | Pflicht-Fix (1 funktional relevant) |
| **Syntax-Defekte (Compile-Risiko)** | **7** (+2 kosmetisch `;;`) | **MUSS vor erstem Compile** |
| Ternary-Operatoren | **0** | ✅ |
| RPC-IDs < 10000 | **0** (einzige native ID: 155494166 ✅) | ✅ (Konstante einführen) |
| Vanilla-APIs, die in 1.29 fehlen | **0** (46 Symbole verifiziert) | ✅ |
| Pre-existing Logik-Bugs (Bonus-Funde) | **12** | §6 |

Das **Ziel-Framework ist zu 100 % regelkonform** (0× GetGame, 0× IsServer/IsClient, 0× delete, durchgängig `g_Game` + `IsDedicatedServer()`): Der gesamte Modernisierungs-Aufwand liegt auf der Quell-Seite.

Phase-0-Zahlen verifiziert: **135× GetGame() exakt bestätigt**; „28× IsServer/IsClient" = 28 Zeilen (30 Vorkommen, 2 Zeilen mit Doppel-Aufruf in `_UAPIBase/Logger.c:70,94`).

---

## 1. Regelverstöße (Datei/Zeile-genau)

Pfade relativ zu `DayZ-UniveralApi-1.3.2\`. **U** = `_UniversalApi/scripts`, **B** = `_UAPIBase/scripts`.

### 1.1 `GetGame()` — 135 Zeilen

Fix-Schema überall: `GetGame()` → `g_Game`, am Methodenanfang `if (!g_Game) return;` (bzw. return-Default), bei `GetGame() && …`-Ketten entfällt der Extra-Check.

| Datei | Zeilen | Anz. | Anmerkung |
|---|---|---:|---|
| U/3_Game/UApi/CallBacks/CheckIfDiscord.c | 12, 13 | 2 | 12 zugleich IsClient (§1.2) |
| U/3_Game/UApi/CallBacks/ObjectCallBack.c | 8, 43, 45, 62, 98, 107, 113 | 7 | alle `GetGame().GameScript.CallFunctionParams` |
| U/3_Game/UApi/CallBacks/UApiDBCallBack.c | 30, 40, 54 | 3 | dito |
| U/3_Game/UApi/CallBacks/UApiStatusCallBack.c | 6, 12, 28, 32, 35 | 5 | dito; zusätzl. `autoptr` in Param4-Template (§1.6) |
| U/3_Game/UApi/ConfigLoader.c | 14, 44, 62 | 3 | alle IsServer-Gates (§1.2) |
| U/3_Game/UApi/DiscordRest.c | 15 | 1 | IsClient |
| U/3_Game/UApi/Endpoints/UApiDSEndpoint.c | 11 | 1 | IsClient |
| U/3_Game/UApi/Objects/ConfigBase.c | 40 | 1 | in Kommentar-Template (trotzdem anpassen — Copy-Paste-Quelle für Drittmods) |
| U/3_Game/UApi/QnAMakerConfig.c | 10, 34, 74 | 3 | 34 = `GetGame().Chat(...)` |
| U/3_Game/UApi/UniversalApi.c | 151, 153, 171, 172, 228, 230, 259, 260, 279, 292, 301 (2×), 308, 309, 347, 349, 406, 410 | 17 | 259 bereits mit `GetGame()`-Nullcheck — wird `if (g_Game)` |
| U/3_Game/UApi/Utilities.c | 11 (3×), 12, 13 (3×), 56, 58, 76, 77, 81, 83, 246–329 (Config-Getter-Block: 246, 247, 250, 251, 254, 255, 262, 263, 266, 267, 270, 271, 278, 279, 281, 282, 284, 285, 290, 291, 294, 295, 298, 299, 305, 306, 309, 310, 313, 314, 320, 321, 324, 325, 328, 329) | 45 | 81 nutzt bereits `IsDedicatedServer()` ✅ |
| U/4_World/UApi/Classes/PluginAdminLog.c | 260 | 1 | `GetPlayers` |
| U/4_World/UApi/Entities/PlayerBase.c | 127, 133, 190, 196 | 4 | `ObjectDelete` |
| U/4_World/UApi/Entities/Weapon_Base.c | 50, 54, 55, 56, 66, 82, 86, 96 (2×) — 23, 120 nur Kommentar | 10 | RemoteObjectTree*-Resync |
| U/4_World/UApi/ItemStore.c | 120, 134, 146, 227, 242 | 5 | `CreateObject`, CallQueue |
| U/5_Mission/UApi/MissionGameplay.c | 10, 15, 26, 30 | 4 | CallQueue + IsServer |
| U/5_Mission/UApi/MissionServer.c | 6 | 1 | CallQueue im Konstruktor |
| B/3_Game/UApi/Logger.c | 70 (2×), 94 (2×), 167 | 3 | 167 = `AdminLog` |
| B/3_Game/UApi/ScriptExecutor.c | 27, 48, 66, 83, 98, 115, 134, 148, 195, 213 | 10 | `GetMission().MissionScript` / `GameScript` |
| B/4_World/UApi/PlayerBase.c | 109, 136, 137, 139, 140, 168, 169, 174, 175 | 9 | Config-Getter + `CreateObjectEx` |
| **Summe** | | **135** | |

### 1.2 `IsServer()` / `IsClient()` — 28 Zeilen / 30 Vorkommen

Semantik-Klassifizierung mit Fix je Fall (Regel: Authority = `g_Game.IsDedicatedServer()`, Client-Seite = `!g_Game.IsDedicatedServer()`):

| Datei:Zeile | Ist | Semantik | Fix |
|---|---|---|---|
| U/CallBacks/CheckIfDiscord.c:12 | `GetGame().IsClient()` | Client-only (OpenURL) | `!g_Game.IsDedicatedServer()` |
| U/ConfigLoader.c:14, 44, 62 | `GetGame().IsServer()` | Authority (Config-Datei nur Server) | `g_Game.IsDedicatedServer()` |
| U/DayZGame.c:6, 15, 20, 48 | nacktes `IsClient()` (CGame-Methode) | Client-only (Discord-Cache/URL) | `!IsDedicatedServer()` (innerhalb modded DayZGame) |
| U/DiscordRest.c:15 | `GetGame().IsClient()` | Client-only (SteamId-Link) | `!g_Game.IsDedicatedServer()` |
| U/Endpoints/UApiDSEndpoint.c:11 | `GetGame().IsClient()` | dito | dito |
| U/Objects/ConfigBase.c:40 | `GetGame().IsServer()` (Kommentar) | Authority-Template | Template-Text mitziehen |
| U/QnAMakerConfig.c:10, 74 | `GetGame().IsServer()` | Authority (Config laden) | `g_Game.IsDedicatedServer()` |
| U/UniversalApi.c:151 | `GetGame().IsServer()` | Authority (FindPlayer) | `g_Game.IsDedicatedServer()` |
| U/UniversalApi.c:228, 230 | `!IsServer()` / `IsServer()` | Token-Quelle Client vs. Server | `!g_Game.IsDedicatedServer()` / `g_Game.IsDedicatedServer()` — **Achtung:** Klasse hat bereits `m_IsServer` (via `#ifdef NO_GUI`), konsolidieren auf EINE Quelle |
| U/UniversalApi.c:308 | `GetGame().IsClient()` | Client-Retry | `!g_Game.IsDedicatedServer()` |
| U/UniversalApi.c:347, 349 | `!IsServer()` / `IsServer()` | RPC-Richtungs-Weiche | `IsDedicatedServer()`-Logik; zusätzlich `CallType` prüfen (CF-Konvention, fehlt komplett!) |
| U/Utilities.c:13 | `GetGame().IsClient()` | Client-SteamId | `!g_Game.IsDedicatedServer()` |
| U/Utilities.c:56 | `GetGame().IsServer()` | Authority (FindPlayer) | `g_Game.IsDedicatedServer()` |
| U/Utilities.c:83 | `GetGame().IsClient()` | Notification-Weiche (81 nutzt schon `IsDedicatedServer()`) | `else`-Zweig reicht |
| U/Weapon_Base.c:82, 86 | `GetGame().IsClient()` | RPC-Empfängerseite | `!g_Game.IsDedicatedServer()` |
| U/Weapon_Base.c:96 | `!IsMultiplayer() \|\| IsServer()` | „nur MP-Client" | `if (g_Game.IsDedicatedServer() \|\| !g_Game.IsMultiplayer()) return;` |
| U/MissionGameplay.c:26 | `!GetGame().IsServer()` | Client-Token-Renewal | `!g_Game.IsDedicatedServer()` |
| B/Logger.c:70, 94 | `!IsServer() \|\| IsClient()` (je 2×) | „alles außer Dedicated-Server raus" | exakt `if (!g_Game \|\| !g_Game.IsDedicatedServer()) return;` |

> **Verhaltens-Hinweis:** In Singleplayer/Offline-Mission ist `IsServer()==true && IsClient()==true`; `IsDedicatedServer()==false`. Die Umstellung ändert SP-Verhalten an den „Authority"-Stellen (Config lädt in SP nicht mehr). Für einen Server-API-Mod korrekt/egal — bewusst abnehmen.

### 1.3 `delete` — 5 Vorkommen

| Datei:Zeile | Code | Fix |
|---|---|---|
| B/4_World/UApi/ItemStore.c:43–47 | `delete m_Cargo; delete m_MagAmmo; delete m_ChamberedRound; delete m_FireModes; delete m_MetaData;` | Destruktor `~UApiEntityStore()` komplett entfernen — alle 5 Member sind `autoptr`/werden `ref`, GC räumt selbst |

### 1.4 Mehrfach-Deklarationen — 11

| Datei:Zeile | Code | Fix |
|---|---|---|
| U/Utilities.c:147, 198, 213 | `int yr, mth, day;` | je 3 Einzeldeklarationen |
| U/Utilities.c:162, 229, 237 | `int hr, min, sec;` | dito |
| B/Logger.c:108 | `int yr, mth, day;` | dito |
| B/Logger.c:124, 144 | `int hr, min, sec;` | dito |
| B/ScriptExecutor.c:177 | `int year, month, day;` | dito |
| B/ScriptExecutor.c:178 | `int hour, minute, second;` | dito |

(Vorbild im Ziel: `PF_Logger.WriteToFile` deklariert alle 6 einzeln — Muster 1:1 übernehmen.)

### 1.5 `ref` auf Parametern — 39 Signaturen

| Datei | Zeilen | Muster | Fix |
|---|---|---|---|
| U/UniversalRest.c | 14, 24, 43, 56, 66, 80, 98, 116, 133, 148, 165, 184, 203, 223, 238, 249, 264, 283, 301, 318 (2× in Signatur), 333, 349 | `ref RestCallback UCBX = NULL` (+ 318 zusätzl. `ref UApiForwarder data`) | `ref` streichen — Parameter halten nie Ownership |
| U/DiscordRest.c | 21, 31, 52, 70, 90, 96, 103, 115, 126, 142 (2×), 158, 174, 190 (2×) | `ref RestCallback UCBX …`, `ref UApiChannelUpdateOptions`, `ref UApiDiscordChannelFilter` | `ref` streichen |
| U/UniversalApi.c | 284, 307, 343, 354 | `ref Object target` (CF-RPC-Handler) | `ref` streichen — Ziel-eigene Handler (`PF_ReloadRequest`) laufen ohne `ref`, CF verlangt es nicht |

**Wichtig (Lifetime):** Nach Streichen von `ref` müssen die Engine-Aufrufe die Callback-Lebensdauer sichern. `ctx.POST(cb, …)` hält den `RestCallback` engine-seitig am Leben (vanilla-Beispiel in `RestApi.c:44-46` arbeitet mit Local ohne ref) — Verhalten identisch zum Ziel (`PF_WebApiBase.Post` übergibt `new PF_RestCallback()` inline). Kein zusätzlicher Halter nötig.

### 1.6 `autoptr` — 194 Vorkommen, Policy

`autoptr` ist Alt-Syntax (heute `ref`); auf Locals/Params ist es nach Projektregel verboten. Verteilung und Fix-Policy:

| Kontext | Umfang (ca.) | Fix |
|---|---|---|
| Locals (`autoptr RestCallback DBCBX;`, `autoptr UApiTransaction …` — Endpoints, Callbacks, UniversalApi 152/382/390, Logger 226, SimpleStore …) | ~120 | `autoptr` ersatzlos streichen |
| Member (`protected autoptr ApiAuthToken m_authToken;`, alle Endpoint-/Map-/Array-Member in UniversalApi.c 193–212, UApiEntityStore 24–35, DayZGame.c:3, Currencys.c map, UApiCallbackLoader.obj, ObjectCallBack.c:156) | ~45 | → `ref` |
| Parameter (UniversalApi.c 61, 73, 93, 103: `autoptr RestCallback UCBX = NULL`, `autoptr UApiCallbackBase cb`) | 4 | ersatzlos streichen |
| Template-Argumente in `Param4<int,int,string, autoptr StatusObject>` (UApiStatusCallBack.c 6, 12, 28, 32, 35) | 5 | `autoptr` aus Template-Arg entfernen |
| Typedef `array<autoptr UCurrencyValue>` (B/Currencys.c:21) + JSON-serialisierte Arrays (`autoptr array<autoptr …>` in UApiEntityStore, QnAMakerConfig, UApiStatus …) | ~20 | → `ref`; **Feldnamen unangetastet** (Wire!) |

### 1.7 Fehlendes `override` — 3

| Datei:Zeile | Methode | Beleg vanilla 1.29 |
|---|---|---|
| U/4_World/UApi/Classes/PluginAdminLog.c:41 | `PlayerHitBy(…)` | `4_World/DayZ/Plugins/PluginBase/PluginAdminLog.c:210` |
| U/4_World/UApi/Classes/PluginAdminLog.c:147 | `OnPlacementComplete(Man, ItemBase)` | dito :337 |
| U/4_World/UApi/Classes/PluginAdminLog.c:171 | `OnContinouousAction(ActionData)` | dito :356 |

Alle drei rufen `super.` bereits auf — nur Keyword ergänzen. (PlayerKilled :3, Suicide :203, BleedingOut :226, PlayerList :249 haben `override` ✅.)

### 1.8 super-Reihenfolge / -Gating — 3 Muster

| Datei:Zeile | Befund | Einordnung |
|---|---|---|
| U/5_Mission/MissionServer.c: `OnClientPrepareEvent` | `UApi().PreparePlayerAuth(…)` **vor** `super.` | Regelverstoß „super zuerst"; funktional unkritisch → super an den Anfang ziehen |
| U/5_Mission/ChatInputMenu.c: `OnChange` | mehrere bedingte `return super.OnChange(…)`-Pfade | Interception-Muster, super-zuerst technisch unmöglich; dokumentieren, nicht „fixen" |
| U/4_World/PluginAdminLog.c (PlayerKilled :5, PlayerHitBy, OnPlacementComplete :149, …) | `super.` nur wenn `UApiConfig().EnableBuiltinLogging != 2` | **Feature** (Wert 2 = vanilla-AdminLog unterdrücken, nur API-Log) — bewusst so lassen, im Header kommentieren |

### 1.9 Shadowing / `m_`-Locals — 4

| Datei:Zeile | Befund | Fix |
|---|---|---|
| U/UniversalApi.c:390 | Local `autoptr ApiAuthToken m_authToken` **shadowt Member :193** — Server-Member bleibt dadurch immer NULL (funktioniert nur, weil `GetAuthToken()` serverseitig `ServerAuth` nimmt) | Local umbenennen (`clientToken`) |
| U/UniversalApi.c:382 | Local `m_ClientConfig` (m_-Prefix auf Local) | umbenennen (`clientConfig`) |
| U/4_World/ItemStore.c:75 | Local `int m_CurrentMuzzle` in `SaveEntity` | `currentMuzzle` |
| U/4_World/ItemStore.c (LoadEntity) | Local `int m_CurrentMuzzle` | dito |

### 1.10 Naming-Verstöße in modded-Klassen & globale Symbole

| Fundort | Symbol | Problem/Fix |
|---|---|---|
| U/1_Core/enMath.c:4 | `Math.m_QRandomNumbers` (static) | → `s_DME_Api_QRandomNumbers` (Phase-0 §2.3 nennt bereits `m_DME_Api_QRandomNumbers`; als static besser `s_`) |
| U/4_World/Mission.c | `MissionBaseWorld.m_UApi_Initialized` | → `m_DME_Api_Initialized` |
| U/3_Game/DayZGame.c:3 | `DayZGame.m_discordUser` | → `m_DME_Api_DiscordUser` |
| B/4_World/PlayerBase.c:3 | Methode `GetQuickBarEntityIndex(EntityAI)` in modded PlayerBase — unprefixt, **dupliziert vanilla `FindQuickBarEntityIndex`** (PlayerBase.c:2203, 1.29) | Methode löschen, Aufrufer (U/4_World/ItemStore.c:36) auf vanilla `FindQuickBarEntityIndex` umstellen |
| B/3_Game/Logger.c:1–4 | globale `LOG_ERROR/LOG_VERBOSE/LOG_INFO/LOG_DEBUG` (file-scope, unprefixt) | Kollisionsrisiko mit Drittmods → `DME_API_LOG_*` (nicht serialisiert, gefahrlos) |
| U/1_Core/Constants.c | `PLAYER_DB`/`OBJECT_DB` unprefixt | Rename in Phase 0 §Block C bereits geplant ✅ |
| U/QnAMakerConfig.c:69 | globale Variable `m_QnAMakerServerAnswers` (file-scope, m_-Prefix) | → `g_DME_Api_QnAAnswers` o. ä. |
| U/ConfigLoader.c:57 | globale `m_UniversalApiConfig` | Phase 0 Block C: → `m_DME_Api_Config` (Empfehlung hier: `g_DME_Api_Config`) |
| diverse | Member ohne Prefix in eigenen (nicht-modded) Klassen sind JSON-Wire (`Status`, `GUID`, `AUTH`, …) | **NICHT umbenennen** (Phase 0 §2.8) |

### 1.11 Syntax-Defekte — 7 harte + 2 kosmetische (MUSS-Fix vor erstem Compile)

Diese Stellen weichen von gültiger EnScript-Syntax ab. Sie existieren **identisch im `stable`-Zweig** (2 Stichproben verifiziert) — d. h. entweder toleriert der Enforce-Parser sie stillschweigend, oder die released PBOs stammen von abweichendem Quellstand. Für den Merge gilt: alle 7 normalisieren, keine Diskussion mit dem Compiler riskieren.

| # | Datei:Zeile | Code | Fix |
|---|---|---|---|
| 1 | U/Endpoints/UApiDBEndpoint.c:181 | `autoptr RestCallback DBCBX = ;` | Zeile ersatzlos löschen (Variable in dieser Funktion nie benutzt; Post() baut Callback inline) |
| 2 | U/Endpoints/UApiDBEndpoint.c:199 | dito | dito |
| 3 | B/4_World/PlayerBase.c:140 | `return GetGame().ConfigGetInt( CFG_VEHICLESPATH + " " + Type + " varQuantityMax" ) );` | überzählige `)` entfernen |
| 4 | U/4_World/ItemStore.c:66 | `{ m_MagAmmo = new array<autoptr UApiAmmoData>}` | `;` ergänzen (+ autoptr→ref) |
| 5 | U/4_World/ItemStore.c:85 | dito | dito |
| 6 | B/4_World/ItemStore.c:270 | `{m_HealthZones = new array<autoptr UApiZoneHealthData>}` | dito |
| 7 | U/Objects/UApiServerStatus.c:3 | `int GamePort` (Semikolon fehlt) | `;` ergänzen — **Membername `GamePort` bleibt (Wire!)** |
| k1 | U/Objects/SimpleLogObject.c:118 | `…JsonMakeData(this);;` | ein `;` entfernen |
| k2 | U/4_World/PlayerBase.c:158 | `UCurrentQuantity(item);;` | dito |

### 1.12 RPC-Inventar

| Art | Fundort | Wert | Urteil |
|---|---|---|---|
| Native `RPCSingleParam`/`OnRPC` | U/Weapon_Base.c:76, 79–92 | `155494166` | ✅ ≥ 10000; Magic Number → `const int DME_API_RPC_WEAPON_FIX = 155494166;` in Constants |
| CF-RPCs (`GetRPCManager()`) | U/UniversalApi.c:272–275 (Add), 298, 315, 350, 371, 393 (Send) | Mod-Key `"UAPI"`, Handler `RPCUniversalApiConfig`, `RPCRequestQnAConfig`, `RPCRequestAuthToken`, `RPCRequestRetry` | String-basiert (kein ID-Risiko); Rename per Phase 0 Block D. **`SingeplayerExecutionType.Both`** (Typo-Enum) → CF-production definiert beide Schreibweisen (`RPCManager.c:1` und `:8`) — auf `SingleplayerExecutionType` normalisieren (Ziel nutzt die korrekte) |
| Handler-Hygiene | U/UniversalApi.c:284 ff. | Kein `CallType`-Check in allen 4 Handlern | CF-Konvention ergänzen (`if (type != CallType.Server) return;` etc. wie `PF_ReloadRequest`) |

Mehrzeilige Funktionsaufrufe: in den gelesenen Dateien **keine** gefunden (daemonforge schreibt einzeilig); beim Compile gegenprüfen.

---

## 2. Veraltete/riskante Engine-APIs

### 2.1 RestApi — **existiert vollständig in 1.29** ✅

Beleg: `Mod Repositories\scripts - 1.29\3_Game\DayZ\Http\RestApi.c` — `class RestCallback : Managed` (Z. 50, Events `OnError/OnTimeout/OnSuccess/OnFileCreated`), `class RestContext` (Z. 95, `GET/GET_now/FILE/FILE_now/POST/POST_now/reset/SetHeader`), `class RestApi` (Z. 147, `GetRestContext/GetContextCount/EnableDebug/DebugList/SetOption`), globale `CreateRestApi()/DestroyRestApi()/GetRestApi()` (Z. 181–183), Enums `ERestResultState`/`ERestOption`. **Jedes von UApi benutzte Symbol ist vorhanden**; keine Signatur-Drift gefunden. Auch alle 46 geprüften Gameplay-APIs (ValidateAndRepair, PushCartridgeToChamber/-InternalMagazine, SetStepZeroing, SetZoom, SetJammed, Get/SetFrequencyByIndex, Get/SetBloodTypeVisible, m_DecayTimer, m_LastDecayStage, ChangeFoodStage, m_FuelAmmount/m_CoolantAmmount/…, SetQuickBarEntityShortcut, FindQuickBarEntityIndex, ServerSetAmmoCount, SetCartridgeAtIndex, DamageZoneMap/GetDamageZoneMap, AfterStoreLoad, GetPersistentID, AdminLog, GetPlayerNetworkIDByIdentityID, GetTitleInitiator, OnClientPrepareEvent, ScriptModule.LoadScript, …) existieren in 1.29.

Randnotiz 1.29: `SetQuickBarEntityShortcut(entity, index, force = false)` hat einen neuen optionalen Param — Aufrufe bleiben kompatibel.

### 2.2 Blockierende `*_now`-Aufrufe — Freeze-Risiko ⚠

| Fundort | Aufruf |
|---|---|
| U/DiscordRest.c:40–44 (`PostNow`) → benutzt von :224 (`GetUserNow`-Pfad) und :257 | `ctx.POST_now("", jsonString)` — laut vanilla-Doku „thread blocking operation!" |

Empfehlung: beim Merge die beiden synchronen Discord-Getter entweder streichen oder deutlich als blocking markieren; Ziel-Framework hat bewusst **keinen** blocking Call.

### 2.3 JSON-Schicht

| API | Nutzung Quelle | Risiko/Urteil |
|---|---|---|
| `JsonFileLoader<T>` | ~20 Stellen: Config-Dateien (ConfigLoader :16/:51, QnAMakerConfig :12/:29) **und** Wire-Serialisierung (`JsonMakeData`/`JsonLoadData` in Objects/*, ItemStore, Utilities:2, ConfigBase:96) | vanilla-vorhanden; bekanntes hartes Fehlerverhalten (wirft/loggt bei Schema-Drift). Für Configs ok (Ziel macht es genauso, `PF_WebConfig.Load`) |
| `JsonSerializer` direkt | UApiAuthCallBack:28, UApiDiscordCallBack:15, UApiStatusCallBack:21, DiscordRest:227/:259 | fehleranfällig dupliziert — auf `UApiJSONHandler` (→ `DME_Api_JSONHandler`) konsolidieren |
| `UApiJSONHandler<T>` (B/JsonHandler.c) | zentraler Wrapper mit Fehlerpfad | **behalten** — bester Kandidat als einzige Wire-JSON-Schicht |

### 2.4 CallQueue-Muster — 16 Stellen, kein Nullcheck

`GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM)` ohne Absicherung (UniversalApi.c 260/279/292/309/406/410, Weapon_Base 50/56, ItemStore 227/242, MissionGameplay 10/15/30, MissionServer 6, + 2 Kommentare). CLAUDE.md: CallQueue kann null sein. Fix-Muster:

```c
if (!g_Game) return;
ScriptCallQueue queue = g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM);
if (queue) queue.CallLater(this.Foo, 1000, false);
```

Zusätzlich Destruktor `~UniversalApi()` (Z. 258–262): Remove-Call im Dtor ist beim Shutdown heikel — Nullcheck zwingend (bereits `GetGame()`-geprüft, wird `g_Game`-geprüft).

### 2.5 CF-Abhängigkeit — Urteil: **behalten (nötig für RPC, sonst nichts)**

- Quelle nutzt aus CF **ausschließlich** `GetRPCManager()` (AddRPC/SendRPC) — 9 Callsites in genau 1 Datei (`UniversalApi.c`). Kein `ScriptInvoker`-Import, kein CF-Logging, kein CF-ModStorage, kein JMModule (0 Treffer).
- Ziel hängt bereits an CF: `requiredAddons {"DZ_Data","JM_CF_Scripts"}` + `GetRPCManager()` in `PF_MissionInit.c`/`PF_MissionClient.c`.
- **Ersetzbarkeit:** technisch ja (4 Handler auf native `g_Game.RPC` mit IDs ≥ 10000 umschreibbar, Aufwand ~0,5 Tag + Testrisiko), aber ohne Nutzen: CF bleibt wegen des Ziel-Frameworks ohnehin Pflicht. → **keine Ablösung in Phase 2.** Nur normalisieren: korrekt geschriebenes `SingleplayerExecutionType`, `CallType`-Checks, Mod-Key `"UAPI"` → `"DME_Api"` (Phase 0 Block D).

### 2.6 `EnableDebug(true)` in Produktion ⚠

`U/Endpoints/UApiEndpointBase.c:21`: beim Erst-Erzeugen der RestApi wird Debug-Konsolen-Output global aktiviert (gilt engine-weit für ALLE Rest-Nutzer, auch PF_!). → an Config-Flag binden (`EnableDebugLogging` des Ziels) oder entfernen. Zudem erzeugt `Api()` bei **jedem** Aufruf einen neuen Kontext + `SetHeader` (Member `m_Context` ohne `ref`; Header-Mutation pro Request) — funktioniert, aber beim Andocken an `PF_WebClient`-Kontext-Cache beachten (§3.1).

### 2.7 ScriptExecutor (B/ScriptExecutor.c) — Remote-Code-Execution-Facility 🔴

`UScriptExec.Func(string code)` schreibt beliebige Strings nach `$profile:UApi/Scripts/*.c` und führt sie via `ScriptModule.LoadScript` im Missions-/Game-Kontext aus. **Im gesamten Mod + WebService gibt es keinen einzigen Aufrufer** (einziger Treffer: Kommentar in SampleExec.c) — reine Drittmod-Komfort-API. Empfehlung: **nicht mergen** (Attack-Surface, AGPL-Service könnte sonst theoretisch Code aufs Spielfeld schieben) — oder als optionales, per Define deaktiviertes Modul (`#ifdef DME_API_ENABLE_SCRIPTEXEC`). Owner-Entscheid; Default: weglassen.

### 2.8 Sonstige Engine-Eigenheiten

| Befund | Fundort | Einordnung |
|---|---|---|
| Server-Erkennung via `#ifdef NO_GUI` | UniversalApi.c:265–268 | funktioniert, aber besser direkt `g_Game.IsDedicatedServer()` in `Init()` — dann kann `m_IsServer` bleiben (eine Quelle, §1.2) |
| String-Reflection-Dispatch `GameScript.CallFunctionParams(instance, "FuncName", …)` | alle Callbacks, UniversalApi.c:301 | vanilla-stabil; tippfehleranfällig, aber Kern des UApi-API-Designs → behalten, nicht umbauen |
| `override static` auf `CreateInstance`/`GetInstance` | B/Logger.c:8, 12 (UApiLog) | statics sind nicht virtuell; `override static` ist zweifelhafte Syntax → beim textuellen Logger-Merge (§4 Fall B) verschwindet das Konstrukt ohnehin |
| Icon-Pfade `"_UniversalApi/images/…"` | QnAMakerConfig.c:51 (via NOTIFICATIONS-ifdef), Utilities.c:80/:88 | Phase 0 §2.4: nach `Psyerns_Framework/images/` umziehen (3 Stellen) |
| `#ifdef NOTIFICATIONS`-Block | QnAMakerConfig.c SendRespone | nicht leer → zulässig; Define stammt von Drittmod („Notifications") — dokumentieren |
| `Math.Randomize(Math.QRandom())` | UniversalApi.c:476 | vanilla `Math.Randomize(int seed)` existiert ✅ |
| `RandomInt(int.MIN, int.MAX)` | enMath.c QRandom-Fallback | Bereichs-Überlauf (max-min > int.MAX) + CLAUDE.md-Warnung zu int.MIN/MAX-Grenzen → Fallback auf `RandomInt(0, int.MAX)` mit Vorzeichen-Flip ändern |

---

## 3. Hebung auf Ziel-Infrastruktur (PF_)

### 3.1 Architektur-Gegenüberstellung HTTP

| Aspekt | Ziel (PF_) | Quelle (UApi) |
|---|---|---|
| Bootstrap | `PF_WebClient` Singleton, `CreateRestApi()` 1× im Ctor | 4 unabhängige `Api()`-Bootstrapper (UniversalRest:3, UniversalApi:217 `RestCore()`, UApiBaseEndpoint:14, DiscordRest:3), je `GetRestApi() ?: CreateRestApi()` |
| Kontexte | pro BaseUrl gecacht (`map<string, RestContext>`) | UniversalRest/DiscordRest: Kontext pro **Voll-URL**; UApiBaseEndpoint: 1 Kontext pro Endpoint-Objekt, bei jedem Call neu geholt |
| Auth-Transport | API-Key als Query-Param `?api_key=` (`PF_RestBase`, dokumentiert: SetHeader kann nur Content-Type) | **JWT im Content-Type-Header** (`ctx.SetHeader(UApi().GetAuthToken())`) — Node-Middleware `ExtractAuthKey` hebt ihn in `auth-key` um. Interop-kritisch (Phase 0 §2.8) — bleibt! |
| Callbacks | `PF_RestCallback` (Logging + `PF_WebResponse`, fire-and-forget) | typisierte Callback-Familie mit JSON→Objekt-Parsing, Status-Mapping, CallId + Cancel (`UApiCallback<T>`, `UApiCallbackLoader<T>`, `UApiJSONCallback`, `UApiDBNestedCallBack`, `UApiDBCallBack`, `UApiSilentCallBack`) |
| Queue/Retry | `PF_WebQueue` + `PF_WebQueueProcessor` (Rate-Limit, Retry) | keine Queue; Retry nur im Auth-Fluss |

**Kernempfehlung:** Die UApi-**Callback-/Endpoint-Schicht ist funktional überlegen und wird übernommen** (Rename per Phase 0). Die **Bootstrap-/Kontext-Schicht wird auf PF_WebClient gestützt**: die 4 `Api()`/`RestCore()`-Duplikate durch `PF_WebClient.GetInstance().GetRestApi()` bzw. eine neue Methode `PF_WebClient.GetRestContextRaw(baseUrl)` **ohne** den `SetHeader("application/json")`-Default ersetzen (der JWT-im-Content-Type-Hack verträgt sich nicht mit dem json-Default und nicht mit geteilten Kontexten zwischen PF_- und DME_Api-Aufrufern → getrennte Kontext-Map oder Key-Präfix `"uapi:"+url`). `ERESTOPTION_READOPERATION`-Setup (15 s bzw. 30 s) einmalig in PF_WebClient zentralisieren.

### 3.2 Empfehlung je UApi-Klasse (Überlappungs-Matrix)

| UApi-Klasse(n) | PF_-Pendant | Empfehlung |
|---|---|---|
| `UniversalRest`, `UniversalDiscordRest` (statische URL-Bau + Post/Get) | `PF_WebApiBase`/`PF_RestBase` | **übernehmen** (Rename), intern auf PF_WebClient-Kontexte stützen (§3.1); blocking `PostNow` streichen (§2.2) |
| `UApiBaseEndpoint` + `UApiDBEndpoint`, `UApiDBGlobalEndpoint`, `UniversalDSEndpoint`, `UApiAPIEndpoint` | `PF_WebApiBase`-Familie | **übernehmen** — Endpoint-Semantik (Sub-Pfade = Wire) 1:1; nur Bootstrap auf PF_WebClient, `EnableDebug` raus |
| `UApiCallback<T>`-Familie, `UApiDBNestedCallBack`, `UApiDBCallBack`, `UApiSilentCallBack`, `UApiAuthCallBack`, … | `PF_RestCallback` | **übernehmen** — kein PF_-Äquivalent (typisiertes Parsing). PF_RestCallback bleibt für PF_-Dienste unverändert. Optional später: PF_RestCallback als Basis der Silent-Variante (nice-to-have, nicht Phase 2) |
| `UApiDBHandler<T>`, `UApiGlobalHandler<T>` (Mini-ORM) | — | **übernehmen** — Alleinstellungsmerkmal des Merges (MongoDB-Persistenz) |
| `UApiJSONHandler<T>` | `PF_JsonBuilder` (manueller String-Builder) | **komplementär, beide behalten**: JSONHandler = typisierte (De-)Serialisierung, JsonBuilder = ad-hoc-Payloads. Quell-interne `JsonSerializer`-Direktnutzung (5 Stellen) auf JSONHandler umziehen |
| `UApiLog`/`ULoggerBase`/`ULoggerBaseInstance`/`ULoggerObject` | `PF_Logger` | **verschmelzen (Fassade):** Datei-/Print-Zweig von ULoggerBaseInstance streichen und auf `PF_Logger` delegieren (der bereits Datei + Maskierung kann); **API-Versand-Zweig (`SendToApi` → `Rest().Log`) behalten** — das ist der Mehrwert (Remote-Logging in Mongo). Log-Level-Modell (LOG_ERROR…LOG_DEBUG) auf PF_Logger-Methoden mappen (Error/Log/Debug). AdminLog-Zweig (Level INFO) behalten |
| `UniversalApiConfig` (+ ConfigLoader-Mechanik `$profile:UApi\UniversalApi.json`) | `PF_WebConfig` (`$profile:DeadmansEcho\PsyernsFramework\PsyernsFrameworkConfig.json`, Versionierung, AutoGenerateApiKeys) | **stützen:** DME_Api-Settings als **eigener Block in PF_WebConfig** (neue Member `DmeApi_ServerURL`, `DmeApi_ServerID`, `DmeApi_ServerAuth`, `DmeApi_QnAEnabled`, `DmeApi_EnableBuiltinLogging`, `DmeApi_PromptDiscordOnConnect`) **oder** (Option B, empfohlen wegen Drop-in-Kompat + RPC-Serialisierung) `DME_Api_Config` als eigene Klasse behalten, aber Lade-Pfad/Versionierung nach PF_WebConfig-Muster inkl. Auto-Migration vom Alt-Pfad (Phase 0 E2, Option A). **Achtung:** `UniversalApiConfig` wird als CF-RPC-Param2 an Clients geschickt — bei Integration in PF_WebConfig müsste ein separates Client-DTO her; das stützt Option B |
| `UApiQnAMakerServerAnswers` (eigene JSON-Config) | — | übernehmen; Lade-Muster an PF_WebConfig-Konventionen angleichen (Verzeichnis `$profile:DeadmansEcho\PsyernsFramework\`) |
| `UniversalApi` (Kern/Accessor `UApi()`) | kein Pendant (PF hat kein zentrales API-Objekt) | **übernehmen** als `DME_Api_Core`/`DME_Api()`; `RestCore()` entfernen → PF_WebClient |
| Discord: `UApiDiscord*`-Objekte + `UniversalDSEndpoint`/`UniversalDiscordRest` (Bot via Node-Service: Rollen, Channels, Nick, Mute, OAuth-Link) | `PF_DiscordWebhook`/`PF_DiscordPayload`/`PF_DiscordEmbed` (reiner Webhook-Post) | **beide behalten — verschiedene Transportwege.** ABER: `UniversalApi.DiscordMessage()`/`DiscordObject()` (UniversalApi.c:414–424) sind **direkte Webhook-Posts** und damit ein echtes Duplikat von `PF_DiscordWebhook.Send/SendSimple` → diese 2 statischen Methoden **nicht übernehmen**, Aufrufer auf PF_DiscordWebhook verweisen; `UApiDiscordObject`/`UApiDiscordEmbed` (Wire-Klassen für Bot-Kanal-Sends via Service) bleiben |
| `UUtil` (FindPlayer, GetSteamId, Notifications, Config-Getter, Datums-Helfer) | teils `PF_Logger`-Datumscode | übernehmen (Rename `DME_Api_Util`); interne Dreifach-Duplikate konsolidieren: `FindPlayer`/`FindPlayerByIdentity`/`GetSteamId` existieren in UniversalApi.c, UUtil **und** DayZGame — auf je 1 Implementierung (UUtil statisch) reduzieren, Rest delegiert |
| `UCurrency`/`UCurrencyValue` + PlayerBase-Money-API | — | übernehmen (kein PF_-Pendant); Bugs §6 beachten |
| `UApiEntityStore` + `UApiAmmoData`/`UApiMetaData`/`UApiZoneHealthData` | — (DME_Virtual_Storage im DME-WAR-Umfeld ist separates Projekt!) | übernehmen; §4 Fall A + Bugfixes §6.3–6.5 |
| `UScriptExec`/`UApiScriptBase` | — | **nicht mergen** (Default) oder ifdef-gated (§2.7) |
| `SimpleValueStore`, `StatusObject`, `UApiStatus`, `UApiServerStatus`, Transaction-/Update-/Query-/QnA-/Crypto-/Translation-Objects | — | übernehmen (reine Wire-DTOs; Membernamen einfrieren) |
| `ApiQueryBase` (Sample), `CheckIfHasDiscord` (Sample), `SampleExec` | — | Samples: übernehmen & auf neue Namen umschreiben (Doku-Wert) — **Hinweis:** `UApiScriptTest` existiert **nur innerhalb eines Blockkommentars** (SampleExec.c ist zu 100 % auskommentiert) → aus Rename-Map streichen oder als „kommentiert" markieren |

### 3.3 Doppelte Infrastruktur — Klartext

1. **Doppelter Logger:** ja (ULogger* vs. PF_Logger) → Fassaden-Merge (Tabelle oben).
2. **Doppelter Config-Loader:** ja (ConfigLoader.c vs. PF_WebConfig) → Mechanik vereinheitlichen (PF-Muster: Versionierung/Upgrade/Verzeichnis), Keys & RPC-DTO behalten.
3. **Doppelte Discord-Anbindung:** dreifach — (a) PF_DiscordWebhook [direkt], (b) UApi statische Webhook-Posts [Duplikat → streichen], (c) UApi-Bot via Node-Service [einzigartig → behalten].
4. **Doppelte HTTP-Bootstrap:** vierfach in Quelle + einmal im Ziel → auf PF_WebClient zentralisieren.
5. **Interne Quell-Duplikate:** FindPlayer/GetSteamId 3×, Status-Switch-Mapping 2× (UApiCallback/UApiCallbackLoader — bei Merge in Helper ziehen), ErrorToString 2× (UniversalApi + UUtil.RestErrorToString).

---

## 4. Struktur-Migrationsplan

### 4.1 Layer-Mapping (Quelle → Ziel)

Quelle lädt via config.cpp: _UAPIBase legt `scripts/Common` in **jedes** Modul; _UniversalApi hat echte 1_Core/3_Game/4_World/5_Mission. Ziel hat aktuell **kein 1_Core und kein Common** → config.cpp-Erweiterung nötig (Phase 0 §2.5 bestätigt).

| Quelle | Ziel (`Psyerns_Framework/scripts/…`) | Inhalt |
|---|---|---|
| U/1_Core/UApi/Constants.c, enMath.c | `1_Core/DME_Api/DME_Api_Constants.c`, `DME_Api_Math.c` | **NEU: engineScriptModule in config.cpp ergänzen** |
| U+B/Common/UApi/Common.c | `Common/DME_Api/DME_Api_Defines.c` (`#define DME_API`, `DME_API_STABLE`) | Common-Pfad in **alle** Modul-files[]-Arrays |
| B/3_Game (Currencys, ItemStoreMetaData, JsonHandler, Logger, ObjectBase, SampleExec, ScriptBase, ScriptExecutor*, SimpleStore) + U/3_Game (Callbacks/, Endpoints/, Objects/, ConfigLoader, DayZGame, DiscordRest, Logger→merge, QnAMakerConfig, UniversalApi, UniversalRest, Utilities) | `3_Game/DME_Api/…` (Unterordner Callbacks/, Endpoints/, Objects/ beibehalten) | B-Dateien zuerst einsortieren (Basisklassen), Compile-Reihenfolge im selben Modul egal — Ausnahme: Logger-Merge (Fall B). *ScriptExecutor nur bei Owner-Ja |
| B/4_World (CarScript, ItemBase, ItemStore, PlayerBase) + U/4_World (Classes/PluginAdminLog, Entities/*, ItemStore, Mission) | `4_World/DME_Api/…` | 4 textuelle Merges (Fälle A, C, D, E) |
| U/5_Mission (ChatInputMenu, MissionBase, MissionGameplay, MissionServer) | `5_Mission/DME_Api/…` | Koexistenz mit PF_Mission* (Fall F) |

### 4.2 Cross-PBO-modded-Fälle — konkreter Merge je Fall

Grundregel: `modded class` auf eine Klasse, die im **selben** Skriptmodul definiert ist, ist unmöglich → Basis + modded **textuell zu einer Definition** verschmelzen.

**Fall A — `UApiEntityStore` → `DME_Api_EntityStore` (1 Datei, `4_World/DME_Api/DME_Api_EntityStore.c`):**
Basis `B/4_World/UApi/ItemStore.c` (Datenfelder, Write/Read/Get-API, leere `SaveEntity/Create/CreateAtPos/LoadEntity`) + modded `U/4_World/UApi/ItemStore.c` (Implementierungen). Merge = Basisklasse übernehmen, die 4 leeren Methoden durch die modded-Implementierungen ersetzen (`override` entfernen). Destruktor mit 5× `delete` streichen (§1.3), Syntax-Fixes §1.11 #4–6, Bugfixes §6.3–6.5, `autoptr`-Member → `ref` (Feldnamen = Wire, unverändert).

**Fall B — `ULoggerBaseInstance` (Basis in B/3_Game/Logger.c, modded in U/3_Game/Logger.c):**
Merge = 5-Zeilen-modded (`override SendToApi` → `UApi().Rest().Log(jsonString)`) in die Basisklasse einsetzen; Datei U/3_Game/UApi/Logger.c entfällt ersatzlos. Danach Fassaden-Umbau auf PF_Logger (§3.2). „MapLink"-Copy-Paste-Strings (B/Logger.c:100, 103) bei der Gelegenheit korrigieren.

**Fall C — `ItemBase` (2× modded):**
B-Version = Leaf-Hooks `OnUApiSave/OnUApiLoad` (leer) + `UAddQuantity/USetQuantity/UHasQuantityBar`. U-Version = **reines No-op** (`override` + nur `super`-Aufruf). Merge = B-Version übernehmen (Methoden-Rename `OnUApiSave` → `OnDME_ApiSave` NICHT nötig — Namen sind API-Fläche für Drittmods; Empfehlung: behalten, da kein Vanilla-Konflikt), U-Version **ersatzlos löschen**.

**Fall D — `CarScript` (2× modded):**
B = leere Hooks `OnUApiSave/OnUApiLoad`; U = Implementierung (Fluids + Engine-/FuelTank-Health). Merge = eine modded-CarScript-Datei mit den U-Implementierungen (ohne `override`/`super`, da die Hook-Deklaration jetzt hier selbst liegt).

**Fall E — `PlayerBase` (2× modded):**
B = `GetQuickBarEntityIndex` (löschen → vanilla `FindQuickBarEntityIndex`, §1.10), `UCreateItemInInventory/UCreateItemGround/UMaxQuantity/UHasQuantity/…` (Item-Utils; Syntax-Fix §1.11 #3). U = Currency-API (`UGetPlayerBalance/UAddMoney/URemoveMoney/URemoveMoneyInventory/UGetItemCount/UCurrentQuantity/…`). Merge = eine modded-PlayerBase-Datei, B-Teil zuerst (U-Teil ruft B-Methoden). Keine override/super-Themen (alles neue Methoden).

**Fall F — Mission-Kette (Konflikt mit Ziel!):**
- Ziel moddet bereits: `MissionServer` **2×** (PF_MissionInit.c, PF_RestInit.c — modded-Stacking innerhalb desselben Moduls funktioniert dort erklärtermaßen: „stacks on top of PF_MissionInit"), `MissionBase` 1× (PF_MissionClient.c).
- UApi moddet: `MissionBaseWorld` (4_World/Mission.c — Leaf-Hook `UniversalApiReady*` + `m_UApi_Initialized`), `MissionBase` (Ctor `UApi()` + `UniversalApiReady`-Override), `MissionServer` (Ctor + `OnClientPrepareEvent` + `UniversalApiReady`), `MissionGameplay` (Token-Renewal), `ChatInputMenu` (QnA-Hook).
- **Empfehlung (minimalinvasiv, dem Ziel-Muster folgend):** UApi-Anteile als **zusätzliche eigene modded-Dateien** unter `5_Mission/DME_Api/` bzw. `4_World/DME_Api/` einbringen (drittes modded-MissionServer, zweites modded-MissionBase, erstes modded-MissionBaseWorld/MissionGameplay/ChatInputMenu) — analog zum bereits praktizierten PF-Stacking. **Kein** Hinein-Mergen in PF_MissionInit/PF_RestInit (Trennung PF_/DME_Api bleibt sauber, AGPL-§5-Notizen bleiben je Datei zuordenbar).
- Risiko-Check dabei: `MissionServer`-**Konstruktor**-Stacking (UApi definiert `void MissionServer()`) — modded-Ctor ohne expliziten super-Aufruf ruft die Kette implizit; beim ersten Servertest verifizieren, dass PF_-OnInit **und** UApi-Ctor beide feuern.
- Reihenfolge in config.cpp-files[] beachten: PF_-Dateien vor DME_Api-Dateien listen, damit DME_Api „oben" stackt (bzw. umgekehrt — nur konsistent dokumentieren).

**Fall G — Rest (1 Definition, nur Rename+Fixes):** `Math` (enMath.c), `DayZGame`, `PluginAdminLog`, `Weapon_Base`, `TransmitterBase`/`Edible_Base`/`BloodContainerBase` (VanillaItems.c) — unverändert übernehmbar nach §1-Fixes.

### 4.3 Datei-Umbenennungen (Auswahl, folgt Klassen-Rename)

`UniversalApi.c`→`DME_Api_Core.c`, `UniversalRest.c`→`DME_Api_Rest.c`, `ConfigLoader.c`→`DME_Api_Config.c`, `ItemStore.c`(merged)→`DME_Api_EntityStore.c`, `Logger.c`(merged)→`DME_Api_Logger.c`, `enMath.c`→`DME_Api_Math.c`, `Currencys.c`→`DME_Api_Currency.c`, Ordner `UApi/`→`DME_Api/`.

---

## 5. Token-Auth-Fluss (EnScript-Seite)

### 5.1 Ablauf (verifiziert, Datei/Zeile)

```
SERVER-BOOT
 ConfigLoader: UApiConfig() lädt $profile:UApi\UniversalApi.json (ServerURL, ServerID, ServerAuth, …)   [ConfigLoader.c:60-70]
 UniversalApi.Init(): #ifdef NO_GUI → m_IsServer; 4 CF-RPCs registriert; Status-Check + QRandom-Timer    [UniversalApi.c:264-282]
 GetAuthToken() serverseitig == UApiConfig().ServerAuth (statischer Server-Key)                          [UniversalApi.c:227-234]

CLIENT-CONNECT (Server-Seite)
 MissionServer.OnClientPrepareEvent → UApi().PreparePlayerAuth(guid)                                     [5_Mission/MissionServer.c]
   → Rest().GetAuth(guid): POST {ServerURL}GetAuth/<guid>, Body "{}",
     Auth = ServerAuth via ctx.SetHeader(...) ["Content-Type-Hack"]                                      [UniversalRest.c:37-41, :20]
   ← Service antwortet {"GUID":"...","AUTH":"<JWT expiresIn 2800s>"}
 UApiAuthCallBack.OnSuccess: parse → UApi().AddPlayerAuth(guid, jwt)                                     [UApiAuthCallBack.c:24-42]
   → Cache PlayerAuths map<guid,jwt>; wenn Spieler online: SendAuthToken(identity, jwt)                  [UniversalApi.c:319-341]
 SendAuthToken: CF-RPC "RPCUniversalApiConfig" mit Param2<ApiAuthToken{GUID,AUTH},
   UniversalApiConfig-Klon (ServerAuth=="null"!)> an Client                                              [UniversalApi.c:379-400]

CLIENT-SEITE
 MissionGameplay.OnMissionStart: RequestAuthToken(true) + Renewal-Timer 1260–1380 s (Math.RandomInt)     [MissionGameplay.c]
 RPCRequestAuthToken (Server-Handler): cached → SendAuthToken; sonst PreparePlayerAuth (Renew);
   sonst "RPCRequestRetry" → Client-Backoff m_AuthRetries*2200 ms, max 20 Versuche                       [UniversalApi.c:354-377, :307-311]
 RPCUniversalApiConfig (Client-Handler): m_authToken + m_UniversalApiConfig setzen
   → CallQueue → OnTokenReceived                                                                          [UniversalApi.c:284-293]
 OnTokenReceived: Status-Check, QnA-Config-RPC, Discord-User-Cache,
   GameScript.CallFunction(Mission, "UniversalApiReadyTokenReceived") → Leaf-Hook UniversalApiReady()    [UniversalApi.c:295-304; 4_World/Mission.c]

JEDER REQUEST (Client wie Server)
 UniversalRest.Post / UApiBaseEndpoint.Api(): ctx.SetHeader(UApi().GetAuthToken()); ctx.POST(cb,route,json)
 → JWT/ServerAuth ersetzt den Content-Type-Header; Node-Middleware ExtractAuthKey verschiebt ihn
   nach req.headers['auth-key'] und setzt Content-Type=application/json                                   [UniversalRest.c:14-31; UApiEndpointBase.c:14-38]
 GET-Requests der Convenience-Schicht senden KEIN Token (UniversalRest.Get setzt keinen Header!)          [UniversalRest.c:24-31]
 Fehlerpfad: AuthError(guid) → Retry GetAuth nach 180 s (online) bzw. Status-Recheck nach 300 s           [UniversalApi.c:402-412]
 Token-Lebensdauer: 2800 s serverseitig signiert; Client erneuert alle 21–23 min (passt: ~2 Fenster Puffer)
```

### 5.2 Interop-kritisch (bleibt) vs. implementierungsfrei (darf sich ändern)

| Interop-kritisch — NICHT anfassen | implementierungsfrei — Phase 2 darf umbauen |
|---|---|
| Route `GetAuth/<guid>` + alle Endpoint-Sub-Pfade | CF-RPC-Namen/Mod-Key (beide Seiten werden zusammen ausgeliefert — Phase 0 Block D) |
| Token-Transport im **Content-Type-Header** (SetHeader), Node-Gegenstück `auth-key` | Bootstrap der RestApi (→ PF_WebClient), Kontext-Handling |
| Wire-Felder `GUID`, `AUTH` (ApiAuthToken) — Klassenname frei | Cache-Struktur `PlayerAuths`, Retry-Zeiten/Backoff, Renewal-Fenster (nur < JWT-Expiry 2800 s halten) |
| Client-Config-Keys `ConfigVersion, ServerURL, ServerID, ServerAuth, QnAEnabled, EnableBuiltinLogging, PromptDiscordOnConnect` (JSON + RPC-DTO) | Ort/Name der Server-Config-Datei (mit Auto-Migration, Phase 0 E2) |
| `UAPI_VERSION`-Wert „1.3.2" (CheckVersion-Handshake, UniversalApi.c:498) | Konstantenname (→ `DME_API_VERSION`) |
| Status-Strings des Service (`noerror`, `noauth`, `NotFound`, `NoAuth`, `InvalidAuth`, …, StatusObject.Status/Error) | Callback-Klassen-/Methodennamen, Status-Mapping-Helper |
| `"null"`-Sentinel für „kein Token" (GetAuthToken-Rückgabe wird als Header gesendet und serverseitig geprüft) | `m_IsServer`-Erkennung (NO_GUI → IsDedicatedServer) |

---

## 6. Pre-existing Logik-Bugs (Bonus-Funde, Fix in Phase 2 empfohlen)

| # | Fundort | Bug | Wirkung |
|---|---|---|---|
| 1 | U/1_Core/enMath.c QRandomFloat (`float num = number / int.MAX;`) | **Integer-Division** → num ist praktisch immer 0 | QRandomFloat liefert immer `min` |
| 2 | U/4_World/ItemStore.c LoadEntity (`if (m_CurrentMuzzle >= weap.GetMuzzleCount() \|\| m_CurrentMuzzle < 0){ weap.SetCurrentMuzzle(...)}`) | Bedingung **invertiert** — setzt Muzzle nur bei UNgültigem Wert | gespeicherte Muzzle-Auswahl geht verloren / Set mit ungültigem Index |
| 3 | U/4_World/ItemStore.c Create() | `if (parent == NULL){...}` gefolgt von `if (m_Slot == -1){ parent.GetInventory()... }` (kein else) | parent==NULL & m_Slot==-1 → **Null-Deref** |
| 4 | U/4_World/ItemStore.c LoadEntity (Magazin-Zweig) | `m_MagAmmo` kann NULL sein (wenn beim Save keine Cartridge-Info valide war), wird aber ohne Check iteriert; zudem `if (i > m_MagAmmo.Count())` statt `>=` | **NPE/Out-of-Range** — gleiche Bug-Klasse wie der bekannte DME_VS-GetCartridgeAtIndex-Crash |
| 5 | U/5_Mission/ChatInputMenu.c OnChange | `UApiConfig().QnAEnabled` — auf Client ist `UApiConfig()` **NULL bis zum Config-RPC** | Chat öffnen vor Token-Empfang → **NPE** |
| 6 | U/CallBacks/UApiAuthCallBack.c:28-35 | `js.ReadFromString(authToken, …)` ohne Null-Check, danach `authToken.GUID` | kaputte Service-Antwort → **NPE** auf Server |
| 7 | U/UniversalApi.c:373 | `else if (UApiConfig().ServerAuth && ServerAuth != "" && ServerAuth != "null")` — logisch unerreichbar nach dem if davor | „Server Auth is empty"-Error feuert nie |
| 8 | U/3_Game/DayZGame.c CBQnAChatMessage* | `UApiQnAMaker()` liefert auf Client NULL bis QnA-RPC ankam | QnA-Antwort vor Config → **NPE** |
| 9 | U/UniversalApi.c:390 | Shadowing-Local `m_authToken` (§1.9) | Member bleibt serverseitig NULL — nur deshalb kein Fehlverhalten, weil GetAuthToken() server-seitig ServerAuth nimmt; beim Refactor Stolperfalle |
| 10 | B/3_Game/Logger.c:100/:103 | Log-Banner/Error sagen „MapLink" (Copy-Paste aus daemonforges MapLink-Mod) | kosmetisch/verwirrend |
| 11 | B/3_Game/SampleExec.c | Datei zu 100 % auskommentiert — Klasse `UApiScriptTest` existiert nicht als Code | **Rename-Map-Korrektur:** Eintrag „UApiScriptTest → DME_Api_ScriptTest" betrifft nur Kommentartext |
| 12 | U/Endpoints/UApiDBEndpoint.c Save(3-arg) | reserviert `cid` via CallId(), gibt ihn zurück, hängt aber `UApiSilentCallBack` an | zurückgegebene CallId feuert nie (Doku-Falle für API-Nutzer; so belassen oder dokumentieren) |

---

## 7. Aufwandsschätzung Phase 2 (EnScript-Umsetzung)

Basis: 70 Quell-.c (~7,8 k LOC) + config.cpp/mod.cpp-Erweiterung des Ziels; Ziel-Code selbst bleibt fast unberührt (nur PF_WebClient-Erweiterung + optional PF_WebConfig-Block).

| Arbeitspaket | Umfang | Schätzung (fokussierte Agent-/Dev-Arbeit) |
|---|---|---|
| P2.1 Mechanischer Rename (104 Klassen + Konstanten + Accessors + Dateinamen, per Skript + Review) | 70 Dateien | 0,5–1 Tag |
| P2.2 Syntax-Defekte (7) + delete (5) + Mehrfach-Deklarationen (11) + `;;` | punktuell | 0,25 Tag |
| P2.3 GetGame→g_Game (135) + IsServer/IsClient→IsDedicatedServer (30) + CallQueue-Nullchecks (16) | mechanisch mit Semantik-Review (§1.2-Tabelle liegt vor) | 1 Tag |
| P2.4 ref-Params (39) + autoptr-Policy (194) | mechanisch | 0,5 Tag |
| P2.5 Cross-PBO-Merges Fälle A–F + Layer-Einsortierung + config.cpp (1_Core/Common) | 6 Merge-Fälle | 1 Tag |
| P2.6 PF_-Hebung: WebClient-Andocken (4 Bootstrapper), Logger-Fassade, Config-Mechanik + Migration, DiscordMessage-Dedup, EnableDebug/PostNow raus | Kern-Refactor | 1–1,5 Tage |
| P2.7 Bugfixes §6 (12) + override/super (§1.7/1.8) + Naming §1.10 + RPC-Hygiene (CallType, Enum-Typo, Konstante) | punktuell | 0,5 Tag |
| P2.8 Compile-Iterationen (Workbench), M0-Selbsttest Server+Client, Token-Roundtrip gegen laufenden Node-Service | Test | 1–1,5 Tage |
| **Summe EnScript Phase 2** | | **~5,5–7 Personentage** (mit Multi-Agent-Workflow realistisch 2–3 Kalendertage inkl. Reviews; Servertest-Slots nicht eingerechnet) |

Reihenfolge-Empfehlung: P2.2 → P2.1 → P2.5 → P2.3/P2.4 (parallelisierbar je Datei-Gruppe) → P2.6 → P2.7 → P2.8. Syntax-Fixes vor dem Rename, damit das Rename-Skript auf parsbarem Code arbeitet.

---

*Erstellt durch Audit-Agent 1A (EnScript), 2026-07-10. Datenbasis: vollständige greps + Datei-Reads beider Quell-PBOs, Ziel-scripts, vanilla 1.29, CF-production; 46 Vanilla-Symbole einzeln verifiziert; keine Code-Änderungen vorgenommen.*

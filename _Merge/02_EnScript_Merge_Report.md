# Phase 2+3 — EnScript Merge Report (UniversalApi 1.3.2 → Psyerns_Framework)

**Agent:** EnScript-Merge-Agent · **Datum:** 2026-07-10 · **Modus:** Code eingebracht/geändert im Ziel
**Quelle:** `DeamonForge/DayZ-UniveralApi-1.3.2/_UniversalApi` + `_UAPIBase` (73 .c) · `-stable` ignoriert
**Ziel:** `Psyerns_Framework/scripts/` (neuer `DME_Api/`-Teilbaum neben dem bestehenden `Psyerns_Framework/`-PF_-Baum)
**Grundlagen:** 00_Baseline (Rename-Map), 01_Audit_EnScript (Arbeitsliste), 01_Audit_Schemas_Wire (Wire-Contract)

---

## (a) Eingebrachte/erzeugte Dateien + Ziel-Layout

**63 `.c`-Dateien** erzeugt (aus 69 einzubringenden Quell-Dateien: 73 − 4 ausgeschlossen; 6 Merge-Paare zu je 1 Datei kollabiert = 63). Jede Datei trägt den AGPL-/daemonforge-Header (Vorlage (a) aus 00_Baseline): **63/63 mit SPDX-Header und daemonforge-Attribution.**

```
scripts/1_Core/DME_Api/            2   DME_Api_Constants.c, DME_Api_Math.c        (NEU: braucht engineScriptModule)
scripts/Common/DME_Api/            1   DME_Api_Defines.c                          (NEU: #define DME_API/_STABLE)
scripts/3_Game/DME_Api/           13   Core, Config, Rest, DiscordRest, Logger, Utilities, QnAMakerConfig,
                                       DayZGame, Currency, ItemStoreMetaData, JsonHandler, ObjectBase, SimpleStore
scripts/3_Game/DME_Api/CallBacks/  9   Auth, DB, Discord, QnA, Silent, Status, Transaction, Object, SampleCheckIfHasDiscord
scripts/3_Game/DME_Api/Endpoints/  7   EndpointBase, DBEndpoint, DBGlobalEndpoint, DBHandler, DSEndpoint, APIEndpoint, GlobalHandler
scripts/3_Game/DME_Api/Objects/   19   AuthToken, Forwarder, ConfigBase, Crypto, Discord*(5), QnA, Query,
                                       SimpleLog, Status, Transaction, Misc, QueryResultTemplate, ServerStatus, Translation, Update
scripts/4_World/DME_Api/           2   DME_Api_EntityStore.c (merge A), DME_Api_Mission.c (MissionBaseWorld)
scripts/4_World/DME_Api/Classes/   1   DME_Api_PluginAdminLog.c
scripts/4_World/DME_Api/Entities/  5   ItemBase(C), CarScript(D), PlayerBase(E), Weapon_Base, VanillaItems
scripts/5_Mission/DME_Api/         4   MissionBase, MissionServer, MissionGameplay, ChatInputMenu
```

**Ausgeschlossen (0 Referenzen im Ziel verifiziert):** `ScriptExecutor.c` (UScriptExec, RCE), `ScriptBase.c` (UApiScriptBase, nur vom Executor genutzt), `SampleExec.c` (100 % Kommentar), `CallBacks/Samples/ApiQuery.c` (SampleQuery). `ModAddons/UApiMass.pbo` (Binär) ohnehin nicht in scripts.

Bilder: 3 `.edds` (Bot/info/warning) müssen nach `Psyerns_Framework/images/` gelegt werden (Code-Pfade sind bereits darauf umgestellt — 3 Stellen: QnAMakerConfig, Utilities ×2).

---

## (b) Rename-Vollständigkeit

Deterministischer Master-Sed (geordnet, wortgrenzengenau) über alle Dateien; danach Header angehängt (damit die Attribution „DayZ-UniversalApi" im Header nicht mit-renamt wird).

- **104 Klassen** (Block A `UApi*`, Block B `Universal*`/`U*`/`Api*`/sonstige) → `DME_Api_*`. Verifiziert über Klassen-Token-Enumeration.
- **Accessoren:** `UApi()`→`DME_Api()`, `UApiConfig()`→`DME_Api_GetConfig()`, `UApiQnAMaker()`→`DME_Api_GetQnAMaker()` (Reihenfolge vor generischer Regel, keine Kollision mit `DME_Api_Config`).
- **Konstanten:** `UAPI_*`→`DME_API_*` (19), `PLAYER_DB`/`OBJECT_DB`→`DME_API_PLAYER_DB`/`_OBJECT_DB`, `LOG_*`→`DME_API_LOG_*`.
- **Defines:** `UNIVERSALAPI`→`DME_API`, `UNIVERSALAPI_STABLE`→`DME_API_STABLE`.
- **Hooks (Methode + Reflection-String):** `UniversalApiReady`→`DME_Api_Ready`, `UniversalApiReadyTokenReceived`→`DME_Api_ReadyTokenReceived`.
- **RPC (Block D):** Mod-Key `"UAPI"`→`"DME_Api"`, Handler `RPCUniversalApiConfig`→`RPC_DME_Api_Config`; `RPCRequestAuthToken/QnAConfig/Retry` behalten.
- **Member/Globals:** `m_UApi*`/`m_Universal*`/`g_UniversalApi` → `m_DME_Api_*`/`g_DME_Api_Core`. Hook-Methoden `OnUApiSave/Load`→`OnDME_ApiSave/Load`. `Math.m_QRandomNumbers`→`s_DME_Api_QRandomNumbers`.
- **CF-Enum-Typo** `SingeplayerExecutionType`→`SingleplayerExecutionType`. **Log-Tags** `[UAPI]`/`[UPAI]`→`[DME_Api]`.

**Rest-Treffer `UApi`/`UniversalApi`/`UAPI` in echtem Code (nicht Kommentar/Header): 0.** Bewusst belassen (Interop, dokumentiert): `DME_API_VERSION="1.3.2"` (Version-Handshake), Legacy-Pfad `"$profile:UApi\UniversalApi.json"` (Migrations-Fallback, Config.c), Header-Attribution „DayZ-UniversalApi" in allen 63 Dateien. Wire-Feldnamen (Status/Error/GUID/AUTH/GamePort/answer/score/…), Routen-Strings, Mongo-Keys, `LetsEncypt`, `/Gobals`, MongoDB-Operator-Werte — **unangetastet**.

---

## (c) Modernisierungs-Zählung (1.29)

| Fix | Umfang (Audit) | Status |
|---|---|---|
| `GetGame()`→`g_Game` | 135 | ✅ alle (Sed); zusätzlich **51 `if (!g_Game) return;`-Guards** an Methodenköpfen |
| `IsServer()`/`IsClient()`→`IsDedicatedServer()`-Logik | 28 Zeilen/30 Vork. | ✅ alle (26× `IsDedicatedServer()`, Rest als `!…`/`else`; Core konsolidiert auf `m_IsServer` via IsDedicatedServer in Init) |
| `delete`-Statements | 5 | ✅ Destruktor `~DME_Api_EntityStore` inkl. 5× delete entfernt (GC) |
| Mehrfach-Deklarationen | 11 | ✅ alle gesplittet (Logger, Utilities, EntityStore-Locals) |
| `ref` auf Params/Returns | 39 Signaturen | ✅ alle entfernt (Rest 20, DiscordRest 15, Core-4-Handler `ref Object target`→`Object target`, DSEndpoint 2) |
| `autoptr` (194) | Member→`ref`, Local/Param/Param4-Arg→streichen | ✅; **owning Objekt-Arrays → `array<ref X>`** (nicht bloß gestrichen — §1.6, GC-Erhalt); 47 owning-ref-Stellen |
| fehlendes `override` | 3 (PluginAdminLog) | ✅ (`OnPlacementComplete`, `OnContinouousAction`; `PlayerHitBy` liegt im Ziel in einem `/* */`-Block → n/a) |
| CallQueue-Null-Checks | 16 | ✅ 14 via `ScriptCallQueue queue=…; if(queue){…}` (2 der 16 waren Kommentare) |
| Blocking `POST_now` / `EnableDebug(true)` | entschärfen | ✅ `EnableDebug(true)` in EndpointBase entfernt; `POST_now` als „blocking" markiert (Wire unverändert) |
| Syntax-Defekte | 7 hart + 2 `;;` | ✅ alle (`DBCBX = ;`×2 gelöscht, extra `)` PlayerBase, 3× fehlendes `;` nach `new array<>`, `GamePort;`, 2× `;;`) **+ 2 weitere gefunden**: extra `)` in DBHandler:172 & GlobalHandler:148 (`defaultJson))`→`)`) |
| RPC-Hygiene | Magic-Number, Enum-Typo | ✅ `155494166`→`DME_API_RPC_WEAPON_FIX` (const in 1_Core), Enum-Typo behoben |

**Pre-existing Logik-Bugs (Audit §6) behoben:** #1 QRandomFloat Integer-Division, #2 LoadEntity Muzzle-Bedingung invertiert, #3 Create() parent==NULL Null-Deref (else-if), #4 LoadEntity `m_MagAmmo`-Null-Iteration + `>` statt `>=`, #5 ChatInputMenu `DME_Api_GetConfig()`-Null auf Client, #6 AuthCallBack Null-Deref nach Parse, #7 unerreichbarer ServerAuth-Error-Zweig, #8 DayZGame QnAMaker-Null auf Client, #9 Shadowing-Locals `m_authToken`/`m_ClientConfig` in SendAuthToken, #10 „MapLink"-Copy-Paste-Strings im Logger. QRandom `int.MIN`-Fallback entschärft (Vorzeichen-Flip). #11/#12 dokumentiert/belassen.

---

## (d) Cross-PBO-Merges (textuell zusammengeführt)

- **Fall A — `DME_Api_EntityStore.c`:** Basis (`_UAPIBase/ItemStore.c`: Felder, Write/Read/Get, leere SaveEntity/Create/CreateAtPos/LoadEntity) + Impl (`_UniversalApi/ItemStore.c`). `override` an den 4 Methoden entfernt (jetzt Direkt-Definition), Destruktor+5×delete raus, autoptr-Member→`ref array<ref …>`, Syntax #4–6, QuickBar→vanilla `FindQuickBarEntityIndex`, Bugs #2/#3/#4, CallQueue-Guards.
- **Fall B — `DME_Api_Logger.c`:** Basis-`DME_Api_LoggerBaseInstance` + modded-`SendToApi` (→`DME_Api().Rest().Log(...)`) inline eingesetzt; `override static` auf `static` reduziert; `LOG_*`→`DME_API_LOG_*`; IsServer/IsClient→IsDedicatedServer; Multi-Decl; „MapLink"-Strings korrigiert; DoLog g_Game-Guard.
- **Fall C — `DME_Api_ItemBase.c`:** Basis (Leaf-Hooks + `UAddQuantity/USetQuantity/UHasQuantityBar`) übernommen, U-No-op ersatzlos gelöscht.
- **Fall D — `DME_Api_CarScript.c`:** U-Implementierungen (Fluids/Engine/FuelTank) als eigene Hook-Definition (ohne override/super); redundantes `extends Car` entfernt.
- **Fall E — `DME_Api_PlayerBase.c`:** B (Item-Utils, zuerst) + U (Currency-API); `GetQuickBarEntityIndex` gelöscht; Syntax #3 (extra `)`), `;;`; g_Game-Guards auf Config/CreateObjectEx/ObjectDelete; `extends ManBase` entfernt.
- **Fall F — Mission-Kette (Koexistenz, NICHT in PF_ hineingemergt):** `DME_Api_Mission.c` (MissionBaseWorld-Leaf-Hooks), `DME_Api_MissionBase/Server/Gameplay/ChatInputMenu.c` als eigene modded-Layer, die auf den PF_-Mission-Layern stacken. Vanilla-Hierarchie (`MissionServer/Gameplay→MissionBase→MissionBaseWorld→Mission`) gegen 1.29 verifiziert → redundante `extends`-Klauseln entfernt (Overrides von `DME_Api_Ready` lösen über die geerbte Kette auf). MissionServer `super` in `OnClientPrepareEvent` nach vorn gezogen; alle CallQueue-Guards.

---

## (e) Benötigte config.cpp/mod.cpp-Änderungen (für Orchestrator — NICHT von mir geändert)

Die Ziel-Modul-Pfade sind **rekursiv**: `3_Game/4_World/5_Mission`-DME_Api-Dateien werden bereits über die bestehenden `Psyerns_Framework/scripts/<Layer>`-Einträge geladen. **Neu nötig** in `config.cpp` **und** `mod.cpp` (beide, `class Psyerns_Framework > class defs`):

1. **engineScriptModule ergänzen** (fehlt komplett — sonst laden `1_Core/DME_Api` Constants+Math **nicht** und der gesamte Mod bricht):
   ```
   class engineScriptModule { value=""; files[]={"Psyerns_Framework/scripts/Common","Psyerns_Framework/scripts/1_Core"}; };
   ```
2. **`"Psyerns_Framework/scripts/Common"` als ersten Eintrag** in `gameScriptModule`, `worldScriptModule`, `missionScriptModule` `files[]` aufnehmen (Sichtbarkeit von `#define DME_API`/`_STABLE` in jedem Modul).
3. **CfgMods `credits`** um AGPL-Attribution ergänzen, z. B. `"… | Contains DayZ-UniversalApi by DaemonForge (AGPL-3.0)"`.
4. requiredAddons bleiben `{"DZ_Data","JM_CF_Scripts"}` (CF wird vom RPCManager gebraucht, bereits vorhanden).

Reihenfolge in `files[]`: PF_-Dateien werden ohnehin vor den DME_Api-Dateien geladen (rekursiv, alphabetisch pro Ordner); für das Mission-Stacking ist nur Konsistenz nötig — beim ersten Servertest verifizieren, dass PF_- **und** DME_Api-`MissionServer()`-Ctor beide feuern.

---

## (f) Offene Punkte / Risiken

1. **config.cpp/mod.cpp** (Punkt e) ist harte Voraussetzung — ohne engineScriptModule kompiliert/lädt 1_Core nicht.
2. **PF-Hebung (Audit §3) NICHT ausgeführt** (bewusst, außerhalb des 1.29-Modernisierungs-Auftrags): 4 RestApi-Bootstrapper (`RestCore()`/`Api()`) laufen weiter eigenständig statt über `PF_WebClient`; Logger delegiert noch nicht an `PF_Logger`; `UniversalApiConfig` bleibt eigene Klasse (kein PF_WebConfig-Block). `DME_Api_Core.DiscordMessage/DiscordObject` sind weiterhin ein Duplikat zu `PF_DiscordWebhook` (funktionsfähig, aber Dedup-Kandidat). Eigenes Arbeitspaket.
3. **CallType-Checks in CF-RPC-Handlern** bewusst NICHT ergänzt (Audit §1.12-Hygiene): Richtung würde bei Fehleinschätzung RPC brechen; die 4 Handler self-gaten korrekt über `m_IsServer`/`sender`-Null-Checks. Optional nachrüstbar.
4. **Config-Migration** (`$profile:DeadmansEcho\PsyernsFramework\DME_Api.json` mit Legacy-Fallback auf `$profile:UApi\UniversalApi.json`) implementiert — beim ersten Server-Boot testen (Alt-Datei bleibt für Rollback erhalten). QnA-Config nur repfadet (kein Migrations-Fallback, da optional/regenerierbar).
5. **`array<X>` vs `array<ref X>`:** Ich habe owning-Objekt-Arrays konsequent auf `array<ref X>` gehoben (Subagenten hatten Template-Args teils nur gestrichen → GC-Risiko). Konsistenz PluginAdminLog↔Utilities (`array<ref DME_Api_LogPlayerPos>`) hergestellt. Bei Compile-Iteration auf etwaige `array<X>`-Restfälle achten.
6. **Blocking `POST_now`** (DiscordRest sync-Getter) nur markiert, nicht entfernt — falls ungenutzt, streichen.
7. **Servertest zwingend** vor Live: M0-Selbsttest Server+Client, Token-Roundtrip gegen laufenden Node-Service, EntityStore Save/Load (Waffen/Magazine/Fahrzeuge), Discord/QnA optional.
8. **MapLink-Nachzug** ist eigene Phase (nutzt `UF*`-Symbole, nicht Teil dieses scripts-Merges).

---

## (g) Verbots-Sweep-Ergebnis (alle 63 Dateien)

| Prüfung | Ergebnis |
|---|---|
| `GetGame()` / `IsServer()` / `IsClient()` in echtem Code | **0** (1 Treffer nur in `/* */`-Beispielblock von ConfigBase) |
| `autoptr` in echtem Code | **0** (Rest nur in `/* */`-Doku-Beispielen & totem `GetWithPlainId`-Block) |
| `delete`-Statement | **0** (Treffer nur in Kommentar-Prosa) |
| `ref` auf Param/Return/Local | **0** |
| Mehrfach-Deklaration | **0** |
| stray `UApi`/`UniversalApi`/`UAPI`-Symbol (ohne Header/Interop) | **0** |
| Klammer-Balance `{}` / `()` je Datei | **63/63 ausgeglichen** |
| Header + daemonforge-Attribution | **63/63** |

*Erstellt durch EnScript-Merge-Agent (Phase 2+3), 2026-07-10. 8 parallele Modernisierungs-Subagenten für mechanische Datei-Gruppen; Core/Config/Rest/Merges/Mission + Verifikation direkt. Keine config.cpp/PBO/Node-Änderungen (Orchestrator-Schritt).*

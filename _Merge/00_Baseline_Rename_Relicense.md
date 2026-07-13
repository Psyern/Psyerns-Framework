# Phase 0 — Baseline: Inventar, Rename-Landkarte, Kollisions-Analyse & Relicense-Plan

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework (Psyern / Deadmans Echo)
**Datum:** 2026-07-09 · **Phase:** 0 (nur Analyse, keine Code-Änderungen)
**Quelle (fixiert):** `C:\Users\Administrator\Desktop\Psyerns_Framework\DeamonForge\DayZ-UniveralApi-1.3.2\` (Ordner `DayZ-UniveralApi-stable` ignoriert)
**Ziel:** `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\`
**Kollisions-Referenz:** `C:\Users\Administrator\Desktop\DME-WAR\`

## Festgelegte Parameter (Eigentümer-Entscheid, nicht verhandelbar)

1. Projektname bleibt **Psyerns_Framework**, Gesamtwerk wird **AGPL-3.0** (Eigentümer relizenziert sein eigenes MIT-Werk — zulässig und beabsichtigt).
2. Neuer Prefix für eingebrachten UApi-Code: **`DME_Api_`** (Sub-Namensraum; vermeidet Kollisionen mit DME-WARs `DME_`-Klassen). Bestehende `PF_`-Klassen bleiben `PF_`.
3. daemonforge-Copyright bleibt in allen übernommenen Dateien erhalten; ergänzt wird:
   - `Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)`
   - `Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo`
   - AGPL-§5-Änderungsnotiz `modified by Deadmans Echo, 2026` in jeder modifizierten daemonforge-Datei.

---

# 1. Inventar

## 1.1 Quelle: DayZ-UniveralApi-1.3.2

| Komponente | Dateien | LOC (Code, geschätzt) | Inhalt | Übernahme |
|---|---:|---:|---|---|
| `_UniversalApi/` | 68 | ~6.400 (.c: 6.391) | Haupt-EnScript-Mod: 1_Core, 3_Game, 4_World, 5_Mission, Common; 3 .edds-Icons + LICENSE + NOTICE | **JA** — Kern des Merges |
| `_UAPIBase/` | 17 | ~1.400 (.c: 1.382) | Basis-EnScript-Mod (minimale Defs, damit Drittmods kompilieren): 3_Game, 4_World, Common | **JA** — mit `_UniversalApi` verschmelzen |
| `DayZWebService/` | 34 | ~5.200 | Node.js-Backend (Express + MongoDB + Discord-Bot + JWT-Auth), 23 JS-Module, EJS-Templates, sample-config.json | **JA** |
| `DesktopManager/` | 71 | ~23.200 gesamt, davon nur ~735 eigener Electron-Code | Electron-GUI-Wrapper (electron-forge); enthält **Duplikat des WebService** (`src/WebServer/`, ~4.900 LOC) + gebündelte Libs (highlight.js, JSONEditor, water.css, Roboto) | **OPTIONAL** — Empfehlung: verwerfen oder später (Electron 18 EOL, Code-Duplikat) → Owner-Entscheid |
| `Schemas/` | 2 | ~275 | `MapLink.json` (Globals-Schema für MapLink-Mod, wird vom Service per Mod-Tag dynamisch geladen) + README | **JA** (Dateiname = Mod-Tag, nicht umbenennen) |
| `ModAddons/` | 1 | — | `UApiMass.pbo` — **kompiliertes Binär-PBO (Prefix `UApiMass`) ohne Quellcode im Repo** | **NEIN — ausschließen** (AGPL §6: Binary ohne Corresponding Source nicht distributierbar; siehe §6 Blocker) |
| Root | 8 | — | `uapi.gproj` (Workbench-Projekt), LICENSE (AGPL-3.0), NOTICE, README.md, CODE_OF_CONDUCT.md, SetupPDrive.bat, .gitignore | gproj ersetzen, LICENSE/NOTICE siehe §4 |
| `.github/` | 5 | — | CodeQL-Workflow, Issue-Templates, CODEOWNERS | **NEIN** (repo-spezifisch daemonforge) |

**EnScript gesamt: 73 .c-Dateien, ~7.770 LOC, 121 Klassen-/modded-Definitionen.**

Script-Modul-Struktur der Quelle (aus `config.cpp` beider Mods):

| Modul | _UAPIBase | _UniversalApi |
|---|---|---|
| engineScriptModule (1_Core) | `scripts/Common` | `scripts/1_Core` |
| gameScriptModule (3_Game) | `scripts/Common` + `scripts/3_Game` | `scripts/3_Game` |
| worldScriptModule (4_World) | `scripts/Common` + `scripts/4_World` | `scripts/4_World` |
| missionScriptModule (5_Mission) | `scripts/Common` | `scripts/5_Mission` |

`Common/Common.c` enthält nur Präprozessor-Defines (`#define UNIVERSALAPI`, `#define UNIVERSALAPI_STABLE`) und wird per config.cpp in **jedes** Modul eingebunden — dieser Mechanismus muss im Ziel nachgebildet werden (siehe §2.5).

`_UniversalApi/config.cpp`: `requiredAddons[] = {"UAPIBase", "JM_CF_Scripts"}` → **CF-Abhängigkeit** (CF-RPC wird genutzt). `_UAPIBase/config.cpp`: keine requiredAddons, aber `#define UNIVERSALAPI` (Config-Ebene).

## 1.2 Ziel: Psyerns_Framework

| Komponente | Dateien | Inhalt |
|---|---:|---|
| `scripts/` | 46 (45 .c + modded_inputs.xml, 5.566 LOC) | EnScript-Framework, **62 Klassendefinitionen, alle `PF_`-Prefix** (+ 2 modded MissionBase/MissionBaseWorld); Layer 3_Game/4_World/5_Mission (kein 1_Core!) |
| `WP-Plugin_Psyerns-Leaderboard/` | 48 | WordPress-Plugin (Leaderboard), PHP, Header „License: MIT" |
| `WP-Plugin_Psyerns_AuctionHouse/` | 45 | WordPress-Plugin (AuctionHouse), PHP, Header „License: MIT" |
| `psyerns-mods/` | 15 | WordPress-Plugin „Psyerns Mods Showreel", Header „License: **GPL-2.0+**" |
| `MISC/` | 118 | Standalone-PHP-API, WP-Plugin-Kopien (`psyerns-framework` MIT, `psyerns-mods` GPL-2.0+), Themes (statische HTML-Previews), Tools (2 PowerShell-Skripte), Orchestrierungs-Prompts (.md), Screenshots |
| `data/` | 12 | Mod-Daten (modded_inputs.xml etc.) |
| Root | — | `config.cpp` (CfgPatches `Psyerns_Framework`, requiredAddons `DZ_Data` + `JM_CF_Scripts`), `mod.cpp`, **LICENSE = MIT** („Copyright (c) 2026 Psyern / Deadmans Echo"), README.md (MIT-Badge Zeile 11), PsyernsFrameworkConfig.example.json |

Funktionale Überschneidung (Architektur-Hinweis, kein Namenskonflikt): Das Ziel hat bereits eigene REST-/Discord-/JSON-Infrastruktur (`PF_WebClient`, `PF_DiscordWebhook`, `PF_RestBase`, `PF_JsonBuilder`, eigener `PF_DiscordEmbed` …). UApi bringt eine zweite, parallel arbeitende REST-Schicht mit. Phase 0 plant **Koexistenz** (getrennte Namensräume `PF_` / `DME_Api_`); eine spätere Konsolidierung ist eigenes Projekt.

## 1.3 Node-Dependencies (aus package.json, Stand Quelle 1.3.2)

`DayZWebService/package.json` — name `dayzwebservice`, license **AGPL-3.0-or-later**:

| Paket | Version | Lizenz (npm-Metadaten) |
|---|---|---|
| @tensorflow-models/toxicity | ^1.2.2 | Apache-2.0 |
| @tensorflow/tfjs, tfjs-converter, tfjs-core | ^3.11.0 | Apache-2.0 |
| body-parser | ^1.20.0 | MIT |
| crypto | ^1.0.1 | ISC — **deprecated npm-Stub** (Node-Builtin), entfernen |
| discord.js | ^13.7.0 | Apache-2.0 |
| ejs / ejs-lint | ^3.1.8 / ^1.2.1 | Apache-2.0 / MIT |
| express | ^4.18.1 | MIT |
| express-rate-limit | ^6.4.0 | MIT |
| gamedig | ^4.0.4 | MIT |
| greenlock-express | ^4.0.3 | MPL-2.0 |
| https | ^1.0.0 | ISC — **deprecated npm-Stub**, entfernen |
| jsonwebtoken | ^8.5.1 | MIT — **Security: < 9.0.0 hat bekannte CVEs (u. a. CVE-2022-23529-Umfeld), Upgrade in Modernisierungs-Phase** |
| languagedetect | ^2.0.0 | MIT (bei Lockfile-Erstellung verifizieren) |
| log4js | ^6.3.0 | Apache-2.0 |
| mongodb | ^4.6.0 | Apache-2.0 |
| node-fetch | ^2.6.7 | MIT |
| node-wit | ^6.2.1 | MIT (bei Lockfile-Erstellung verifizieren) |
| saslprep | ^1.0.3 | MIT |
| serve-favicon | ^2.5.0 | MIT |
| websocket | ^1.0.34 | Apache-2.0 |
| dev: nodemon ^2.0.14, pkg ^5.5.2 | | MIT |

`DesktopManager/package.json` — name `universalapiwebservice`, productName `UniversalAPIWebService`, license **AGPL-3.0-or-later**; zusätzlich: ejs-electron ^2.1.1 (MIT), electron-squirrel-startup ^1.0.0 (Apache-2.0), trim-newlines ^4.0.2 (MIT), xmldom `github:xmldom/xmldom#0.7.0` (MIT), dev: electron ^18.2.3 + @electron-forge 6.0.0-beta.63 (MIT).

> Kein Paket ist GPLv2-only. `node_modules` ist **nicht** im Repo gebündelt → Lizenzpflichten entstehen erst beim Distributieren von Builds; bei Lockfile-Erstellung Lizenzfelder maschinell gegenprüfen (`license-checker`).

---

# 2. Rename-Landkarte

## 2.1 Schema & Regeln

- **`UApi<X>` → `DME_Api_<X>`** (Grundschema).
- Klassen ohne `UApi`-Prefix (`Universal*`, `U<X>`, `Api<X>`, sonstige) erhalten ebenfalls `DME_Api_`-Namen (Tabelle 2.2 Block B) — **keine unprefixten Klassen übernehmen**.
- **Vanilla-modded-Klassen behalten ihre Namen** (Tabelle 2.3); neue/übernommene Member-Variablen darin bekommen in der Umsetzungs-Phase den Prefix `m_DME_Api_…` (CLAUDE.md-Regel für modded classes).
- **Serialisierte Member-Variablen (JSON-Wire/Config-Keys) werden NIEMALS umbenannt** — `JsonFileLoader`/RestApi serialisieren Feldnamen 1:1 (siehe §2.8).
- Dateinamen folgen den neuen Klassennamen (z. B. `UniversalApi.c` → `DME_Api_Core.c`); Ordner `UApi/` → `DME_Api/`.

## 2.2 EnScript-Klassen: alt → neu (104 Klassen + 1 typedef)

Quelle der Liste: `grep -rE "^\s*(class|modded class) " --include="*.c" _UniversalApi _UAPIBase` (121 Definitionen; davon 104 eigene Klassen, 15 vanilla-modded, 2 cross-PBO-modded Duplikate eigener Klassen).

### Block A — `UApi*`-Klassen (83)

| Alt | Neu |
|---|---|
| UApiAmmoData | DME_Api_AmmoData |
| UApiAPIEndpoint | DME_Api_APIEndpoint |
| UApiAuthCallBack | DME_Api_AuthCallBack |
| UApiBaseEndpoint | DME_Api_BaseEndpoint |
| UApiCallback\<Class T\> | DME_Api_Callback\<Class T\> |
| UApiCallbackBase | DME_Api_CallbackBase |
| UApiCallbackLoader\<Class T\> | DME_Api_CallbackLoader\<Class T\> |
| UApiChannelCreateOptions | DME_Api_ChannelCreateOptions |
| UApiChannelOptions | DME_Api_ChannelOptions |
| UApiChannelPermissions | DME_Api_ChannelPermissions |
| UApiChannelUpdateOptions | DME_Api_ChannelUpdateOptions |
| UApiConfigBase | DME_Api_ConfigBase |
| UApiCreateChannelObject | DME_Api_CreateChannelObject |
| UApiCryptoConvertRequest | DME_Api_CryptoConvertRequest |
| UApiCryptoConvertResult | DME_Api_CryptoConvertResult |
| UApiCryptoRequest | DME_Api_CryptoRequest |
| UApiCryptoResults | DME_Api_CryptoResults |
| UApiDBCallBack | DME_Api_DBCallBack |
| UApiDBEndpoint | DME_Api_DBEndpoint |
| UApiDBGlobalEndpoint | DME_Api_DBGlobalEndpoint |
| UApiDBHandler\<Class T\> | DME_Api_DBHandler\<Class T\> |
| UApiDBHandlerBase | DME_Api_DBHandlerBase |
| UApiDBNestedCallBack | DME_Api_DBNestedCallBack |
| UApiDBQuery | DME_Api_DBQuery |
| UApiDBQueryUpdate | DME_Api_DBQueryUpdate |
| UApiDiscordAuthor | DME_Api_DiscordAuthor |
| UApiDiscordBasicMessage | DME_Api_DiscordBasicMessage |
| UApiDiscordCallBack | DME_Api_DiscordCallBack |
| UApiDiscordChannelFilter | DME_Api_DiscordChannelFilter |
| UApiDiscordEmbed | DME_Api_DiscordEmbed |
| UApiDiscordField | DME_Api_DiscordField |
| UApiDiscordFooter | DME_Api_DiscordFooter |
| UApiDiscordImage | DME_Api_DiscordImage |
| UApiDiscordMessage | DME_Api_DiscordMessage |
| UApiDiscordMessagesResponse | DME_Api_DiscordMessagesResponse |
| UApiDiscordMute | DME_Api_DiscordMute |
| UApiDiscordNickname | DME_Api_DiscordNickname |
| UApiDiscordObject | DME_Api_DiscordObject |
| UApiDiscordRoleReq | DME_Api_DiscordRoleReq |
| UApiDiscordStatusObject | DME_Api_DiscordStatusObject |
| UApiDiscordUser | DME_Api_DiscordUser |
| UApiEntityStore | DME_Api_EntityStore |
| UApiForwarder | DME_Api_Forwarder |
| UApiGlobalHandler\<Class T\> | DME_Api_GlobalHandler\<Class T\> |
| UApiGlobalHandlerBase | DME_Api_GlobalHandlerBase |
| UApiHeaders | DME_Api_Headers |
| UApiJSONCallback | DME_Api_JSONCallback |
| UApiJSONHandler\<Class T\> | DME_Api_JSONHandler\<Class T\> |
| UApiLog | DME_Api_Log |
| UApiLogBase | DME_Api_LogBase |
| UApiLogKilled | DME_Api_LogKilled |
| UApiLogMisc | DME_Api_LogMisc |
| UApiLogPlayerPos | DME_Api_LogPlayerPos |
| UApiMetaData | DME_Api_MetaData |
| UApiObject_Base | DME_Api_Object_Base |
| UApiQnACallBack | DME_Api_QnACallBack |
| UApiQnAMakerServerAnswers | DME_Api_QnAMakerServerAnswers |
| UApiQueryBase | DME_Api_QueryBase |
| UApiQueryObject | DME_Api_QueryObject |
| UApiQueryResult\<Class T\> | DME_Api_QueryResult\<Class T\> |
| UApiQueryUpdateResponse | DME_Api_QueryUpdateResponse |
| UApiQuestionRequest | DME_Api_QuestionRequest |
| UApiRandomNumberRequest | DME_Api_RandomNumberRequest |
| UApiRandomNumberResponse | DME_Api_RandomNumberResponse |
| UApiScriptBase | DME_Api_ScriptBase |
| UApiScriptTest | DME_Api_ScriptTest |
| UApiServerStatus | DME_Api_ServerStatus |
| UApiSilentCallBack | DME_Api_SilentCallBack |
| UApiStatus | DME_Api_Status |
| UApiStatusCallBack | DME_Api_StatusCallBack |
| UApiTextObject | DME_Api_TextObject |
| UApiToxicityResponse | DME_Api_ToxicityResponse |
| UApiTransaction | DME_Api_Transaction |
| UApiTransactionCallBack | DME_Api_TransactionCallBack |
| UApiTransactionResponse | DME_Api_TransactionResponse |
| UApiTranslation | DME_Api_Translation |
| UApiTranslationRequest | DME_Api_TranslationRequest |
| UApiTranslationResponse | DME_Api_TranslationResponse |
| UApiUpdateChannelObject | DME_Api_UpdateChannelObject |
| UApiUpdateData | DME_Api_UpdateData |
| UApiUpdateResponse | DME_Api_UpdateResponse |
| UApiValidatedTransaction | DME_Api_ValidatedTransaction |
| UApiZoneHealthData | DME_Api_ZoneHealthData |

### Block B — Klassen ohne `UApi`-Prefix (21) + typedef

| Alt | Neu | Anmerkung |
|---|---|---|
| ApiAuthToken | DME_Api_AuthToken | |
| ApiQueryBase | **DME_Api_SampleQuery** | Sample-Callback (`CallBacks/Samples/ApiQuery.c`). **Sonderregel:** naives Schema ergäbe `DME_Api_QueryBase` = Kollision mit Rename von `UApiQueryBase`! |
| CheckIfHasDiscord | DME_Api_SampleCheckIfHasDiscord | Sample-Callback (`CallBacks/CheckIfDiscord.c`) |
| DSPerms | DME_Api_DSPerms | Discord-Permission-Konstanten; **String-Werte unverändert lassen** (Discord-API-Wire) |
| QnAAnswer | DME_Api_QnAAnswer | |
| QnAMakerServerAnswer | DME_Api_QnAMakerServerAnswer | |
| SimpleValueStore | DME_Api_SimpleValueStore | |
| StatusObject | DME_Api_StatusObject | Basisklasse fast aller Responses; Wire-Felder `Status`/`Error` **nicht** anfassen |
| UCurrency | DME_Api_Currency | |
| UCurrencyValue | DME_Api_CurrencyValue | |
| `typedef array<autoptr UCurrencyValue> UCurrencyBase` | `typedef array<autoptr DME_Api_CurrencyValue> DME_Api_CurrencyBase` | `_UAPIBase/.../Currencys.c:21` |
| ULoggerBase | DME_Api_LoggerBase | |
| ULoggerBaseInstance | DME_Api_LoggerBaseInstance | in `_UAPIBase` definiert, in `_UniversalApi` **ge-modded** → beim Single-PBO-Merge textuell zusammenführen (§6.4) |
| ULoggerObject | DME_Api_LoggerObject | |
| UniversalApi | **DME_Api_Core** | Hauptklasse |
| UniversalApiConfig | **DME_Api_Config** | |
| UniversalDiscordRest | DME_Api_DiscordRest | |
| UniversalDSEndpoint | DME_Api_DSEndpoint | |
| UniversalRest | DME_Api_Rest | |
| UpdateOpts | DME_Api_UpdateOpts | **String-Werte (`set`,`pull`,`push`,`unset`,`mul`,`rename`,`pullAll`) unverändert** (MongoDB-Wire) |
| UScriptExec | DME_Api_ScriptExec | |
| UUtil | DME_Api_Util | |

### Block C — Globale Funktionen, Variablen, Konstanten, Defines

| Art | Alt | Neu | Anmerkung |
|---|---|---|---|
| Global-Accessor | `static UniversalApi UApi()` | `static DME_Api_Core DME_Api()` | Meistgenutztes API-Symbol (`UApi().…`). **Nicht** `GetUApi…` nennen — `GetUApi()` ist die Vanilla-Input-API (`UAInputAPI`)! |
| Global-Accessor | `static UniversalApiConfig UApiConfig()` | `static DME_Api_Config DME_Api_GetConfig()` | Name ≠ Klassenname `DME_Api_Config` (sonst Funktions-/Klassen-Konflikt) |
| Global-Accessor | `static UApiQnAMakerServerAnswers UApiQnAMaker()` | `static DME_Api_QnAMakerServerAnswers DME_Api_GetQnAMaker()` | |
| Globale Variable | `ref UniversalApiConfig m_UniversalApiConfig` (ConfigLoader.c:57, File-Scope) | `ref DME_Api_Config m_DME_Api_Config` | |
| Konstanten (1_Core/Constants.c) | `UAPI_VERSION`, `UAPI_SUCCESS`, `UAPI_EMPTY`, `UAPI_NOTSETUP`, `UAPI_TIMEOUT`, `UAPI_CLIENTERROR`, `UAPI_SERVERERROR`, `UAPI_ERROR`, `UAPI_JSONERROR`, `UAPI_NOTFOUND`, `UAPI_TOOEARLY`, `UAPI_UNAUTHORIZED`, `UAPI_DBSUCCESS`, `UAPI_DBEMPTY`, `UAPI_DBTIMEOUT`, `UAPI_DBSERVERERROR`, `UAPI_DBUNAUTHORIZED`, `UAPI_DBERROR`, `UAPI_DBTOOEARLY` (19) | `DME_API_VERSION`, `DME_API_SUCCESS`, … (Schema `UAPI_` → `DME_API_`) | Werte (HTTP-Codes) unverändert; `UAPI_VERSION`-**Wert** „1.3.2" behalten (Version-Handshake mit Service, §2.8) |
| Konstanten (unprefixt!) | `PLAYER_DB` (100), `OBJECT_DB` (101) | `DME_API_PLAYER_DB`, `DME_API_OBJECT_DB` | Bisher global unprefixt = latentes Kollisionsrisiko mit anderen Mods — Rename ist Pflicht |
| Script-Define | `#define UNIVERSALAPI` (Common.c beider Mods) | `#define DME_API` | **Breaking** für Drittmods, die `#ifdef UNIVERSALAPI` prüfen — bewusster Rebrand-Bruch, dokumentieren (§6.5) |
| Script-Define | `#define UNIVERSALAPI_STABLE` (_UAPIBase) | `#define DME_API_STABLE` | dito |
| Config-Define | `#define UNIVERSALAPI` (config.cpp _UAPIBase) | entfällt | nur Config-Preprocessing, vestigial |

### Block D — CF-RPC-Namen (interner Wire, beide Seiten werden synchron ausgeliefert → Rename sicher)

| Art | Alt | Neu |
|---|---|---|
| RPCManager-Mod-Key | `"UAPI"` | `"DME_Api"` |
| Handler | `"RPCUniversalApiConfig"` | `"RPC_DME_Api_Config"` |
| Handler | `"RPCRequestAuthToken"` | behalten (kein Brand im Namen) |
| Handler | `"RPCRequestQnAConfig"` | behalten |
| Handler | `"RPCRequestRetry"` | behalten |

> Hinweis: Alt-Client ↔ Neu-Server-Mischbetrieb existiert nicht (der Mod wird als neues PBO unter neuem Namen verteilt), daher kein Kompat-Zwang.

## 2.3 Vanilla-modded-Klassen (Namen bleiben — 15 Stück)

`BloodContainerBase`, `CarScript` (in beiden Quell-PBOs → beim Merge zu **einer** modded-Definition zusammenführen), `ChatInputMenu`, `DayZGame`, `Edible_Base`, `ItemBase` (dito 2×), `Math` (enMath.c), `MissionBase`, `MissionBaseWorld`, `MissionGameplay`, `MissionServer`, `PlayerBase` (2×), `PluginAdminLog`, `TransmitterBase`, `Weapon_Base`.

Pflicht in Umsetzungs-Phase: Member in diesen modded classes auf `m_DME_Api_…` prefixen (z. B. `Math.m_QRandomNumbers` → `m_DME_Api_QRandomNumbers`), `override` + `super`-Aufrufe prüfen. **Achtung:** Ziel moddet bereits `MissionBase`/`MissionBaseWorld` (PF_) — beide modded-Definitionen koexistieren als getrennte Mods nicht mehr, im Single-PBO müssen PF_- und DME_Api-Anteile in **eine** modded-Definition pro Klasse (siehe §6.4).

## 2.4 `UniversalApi`-Vorkommen außerhalb von Klassennamen

| Artefakt | Ist | Plan |
|---|---|---|
| CfgPatches | `UAPIBase`, `UniversalApi` | entfallen — aufgehen in bestehendem CfgPatches `Psyerns_Framework`. **Breaking** für Drittmods mit `requiredAddons[]={"UAPIBase"}` (§6.5) |
| CfgMods | `UAPIBase`, `UniversalApi` (author DaemonForge, v1.3.2) | entfallen — CfgMods `Psyerns_Framework`; Credits-Feld ergänzen: „Contains DayZ-UniversalApi by DaemonForge (AGPL-3.0)" |
| PBO-/Ordnernamen | `_UAPIBase`, `_UniversalApi` | Inhalte nach `Psyerns_Framework/scripts/<Layer>/DME_Api/…` (ein PBO) |
| Workbench-Projekt | `uapi.gproj` (referenziert `_UniversalApi`-, `_UAPIBase`-, `JM/CF`- und MapLink-Pfade) | nicht übernehmen; neues `Psyerns_Framework.gproj` in späterer Phase |
| Icon-Pfade im Code | `"_UniversalApi/images/Bot.edds"` (QnAMakerConfig.c:51), `"_UniversalApi\\images\\info.edds"` (Utilities.c:80, :88) + `warning.edds` | Images nach `Psyerns_Framework/images/`, 3 Code-Stellen anpassen |
| Server-Config-Pfad | `$profile:UApi\UniversalApi.json` (ConfigLoader.c) | **Entscheid nötig:** Option A (empfohlen): neuer Pfad `$profile:PsyernsFramework\DME_Api.json` mit Lese-Fallback/Auto-Migration vom Alt-Pfad; Option B: Alt-Pfad behalten (maximale Drop-in-Kompat). **Keys unverändert** (§2.8) |
| Node package name / productName | `dayzwebservice` / `UniversalAPIWebService` | `psyerns-api-service` / `Psyerns Framework API Service` (kein Wire-Impact) |
| Node-Zielordner | `DayZWebService/` | `Psyerns_Framework/ApiService/` (Vorschlag; Reponame frei) |
| README/Doku | „Universal Api" | Psyerns Framework / DME_Api + Herkunftsvermerk |
| SetupPDrive.bat | dev-Tool mit UApi-Pfaden | verwerfen oder anpassen |

## 2.5 Ziel-Struktur & config.cpp-Änderungen (Plan)

```
Psyerns_Framework/
  scripts/1_Core/DME_Api/            ← NEU (Constants.c, enMath.c)
  scripts/Common/DME_Api/Defines.c   ← NEU (#define DME_API, DME_API_STABLE)
  scripts/3_Game/DME_Api/…           ← _UAPIBase/3_Game + _UniversalApi/3_Game (gemergt)
  scripts/4_World/DME_Api/…          ← _UAPIBase/4_World + _UniversalApi/4_World (gemergt)
  scripts/5_Mission/DME_Api/…        ← _UniversalApi/5_Mission
  images/                            ← Bot/info/warning.edds (+ .meta)
  ApiService/                        ← DayZWebService (Node)
  Schemas/                           ← MapLink.json (Name unverändert)
```

`config.cpp`/`mod.cpp` des Ziels: `engineScriptModule` (1_Core) **ergänzen** (fehlt aktuell) und `scripts/Common` in **jedes** Modul-files[]-Array aufnehmen (Defines-Sichtbarkeit pro Modul, wie Quelle es via _UAPIBase löste). requiredAddons bleiben `{"DZ_Data","JM_CF_Scripts"}` — CF wird von UApi-Code (RPCManager) gebraucht und ist schon Pflicht.

## 2.6 Node-Modul-/Routen-Namen

Interne JS-Dateinamen (`app.js`, `player.js`, `Object.js`, `discordConnector.js`, …) können bleiben (kein Publikums-Symbol). **Alle HTTP-Routen-Mounts und Sub-Pfade bleiben unverändert** (siehe §2.8) — nur Branding-Strings (Konsolen-Banner, productName, Fenster-Titel) werden getauscht.

## 2.7 WP-Plugins / PHP (Ziel-Bestand)

Kein Rename nötig (eigener `pf_`/`psyerns`-Namensraum, keine Berührung mit UApi-Symbolen).

## 2.8 INTEROP-KRITISCH — NICHT umbenennen (Empfehlung: behalten)

Diese Strings sind Wire-/Persistenz-Format. Rename würde bestehende Server-Datenbanken, laufende Web-Services, WordPress-Anbindungen oder Client-Flows brechen:

| Kategorie | Werte (behalten!) | Fundort |
|---|---|---|
| **HTTP-Routen (Mounts)** | `/Object`, `/Player`, `/Globals` **und Alias `/Gobals`** (Tippfehler, absichtlich für Rückwärtskompat!), `/GetAuth`, `/Status`, `/QnAMaker`, `/QnA`, `/Forward`, `/Logger`, `/Discord`, `/Wit`, `/LUIS`, `/Translate`, `/ServerQuery`, `/Toxicity`, `/Random`, `/Crypto` | `DayZWebService/app.js:90–107` |
| **Endpunkt-Sub-Pfade (EnScript-Seite)** | `Object/Save|Load|Update|Query|Transaction`, `Player/…`, `Globals/…`, `Query/Update/`, `Channel/Create|Send|Edit|Delete|Messages`, `Discord/Check|Get|GetWithPlainId|CheckRole|AddRole|RemoveRole|Channel/…`, `Crypto/Price|Convert`, `Random/Full`, `ServerQuery/Status/`, `Logger/One|Many`, `GetAuth/<guid>` | `_UniversalApi/scripts/3_Game/UApi/Endpoints/*`, `UniversalRest.c:38` |
| **Auth-Wire-Format** | JWT (`jsonwebtoken`), signiert mit ServerAuth, `expiresIn: 2800`; Transport-Hack: EnScript sendet Token als **Content-Type-Header** (`ctx.SetHeader(UApi().GetAuthToken())`, UniversalRest.c:20), Node-Middleware `ExtractAuthKey` (utils.js:274) verschiebt Nicht-Standard-Content-Type nach `req.headers['auth-key']` und setzt Content-Type auf `application/json`. Header-Name **`auth-key`** und dieser Mechanismus dürfen nicht verändert werden | utils.js, AuthChecker.js, UniversalRest.c |
| **MongoDB** | Collections `Players`, `Objects`, `Globals`, `Logs`, `QnAMaker`; DB-Name-Default `DayZ`; Dokument-Feldnamen | alle Router-JS |
| **JSON-Wire-Felder (EnScript-Member serialisierter Klassen)** | z. B. `Status`, `Error` (StatusObject), `Version`, `Discord`, `Translate`, `Wit`, `QnA`, `LUIS` (UApiStatus), Felder aller `UApiObject_Base`-/Response-Klassen, `Key`/`Value` (UApiHeaders) | Objects/*.c — **Klassen umbenennen ja, Member nie** |
| **Node config.json-Keys** | `DBServer`, `DB`, `ServerAuth`, `AllowClientWrite`, `IP`, `Port`, `LogToFile`, `Certificate`, `CertificateKey`, `CheckForNewVersion`, `CreateIndexes`, `RateLimitWhiteList`, `Discord{Client_Id,…}`, `Wit`, `LUIS`, `QnA`, `Translate`, **`LetsEncypt` (sic — Tippfehler ist Bestands-Key, nicht fixen!)**, `debug` | sample-config.json, configLoader.js |
| **EnScript-Server-Config-Keys** | `ConfigVersion`, `ServerURL`, `ServerID`, `ServerAuth`, `QnAEnabled`, `EnableBuiltinLogging`, `PromptDiscordOnConnect` (JSON-Keys in `UniversalApi.json`) | ConfigLoader.c |
| **Versions-Handshake** | `UAPI_VERSION`-Wert `"1.3.2"` ↔ Service-Version (UApiStatus.CheckVersion vergleicht major.minor.patch) — Konstante darf `DME_API_VERSION` heißen, **Wert/Format bleibt** | Constants.c, Status.js |
| **Discord-/Mongo-Operator-Strings** | `DSPerms`-Werte (`ADD_REACTIONS`, …), `UpdateOpts`-Werte (`set`, `pull`, `push`, `unset`, `mul`, `rename`, `pullAll`) | Constants.c |
| **Schema-Dateinamen** | `Schemas/MapLink.json` — Dateiname = Mod-Tag, wird vom Service dynamisch per HTTP geladen | Schemas/README.MD |
| **OAuth-Flow** | Discord-OAuth-Redirect-Pfade/Templates (`discordLogin.ejs`, …) | templates/ |

## 2.9 Kollisionsgefahren INNERHALB der Rename-Map (verifiziert)

1. `ApiQueryBase` und `UApiQueryBase` → naives Schema ergäbe beide `DME_Api_QueryBase`. **Gelöst** durch Sonderregel (Sample → `DME_Api_SampleQuery`).
2. Accessor `UApiConfig()` vs. Klasse `UniversalApiConfig` → beide dürfen nicht auf denselben Namen `DME_Api_Config` mappen. **Gelöst**: Klasse `DME_Api_Config`, Funktion `DME_Api_GetConfig()`.
3. Vanilla-Global `GetUApi()` (= `UAInputAPI`, Input-Bindings!) existiert bereits in DayZ und wird u. a. von DME-WAR 2× genutzt — neue Accessor-Namen meiden jede `GetUApi`-Ähnlichkeit. **Gelöst**: `DME_Api()`.
4. Ziel-Framework hat `PF_DiscordEmbed` — UApi bringt `UApiDiscordEmbed` → `DME_Api_DiscordEmbed`. Kein Konflikt (verschiedene Namen), aber inhaltliche Dopplung — bewusst toleriert.

---

# 3. Kollisions-Analyse gegen DME-WAR

**Methode:** `grep -rE "^\s*(class|modded class) DME_…" --include="*.c"` über `C:\Users\Administrator\Desktop\DME-WAR`, dedupliziert; Abgleich case-sensitiv **und** case-insensitiv gegen alle geplanten `DME_Api_*`-Zielnamen.

| Messwert | Ergebnis |
|---|---|
| In DME-WAR definierte `DME_*`-Klassen | **321** (eindeutige Namen; u. a. `DME_ConfigManager`, `DME_Logger`, `DME_FactionService`, `DME_ElectroBall*`, `DME_ContainerStore`, …) |
| Davon mit Prefix `DME_Api` | **0** (auch case-insensitiv 0) |
| Kollisionen mit geplanten `DME_Api_*`-Namen | **KEINE** ✅ |
| Nutzung von daemonforge-UniversalApi in DME-WAR | **Keine.** Die 2 grep-Treffer (`DME_Startscreen/Scripts/3_Game/Widgets/PsyernsScriptedMenu.c:268`, `DME_War/scripts/5_Mission/MissionGameplay.c:145`) sind `GetUApi()` = **Vanilla-Input-API** (`UAInputAPI`), kein Bezug zu UniversalApi |

**Fazit:** Der Sub-Namensraum `DME_Api_` ist frei. Erwartung („keine, da Sub-Namensraum") **verifiziert**. Regel für die Zukunft: DME-WAR vergibt keine neuen Klassen mit `DME_Api`-Prefix (Namensraum reserviert für Psyerns_Framework-API-Schicht).

Zusätzlich geprüft: Ziel-Framework selbst (62 Klassen, alle `PF_*`) — ebenfalls keine Kollision mit `DME_Api_*`.

---

# 4. Relicense-Plan

## 4.1 Ist-Zustand Lizenz-Dateien

| Datei | Inhalt | Anmerkung |
|---|---|---|
| Quelle `/LICENSE` | **AGPL-3.0** (Standard-FSF-Volltext, 34.481 Bytes, keine eigene Copyright-Zeile eingesetzt) | maßgebliche Lizenz des Mods |
| Quelle `/_UniversalApi/LICENSE`, `/_UAPIBase/LICENSE` | AGPL-3.0 (identisch, 34.481 Bytes) | |
| Quelle `/DayZWebService/LICENSE`, `/DesktopManager/src/WebServer/LICENSE` | **GPL-3.0** (35.129 Bytes) | ⚠ Widerspricht `package.json` (`"license": "AGPL-3.0-or-later"`) — upstream-Inkonsistenz desselben Urhebers. Behandlung: package.json-Deklaration (AGPL-3.0-or-later) als Intention werten; selbst wenn GPL-3.0 gälte, ist die Kombination mit AGPL-3.0 über die beidseitige §13-Brücke ausdrücklich erlaubt. Dokumentieren, nicht stillschweigend löschen. |
| Quelle `/NOTICE`, `/_UniversalApi/NOTICE`, `/_UAPIBase/NOTICE` | Attribution (Daemon Forge / Kevin Hoddinott, Repo-Link, Donate-Link) **plus Repacking-Verbot** („For this mod right now it is not allowed") | ⚠ Das Repack-Verbot ist eine „further restriction" i. S. v. AGPL **§7 Abs. 4** — als Empfänger dürfen wir sie entfernen („If the Program as you received it … contains a … term that is a further restriction, you may remove that term."). Attribution-Kern bleibt erhalten (§4.4). |
| Quelle `package.json` (beide) | `AGPL-3.0-or-later` | |
| Ziel `/LICENSE` | **MIT**, „Copyright (c) 2026 Psyern / Deadmans Echo" (1.079 Bytes) | wird ersetzt |
| Ziel README.md | MIT-Badge (Zeile 11) | wird ersetzt |
| Ziel WP-Plugin-Header | Leaderboard: MIT · AuctionHouse: MIT · psyerns-mods: **GPL-2.0+** | siehe §5 |
| Quell-Quelldateien (.c/.js) | **keinerlei** Copyright-Header in Dateien (0 Treffer) | unsere neuen Header sind die ersten In-File-Notices |

## 4.2 Ziel-Lizenz des Gesamtwerks

- `Psyerns_Framework/LICENSE` → **vollständiger AGPL-3.0-Text** (Übernahme des FSF-Standardtexts, z. B. 1:1 aus Quelle `/LICENSE`).
- SPDX für das Gesamtwerk: **`AGPL-3.0-only`** (konservativer gemeinsamer Nenner: Mod-LICENSE der Quelle ist AGPL-3.0 ohne „or later"-Zusage; „or-later"-Teile (Node) dürfen in ein AGPL-3.0-only-Gesamtwerk einfließen). Option: `AGPL-3.0-or-later`, falls der Orchestrator die package.json-Deklaration auf alles ausdehnen will — dann für den EnScript-Teil (nur „AGPL-3.0") Rückfrage/Nachweis nötig. **Empfehlung: AGPL-3.0-only.**
- Rechtsgrundlage der Relizenzierung des Ziel-Bestands: Psyern/Deadmans Echo ist alleiniger Urheber des bisherigen MIT-Codes → darf sein eigenes Werk unter AGPL-3.0 neu lizenzieren. Bereits veröffentlichte MIT-Stände bleiben für Dritte MIT (nicht rückholbar) — betrifft nur Alt-Releases, kein Handlungsbedarf.
- Der übernommene daemonforge-Code **bleibt AGPL-3.0** (keine Umlizenzierung fremden Codes — nur Integration unter kompatibler Gesamtlizenz).

## 4.3 Header-Vorlagen

**(a) Übernommene daemonforge-Datei (modifiziert — Regelfall, gilt ab dem Rename):**

```c
/**
 * Psyerns_Framework — DME_Api
 *
 * Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)
 *   https://github.com/daemonforge/DayZ-UniveralApi
 * Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo
 * Modified by Deadmans Echo, 2026.   [AGPL §5(a)-Änderungsnotiz]
 *
 * This file is part of Psyerns_Framework and is licensed under the
 * GNU Affero General Public License v3.0. See LICENSE in the repo root.
 * SPDX-License-Identifier: AGPL-3.0-only
 */
```

*(Für .js-Dateien identisch mit `//`- bzw. `/* */`-Kommentar; für .php mit PHPDoc-Block.)*

**(b) Unverändert übernommene daemonforge-Datei (Ausnahme, z. B. Schemas):** wie (a), aber ohne die Zeilen „Modifications …" / „Modified by …".

**(c) Eigene (neue/bestehende PF_-) Datei:**

```c
/**
 * Psyerns_Framework
 * Copyright (c) 2026 Psyern / Deadmans Echo
 * SPDX-License-Identifier: AGPL-3.0-only
 */
```

> Da praktisch **jede** übernommene Datei durch den `DME_Api_`-Rename modifiziert wird, ist Vorlage (a) der Standard. Die §5(a)-Notiz („Modified by Deadmans Echo, 2026") mit Datum erfüllt die AGPL-Pflicht „carry prominent notices stating that you modified it, and giving a relevant date".

## 4.4 NOTICE (neu, Repo-Root) — Entwurf

```
Psyerns_Framework
Copyright (c) 2026 Psyern / Deadmans Echo
Licensed under the GNU Affero General Public License v3.0 (see LICENSE).

This project incorporates and modifies:
  DayZ-UniversalApi (v1.3.2)
  Original work Copyright (c) daemonforge (Kevin Hoddinott, Daemon Forge Developments)
  https://github.com/daemonforge/DayZ-UniveralApi — AGPL-3.0
  Modified by Deadmans Echo, 2026.
  Support the original author: https://www.paypal.me/DaemonForge

Bundled third-party components (DesktopManager, falls übernommen):
  highlight.js — BSD-3-Clause, Copyright (c) 2006 Ivan Sagalaev
  JSONEditor — Apache-2.0
  water.css — MIT, Copyright (c) Kognise
  Roboto fonts — Apache-2.0, Copyright Google

Source offer (AGPL §13): The complete corresponding source of this work,
including the network-facing API service, is available at:
  <REPO-URL EINSETZEN>
```

- Das upstream-NOTICE-**Repack-Verbot wird nicht übernommen** (AGPL §7: „further restriction" → entfernbar). Attribution + Donate-Link bleiben aus Respekt/Transparenz erhalten. Diese Entscheidung wird hier dokumentiert (kein stilles Löschen).
- Alte NOTICE-Dateien der Quelle werden nicht mitkopiert; ihr Attribution-Inhalt geht im neuen NOTICE auf.

## 4.5 README-Ergänzung (Plan)

1. Badge `License-MIT` (Zeile 11) → `License-AGPL--3.0`.
2. Abschnitt **„Lizenz & Herkunft"**: AGPL-3.0-Gesamtwerk; enthält modifizierten DayZ-UniversalApi-Code (daemonforge, AGPL-3.0); Copyright-Vermerke gem. §4.3; Hinweis, dass frühere Releases MIT waren.
3. Abschnitt **„Quellcode-Angebot (AGPL §13)"**: Wer den API-Service über ein Netzwerk betreibt und modifiziert, muss Nutzern die Korresponding Source anbieten. Konkrete Maßnahme (Umsetzungs-Phase): Der Node-Service liefert auf der Root-/Status-Route einen `Source: <repo-url>`-Link bzw. ein `source`-Feld im Status-JSON aus.
4. WP-Plugin-Abschnitte: Lizenzstatus je Plugin klarstellen (§5).

## 4.6 Umsetzungs-Reihenfolge Relicense (für spätere Phase)

1. `LICENSE` (Ziel) durch AGPL-3.0-Volltext ersetzen; `LICENSE.MIT-history` **nicht** nötig (Git-History genügt).
2. Neues `NOTICE` (§4.4) anlegen.
3. README-Änderungen (§4.5).
4. Header-Rollout: (a) auf alle übernommenen Dateien, (c) auf eigene Dateien (optional, empfohlen mindestens SPDX-Zeile).
5. package.json des Service: `"license": "AGPL-3.0-only"` (oder or-later je Entscheid), `"author"`-/`"repository"`-Felder aktualisieren, daemonforge-Attribution in package.json-`"contributors"` oder README des Service.
6. WP-Plugin-Header: Leaderboard/AuctionHouse von MIT auf `GPL-2.0-or-later` **oder** AGPL umstellen? → **Empfehlung: unverändert lassen** (eigenständige Werke, MIT ist WordPress-kompatibel); nur wenn Code aus dem AGPL-Kern hineinkopiert wird, müssen sie AGPL-kompatibel nachziehen.

---

# 5. Dep-Lizenz-Scan (AGPL-3.0-Kompatibilität)

Legende: 🟢 kompatibel · 🔴 inkompatibel/blockierend · ⚠ Hinweis

## 5.1 Im Repo gebündelte Komponenten (Quelle)

| Komponente | Lizenz (exakt zitiert/Datei) | Urteil |
|---|---|---|
| `_UniversalApi`, `_UAPIBase`, Root | AGPL-3.0 (LICENSE-Volltext) | 🟢 Basislizenz des Merges |
| `DayZWebService/`, `DesktopManager/src/WebServer/` | LICENSE-Datei: **GPL-3.0**; package.json: **„AGPL-3.0-or-later"** | 🟢 — beide Lesarten kompatibel (GPLv3↔AGPLv3 §13-Brücke); ⚠ upstream-Widerspruch im Plan-Dok festgehalten (§4.1) |
| `DesktopManager/src/lib/highlight/` | **BSD 3-Clause** („Copyright (c) 2006, Ivan Sagalaev") | 🟢 |
| `DesktopManager/src/lib/JsonEditor/` | **Apache-2.0** (Header in jsoneditor.min.js) | 🟢 (einweg-kompatibel in (A)GPLv3) |
| `DesktopManager/src/styles/water/` (water.css min-Builds) | **MIT** (upstream Kognise; ⚠ kein Lizenz-Header im Bundle → Attribution im NOTICE ergänzen) | 🟢 |
| `DesktopManager/src/fonts/` (Roboto woff/woff2) | **Apache-2.0** (Google Roboto; ⚠ keine Lizenzdatei gebündelt → NOTICE) | 🟢 |
| `ModAddons/UApiMass.pbo` | **unbestimmbar** — kompiliertes PBO (Prefix `UApiMass`, enthält `scripts/4_World`-Binary), **keine Quelle im Repo** | 🔴 **AUSSCHLIESSEN** — ohne Corresponding Source nicht AGPL-§6-konform distributierbar |

## 5.2 WordPress-Plugins (Ziel-Bestand)

| Plugin | Header (exakt) | Urteil |
|---|---|---|
| `WP-Plugin_Psyerns-Leaderboard/psyerns-framework.php:9` | `License: MIT` + `License URI: https://opensource.org/licenses/MIT` | 🟢 eigenes Werk; MIT ist GPL-/AGPL-kompatibel; darf bleiben oder auf AGPL umgestellt werden |
| `WP-Plugin_Psyerns_AuctionHouse/psyerns-auctionhouse.php:9` | `License: MIT` + URI | 🟢 dito |
| `psyerns-mods/psyerns-mods.php:10` (+ Kopie unter `MISC/wordpress-plugin/`) | `License: GPL-2.0+` | 🟢 — **„GPL-2.0+" = „GPLv2 or later"**, das ist NICHT GPLv2-only: via „or later" unter GPLv3 nutzbar und damit über die GPLv3↔AGPLv3-§13-Brücke mit dem AGPL-Gesamtwerk kombinierbar. Zusätzlich entschärfend: eigenständiges WordPress-Plugin (kein Link/Kombination mit dem Mod-Code) **und** Psyern ist selbst Urheber (könnte jederzeit umlizenzieren). **Kein GPLv2-only-Fall im Repo gefunden.** |
| `MISC/` Standalone-PHP/Themes/Tools | keine Lizenz-Header (eigenes Werk) | 🟢 fällt unter neue Repo-Lizenz |

## 5.3 npm-Dependencies (nicht gebündelt; Pflicht erst bei Build-Distribution)

| Lizenz | Pakete | Urteil |
|---|---|---|
| MIT | express, body-parser, jsonwebtoken, node-fetch, express-rate-limit, gamedig, serve-favicon, saslprep, trim-newlines, ejs-lint, xmldom@0.7.0, ejs-electron, electron, @electron-forge/*, pkg, nodemon, languagedetect ⚠(verify), node-wit ⚠(verify) | 🟢 |
| Apache-2.0 | discord.js@13, ejs, mongodb, log4js, websocket, @tensorflow/*, @tensorflow-models/toxicity, electron-squirrel-startup | 🟢 |
| MPL-2.0 | greenlock-express@4 | 🟢 (file-level copyleft, (A)GPLv3-kompatibel, solange nicht „Incompatible With Secondary Licenses" — bei Lockfile prüfen) |
| ISC (Stubs) | crypto@1.0.1, https@1.0.0 | 🟢 lizenzseitig, aber **entfernen** (deprecated Platzhalter für Node-Builtins) |

**Gesamturteil: kein 🔴 außer `UApiMass.pbo`. Kein GPLv2-only im gesamten Scan.**

---

# 6. Blocker & Überraschungen

1. **🔴 `ModAddons/UApiMass.pbo`** — Binär-PBO ohne Quellcode. Nicht mergen, nicht redistributieren (AGPL §6). Falls Funktion gebraucht: Quelle bei daemonforge-Repo (master-Branch) suchen oder Feature neu implementieren.
2. **NOTICE-Repack-Verbot** der Quelle vs. AGPL-Freiheiten — als §7-„further restriction" entfernbar; Entscheidung + Begründung in §4.4 dokumentiert (Attribution bleibt).
3. **Lizenz-Inkonsistenz upstream** (GPL-3.0-LICENSE-Dateien im WebService vs. AGPL-3.0-or-later in package.json) — kein Kompatibilitätsproblem, aber im NOTICE/Plan festgehalten (§4.1).
4. **Cross-PBO-modded-Klassen**: `_UniversalApi` moddet `UApiEntityStore` (4_World/ItemStore.c) und `ULoggerBaseInstance` (3_Game/Logger.c) aus `_UAPIBase`. Im Single-PBO-Ziel ist „modded class" auf eine Klasse desselben Moduls nicht möglich → **textuelles Zusammenführen** der Basis- + modded-Definitionen (auch `CarScript`, `ItemBase`, `PlayerBase` existieren in beiden Quell-PBOs modded → je 1 Definition mergen). Zusätzlich moddet das Ziel bereits `MissionBase`/`MissionBaseWorld` (PF_) → UApi-MissionBase-Anteile dort integrieren.
5. **Bewusste Kompat-Brüche des Rebrands** (dokumentieren, nicht „fixen"): CfgPatches `UAPIBase`/`UniversalApi` entfallen (Drittmods mit `requiredAddons {"UAPIBase"}` laden nicht mehr); `#ifdef UNIVERSALAPI`-Checks von Drittmods greifen nicht mehr; alle `UApi*`-Klassennamen verschwinden aus dem globalen Namensraum. Wer Drittmod-Kompat braucht, müsste ein separates Compat-Stub-PBO bauen — **nicht Teil dieses Plans**.
6. **Vanilla-Namensfalle `GetUApi()`**: existiert bereits als DayZ-Input-API — Accessor-Benennung `DME_Api()` gewählt; niemals `GetUApi*` verwenden.
7. **Rename-interne Kollision** `ApiQueryBase`/`UApiQueryBase` → Sonderregel (§2.9.1).
8. **Modernisierungs-Volumen** (spätere Phase, CLAUDE.md-Regeln): 135× `GetGame()` (→ `g_Game`), 28× `IsServer()`/`IsClient()` (→ `IsDedicatedServer()`-Logik), unprefixte Member in modded classes, `autoptr`-Bestand, jsonwebtoken<9-CVE, deprecated npm-Stubs, Node-Deps von 2022 (mongodb 4.6, discord.js 13, electron 18 EOL), `JsonFileLoader`-Fehlerverhalten. **Ternary-Operatoren: 0 Treffer** (gut).
9. **DesktopManager enthält eine ~4.900-LOC-Kopie des WebService** (9 Datei-Diffs zur Hauptversion) — bei Übernahme Dedup zwingend; Empfehlung: DesktopManager nicht übernehmen (Owner-Entscheid offen).
10. **Ziel hat kein 1_Core-Modul** — config.cpp/mod.cpp müssen `engineScriptModule` + Common-Defines-Einbindung erhalten (§2.5).
11. **Typos sind Wire-Format**: Route `/Gobals`, Config-Key `LetsEncypt` — bleiben (Interop); nur interne, nicht-serialisierte Namen (z. B. `SimpleNoticiation`-Aufrufe) dürfen später normalisiert werden.

# 7. Offene Entscheidungen für den Orchestrator

| # | Frage | Empfehlung Phase 0 |
|---|---|---|
| E1 | DesktopManager übernehmen? | Nein (EOL-Electron, Code-Duplikat); nur `DayZWebService` als `ApiService/` |
| E2 | Server-Config-Pfad `$profile:UApi\UniversalApi.json` | Neuer Pfad + Auto-Migration mit Lese-Fallback (Option A, §2.4) |
| E3 | SPDX `AGPL-3.0-only` vs. `-or-later` | `AGPL-3.0-only` (konservativ, §4.2) |
| E4 | WP-Plugins Leaderboard/AuctionHouse umlizenzieren? | Nein, MIT belassen (eigenständige Werke) |
| E5 | Compat-Stub-PBO für Drittmods (`UAPIBase`-requiredAddons)? | Nein (kein bekannter Konsument in eigener Mod-Familie; DME-WAR nutzt UApi nicht) |

---

*Erstellt durch Phase-0-Analyse-Agent, 2026-07-09. Datenbasis: vollständige greps/Datei-Inventare beider Bäume + DME-WAR; keine Code-Änderungen vorgenommen.*

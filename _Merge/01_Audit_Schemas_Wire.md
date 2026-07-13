# Phase 1D — Audit: Schemas, Config & Wire-Format (verbindlicher Wire-Contract)

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework
**Datum:** 2026-07-10 · **Agent:** 1D (Schemas/Config/Wire-Format) · **Status: NUR ANALYSE, keine Änderungen**
**Basis:** `00_Baseline_Rename_Relicense.md` (§2.8 Interop-Liste — hier vertieft zum verbindlichen Kontrakt)
**Quelle:** `C:\Users\Administrator\Desktop\Psyerns_Framework\DeamonForge\DayZ-UniveralApi-1.3.2\`
**Ziel:** `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\`
**Abnehmer-Mod:** `C:\Users\Administrator\Desktop\DME-WAR\DME_Map-Link\MapLink\`

---

## 0. KERNBEFUND (zuerst lesen)

**DME_Map-Link ist KEIN Client der UniversalApi 1.3.2, sondern des Nachfolge-Projekts „UniversalFramework" (UF).**

Belege:
- `MapLink/config.cpp:8-10`: `requiredAddons[]={"UFramework"}` — nicht `UniversalApi`/`UAPIBase`.
- MapLink ruft durchgehend `UF()` / `UFConfig()` / `UF_SUCCESS` / `UFServerStatus` / `UJSONHandler` / `UFConfigBase` / `UFObject_Base` / Hooks `UFrameworkReady()`+`UFrameworkReadyTokenReceived()` auf — **keines dieser Symbole existiert in der 1.3.2-Quelle** (dort: `UApi()`, `UApiConfig()`, `UAPI_SUCCESS`, `UApiServerStatus`, `UApiJSONHandler`, `UApiConfigBase`, `UApiObject_Base`, `UniversalApiReady()`/`UniversalApiReadyTokenReceived()`).
- Der lokale Ordner `DeamonForge\DayZ-UniveralApi-stable\` ist **inhaltlich ebenfalls die UApi-Namenswelt** (grep nach `UF()`/`UFConfig`/`UF_SUCCESS` = 0 Treffer) — die UFramework-Quelle liegt **nicht** auf der Platte.

Die gute Nachricht: Die UF-API ist zur 1.3.2-API **signaturkompatibel** (daemonforge hat beim UF-Rewrite primär Symbole umbenannt). Alle 30 MapLink-Aufrufstellen wurden einzeln gegen 1.3.2 abgeglichen (→ §3): **21 von 22 Symbolen haben ein exakt signaturgleiches 1.3.2-Pendant**; einzige Lücke ist die Settings-Registry `UF().Settings().Register(...)` (UFramework-Feature, in 1.3.2 nicht vorhanden).

**Konsequenz für den Merge:** Der geplante `DME_Api_`-Rename der 1.3.2-Codebasis bedient MapLink nicht automatisch — MapLink muss nachgezogen werden (Empfehlung §3.4) oder bekommt eine Alias-Schicht (abgeraten).

---

## 1. Wire-Contract — verbindlich („bleibt exakt so")

### 1.1 Transport & Auth (unantastbar)

| Aspekt | Kontrakt | Fundort |
|---|---|---|
| Transport | HTTPS, JSON-Body (`body-parser`, Limit **64 MB**), alle Fach-Routen **POST** (Ausnahmen: `/Status` auch GET; `/Discord/*` OAuth-Routen GET) | `app.js:76-88` |
| **Auth-Header-Hack** | EnScript kann keine Custom-Header setzen → Token wird als **Content-Type-Header** gesendet: `ctx.SetHeader(UApi().GetAuthToken())` (`UniversalRest.c:20`, `UApiEndpointBase.c:24`). Node-Middleware `ExtractAuthKey` (`utils.js:274-289`): wenn Content-Type **nicht** mit `text/ application/ multipart/ audio/ image/ video/` beginnt → Wert nach `req.headers['auth-key']` verschieben, Content-Type := `application/json`. Header-Name **`auth-key`** und diese Logik sind Wire-Format. | utils.js, UniversalRest.c, UApiEndpointBase.c |
| Server-Auth | `config.json:ServerAuth` — **String ODER String-Array** (`AuthChecker.js:105-118`); Vergleich als Klartext-Gleichheit. Array[0] ist JWT-Signing-Key. EnScript-Server sendet `UApiConfig().ServerAuth` roh. | AuthChecker.js |
| Client-Auth (JWT) | Server ruft `POST /GetAuth/<guid>` → Node signiert `{GUID}` mit `GetSigningAuth()`, **`expiresIn: 2800`** (46,5 min; Client erneuert via RPC alle ~21–23 min). SHA-256(base64) des Tokens wird in Mongo `Players.AUTH` abgelegt; `CheckPlayerAuth` = JWT-verify **plus** DB-Match. | Auth.js, AuthChecker.js:122-127 |
| Token-Verteilung ins Spiel | CF-RPC `"UAPI"`: `RPCUniversalApiConfig` sendet `Param2<ApiAuthToken, UniversalApiConfig>` an Client (ServerAuth wird durch `"null"` ersetzt, `UniversalApi.c:379-393`). Client-Retry via `RPCRequestAuthToken`/`RPCRequestRetry` (max. 20 Versuche, Backoff n·2200 ms). | UniversalApi.c |
| Schreibrechte | `AllowClientWrite` (Node-Config): Client-JWT darf nur schreiben, wenn `true`; Server-Auth darf immer. `PublicLoad` ist ohne Auth lesbar (nur Anlage erfordert Auth). | player.js, Object.js, globals.js |
| Rate-Limit | global 500 Req/10 s (`RequestLimit`), Whitelist `RateLimitWhiteList` (IP-Gleichheit); je Router eigene Limits: Status 100/10 s, Query 400/10 s, Logger 500/10 s, ServerQuery 200/5 s, Toxicity 100/10 s, Wit 300/10 s, LUIS 200/10 s, Translate 200/10 s, Crypto 150/5 s, QnA 100/5 s, Discord `RequestLimitDiscord`·2 s (Default 300). Fehlertext: `{ "Status": "Error", "Error": "RateLimited" }` | app.js, utils.js:296, je Router |
| Fallback | Unbekannte URL → HTTP 501 `{Status:"Error", Error:"Reqested bad URL"}` (sic, Tippfehler ist Bestand) | app.js:109-115 |

### 1.2 EnScript-Callback-ABI (Wire zur Mod-Seite)

Function-based Callbacks werden IMMER so aufgerufen (`UApiDBCallBack.c`, via `GameScript.CallFunctionParams`):

```
Param4<int, int, string, string>(CallId, status, OID, data)      // ReturnString-Variante
Param4<int, int, string, T>(CallId, status, OID, typedObj)       // UApiCallback<T>-Variante
```

Status-Konstanten (EnScript-Abstraktion der HTTP-/REST-Ergebnisse, `1_Core/Constants.c`):
`UAPI_SUCCESS=200` (jede 2xx-Antwort mit Body ≠ `{}`), `UAPI_EMPTY=204` (2xx mit Body `{}`/leer), `UAPI_CLIENTERROR=400` (EREST_ERROR_CLIENTERROR), `UAPI_SERVERERROR=500` (sonstige Fehler), `UAPI_TIMEOUT=408`. Wichtig: HTTP **201/203 landen in OnSuccess** (Enfusion-RestCallback zählt 2xx als Erfolg) — deshalb echoen Load-Routen bei Nichtexistenz den Request-Body zurück (= „Defaults gelten als geladen"). Diese Semantik ist Kontrakt.

### 1.3 HTTP-Routen (Node) ↔ EnScript-Aufrufer — **60 Routen auf 17 Router-Familien (19 Mounts inkl. Aliasse)**

Mounts (`app.js:90-107`): `/Object`, `/Player`, `/Gobals` **(Tippfehler-Alias, bleibt!)**, `/Globals`, `/GetAuth`, `/Status`, `/QnAMaker`, `/QnA` (Alias), `/Forward`, `/Logger`, `/Discord`, `/Wit`, `/LUIS`, `/Translate`, `/ServerQuery`, `/Toxicity`, `/Random`, `/Crypto`.

Auth-Legende: **S** = ServerAuth · **C** = Client-JWT · **C(W)** = Client-JWT nur mit `AllowClientWrite` · **C(P)** = `CheckPlayerAuth` (JWT+DB, nur eigene GUID) · **–** = ohne Auth.

#### /Player (Mongo-Collection **`Players`**, Dokument `{GUID, AUTH, <mod>: {...}, Public: {<mod>: "<str>"}}`)

| Route | Auth | Request-Body | Response | EnScript-Aufrufer |
|---|---|---|---|---|
| POST `/Player/Load/:GUID/:mod` | S ∨ C(P) | beliebiges Mod-JSON (= Defaults) | 200 + `<mod>`-Objekt; wenn fehlend: 201/203 + Body-Echo (legt bei S/W an) | `UApiDBEndpoint.Load` („Load/oid/mod"), `UniversalRest.PlayerLoad` |
| POST `/Player/Save/:GUID/:mod` | S ∨ C(P)(W) | Mod-JSON | 200 Body-Echo / 203 / 401 | `UApiDBEndpoint.Save`, `UniversalRest.PlayerSave` |
| POST `/Player/Update/:GUID/:mod` | S ∨ C(P)(W) | `{Element, Value, Operation?}` | `{Status:"Success"\|"NotFound"\|"Error", Element, Mod, ID}` | `UApiDBEndpoint.Update` (`UApiUpdateData`) |
| POST `/Player/PublicLoad/:GUID/:mod` | – (lesen) | `{Value}` (Default) | `{Value: "<string>"}`; 201/203 Echo bei Anlage | `UApiDBEndpoint.PublicLoad` |
| POST `/Player/PublicSave/:GUID/:mod` | S ∨ C(P)(W) | `{Value: "<string>"}` (`SimpleValueStore`) | 200 Echo / 203 / 401 | `UApiDBEndpoint.PublicSave` — **MapLink!** |
| POST `/Player/Query/:mod` | **nur S** (Player-Query) | `{Query:"<json-str>", OrderBy:"<json-str>", MaxResults, ReturnObject, FixQuery}` | `{Status:"Success"\|"NoResults"\|"Error", Count, Results[]}` | `UApiDBEndpoint.Query` (`UApiDBQuery`) |
| POST `/Player/Query/Update/:mod` | S ∨ C(W) | `{Query:{...}, Element, Value, Operation}` | `{Status, Element, Mod, Count}` | `UApiDBEndpoint.QueryUpdate` |
| POST `/Player/Transaction/:id/:mod` | S ∨ C(W) | `{Element, Value}` bzw. `+{Min,Max}` (validated) | `{Status:"Success"\|"NotFound"\|"Error", ID, Mod, Value, Element[, Error]}` | `UApiDBEndpoint.Transaction/Increment` |

GUID-Normalisierung: 17-stellige Steam64-IDs werden serverseitig zu `sha256→base64url` (`NormalizeToGUID`, utils.js:264) — Kontrakt für alle `/Player`- und Player-`/Transaction`-Routen.

#### /Object (Collection **`Objects`**, Dokument `{ObjectId, Mod, Data:{...}}`)

| Route | Auth | Besonderheit | EnScript |
|---|---|---|---|
| POST `/Object/Load/:ObjectId/:mod` | S ∨ C | `ObjectId=="NewObject"` ⇒ Server generiert ID (sha256-base64url) und liefert sie im Echo-Feld `ObjectId` zurück | `UApiDBEndpoint.Load`, `UniversalRest.ObjectLoad` |
| POST `/Object/Save/:ObjectId/:mod` | S ∨ C(W) | dito NewObject; 201 bei Erfolg | `.Save` |
| POST `/Object/Update/:ObjectId/:mod` | S ∨ C(W) | wie Player/Update, Ziel `Data.<element>` | `.Update` |
| POST `/Object/Query/:mod` | S ∨ **C (lesend erlaubt!)** | Query-Default `{Mod: mod}` wird ergänzt | `.Query` |
| POST `/Object/Query/Update/:mod` | S ∨ C(W) | | `.QueryUpdate` |
| POST `/Object/Transaction/:id/:mod` | S ∨ C(W) | `$inc` auf `Data.<element>` | `.Transaction` |

#### /Globals + Alias /Gobals (Collection **`Globals`**, Dokument `{Mod, Data:{...}}`)

| Route | Auth | Response | EnScript |
|---|---|---|---|
| POST `/Globals/Load/:mod` | S ∨ C | Daten-Objekt bzw. 201 Body-Echo bei Anlage | `UApiDBGlobalEndpoint.Load`, `UniversalRest.GlobalsLoad` — **MapLink (Config)** |
| POST `/Globals/Save/:mod` | S ∨ C(W) | 201 Echo / 203 | `.Save`, `UniversalRest.GlobalsSave` — **MapLink** |
| POST `/Globals/Transaction/:mod` | S ∨ C(W) | `{Status, ID:mod, Value, Element}` | `.Transaction`, `UniversalRest.GlobalsTransaction` |
| POST `/Globals/Update/:mod` | S (Client-Pfad hat upstream-Bug, §6.3) | `{Status, Element, Mod, ID:"Globals"}` | `.Update` |

#### Infrastruktur-Routen

| Route | Auth | Request → Response | EnScript |
|---|---|---|---|
| POST `/GetAuth/:GUID` | **nur S** | `{}` → `{GUID, AUTH:"<jwt>"}` bzw. `AUTH:"ERROR"` | `UniversalRest.GetAuth` ← `UApi().PreparePlayerAuth` (OnClientPrepareEvent), Ergebnis via `UApiAuthCallBack` → CF-RPC an Client |
| POST/GET `/Status` u. `/Status/:Auth` | – (Auth optional) | `{}` → `{Status, Error:"noerror"\|"noauth"\|…, Version, Discord, Translate, Wit[], QnA[], LUIS[]}`; macht Test-Write in `Globals` (Mod `UniversalApiStatus`) | `UApiAPIEndpoint.Status` → `UApiStatus` (Version-Handshake §5) |
| POST `/Logger/One/:id` | S ∨ C | beliebiges Log-JSON; Server ergänzt `ServerId, LoggedDateTime, ClientId(sha256-IP, 32 Z.), ClientType:"Server"\|"Client"` → Collection **`Logs`** → `{Status:"Success", Error:""}` | `UniversalRest.Log` (`Logger/One/<ServerID>`) — **MapLink (MLLog)**; `ULoggerBaseInstance.SendToApi`; `PluginAdminLog` |
| POST `/Logger/Many/:id` | S ∨ C | JSON-**Array** von Log-Objekten (insertMany) | `UniversalRest.LogBulk` (LogPlayerPos) |
| POST `/Forward` | S ∨ C | `{URL, Method, Headers:[{Key,Value}], Body, ReturnValue, ReturnValueArrayIndex}` → Antwort der Fremd-API (ggf. auf `ReturnValue`[/Index] reduziert) | `UniversalRest.Request(UApiForwarder)` |
| POST `/ServerQuery/Status/:ip/:port` | S ∨ C | `{}` → `UApiServerStatus`-Form: `{Status:"Online"\|"Offline", Error, IP, GamePort, QueryPort, Name, ServerVersion, Players, QueuePlayers, MaxPlayers, GameTime, GameMap, Password:0\|1, FirstPerson:0\|1}` (gamedig) | `UApiAPIEndpoint.SteamQuery/ServerQuery/ServerQueryObj` — **MapLink (Server-Browser)** |
| POST `/Random` · `/Random/Full` | S ∨ C | `{Count}` (≤2048 / ≤4096) → `{Status, Error, Numbers:[int]}` (ANU-QRNG; Full: int32-Bereich) | `UApiAPIEndpoint.RandomNumbers/RandomNumbersFull` (Math-QRandom-Feed) |
| POST `/Crypto/Price/:from/:to` · `/Crypto/Convert/:from/:to` | S ∨ C | `{}` bzw. `{Value}` → `{Status, Error, Value:float}` | `.CryptoPrice/.CryptoConvert` |
| POST `/Crypto/:from` | S ∨ C | `{From:[..]}` oder `{To:[..]}` → `{Status, Error, Values:{k:v}}` | `.Crypto` (`UApiCryptoRequest`) |

#### KI-Routen

| Route | Auth | Request → Response | EnScript |
|---|---|---|---|
| POST `/QnA[Maker]` u. `/QnA[Maker]/:key` | S ∨ C | `{Question}` → `{Status, answer, score[, Error]}` (lowercase-Felder = Kontrakt, `QnAAnswer`) | `UApiAPIEndpoint.QnA/ChatQnA` |
| POST `/Wit/:key` | S ∨ C | `{Question}` → Wit-Rohantwort bzw. `{Status:"Error",…}` | `.Wit` |
| POST `/LUIS/:key` | S ∨ C | `{Question}` → LUIS-Rohantwort | `.LUIS` |
| POST `/Translate` | S ∨ C | `{Text, To:[..], From?}` (`UApiTranslationRequest`) → `{Status, Error, Translations:[{text,to}], Detected}` | `.Translate` → `UApiTranslationResponse` |
| POST `/Toxicity` | S ∨ C | `{Question}` → `{Status, Error, Toxicity, IdentityAttack, Insult, Obscene, SevereToxicity, SexualExplicit, Threat}` (0–1) | `.Toxicity` → `UApiToxicityResponse` |

#### /Discord (discordConnector.js — 22 Routen)

Browser/OAuth (GET, kein API-Auth): `/Discord/` (Info), `/Discord/callback` (OAuth-Redirect-Ziel), `/Discord/:id` (Login-Start per Steam64/GUID), `/Discord/login/:id`, `/Discord/test/:id`. **Redirect-URI-Kontrakt mit der Discord-App-Konfiguration — Pfade nie ändern.**

API (POST, S ∨ C je nach Route): `/AddRole/:GUID` u. `/RemoveRole/:GUID` (`{RoleId}` → `UApiDiscordUser`), `/Get/:GUID` (akzeptiert GUID **oder** Steam64), `/GetChannel/:GUID`, `/Mute/:GUID` (`{Mute}`), `/Kick/:GUID` (`{Text}`), `/Move/:GUID/:id`, `/Send/:GUID` (`{Message}`), `/SetNickname/:GUID` (`{Nickname}`), `/Check/:ID/`, `/CheckRole/:ID/:ROLEID` (Pre-Join-Checks, `StatusObject`), `/Channel/Create` (`UApiCreateChannelObject`), `/Channel/Delete/:id`, `/Channel/Edit/:id`, `/Channel/Invite/:id`, `/Channel/Send/:id` (`{Message}` oder Embed-JSON), `/Channel/Messages/:id` (`UApiDiscordChannelFilter` → `UApiDiscordMessagesResponse`).

⚠ **Skew in der Quelle:** `DiscordRest.c:97,255` ruft `Discord/GetWithPlainId/<id>` — diese Route existiert in Node 1.3.2 **nicht** mehr (nur `/Get/:GUID`, das beides akzeptiert; die alte EnScript-Methode dazu ist auskommentiert, `UApiDSEndpoint.c:427-459`). Aufrufe laufen in den 501-Fallback. Toter Alt-Code, beim Merge als bekannt markieren (§6.2).

### 1.4 MongoDB-Kontrakt

| Collection | Schlüssel/Indizes (`utils.js:InstallIndexes`) | Dokumentform |
|---|---|---|
| `Players` | `{GUID:1}`, `{GUID:1, AUTH:1}` | `{GUID, AUTH:sha256b64(jwt), "<ModTag>": {…}, Public: {"<ModTag>": "<string>"}}` |
| `Objects` | `{ObjectId:1, Mod:1}` | `{ObjectId, Mod, Data: {…}}` |
| `Globals` | `{Mod:1}` | `{Mod, Data: {…}}` — MapLink-Config lebt hier unter `Mod:"MapLink"` |
| `Logs` | – | freies Log-JSON + `ServerId, LoggedDateTime, ClientId, ClientType` |
| (`QnAMaker`) | – | nur via `LogUnAnswerable` (unbeantwortete Fragen) |

DB-Name-Default **`DayZ`** (`config.json:DB`). Feldnamen `GUID`, `AUTH`, `Mod`, `Data`, `ObjectId`, `Public` sind Persistenz-Kontrakt — nie umbenennen (bestehende Server-Datenbanken!). `RemoveBadProperties` (utils.js) ersetzt Sonderzeichen in Keys durch `_` — Verhalten beibehalten.

### 1.5 JSON-Wire-Felder der EnScript-Klassen (Stichprobe verifiziert)

`StatusObject{Status,Error}` · `UApiStatus{Version,Discord,Translate,Wit,QnA,LUIS}` · `ApiAuthToken{GUID,AUTH}` · `UApiTransaction{Element,Value}` / `UApiValidatedTransaction{+Min,Max}` · `UApiUpdateData{Element,Value,Operation}` · `UApiDBQuery{Query,OrderBy,MaxResults,ReturnObject,FixQuery}` · `UApiForwarder{URL,Headers[{Key,Value}],Method,Body,ReturnValue,ReturnValueArrayIndex}` · `UApiQuestionRequest{Question}` · `UApiRandomNumberRequest{Count}` / `…Response{Numbers}` · `UApiServerStatus{IP,GamePort,QueryPort,Name,ServerVersion,Players,QueuePlayers,MaxPlayers,GameTime,GameMap,Password,FirstPerson}` · `QnAAnswer{answer,score}` (lowercase!) · `SimpleValueStore{Value}`. **Klassen umbenennen ja — Member nie** (Baseline §2.8 bestätigt und hiermit endpunktgenau belegt).

---

## 2. Config-Mapping (Quelle → Ziel)

### 2.1 Bestandsaufnahme Quelle

**(a) EnScript-Serverconfig** — `$profile:UApi\UniversalApi.json` (`ConfigLoader.c:3-4`), Auto-Anlage bei Fehlen:

| Key | Typ/Default | Zweck |
|---|---|---|
| `ConfigVersion` | string `"1"` | interne Migration (erzwingt Reset von `PromptDiscordOnConnect` bei Fremdwert) |
| `ServerURL` | string | Basis-URL des Node-Service; Trailing-`/` wird erzwungen und zurückgespeichert |
| `ServerID` | string | Server-Name — **MapLink-kritisch**: muss `Servers[].Name`/`SpawnPoints[].ServerName` im Globals-Dokument matchen |
| `ServerAuth` | string | Klartext-Key = `config.json:ServerAuth` der Node-Seite |
| `QnAEnabled` | bool false | aktiviert QnA-Chat + lädt (b) |
| `EnableBuiltinLogging` | int 0 | steuert `PluginAdminLog`-→`/Logger`-Mirroring |
| `PromptDiscordOnConnect` | int 0 | Discord-Link-Prompt beim Connect |

Wichtig: Die Klasse `UniversalApiConfig` ist gleichzeitig **RPC-Payload** an jeden Client (`Param2<ApiAuthToken, UniversalApiConfig>`, ServerAuth → `"null"` maskiert). Sie muss schlank und Secret-frei (bis auf die Maskierung) bleiben.

**(b) EnScript-QnA-Config** — `$profile:UApi\QnAMakerServerAnswers.json` (`QnAMakerConfig.c:3-4`), wird komplett an Clients ge-RPC-t.

**(c) Node-Service** — `config.json` neben der Binary (`global.SAVEPATH`, Auto-Anlage aus `sample-config.json` mit generiertem ServerAuth). Dokumentierte Keys (sample-config.json): `DBServer`, `DB`, `ServerAuth` (string|array), `AllowClientWrite`, `IP`, `Port`, `LogToFile`, `Certificate`, `CertificateKey`, `CheckForNewVersion`, `CreateIndexes` (wird nach Index-Anlage auf false **zurückgeschrieben**!), `RateLimitWhiteList`, `Discord{Client_Id, Client_Secret, Bot_Token, Guild_Id, Required_Role, BlackList_Role, Restrict_Sign_Up, Restrict_Sign_Up_Countries, AllowToReRegister}`, `Wit{<key>:{…}}`, `LUIS{<key>:{Endpoint,SubscriptionKey,Verbose,Log}}`, `QnA{<key>:{Endpoint,EndpointKey,MinScore,LogUnAnswerable}}`, `Translate{Endpoint,Type,SubscriptionRegion,SubscriptionKey}`, **`LetsEncypt`** (sic!) `{Enabled,Domain,Email,AltNames}`, `debug` (0/1/2). Zusätzlich **undokumentiert im Code**: `cpuCount` (Cluster-Worker), `RequestLimit`, `RequestLimitStatus`, `RequestLimitQuery`, `RequestLimitLogger`, `RequestLimitServerQuery`, `RequestLimitToxicity`, `RequestLimitWit`, `RequestLimitLUIS`, `RequestLimitTranslate`, `RequestLimitCrypto`, `RequestQnAMaker`, `RequestLimitDiscord`, `RequestLimitFowarder` (sic). Alle Keys sind Wire/Betriebs-Kontrakt — **unverändert übernehmen**.

### 2.2 Bestandsaufnahme Ziel

`PF_WebConfig` (Singleton, `CURRENT_VERSION = 4`, int-`ConfigVersion` mit Auto-Upgrade) lädt **eine** Datei `$profile:DeadmansEcho\PsyernsFramework\PsyernsFrameworkConfig.json` (per `PsyernsFrameworkConfig.example.json` dokumentiert; PF_AH_Config nutzt denselben Ordner). Struktur: flache Feature-Toggles + `Endpoints[]{Name,BaseUrl,ApiKey,Enabled,RateLimitMs}` + `AlertRules[]` + `AdminIDs[]`. `PF_RestConfig` ist nur Proxy darauf.

### 2.3 Empfehlung (mit Blick auf Owner-Entscheid „Config in vorhandene Struktur")

**Einordnung in die vorhandene Struktur = gleicher Profil-Ordner, aber EIGENE Datei. Kein Key-Merge in PsyernsFrameworkConfig.json.**

| Quelle | Ziel (empfohlen) |
|---|---|
| `$profile:UApi\UniversalApi.json` | **`$profile:DeadmansEcho\PsyernsFramework\DME_Api.json`** — Klasse `DME_Api_Config`, **Keys 1:1 unverändert** (`ConfigVersion`,`ServerURL`,`ServerID`,`ServerAuth`,`QnAEnabled`,`EnableBuiltinLogging`,`PromptDiscordOnConnect`) |
| `$profile:UApi\QnAMakerServerAnswers.json` | `$profile:DeadmansEcho\PsyernsFramework\DME_Api_QnA.json` — Keys unverändert |
| Node `config.json` | bleibt wo sie ist (SAVEPATH des Service, künftig `ApiService/`); Keys/Format unverändert; nur Branding-Ausgaben ändern |

Begründung gegen den Voll-Merge in `PsyernsFrameworkConfig.json` (drei harte Gründe):
1. **RPC-Leak-Gefahr:** `UniversalApiConfig` geht als CF-RPC-Payload an jeden Client. Wären die UApi-Keys Teil von `PF_WebConfig`, müsste entweder die ganze PF-Config (inkl. WordPress-ApiKeys, Webhook-Tokens, AdminIDs) client-serialisierbar werden — inakzeptabel — oder man bräuchte ohnehin eine separate schlanke RPC-Klasse. Dann kann die Datei gleich getrennt bleiben.
2. **ConfigVersion-Kollision:** PF nutzt `int ConfigVersion` (aktuell 4, Auto-Upgrade-Logik), UApi `string ConfigVersion` (`"1"`, eigene Reset-Semantik). Ein Merge zwingt zu Semantik-Änderungen an mindestens einem Bestand — unnötiges Risiko.
3. **Lade-Reihenfolge/Layer:** `UApiConfig()` wird lazy im 3_Game-Layer beim ersten Zugriff geladen (u. a. aus Konstruktor-Pfaden von `MissionBase`); `PF_WebConfig.GetInstance()` hat eigenes Timing. Entkoppelte Dateien halten die Initialisierungspfade unabhängig.

### 2.4 Migrationsregeln für bestehende Server-Instanzen

Load-Reihenfolge von `DME_Api_Config.Load()` (Umsetzungs-Phase):
1. Neuer Pfad existiert → laden (Normalfall).
2. Sonst: Alt-Pfad `$profile:UApi\UniversalApi.json` existiert → laden, `MakeDirectory` auf neuen Ordner, sofort unter neuem Pfad speichern, **Alt-Datei unangetastet lassen** (Rollback-Fähigkeit), eine Log-Zeile „migrated legacy UApi config".
3. Sonst: Defaults anlegen wie bisher (leere `ServerURL` ⇒ Modul offline, identisch zum Quellverhalten).
4. Identisch für `QnAMakerServerAnswers.json`.
5. **Keine** Mongo-Migration (Collections/Feldnamen unverändert); **keine** Node-Config-Migration (Datei bleibt am Service-Root); JWT-Signing-Key bleibt `ServerAuth` ⇒ laufende Client-Tokens bleiben bei einem Rolling-Update gültig.
6. `ConfigVersion`-Wert `"1"` beibehalten — die eingebaute Reset-Logik (`ConfigLoader.c:27-31`) nicht triggern.

---

## 3. DME_Map-Link — Kompat-Anforderungsliste (vollständig, aus grep aller 31 .c-Dateien)

### 3.1 Von MapLink referenzierte externe (UFramework-)Symbole — **22 Symbole, 30 Aufrufstellen**

| # | UF-Symbol (wie von MapLink benutzt) | Aufrufstellen | 1.3.2-Pendant | Signatur-Match? | Zielname laut Rename-Map |
|---|---|---|---|---|---|
| 1 | `UF()` (Kern-Accessor) | 14× gesamt | `UApi()` | ✅ | **`DME_Api()`** |
| 2 | `UFConfig().ServerID` | 15× (Config.c:139,146; MissionServer.c:10,84,85,97,98,107,134; PlayerBase.c:120; PlayerDataStore.c:375; DepaturePointMenu.c:86; RespawnServerMenu.c:103,197,210) | `UApiConfig().ServerID` | ✅ (Feld identisch) | **`DME_Api_GetConfig().ServerID`** |
| 3 | `UF().Rest().GlobalsLoad(mod, cb, json)` | Config.c:72 | `UniversalRest.GlobalsLoad(string, ref RestCallback, string="{}")` | ✅ | `DME_Api().Rest().GlobalsLoad` |
| 4 | `UF().Rest().GlobalsSave(mod, json)` | Config.c:78 | `UniversalRest.GlobalsSave(string, string, ref RestCallback=NULL)` | ✅ | `DME_Api().Rest().GlobalsSave` |
| 5 | `UF().Rest().Log(json)` | Logger.c:166 | `UniversalRest.Log(string, ref RestCallback=NULL)` | ✅ | `DME_Api().Rest().Log` |
| 6 | `UF().db(PLAYER_DB).Load(mod, oid, inst, "fn")` → int | MissionServer.c:48 | `UApiDBEndpoint.Load(string,string,Class,string,string="{}")` | ✅ | `DME_Api().db(DME_API_PLAYER_DB).Load` |
| 7 | `UF().db(PLAYER_DB).Save(mod, oid, json)` → int | PlayerBase.c:118 | `UApiDBEndpoint.Save(string,string,string)` | ✅ | `.Save` |
| 8 | `UF().db(PLAYER_DB).PublicSave(mod, oid, json, NULL, "")` → int | PlayerBase.c:120,122 | `UApiDBEndpoint.PublicSave(string,string,string,Class=NULL,string="")` | ✅ | `.PublicSave` |
| 9 | `UF().api().SteamQuery(ip, portStr, inst, "fn")` → int | RespawnServerWidget.c:53; DeparturePointWidget.c:76 | `UApiAPIEndpoint.SteamQuery(string,string,Class,string,string="",bool=false)` | ✅ | `DME_Api().api().SteamQuery` |
| 10 | `UF().RequestCallCancel(cid)` | RespawnServerWidget.c:67,76; DeparturePointWidget.c:128,139 | `UniversalApi.RequestCallCancel(int)` | ✅ | `DME_Api().RequestCallCancel` |
| 11 | **`UF().Settings().Register("maplink","MapLink","Psyern", html, TStringArray)`** | MissionBase.c:26 | **FEHLT in 1.3.2** (UFramework-Settings-Registry; lädt `MapLink/data/mod-settings.html` und meldet Globals-Namen an) | ❌ **GAP** | Entscheid nötig (§3.4) |
| 12 | `UFConfigBase` (Basisklasse `MapLinkConfig`; nutzt `m_DataReceived` protected, `SetDefaults/OnDataReceive/Load/Save/ToJson/OnSuccess(string,int)/SetDataReceived`) | Config.c:6,25,39,66,76,83,88 | `UApiConfigBase : RestCallback` (ConfigBase.c) — alle Member/Methoden vorhanden und protected-kompatibel | ✅ | `DME_Api_ConfigBase` |
| 13 | `UFObject_Base` (Basisklasse `MapLinkLogObject`, virtuelles `ToJson()`) | Logger.c:216 | `UApiObject_Base` (ObjectBase.c) | ✅ | `DME_Api_Object_Base` |
| 14 | `UFServerStatus` (genutzte Felder: `Status=="Online"`, `Players`, `MaxPlayers`, `QueuePlayers`) | RespawnServerWidget.c:110; DeparturePointWidget.c:149 | `UApiServerStatus extends StatusObject` — alle Felder vorhanden | ✅ | `DME_Api_ServerStatus` |
| 15 | `UJSONHandler<T>.ToString/FromString` (statics) | Config.c:84,89; PlayerDataStore.c:54; MissionServer.c:19 | `UApiJSONHandler<Class T>` — identische statics | ✅ | `DME_Api_JSONHandler<T>` |
| 16 | `SimpleValueStore.StoreValue(string)` (static, Wire-Feld `Value`) | PlayerBase.c:120,122 | `SimpleValueStore` — **namensgleich in 1.3.2** | ✅ | `DME_Api_SimpleValueStore` (Rename-Map Block B) |
| 17 | `UCurrency.Register(string, TStringIntMap)` (static) | Config.c:54 | `UCurrency.Register` — namensgleich | ✅ | `DME_Api_Currency.Register` |
| 18 | `PLAYER_DB` (Konstante) | MissionServer.c:48; PlayerBase.c:118,120,122 | `PLAYER_DB = 100` (Constants.c:27) | ✅ | `DME_API_PLAYER_DB` |
| 19 | `UF_SUCCESS` | MissionServer.c:17; RespawnServerWidget.c:112; DeparturePointWidget.c:150 | `UAPI_SUCCESS = 200` | ✅ (Wert identisch) | `DME_API_SUCCESS` |
| 20 | `UF_EMPTY` | MissionServer.c:33 | `UAPI_EMPTY = 204` | ✅ | `DME_API_EMPTY` |
| 21 | Hook `override void UFrameworkReady()` in `modded MissionBase` (+super) | MissionBase.c:4-5 | `UniversalApiReady()` — identische Mechanik (leere virtuelle Methode in modded `MissionBaseWorld`, Mission.c 4_World; Server ruft sie via CallQueue, Client nach Token) | ✅ (nur Name anders) | Hook-Name im Ziel festlegen: **`DME_Api_Ready()`** (Vorschlag) |
| 22 | Hook `override void UFrameworkReadyTokenReceived()` in `modded MissionGameplay` (+super) | ModdedMissionGameplay.c:8-10 | `UniversalApiReadyTokenReceived()` (Mission.c:9; wird von `UniversalApi.c:301` per `GameScript.CallFunction` auf der Mission aufgerufen) | ✅ (nur Name anders) | **`DME_Api_ReadyTokenReceived()`** (Vorschlag) |

Zusätzlich verifiziert: die function-based Callback-ABI, die MapLink erwartet (`LoadPlayerFromUDB(int cid, int status, string oid, string data)` bzw. `UpdateServerStatus(int cid, int status, string oid, UFServerStatus data)`), entspricht exakt der 1.3.2-`UApiDBCallBack`/`UApiCallback<T>`-Aufrufkonvention (Param4, §1.2).

**Nur-Namensstil, KEINE externen Abhängigkeiten** (bei der Portierung nicht anfassen, nur wissen): `UServerData` (MapLink-eigene Klasse, ServerData.c — inkl. DME-Erweiterung `ItemBlacklist`!), `UFOnClientNewEvent`, `OnUFSave`/`OnUFLoad`, `UFSaveTransferPoint`, `UFDoTravel`, `UFKillAndDeletePlayer`, `UFAfterLoadClient`, `SendUFAfterLoadClient`, `SavePlayerToU`, `MLLog`/`MLLogFileInstance` (kopiertes ULogger-Muster, self-contained), `MapLinkLogObject` (Wire-Felder `Type:"MapLink"`, `Message`, `Level`).

MapLink-Wire-Nutzung serverseitig (bleibt durch §1 gedeckt): `Globals Mod:"MapLink"` (Config), `Players.<"MapLink">` (PlayerDataStore-JSON), `Players.Public.MapLink` (= `"<ServerID>~<TransferPoint>"` via SimpleValueStore), `Logs` (MapLinkLogObject), `/ServerQuery/Status` (Server-Browser).

### 3.2 config.cpp-Abhängigkeit

`requiredAddons[]={"UFramework"}` (MapLink/config.cpp:9). Im Merge-Ziel existiert nur CfgPatches `Psyerns_Framework` → **muss auf `"Psyerns_Framework"` umgestellt werden**, sonst lädt MapLink nicht (bzw. lädt gegen das weiterhin installierte UFramework-Workshop-PBO — Doppelbetrieb-Risiko §6.1).

### 3.3 Alias-Schicht vs. Nachziehen — Abwägung

**Option A — Kompat-/Alias-Schicht im Framework** (`UF()`-Funktion + `UFConfig()` + `typedef`/leere Subklassen `UFConfigBase: DME_Api_ConfigBase`, `UFServerStatus: DME_Api_ServerStatus`, `UJSONHandler = DME_Api_JSONHandler`, Konstanten `UF_SUCCESS`… + Hook-Weiterleitungen):
- (+) MapLink-PBO bleibt binär unverändert.
- (–) Re-importiert genau die unprefixten Global-Symbole, die Phase 0 eliminieren will (`UF`, `UF_SUCCESS`, `PLAYER_DB`…) — Kollisionsrisiko im DME-WAR-Umfeld bleibt.
- (–) Hooks sind **nicht aliasbar**: `UFrameworkReady()`/`UFrameworkReadyTokenReceived()` müssten als echte virtuelle Methoden in den modded Mission-Klassen des Frameworks existieren und vom Kern aufgerufen werden — das ist keine Alias-, sondern Doppel-API.
- (–) `UF().Settings()` müsste trotzdem neu gebaut werden (existiert in 1.3.2 nicht).
- (–) EnScript kennt keine Klassen-Aliase; für generische Klassen (`UJSONHandler<T>`) bräuchte es Vererbungs-Stubs, die daemonforge-Namen dauerhaft im Namensraum halten → AGPL-§5-/Rebrand-Ziel verwässert.

**Option B — DME_Map-Link nachziehen (EMPFOHLEN):**
- Nur **30 Aufrufstellen in 14 Dateien**, alle oben einzeln kartiert; rein mechanische Renames (UF→DME_Api-Namen, 2 Hook-Namen, 3 Konstanten, 4 Klassennamen, 1 requiredAddons-Eintrag).
- MapLink ist eigener Code (author Psyern) und wird ohnehin mit dem Framework zusammen deployed — kein Fremd-Kompat-Zwang (Baseline §2.9/E5 bestätigt: kein weiterer UApi/UF-Konsument in DME-WAR).
- Einziger Substanz-Punkt: `UF().Settings().Register(...)` (MissionBase.c:26) — Entscheid:
  - **B1 (empfohlen):** Aufrufblock in MapLink entfernen/`#ifdef`-tot legen, solange das Ziel-Framework keine Settings-Registry hat. Der Block liest nur `MapLink/data/mod-settings.html` ein und meldet es an — ohne UFramework-Webpanel ist er funktionslos.
  - B2: Settings-Registry als kleines `DME_Api_SettingsRegistry`-Feature nachbauen (Registrierung → `/Globals`-Save unter reserviertem Mod-Tag), falls das Webpanel-Feature gewünscht ist — eigenes Arbeitspaket, nicht Teil des Wire-Contracts 1.3.2.

**Empfehlung: Option B (nachziehen) + B1.** Aufwand ≈ 1 kurze Umbenennungs-Session mit anschließendem Servertest (Login-Flow, Transfer, Server-Browser, Globals-Load).

---

## 4. Schemas/MapLink.json — Zweck & Kontrakt

- **Zweck:** JSON-Schema (JSON-Schema-Dialekt, `additionalProperties:false`) für das **Globals-Dokument des MapLink-Mods** (`Globals`-Collection, `Mod:"MapLink"`, also exakt die `MapLinkConfig`-Klasse). Konsument ist der **Settings-Editor des DesktopManagers**: `DesktopManager/src/scripts/editor.js:227` lädt zur Laufzeit `https://raw.githubusercontent.com/daemonforge/DayZ-UniveralApi/stable/Schemas/<ModTag>.json` und füttert damit den gebündelten JSONEditor (validierte GUI-Bearbeitung der Globals in Mongo). Der Node-Service selbst liest die Schemas **nicht**.
- **Kontrakt Dateiname:** `<ModTag>.json` — der Mod-Tag ist der String aus `GlobalsSave/Load("MapLink", …)`. **`MapLink.json` darf nicht umbenannt werden**, solange irgendein Editor-Tooling (DesktopManager oder Nachbau) per Mod-Tag lädt. Ordnername `Schemas/` ebenso beibehalten (Baseline-Zielstruktur §2.5 sieht das bereits vor).
- **Rebrand-Hinweis:** Die Lade-URL zeigt hart auf daemonforges Repo/stable-Branch. Da der DesktopManager laut Baseline-Empfehlung E1 **nicht** übernommen wird, gibt es im Ziel zunächst keinen Konsumenten — Schema trotzdem mitnehmen (Dokumentationswert + Basis für spätere eigene Settings-UI; Header-Vorlage (b) „unverändert übernommen" aus §4.3 der Baseline).
- ⚠ **Inhaltlich veraltet gegenüber dem DME-Fork:** Die DME-MapLink-Version hat `UServerData.ItemBlacklist` (ServerData.c:10) ergänzt; das Schema (`Servers.items`: `required` + `additionalProperties:false` ohne `ItemBlacklist`) würde gültige DME-Configs **ablehnen**. Bei Übernahme Schema um `ItemBlacklist: {type:"array", items:{type:"string"}}` erweitern — der Dateiname ist Kontrakt, der Inhalt ist weiterentwickelbar.

---

## 5. Versions-Handshake „1.3.2"

| Wo | Mechanik |
|---|---|
| EnScript-Konstante | `UAPI_VERSION = "1.3.2"` (`1_Core/Constants.c:1`) |
| Node-Version | `global.APIVERSION` aus `package.json:version` = `1.3.2` (`app.js:1-3`) |
| Handshake | Server-seitig bei `Init()`/Token-Erhalt: `UApi().api().Status(...)` → `/Status` liefert `Version` → `UApiStatus.CheckVersion(UAPI_VERSION)` (`StatusObject.c:18-51`) vergleicht **major.minor.patch** und liefert Offset ±1 (Patch), ±2 (Minor), ±3 (Major) |
| Verhalten bei Mismatch | Offset +1/-1: nur Print-Hinweis · +2/-2: `Error(...)`-Log „should be updated right away" · >+2/<-2: **`Error2`-Popup** („Needs Update") — Betrieb läuft weiter, `m_UApiOnline` wird davon nicht ausgeschaltet (`UniversalApi.c:498-518`). `Error:"noauth"` im Status setzt dagegen `m_UApiOnline=false`. |
| Zweiter Kanal (nur Node) | `CheckForNewVersion:true` ⇒ `CheckRecentVersion()` ruft **`api.github.com/repos/daemonforge/DayZ-UniveralApi/releases`** ab und warnt im Log bei Abweichung (`utils.js:237-261`, `app.js:5`). Kein Wire-Effekt Richtung Mod. |

**Empfehlung fürs Rebrand:**
1. Konstante heißt künftig `DME_API_VERSION`, **Wert bleibt `"1.3.2"`**, und `ApiService/package.json:version` bleibt synchron `1.3.2` — beide Seiten werden ohnehin nur gemeinsam ausgeliefert; ein „Rebrand-Bump" würde nur kosmetische Offset-Warnungen erzeugen. Künftige eigene Releases: **beide Seiten synchron** bumpen, Format zwingend dreiteilig `x.y.z` (`CheckVersion` macht ungeprüfte `Split(".")→Get(0..2)`-Zugriffe — ein zweiteiliges „2.0" führt zu Out-of-bounds).
2. `CheckRecentVersion`/GitHub-Poll **entschärfen**: entweder Default `CheckForNewVersion:false` in der ausgelieferten Config oder Funktion aufs eigene Repo umstellen bzw. entfernen. Sonst telefoniert jeder Psyerns-Server zu daemonforges Release-Feed und loggt dauerhaft falsche „out of date"-Warnungen (deren Releases korrelieren nie wieder mit unserer Version). Auch `global.NEWVERSIONDOWNLOAD` (Download-Link im Log) zeigt auf daemonforge.
3. Der `/Status`-Feldname `Version` und die Offset-Semantik bleiben unverändert (Wire-Contract §1.3).

---

## 6. Risiken & Auffälligkeiten

1. **Doppelbetrieb UFramework:** Läuft auf dem Live-Server aktuell das UFramework-Workshop-PBO (MapLinks requiredAddon), darf das gemergte Psyerns_Framework nicht parallel dieselben Mongo-Collections/Node-Instanz bedienen, bevor MapLink umgestellt ist — sonst zwei Frameworks mit identischen Globals-/Players-Zugriffen. Umstellung als ein Schnitt deployen (Framework + nachgezogenes MapLink + Node-Service zusammen).
2. **Toter Endpunkt `Discord/GetWithPlainId/`** (`DiscordRest.c:97,255`): Node 1.3.2 kennt die Route nicht → 501. Kein Handlungszwang für den Wire-Contract (Antwort ist wohldefiniert), aber beim Rename als bekannter Alt-Skew markieren; die aktiven Pfade laufen über `UniversalDSEndpoint` (deckungsgleich mit Node).
3. **Upstream-Bug `globals.js:150` (`runUpdate`):** referenziert `CheckPlayerAuth`/`GUID`, die dort weder importiert noch definiert sind → Client-Auth-Zweig wirft ReferenceError (Express→500). Server-Auth-Pfad funktioniert. Verhalten ist de-facto-Kontrakt „Globals/Update ist Server-only"; bei Modernisierung bewusst entscheiden (fixen vs. dokumentieren), nicht stillschweigend ändern.
4. **Auth-über-Content-Type ist fragil:** Reverse-Proxies/CDN (Cloudflare-Normalisierung, nginx `proxy_set_header`) können den Nicht-Standard-Content-Type verwerfen/ersetzen → Totalausfall der Auth. In Betriebs-Doku aufnehmen: Service möglichst direkt exponieren oder Proxy so konfigurieren, dass Content-Type unangetastet bleibt. Mechanismus selbst NIE ändern (§1.1).
5. **`ServerAuth` als Array** ist ein dokumentationswürdiges Feature (Key-Rotation: neuer Key vorn = Signing-Key, alte bleiben gültig) — EnScript-Seite kennt nur einen String; bei Rotation zuerst Node-Array erweitern, dann Mod-Config tauschen.
6. **64-MB-Body-Limit** (`app.js:78`) ist bewusst groß (EntityStore/ItemStore-Payloads) — nicht „härten" ohne Abgleich mit UApiEntityStore-Nutzung.
7. **`CreateIndexes`/Cluster-Race:** Node schreibt `config.json` zur Laufzeit zurück (CheckIndexes, IP/LetsEncypt-Defaults) — bei `cpuCount>1` konkurrierende Writes möglich (upstream-Bestand). Betriebshinweis, kein Wire-Thema.
8. **Rename-Kaskade in MapLink:** `SimpleValueStore` und `UCurrency` heißen in 1.3.2 exakt wie von MapLink erwartet — **nach** dem geplanten Rename (→ `DME_Api_SimpleValueStore`, `DME_Api_Currency`) bricht MapLink auch an diesen zwei Stellen; in der Nachzieh-Checkliste (§3.1 #16/#17) enthalten.
9. **Schema vs. DME-Fork-Drift** (§4): `ItemBlacklist` fehlt im Schema; ohne Update validiert künftiges Settings-Tooling die Live-Config als ungültig.
10. **QnA-Wire-Felder lowercase** (`answer`, `score` — QnAAnswer/QnAMaker.js): leicht zu übersehende Ausnahme von der PascalCase-Konvention; nie „normalisieren".
11. **`/Status` macht einen Schreibtest** (Upsert in `Globals`, Mod `UniversalApiStatus`): Monitoring-Tools, die /Status pollen, erzeugen Writes — Mod-Tag `UniversalApiStatus` ist damit reserviert (nicht als Mod-Tag vergeben; beim Rebrand NICHT umbenennen, alte DBs enthalten ihn bereits).
12. **MapLink referenziert `MissionBase extends MissionBaseWorld`-Kette:** Das Ziel-Framework moddet MissionBase/MissionBaseWorld bereits (PF_) — die UApi-Hooks (`…Ready/…ReadyTokenReceived`) müssen beim Single-PBO-Merge in **dieselben** modded-Definitionen integriert werden (Baseline §6.4); MapLinks `modded MissionBase`/`modded MissionGameplay` overriden sie dann regulär.

---

## Anhang — Primärquellen (alle gelesen/gegriffen)

EnScript Quelle: `_UniversalApi/scripts/3_Game/UApi/{UniversalApi.c, UniversalRest.c, ConfigLoader.c, QnAMakerConfig.c, DiscordRest.c, Logger.c, Endpoints/{UApiEndpointBase.c, UApiDBEndpoint.c, UApiDBGlobalEndpoint.c, UApiAPIEndpoint.c, UApiDSEndpoint.c, UApiDBHandler.c, UApiGlobalHandler.c}, CallBacks/{UApiDBCallBack.c, UApiAuthCallBack.c}, Objects/{StatusObject.c, UApiMiscObjects.c, UApiServerStatus.c, ConfigBase.c, ApiForwarder.c, QnAObjects.c}}`, `_UniversalApi/scripts/1_Core/UApi/Constants.c`, `_UniversalApi/scripts/4_World/UApi/Mission.c`, `_UniversalApi/scripts/5_Mission/UApi/{MissionBase.c, MissionServer.c}`, `_UAPIBase/scripts/3_Game/UApi/{SimpleStore.c, Currencys.c, JsonHandler.c, ObjectBase.c, Logger.c}` ·
Node: `DayZWebService/{app.js, utils.js, AuthChecker.js, Auth.js, configLoader.js, sample-config.json, player.js, Object.js, globals.js, Query.js, Transaction.js, logger.js, serverquery.js, TrueRandom.js, crypto.js, QnAMaker.js, apiFowarder.js, witConnector.js, luisConnector.js, TranslateConnector.js, toxicityConnector.js, discordConnector.js (Routen)}` ·
Schemas: `Schemas/{MapLink.json, README.MD}`, `DesktopManager/src/scripts/editor.js` ·
Ziel: `PsyernsFrameworkConfig.example.json`, `scripts/3_Game/Psyerns_Framework/Web/Config/PF_WebConfig.c`, `REST/Config/PF_RestConfig.c` ·
MapLink: alle 31 `.c` unter `DME_Map-Link/MapLink/scripts/` + `MapLink/config.cpp` (UF-Symbole per grep vollerfasst, Aufrufstellen einzeln gelesen).

*Erstellt durch Audit-Agent 1D, 2026-07-10. Keine Code-Änderungen vorgenommen.*

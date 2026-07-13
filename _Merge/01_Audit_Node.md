# Phase 1B — Audit: Node-Webservice (DayZWebService)

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework
**Gegenstand:** `DeamonForge\DayZ-UniveralApi-1.3.2\DayZWebService\` — 34 Dateien, ~5.200 LOC (23 JS-Module, EJS-Templates, sample-config.json)
**Datum:** 2026-07-09 · **Phase:** 1 (nur Analyse, keine Code-Änderungen, kein npm install)
**Stack:** Express 4 · MongoDB-Driver 4 · discord.js 13 · jsonwebtoken 8 · @tensorflow-toxicity 3.11 · gamedig 4 · greenlock-express 4 · node-fetch 2
**Scope-Ausschluss:** DesktopManager (Owner-Entscheid, nicht portiert). `bin.js` = `require('.')`, pkg-Build-Entry.

> **INTEROP-Bindung (aus 00_Baseline §2.8): unverändert lassen** — HTTP-Routen (`/Object /Player /Globals + Alias /Gobals /GetAuth /Status /QnAMaker /QnA /Forward /Logger /Discord /Wit /LUIS /Translate /ServerQuery /Toxicity /Random /Crypto`), Header **`auth-key`** + `ExtractAuthKey`-Content-Type-Hack, Mongo-Collections (`Players Objects Globals Logs QnAMaker`), DB-Default `DayZ`, alle config.json-Keys inkl. Tippfehler **`LetsEncypt`**, JWT-Wire (`expiresIn: 2800`), Response-Feldnamen (`Status`, `Error`, `Version`, …), Status-Sentinel `Mod: "UniversalApiStatus"`. Modernisierung darf **keinen** dieser Strings ändern.

---

## 1. Dep-Upgrade-Matrix

Legende Aufwand/Risiko: 🟢 trivial · 🟡 mittel · 🔴 großer Umbau. Zielversionen = aktuelle Stable, Node-LTS-kompatibel (bei Lockfile-Erstellung final verifizieren).

| Paket | Ist | Ziel | R | Breaking-Changes, die DIESEN Code treffen (Datei:Zeile) |
|---|---|---|:--:|---|
| **discord.js** | ^13.7.0 | **14.26.x** | 🔴 | Voll-Rewrite in `discordConnector.js`, siehe §4. Intents-Objekt→`GatewayIntentBits`-Array (10-17); String-Permissions→`PermissionFlagsBits` (854, 1108, 1175); `voice.channelID`→`channelId` (595,646,704,744,777,779,815,856); `channels.create(name,opts)`→`create({name,...opts})` (971); `{embed:…}`→`{embeds:[EmbedBuilder]}` (1097/1112); `Collection.array()` entfernt (1178, **bereits in v13 tot**); `disconnect`-Event entfernt (67); `reference.messageID`→`messageId` (1212); `DiscordAPIError`-Message-Vergleich bricht (905). Node ≥18 (offiziell ≥22.12). |
| **mongodb** | ^4.6.0 | **6.18.x** | 🟡 | **KEINE Callback-Umbauten nötig** — Code ist bereits vollständig `async/await`. `useUnifiedTopology`-Option entfernt → **29 Vorkommen löschen** (kein Fehler, nur no-op-Warnung). `result.ops[0]` in mongodb 4+ entfernt → `Object.js:54` (toter Assign, harmlos, entfernen). Result-Shapes `matchedCount/upsertedCount/modifiedCount/insertedId/insertedCount` in v6 unverändert → OK. `.find/.sort/.limit/.toArray/.distinct/.countDocuments` API-stabil. **Anti-Pattern (kein Breaking, aber jetzt fixen): 31× `new MongoClient` pro Request** (connect/close je Call, 12 Dateien) → 1 gepoolter Shared-Client. Node ≥16.20. |
| **jsonwebtoken** | ^8.5.1 | **9.0.2** | 🟡 | CVE-Fix (CVE-2022-23529 u. a.). v9 verlangt striktere Optionen; `verify()` ohne `algorithms`-Pin = Alg-Confusion-Risiko → in 3 Callsites `{algorithms:['HS256']}` ergänzen. **Zugleich Bugfix:** die 3 `verify(token,secret,function(err,decoded){…})`-Callback-Formen (`AuthChecker.js:19,39,60`) verwerfen ihren Return-Wert (verify liefert im Callback-Modus `undefined`) → auf **synchrones** `verify`+try/catch umstellen (siehe §2/§4). |
| **node-fetch** | ^2.6.7 | **entfernen** → natives `fetch` | 🟡 | Node ≥18 hat globales `fetch`. 6 `require('node-fetch')` (discord 20, luis 5, apiFowarder 2, Translate 4, QnAMaker 5) + `global.fetch=nodeFetch` (`app.js:22-23`) streichen. `crypto.js`/`TrueRandom.js`/`utils.js` nutzen bereits bares `fetch` (Global) → laufen dann nativ. **Nicht** node-fetch 3 nehmen (ESM-only, bricht CommonJS). `URLSearchParams`-Body + `response.json()` von nativem fetch unterstützt. |
| **express** | ^4.18.1 | **4.21.x** (empf.) / 5.1.x optional | 🟢/🟡 | 4.21 = Drop-in (nur Bugfix/Sec). Express 5 optional: Router-Pfad-Syntax + `req.query`-Getter + Middleware-Error-Propagation ändern sich → für dieses Projekt **jetzt bei 4.21 bleiben**, 5 als eigener Schritt. |
| **express-rate-limit** | ^6.4.0 | **7.5.x** | 🟡 | `onLimitReached` **entfernt** (v7) → 3 Definitionen betroffen (`app.js:58`, `utils.js:306` GenerateLimiter, `apiFowarder.js:29`); Logik in `handler` verlagern. Standard-Header-Verhalten geändert (`standardHeaders`/`legacyHeaders` explizit setzen). `keyGenerator`/`skip` müssen `trust proxy` beachten (Sicherheits-Konsequenz, §5). |
| **@tensorflow/tfjs, -core, -converter** | ^3.11.0 | **4.22.x** (oder streichen) | 🟡 | Nur in `toxicityConnector.js` genutzt. tfjs 4 API-kompatibel für diesen Trivial-Use (`toxicity.load().then(model=>model.classify())`), aber ~250 MB native Deps + langsamer Cold-Start. Empfehlung §3. |
| **@tensorflow-models/toxicity** | ^1.2.2 | 1.2.2 (unverändert) | 🟡 | Braucht tfjs 4; sonst siehe §3. |
| **gamedig** | ^4.0.4 | **5.x** | 🟡 | Nur `serverquery.js:1,51` (`query({type:'dayz',…})`). v5: Import/Export geändert (`GameDig.query` statt destrukturiertem `{query}`), Options-/Result-Shape teils angepasst (`state.raw.*`, `state.connect` prüfen). Betrifft 1 Datei. |
| **greenlock-express** | ^4.0.3 | **entfernen** | 🟡 | Paket **verwaist/unmaintained**. Nutzung nur `app.js:125-183` (LetsEncypt-Zweig). Empfehlung §5/§6: TLS am Reverse-Proxy (nginx/Caddy) oder certbot terminieren, Service nur `http` hinter Proxy; greenlock + `LetsEncypt`-Auto-Setup (`configLoader.js:66-96`) raus. Config-Key `LetsEncypt` (Tippfehler) im Schema behalten, aber deaktiviert. |
| **crypto** (npm) | ^1.0.1 | **entfernen** | 🟢 | Deprecated Stub, **überschattet Node-Builtin** → Risiko. Code nutzt echtes `require('crypto')` (Builtin). Aus package.json löschen. |
| **https** (npm) | ^1.0.0 | **entfernen** | 🟢 | Deprecated Stub für Builtin. `app.js:15` nutzt Builtin. Löschen. |
| **body-parser** | ^1.20.0 | **1.20.3** oder `express.json()` | 🟢 | Nur `app.js:16,77` (`json({limit:'64mb'})`). Express 4.16+ bringt `express.json()` eingebaut → body-parser entbehrlich. 1.20.3 = Sec-Bump falls beibehalten. |
| **ejs** | ^3.1.8 | **3.1.10** | 🟢 | CVE-2024-33883 (Template-Pollution, ejs <3.1.10). Nur Bump. Genutzt für Discord-OAuth-Templates. |
| **ejs-lint** | ^1.2.1 | 1.2.x | 🟢 | Dev-Lint der EJS-Templates (`discordConnector.js:22`). Kein Impact. |
| **log4js** | ^6.3.0 | **6.9.x** | 🟢 | `log.js`. Minor-Bump, keine relevanten Breaking-Changes im genutzten File-Appender. |
| **languagedetect** | ^2.0.0 | 2.0.0 | 🟢 | `TranslateConnector.js:5` (LibreTranslate-Autodetect). Stabil. |
| **node-wit** | ^6.2.1 | 6.x/7.x | 🟢 | `witConnector.js:1` (`new Wit({accessToken})`, `.message()`). Bei Bump v7-Changelog prüfen (gering). |
| **saslprep** | ^1.0.3 | **entfernen** oder `@mongodb-js/saslprep` | 🟢 | Nicht direkt required (transitiv für Mongo-SCRAM). mongodb 6 zieht eigenes. Aus direkten Deps nehmen; nur `pkg.assets` (`code-points.mem`) anpassen. |
| **serve-favicon** | ^2.5.0 | 2.5.0 | 🟢 | `app.js:13,89`. Stabil, niedrige Prio. |
| **websocket** | ^1.0.34 | **entfernen (verify)** | 🟢 | **Nirgends `require`d** in den 34 Dateien (vermutlich Alt-Peer von discord.js 12). Nach Gegencheck streichen. |
| dev **nodemon** | ^2.0.14 | 3.x | 🟢 | Dev-only. |
| dev **pkg** | ^5.5.2 | **verwaist** → Alternative | 🟡 | vercel/pkg archiviert. Falls Single-Binary weiter gewünscht: Node 20+ `--experimental-sea` oder `@yao-pkg/pkg`. Sonst normaler `node app.js`-Betrieb. Betrifft `pkg`-Block package.json:42-49 + `bin.js`. |

**Node-`engines`-Empfehlung:** `"engines": { "node": ">=22" }` (Node 20 EOL ~2026-04; discord.js 14.26 empfiehlt ≥22.12; mongodb 6 ≥16.20; natives fetch stabil ≥18). Ziel Node 22 LTS (Jubilee).

---

## 2. Callback→Promise-Umbauten (Zählung + Lokalisierung)

**Kernbefund: Der erwartete „große mongodb-Callback-Umbau existiert NICHT."** Der gesamte MongoDB-Zugriff ist bereits `async/await`/Promise-basiert (41 `await`-Driver-Calls über 11 Dateien). mongodb 4→6 ist hier **API-/Options-Bereinigung, keine Callback-Migration.**

**Echte Node-Style-Callbacks (err-first), die umzubauen sind: 3 — alle in `AuthChecker.js`:**

| # | Datei:Zeile | Muster | Umbau |
|---|---|---|---|
| 1 | `AuthChecker.js:19` `CheckAuth` | `verify(auth, secret, function(err,decoded){…})` | synchrones `verify()` in try/catch; `algorithms:['HS256']` |
| 2 | `AuthChecker.js:39` `CheckAuthAgainstGUID` | dito | dito |
| 3 | `AuthChecker.js:60` `AuthPlayerGuid` | dito | dito |

> **Zusatz-Befund (Korrektheit):** Diese 3 `verify`-Callback-Formen geben `undefined` zurück (der Callback-Rückgabewert wird von `jsonwebtoken` verworfen). `CheckAuth`/`AuthPlayerGuid` liefern daher effektiv immer falsy → der **Client-JWT-Auth-Pfad ist praktisch funktionslos**, nur `CheckServerAuth` (Plain-Key-Vergleich) trägt. Der Umbau auf synchrones `verify` repariert das nebenbei — Verhalten vor/nach mit Live-Client gegentesten (Interop!).

**MongoDB 4→6 Mechanik-Umbauten (kein Callback, aber Pflicht/empfohlen):**
- `useUnifiedTopology: true` entfernen — **29 Vorkommen / 12 Dateien**.
- `new MongoClient` pro Request → 1 gepoolter Shared-Client — **31 Instanzen / 12 Dateien** (Perf + Stabilität; jeder Request macht sonst Handshake+Auth+TLS).
- `result.ops[0]` toter Zugriff — **1** (`Object.js:54`).

**Promise-Chains (`.then().catch()`), Kandidaten für async/await-Cleanup — optional, nicht blockierend:**
- `discordConnector.js`: `channel.delete().then().catch()` (1001), `channel.edit().then().catch()` (1042), `roles.add().catch()` (603), `roles.remove().catch()` (651), `client.login()` try/catch (34), diverse `fetch().then(r=>r.json())`.
- `toxicityConnector.js:54,58`: verschachtelte `toxicity.load().then(model=>model.classify().then(...))` → 2 Ebenen, unsauberes Error-Handling (inneres try/catch greift nicht im then) → auf await umstellen.
- `crypto.js:120-147`: `(await fetch()).json().then()` Mischform + `while(x===undefined) await wait(9)`-Busy-Wait → Umbau empfohlen.
- `serverquery.js:51-85`: `query().then().catch()` → await.

**Event-Listener (KEINE Conversions, nur zur Klarstellung):** `client.on(ready/disconnect/error/warn)`, `cluster.on('exit')`, `server.on('error')`, `setTimeout(CheckIndexes)` — bleiben Callbacks.

---

## 3. TensorFlow-Toxicity — Empfehlung

**Nutzungsumfang:** genau **1 Datei** (`toxicityConnector.js`), 1 Route `/Toxicity`. Lädt bei **jedem Request** das Modell neu (`toxicity.load(threshold).then(...)`, Zeile 54) — kein Caching, hoher Overhead. Deps `@tensorflow/tfjs(+core+converter)` + toxicity-Modell ziehen ~250 MB native Binaries, verlangsamen `npm install`, Cold-Start und Docker-Images erheblich; blockiert zudem `pkg`-Builds.

**Empfehlung: OPTIONAL machen (lazy + Feature-Flag), nicht im Default-Pfad.**
- Ein-Satz-Begründung: Ein einzelner selten genutzter Endpunkt rechtfertigt keine 250-MB-ML-Dauerlast im Kernservice — hinter Config-Flag (`Toxicity.Enabled`) lazy laden und Modell **einmalig** cachen (statt pro Request), sonst Route mit `501/Disabled` beantworten.
- Falls behalten: tfjs auf 4.22 heben **und** Modell-Load aus dem Request-Handler in einen einmaligen Init ziehen (Bug-Fix nebenbei).
- Streichen ist vertretbar, falls kein Konsument den Endpunkt nutzt (vor Entfernen EnScript-Seite `/Toxicity` prüfen — Interop-Route, daher Flag > Löschen).

---

## 4. discord.js 13→14 — alle betroffenen Stellen (nur `discordConnector.js`)

~30 Callsites in 8 Breaking-Kategorien (alle in einer Datei):

1. **Intents** (10-17): `new Intents()` + `Intents.FLAGS.GUILDS|GUILD_MEMBERS|DIRECT_MESSAGES|DIRECT_MESSAGE_REACTIONS|GUILD_INVITES|GUILD_MESSAGES|GUILD_MESSAGE_REACTIONS|GUILD_VOICE_STATES` → `GatewayIntentBits`-Array. Zusatz: für DM-Inhalte/`MessageContent` ggf. Intent + Partials (`Partials.Channel`) ergänzen.
2. **Permission-Flags** (854×2, 1108×2, 1175×2): `.permissionsFor(x).has('VIEW_CHANNEL'|'CONNECT'|'SEND_MESSAGES'|'READ_MESSAGE_HISTORY')` → `PermissionFlagsBits.ViewChannel|Connect|SendMessages|ReadMessageHistory`.
3. **Voice-Casing** (595,646,704,744,777×2,779,815×2,856): `player.voice.channelID` → `channelId`.
4. **Channel-Create** (971): `guild.channels.create(RawData.Name, options)` → `guild.channels.create({ name: RawData.Name, ...options })`. **Interop:** `options`-Shape (Typ-Enums `ChannelType`, Permission-Overwrites) kommt aus EnScript `DME_Api_ChannelCreateOptions` — Mapping-Layer nötig, Wire-Feldnamen behalten.
5. **Embeds** (1097, 1112): `{embed: RawData}` → `{embeds: [EmbedBuilder|APIEmbed]}`. **Interop-kritisch:** EnScript sendet Embed-JSON (`DME_Api_DiscordEmbed`) als Body — v14-Embed-Feldnamen müssen die vom Client gesendeten Keys 1:1 aufnehmen.
6. **Collection.array()** (1178): `(await channel.messages.fetch(filter)).array()` → `[...coll.values()]`. (Bereits in v13 entfernt → Endpoint `/Discord/Channel/Messages` derzeit vermutlich defekt.)
7. **Events** (59,67,70,73): `'ready'` → `Events.ClientReady` (String deprecated); **`'disconnect'` in v14 entfernt** (67 feuert nie) → `shardDisconnect`.
8. **Message-Casing/Errors** (1212 `reference.messageID`→`messageId`; 905 `DiscordAPIError`-Message-String-Vergleich → v14 nutzt Error-Codes, brittle Vergleich ersetzen).

Stabil in v14 (kein Change nötig): `guild.members.fetch`, `guild.roles.fetch`, `guild.channels.cache.get`, `client.users.fetch`, `user.createDM`, `dm.send`, `player.roles.add/remove/cache.has`, `member.setNickname`, `voice.setMute/kick/setChannel`.

---

## 5. Sicherheits-Snapshot

**Top-5 (nach Schwere):**

| # | Fund | Datei:Zeile | Schwere |
|---|---|---|:--:|
| S1 | **TLS-Privatkey im Repo ausgeliefert.** `defaultkeys.json` enthält realen PEM-Privatschlüssel + selbstsigniertes Cert; wird als HTTPS-Default verwendet, wenn keine Certs konfiguriert. Öffentlicher Key = jeder kann Traffic entschlüsseln/MITM. | `app.js:116-121`, `defaultkeys.json:2-3` | 🔴 |
| S2 | **JWT-Signaturschlüssel = ServerAuth-Bearer-Token**, und Erstgenerierung mit `Math.random()` (kein CSPRNG). Der als `auth-key` auf **jedem** Server-Request übertragene Klartext-Key ist zugleich das HMAC-Signing-Secret → Leak ⇒ beliebige Spieler-Token fälschbar. Zusätzlich Default-`ServerAuth` hartkodiert in `sample-config.json:3`. | `AuthChecker.js:112-126` (GetSigningAuth/makeAuthToken), `utils.js:92-100` (`Math.random`) | 🔴 |
| S3 | **NoSQL-/Operator-Injection.** Client-kontrollierte Strings via `JSON.parse` direkt in Mongo-Query/Update: `Query.js:50-51` (`JSON.parse(RawData.Query/OrderBy)` → `.find()`, erlaubt `$where`/`$regex`-DoS); String-konkatenierte Update-Docs aus URL-Param `mod`/`GUID`/`element` in `player.js:59,117,169,287`, `Object.js:150-156`, `globals.js:124`. Kein Escaping der Struktur-Delimiter. | s. links | 🔴 |
| S4 | **jsonwebtoken 8.5.1 (bekannte CVEs) + `verify` ohne `algorithms`-Pin** → Alg-Confusion/CVE-2022-23529-Umfeld. | `package.json:28`, `AuthChecker.js:19,39,60` | 🟠 |
| S5 | **Rate-Limit & Whitelist auf spoofbare Forwarded-IP-Header.** `keyGenerator`/`skip` vertrauen `CF-Connecting-IP`/`x-forwarded-for` **ohne** `trust proxy`-Validierung → Limit-Bypass + `RateLimitWhiteList`-Bypass per Header-Spoofing. | `app.js:55-65`, `utils.js:303-313`, `apiFowarder.js:26-36` | 🟠 |

**Weitere (dokumentiert, nicht Top-5):**
- **SSRF im Forwarder:** `/Forward` holt beliebige vom Client gelieferte `RawData.URL` (`apiFowarder.js:60`) — nur durch Auth geschützt, kein Host-Allowlist. Bei `AllowClientWrite`/geleaktem Token interner-Netz-Zugriff. 🟠
- **Debug-Logging von Secrets:** `debug===2` loggt `auth-key` + Body auf Konsole (`utils.js:285-287`). 🟡
- **`AllowClientWrite` + Client-JWT** erlaubt beliebige DB-Feld-Writes (kein Schema/Whitelist außer inkonsistentem `RemoveBadProperties`). 🟡
- **Kein Helmet/Security-Header, kein Body-Type-Enforcement** (Content-Type-Hack in `ExtractAuthKey` setzt pauschal `application/json`). 🟡
- **`greenlock-express` verwaist** — unmaintainte TLS-/ACME-Lib im Sicherheits-Pfad. 🟠 (→ §6)
- Kein `eval`/`exec`/`child_process` gefunden. ✅ Crypto: nur `sha256`/HMAC (kein MD5/SHA1 in Sicherheitsfunktion).

---

## 6. Start-/Betriebspfad & Modernisierung

**Ist-Start:** `npm start` → `node app.js` (oder pkg-Binary via `bin.js`). `app.js` → `configLoader` erzeugt beim Erststart `config.json` aus `sample-config.json` und generiert `ServerAuth` (`Math.random`). `Start()`: bei `cpuCount>1` `cluster.fork()` je CPU; sonst Single-Process. `CheckIndexes` (Mongo-Indizes auf `Players/Objects/Globals`) + optional `CheckRecentVersion` (GitHub-Poll gegen daemonforge-Repo).

**Ports/TLS (Ist):** `Port = process.env.PORT || config.Port || 8443` (sample-config: 443). Zwei TLS-Wege: (a) `LetsEncypt.Enabled` → greenlock-express bindet 80+443 mit Auto-ACME; (b) sonst `https.createServer({key,cert})` aus `config.Certificate`/`CertificateKey`-Dateien, **Fallback** = eingebettetes Self-Signed aus `defaultkeys.json`.

**Änderungen durch Modernisierung:**
1. **TLS raus aus Node:** greenlock entfernen; Service als reines `http.createServer` **hinter Reverse-Proxy** (nginx/Caddy/Cloudflare) betreiben, der TLS + ACME terminiert. `defaultkeys.json`-Fallback **streichen** (S1). `LetsEncypt`-Config-Key bleibt (Interop) aber deaktiviert/ignoriert.
2. **Mongo-Client poolen:** 1 Shared-`MongoClient` statt 31× pro Request; `useUnifiedTopology` weg.
3. **fetch nativ:** node-fetch + `global.fetch`-Zuweisung entfernen (Node ≥18).
4. **Secrets:** `ServerAuth`-Erstgenerierung auf `crypto.randomBytes` (CSPRNG); JWT-Signing-Secret **vom** ServerAuth-Bearer entkoppeln (eigener `JwtSecret`-Key — aber Achtung: neuer Config-Key, Interop mit EnScript-Seite prüfen, ggf. optional mit Fallback).
5. **Rate-Limit v7:** `trust proxy` gezielt setzen, `onLimitReached`→`handler`, Header-Modus explizit.
6. **cluster:** hinter Proxy meist entbehrlich (Proxy load-balanced) — optional beibehalten.
7. **`engines`:** `node >=22`; `pkg` (verwaist) durch Node-SEA/`@yao-pkg/pkg` ersetzen oder Plain-Node-Betrieb.
8. **Zielordner** (aus Baseline §2.4): `DayZWebService/` → `Psyerns_Framework/ApiService/`, package `name` → `psyerns-api-service` (kein Wire-Impact).

---

## 7. Aufwandsschätzung Phase 2 (Node)

| Arbeitspaket | Aufwand (Dev-Tage) |
|---|---|
| Dep-Bumps + package.json/`engines` + Stubs (crypto/https/websocket) raus | 0,5 |
| node-fetch → natives fetch (7 Stellen) | 0,5 |
| mongodb 4→6: `useUnifiedTopology` (29) + `result.ops` + **Shared-Client-Refactor** (31/12 Dateien) inkl. Test | 2,0–3,0 |
| jsonwebtoken 8→9: 3 verify-Callbacks → sync + `algorithms`, Client-Auth-Pfad-Reparatur + Gegentest | 0,5–1,0 |
| express-rate-limit 6→7 (3 Limiter, `trust proxy`) | 0,5 |
| **discord.js 13→14** (8 Kategorien, ~30 Stellen, Embed/Options-Interop-Mapping) + Live-Bot-Test | 2,0–3,0 |
| greenlock raus + Reverse-Proxy-TLS + Start-Pfad/Docs + Config-Migration | 1,0 |
| TensorFlow: lazy + Feature-Flag (oder streichen) | 0,5–1,0 |
| gamedig 4→5 (1 Datei) + Test | 0,5 |
| Security-Fixes (S1 TLS-Key, S2 Secrets, S3 Injection-Hardening, S5 trust-proxy) | 1,5–2,5 |
| Interop-Regressionstest gegen EnScript-Client + Mongo (alle Routen) | 1,0–2,0 |
| **Summe** | **≈ 11–16 Dev-Tage** (2–3 Wochen, 1 Entwickler) |

Kritischer Pfad/Aufwandstreiber: **discord.js 14** + **mongodb-Shared-Client-Refactor** + **Security (Injection/TLS/Secrets)**. Alles Übrige ist mechanisch. Kein Blocker, der die Portierung verhindert.

---

*Erstellt durch Audit-Agent 1B (Node-Webservice), 2026-07-09. Datenbasis: vollständige Lektüre aller 23 JS-Module + Configs; greps für Callsite-Zählung; Versionsabgleich (discord.js 14.26.x, mongodb 6.18.x). Keine Code-Änderungen, kein npm install.*

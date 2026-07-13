# Phase 2+3 — Umsetzung: PHP-Fixes, Web-UI-Rebrand, Header & Carve-out-LICENSE

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework
**Datum:** 2026-07-10 · **Agent:** PHP+Web-UI-Merge (Phase 2+3) · **Modus:** Datei-Änderungen, KEIN Deployment
**Basis:** `_Merge/01_Audit_PHP_WebUI.md`, `_Merge/00_Baseline_Rename_Relicense.md`
**Verifikation:** PHP 8.4.22 `php -l` (alle geänderten Dateien clean) · PHP-Funktionstests (13/13) · EJS-Render mit `ejs` (14/14, inkl. `defaultTemplates.json`-Duplikate)

---

## 1. PHP-8-Sicherheitsfixes (F1 / F2 / F3) — erledigt

### F1 — Path Traversal im Avatar-Cache — BEHOBEN
**Datei:** `MISC/standalone/api/config.php`, Funktion `pf_get_avatar_url()`.
Erste Zeile der Funktion validiert jetzt strikt:
```php
if (!preg_match('/^\d{17}$/', $steam_id)) return '';
```
Deckt beide Angriffsflächen ab (Cache-Pfad `PF_CACHE_DIR . $steam_id . '.json'` **und** unenkodierte Interpolation in die Steam-API-URL). Nur kanonische 17-stellige SteamID64 werden akzeptiert; `../…`, Suffixe (`…​.json`), Leerwert etc. liefern `''` **bevor** irgendein Pfad gebaut wird. Verhaltensbewahrend: gültige IDs funktionieren unverändert; die frühere `empty($steam_id)`-Prüfung ist durch die (striktere) Regex ersetzt. Aufrufer (`avatar.php`, `leaderboard.php:38`, `top3.php:64`) unverändert.

### F2 — Leaderboard-API-Key: kein hash_equals, Key als Query-Param — BEHOBEN
**Datei:** `WP-Plugin_Psyerns-Leaderboard/includes/class-pf-auth.php` (komplett überarbeitet).
Angeglichen an das AuctionHouse-Vorbild (`class-psyern-ah-auth.php`):
- **`hash_equals()`** für konstante Vergleichszeit (kein Timing-Seitenkanal).
- **Header-Transport bevorzugt:** `X-API-Key`, zusätzlich `Authorization: Bearer <key>` (curl/Tooling-Parität).
- **Query/Body-Param `api_key` als Deprecated-Fallback behalten** → DayZ-Server-Bridge bricht NICHT (der Enforce-RestContext kann keine beliebigen Header setzen; sendet `?api_key=`). Läuft weiter über `get_param('api_key')` → `hash_equals`.
- **Kein Short-Circuit:** Header- und Query-Vergleich laufen beide gegen denselben `$stored`-Key, Ergebnis wird erst danach `||`-verknüpft (Timing unabhängig vom Transport).
- Rückgabe-Contract `true|WP_Error` und die Fehlermeldung „Invalid API key" (401) **unverändert** erhalten (keine Bruchstelle für Konsumenten).
- **Kein AGPL-Header** (WP-Plugin bleibt MIT — nur der Sicherheitsfix, wie vorgeschrieben).

Die WP-Bridge wurde gegen das AuctionHouse-Muster geprüft und exakt danach modelliert.

### F3 — Standalone-API: Placeholder-Key ohne Deploy-Guard + `!==` — BEHOBEN
**Datei:** `MISC/standalone/api/config.php`, Funktion `pf_validate_key()`.
- **Deploy-Guard:** Ist `PF_API_KEY` noch der Platzhalter (`CHANGE_ME_TO_A_SECRET_KEY`) oder leer, wird die Anfrage mit **HTTP 503** abgewiesen **und laut per `error_log()` gewarnt** — *vor* jedem Key-Vergleich (verhindert auch den Bypass `?api_key=CHANGE_ME_TO_A_SECRET_KEY`).
- **`hash_equals()`** statt `!==`.
- Neue Helper `pf_extract_api_key()`: liest **`X-API-Key`-Header bevorzugt**, fällt auf `?api_key=`/POST zurück (Key raus aus Access-/Proxy-Logs, ohne bestehende Uploader zu brechen).

**Design-Hinweis (bewusst):** Der Guard sitzt in `pf_validate_key()` (dem Schreib-Gate von `receive.php`), nicht als globaler Startup-`exit` in `config.php`. Grund: `config.php` wird auch von den **öffentlichen Read-Endpunkten** (`avatar/leaderboard/top3/players/status/ping`) eingebunden — ein globaler 503 würde diese lesenden Endpunkte unnötig lahmlegen. Der Guard verweigert präzise die gefährdete Operation (Writes) und lässt Reads unangetastet → maximal verhaltensbewahrend bei voller Schließung der Lücke.

**Funktionstests (`php` CLI, 13/13 grün):** Traversal/Kurz-ID/Leer/Suffix → `''`; 17-Digit akzeptiert; Header-vor-Query-Präzedenz; Fallback auf Query; Guard trippt auf ausgeliefertem Platzhalter; `hash_equals` korrekt/falsch.

---

## 2. Web-UI (EJS) — PII raus, Rebrand, water.css lokal, §13-Link

Ziel-Ort ist bereits vom Node-Agenten angelegt: `DME_Api_WebService/templates/` (3 EJS + `defaultTemplates.json` lagen als unveränderte daemonforge-Kopien vor). Bearbeitet wurden diese Ziel-Kopien.

### PII entfernt? — JA
`discordLogin.ejs`: Der clientseitige `fetch_retry('https://api.daemonforge.dev/user/<SteamId>')`-Block samt `resolveID()` wurde **vollständig entfernt**. Damit fließen keine Spieler-SteamIDs mehr an daemonforge ab. Die SteamCard rendert jetzt **nur die SteamID** (Name-Zeile + Avatar-`<img>` entfernt, keine hängenden Referenzen `cardName`/`cardAvatar`/`avatar_small`). `login()`/`Login_URL`-Flow unverändert.
**Verifiziert:** 0 Treffer für `api.daemonforge.dev` im gesamten `DME_Api_WebService/`.

### Rebrand? — JA
- **water.css lokal statt CDN:** In allen 3 EJS ersetzt das `<link … cdn.jsdelivr.net/water.css@2/dark.css>` (inkl. der zwei auskommentierten CDN-Varianten) durch einen **inline `<style>`-Block** mit dem echten water.css-v2-Dark-Build (aus dem gebündelten `DesktopManager/.../water/dark.min.css`; `@charset`/sourceMap entfernt). MIT/Kognise-Attribution als CSS-Kommentar. **Keine Third-Party-Requests mehr auf den Auth-Seiten.** Verifiziert: 0 Treffer `cdn.jsdelivr.net`.
- **Texte/Titel gebrandet:** Login-Titel „Connect To Discord — Deadmans Echo"; Success-Titel „Successfully Linked — Deadmans Echo" + „…connected your Steam account to the **Deadmans Echo** Discord". Error-Copy inhaltlich belassen (trug kein daemonforge-Branding). Die alten „EJS-Syntax-Reference"-Doku-Kommentare mit daemonforge-Wiki-URL wurden zu neutralen „Passed Variables"-Blöcken gekürzt.
- **favicon:** NICHT ersetzt — siehe offene Punkte (Binär, kein Psyerns-Asset vorhanden; wird zudem Node-seitig via `serve-favicon` ausgeliefert, nicht aus den EJS referenziert).

### §13-Footer-Link? — JA
Alle 3 EJS erhalten vor `</body>` einen prominenten Footer:
```html
<footer …><a href="https://deadmans-echo.de" rel="noopener">Source (AGPL-3.0)</a></footer>
```
Erfüllt das Corresponding-Source-Angebot für die einzigen menschenlesbaren Seiten des netzwerk-exponierten Dienstes. **URL-Vorbehalt:** `https://deadmans-echo.de` ist die einzige konkrete Projekt-URL im Repo (README-Badge, NOTICE, psyerns-mods Author-URI). Sobald das öffentliche Quell-Repo steht, sollte der Link auf die kanonische Repo-URL zeigen (siehe offene Punkte).

### defaultTemplates.json neu erzeugt? — JA
`defaultTemplates.json` (eingebettete String-Duplikate, Keys `Error`/`Login`/`Success`) wurde **aus den geheaderten/rebrandeten EJS regeneriert** (`json_encode`, `JSON_PRETTY_PRINT|JSON_UNESCAPED_SLASHES`). Damit erzeugt der Service auf frischen Installationen (`LoadLoginTemplate`/`…Success`/`…Error`) **nicht** wieder die alten daemonforge-Seiten. Die Header stehen jetzt als HTML-Kommentare *innerhalb* der JSON-Strings. Verifiziert: JSON re-parst, alle 3 Keys vorhanden, kein PII/CDN in den Duplikaten, alle 3 rendern fehlerfrei.

### Zusatz: Upstream-Bug `discordConnector.js:1421` — FIXED (1 Zeile)
`LoadSuccessTemplate()` setzte bei Lint-Fehler fälschlich `LoginTemplate = DefaultTemplates.Success` (überschrieb das Login-Template). Korrigiert zu `SuccessTemplate = DefaultTemplates.Success`. Direkt template-bezogen (Fallback der von mir gebrandeten Seiten) und daher hier miterledigt.
**Abgrenzung zum Node-Agenten:** Dies ist die **einzige** Änderung an `discordConnector.js`; es wurden **keine** JS-Header gesetzt und nichts anderes am JS-Backend angefasst (Header/Modernisierung der `*.js` = Node-Agent-Scope laut Header-Plan §4.1).

---

## 3. Header gesetzt (Zählung)

| Vorlage | Wo | Dateien |
|---|---|---|
| **(a)** daemonforge + §5-Änderungsnotiz (HTML-Kommentar) | `DME_Api_WebService/templates/*.ejs` | **3** (discordLogin, discordSuccess, discordError) |
| **(a)** — als HTML-Kommentar *in* den JSON-Strings | `defaultTemplates.json` | via Regeneration (3 Template-Strings) |
| **(c)** eigener Psyern-AGPL-Header (PHPDoc) | `MISC/standalone/api/*.php` | **9** (config, avatar, ping, whitelist, leaderboard, top3, players, receive, status) |
| **keine** (WP-Plugin bleibt MIT) | `class-pf-auth.php` | 0 — nur Sicherheitsfix |

**Summe neu geheaderte Dateien: 12** (3 EJS + 9 PHP), plus die 3 Header nun auch in `defaultTemplates.json` eingebettet.
Alle 9 PHP nach dem Header-Rollout `php -l`-clean. WP-Plugin-Dateien erhielten **keine** AGPL-Header (E4).

---

## 4. WP-Plugin-Carve-out-LICENSE (3×) — angelegt

Je Plugin-Ordner eine neue `LICENSE`-Datei (zuvor existierte keine):

| Datei | SPDX | Inhalt |
|---|---|---|
| `WP-Plugin_Psyerns-Leaderboard/LICENSE` | `MIT` | Carve-out-Hinweis („licensed under MIT, not AGPL-3.0; see NOTICE in repo root") + MIT-Volltext, © 2026 Psyern / Deadmans Echo |
| `WP-Plugin_Psyerns_AuctionHouse/LICENSE` | `MIT` | dito |
| `psyerns-mods/LICENSE` | `GPL-2.0-or-later` | Carve-out-Hinweis + GPLv2-„or later"-Notice + GPL-2.0/3.0-URLs + Begründung der §13-Brücke |

Konsistent mit dem Repo-Root-`NOTICE` (das die 3 Plugins bereits als eigenständige MIT/GPL-Inseln listet). Autor laut Plugin-Headern: „Psyern".

---

## 5. Offene Punkte (für den Orchestrator)

1. **§13-Source-URL:** In den 3 EJS-Footern steht `https://deadmans-echo.de` (einzige konkrete Projekt-URL). Sobald das öffentliche AGPL-Quell-Repo existiert, auf die kanonische Repo-URL setzen (auch im Repo-`NOTICE` §13 und README nachziehen). Randnotiz: Plugin-Header nutzen zwei Schreibweisen (`deadmansecho.com` vs. `deadmans-echo.de`) — vereinheitlichen.
2. **favicon (`DME_Api_WebService/public/favicon.ico`):** trägt daemonforge-Branding, **nicht ersetzt** (kein Psyerns-`.ico`-Asset vorhanden; Auslieferung via `serve-favicon` in `app.js` = Node-seitig). Ein gebrandetes `.ico` beibringen und ersetzen (bis dahin NOTICE-Eintrag genügt).
3. **`DME_Api_WebService/LICENSE` = GPL-3.0** (daemonforge-WebService-Stand) vs. `package.json` „AGPL-3.0-or-later" — bekannte Upstream-Inkonsistenz (Baseline §4.1). Vereinheitlichung auf AGPL-3.0 + `package.json`-Rebrand (`name`/`repository`/`author`) ist **Node-Agent-/Orchestrator-Scope**, nicht angefasst.
4. **Header-Rollout §4.2 Rest:** Aufgabe 3 war auf „MISC/standalone-eigene PHP" begrenzt (erledigt: 9 PHP). Der breitere (c)-Rollout auf `MISC/standalone/*.html`, `MISC/standalone/assets/**`, `MISC/templates`, `MISC/Themes`, `MISC/tools/*.ps1` (Audit §4.2) steht noch aus — bewusst nicht in diesen Auftrag gezogen.
5. **water.css-Attribution:** MIT/Kognise ist als CSS-Kommentar in den EJS vermerkt; im Repo-`NOTICE` bereits gelistet — passt.
6. **Empfohlene (nicht beauftragte) Audit-Punkte R1–R3 / K1–K5** (u. a. psyerns-mods `GPL-2.0+`→`GPL-2.0-or-later` im Plugin-Header, CORS-Wildcard nur auf Reads) bleiben offen — außerhalb der 3 beauftragten Fixes.

---

*Erstellt durch PHP+Web-UI-Merge-Agent (Phase 2+3), 2026-07-10. Geänderte Dateien: `config.php`, `class-pf-auth.php`, 8× `MISC/standalone/api/*.php` (Header), 3× EJS, `defaultTemplates.json`, `discordConnector.js` (1 Zeile), 3× neue Plugin-`LICENSE`. Verifikation: php -l, PHP-Funktionstests, EJS-Render-Tests — alle grün.*

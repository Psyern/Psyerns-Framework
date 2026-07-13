# Phase 1C — Audit: PHP/WordPress & Web-UI

**Merge:** DayZ-UniversalApi 1.3.2 (daemonforge, AGPL-3.0) → Psyerns_Framework
**Datum:** 2026-07-10 · **Agent:** 1C (PHP/WordPress + Web-UI) · **Modus:** nur Analyse, keine Änderungen
**Basis:** `_Merge/00_Baseline_Rename_Relicense.md` (Phase 0)
**Scope-Hinweis:** DesktopManager per Owner-Entscheid ausgeschlossen — nicht auditiert. Der AuctionHouse-Ordner heißt tatsächlich `WP-Plugin_Psyerns_AuctionHouse` (Unterstrich, nicht Bindestrich).

---

# 1. Lizenz-Header-Verifikation (Ziel-Seite)

## 1.1 Exakte Zitate (Datei + Zeile)

**Leaderboard** — `WP-Plugin_Psyerns-Leaderboard/psyerns-framework.php`, Zeilen 9–10 (Plugin „Psyerns Leaderboard", v1.1.4):

```
 * License:     MIT
 * License URI: https://opensource.org/licenses/MIT
```

**AuctionHouse** — `WP-Plugin_Psyerns_AuctionHouse/psyerns-auctionhouse.php`, Zeilen 9–10 (v1.0.0):

```
 * License:     MIT
 * License URI: https://opensource.org/licenses/MIT
```

Zusätzlich `WP-Plugin_Psyerns_AuctionHouse/readme.txt`, Zeilen 8–9:

```
License: MIT
License URI: https://opensource.org/licenses/MIT
```

**psyerns-mods** — `psyerns-mods/psyerns-mods.php`, Zeile 10 (Plugin „Psyerns Mods Showreel", v1.0.0):

```
 * License:     GPL-2.0+
```

Kein `License URI`, keine weitere Formulierung im Plugin (kein readme.txt, keine LICENSE-Datei im Ordner).

**MISC-Kopien:**
- `MISC/wordpress-plugin/psyerns-framework/psyerns-framework.php`, Zeilen 9–10: `License: MIT` + URI — **ältere Kopie v1.0.0** (Root-Plugin ist v1.1.4; Plugin Name dort noch „Psyerns Framework").
- `MISC/wordpress-plugin/psyerns-mods/psyerns-mods.php`, Zeile 10: `License: GPL-2.0+` — identisch zur Root-Version.

## 1.2 Finales Urteil

| Plugin | Header | Urteil |
|---|---|---|
| Leaderboard | `MIT` (Header + URI) | ✅ Phase-0 bestätigt. MIT ist GPL-/AGPL-kompatibel; darf im AGPL-Repo als eigenständiges MIT-Werk bleiben (Entscheid E4). |
| AuctionHouse | `MIT` (Header + URI + readme.txt) | ✅ dito. |
| psyerns-mods | `GPL-2.0+` | ✅ Phase-0 bestätigt: **„GPL-2.0+" ist der (veraltete) SPDX-Ausdruck für „GPL v2 or later"** — NICHT GPLv2-only. Via „or later" unter GPLv3 nutzbar → über die GPLv3↔AGPLv3-§13-Brücke mit dem AGPL-Gesamtwerk kombinierbar. Zusätzlich: Psyern ist Alleinurheber und könnte jederzeit umlizenzieren. |

**Kein GPLv2-only-Fall. Phase-0-Urteil §5.2 vollumfänglich bestätigt.**

Kosmetische Empfehlung: `GPL-2.0+` in beiden psyerns-mods-Kopien auf die moderne Schreibweise `GPL-2.0-or-later` normalisieren und eine `License URI: https://www.gnu.org/licenses/gpl-2.0.html` ergänzen (kein Rechts-, nur Klarheitsgewinn).

## 1.3 WordPress.org-Kompatibilität (nur notiert — Plugins sind privat gehostet)

- Das offizielle Plugin-Directory verlangt eine **GPLv2-or-later-kompatible** Lizenz. **MIT und GPL-2.0+ erfüllen das beide problemlos** — es gäbe keinerlei Hindernis für eine spätere Submission.
- Würden die Plugins dagegen auf **AGPL** umgestellt, wäre eine Directory-Submission strittig (AGPLv3 ist GPLv3-, aber nicht GPLv2-kompatibel). Bei Privathosting irrelevant — aber ein weiterer Grund, Entscheid **E4 (MIT belassen)** beizubehalten.
- Keine sonstigen WordPress.org-Anforderungen einschlägig (kein `Stable tag`-Zwang, keine readme.txt-Pflicht außerhalb des Directories). `readme.txt` existiert nur beim AuctionHouse; `Tested up to: 6.4` dort ist veraltet (kosmetisch).

---

# 2. PHP-8.x-Audit (Ziel-Plugins + MISC/standalone)

## 2.1 Methodik

- **PHP 8.4.22 CLI-Lint (`php -l`) über alle 104 .php-Dateien** der Ziel-Seite (3 Plugins + MISC/wordpress-plugin-Kopien + MISC/standalone): **104/104 fehlerfrei** — keine Parse-Inkompatibilität mit PHP 8.
- Pattern-Scans: entfernte/deprecatede Funktionen, Curly-Brace-String-Offsets, reversed `implode`, `eval`/`extract`/`unserialize`/`assert`, Dynamic Properties, curl, Superglobals, unescapte `echo`s, unprepared SQL, ORDER-BY-/LIMIT-Interpolation, Nonces, Capabilities, REST-`permission_callback`s.
- Manuelle Reviews der sicherheitsrelevanten Pfade (Auth-Klassen, Steam-OpenID, Upload-/Sync-Endpoint, Admin-Handler, Standalone-API).

## 2.2 Positivbefunde (keine Aktion nötig)

| Prüfpunkt | Ergebnis |
|---|---|
| Entfernte/deprecatede Funktionen (`create_function`, `each()`, `get_magic_quotes_*`, `FILTER_SANITIZE_STRING`, `strftime`, `utf8_en/decode`, `money_format`, `mysql_*`, `ereg`, `split()`) | **0 Treffer** |
| Curly-Brace-Offsets `$s{0}` (Fatal in PHP 8.0), reversed `implode($arr, $glue)` | **0 Treffer** |
| `eval` / `extract` / `unserialize` / `assert` | **0 Treffer** |
| **Dynamic Properties (PHP-8.2-Deprecation)** | Keine. Alle drei Kandidaten sind deklariert: `PF_Shortcodes::$enqueued` (public/class-pf-shortcodes.php:26), `Psyern_Shortcode::$enqueued` (includes/class-psyern-shortcode.php:25), `PSM_Admin::$settings_hook` (admin/class-psm-admin.php:28) |
| Direktes curl | **0** — alle HTTP-Calls via `wp_remote_get/post` (7 Aufrufe, 19 `is_wp_error`-Checks) bzw. Stream-Context (standalone) |
| SQL-Writes | Durchgängig `$wpdb->prepare()` / `$wpdb->insert()` / `$wpdb->update()` (z. B. class-psyern-database.php:112–195) |
| ORDER-BY-Interpolation | Beide Fundstellen whitelist-validiert: `class-psyern-ah-balances.php:204–207` (orderby ∈ {updated_at, balance, player_uid}, order ∈ {ASC, DESC}), `class-psyern-ah-listings.php:812–832` (switch über `self::$allowed_orderby`) |
| Nonces + Capabilities | Flächendeckend: 38 Nonce-Fundstellen (`check_admin_referer`/`check_ajax_referer`/`wp_verify_nonce` vor jeder Mutation), 12 `current_user_can`-Gates (`manage_options` bzw. `edit_posts`/`edit_post`) |
| AuctionHouse-Auth | Vorbildlich: `hash_equals()` (class-psyern-ah-auth.php:57–58), Header bevorzugt, Key-Generierung via `random_bytes(24)` (:109–114) |
| Steam-OpenID (AuctionHouse) | Solide: Single-Use-Login-Nonce als Transient (Replay-Schutz, class-psyern-ah-steam-auth.php:44,164–172), `check_authentication`-Roundtrip gegen Steam (:219–233), strikte claimed_id-Regex `7656119\d{10}` (:39) |
| Admin-Routing AH | `$_GET['tab']` via `sanitize_key` + Whitelist gegen Tab-Definitionen (class-psyern-ah-admin.php:622–630) |
| REST `__return_true`-Routen | Alle rein lesend (GET public leaderboard/status/stats/top3/player/avatar) bzw. Steam-OpenID-Flow (naturgemäß öffentlich); Writes überall key- oder capability-geschützt |
| uninstall.php (alle 3 Plugins) | `WP_UNINSTALL_PLUGIN`-Guard vorhanden |
| „Upload"-Endpoint AH | Kein Datei-Upload — reiner JSON-Daten-Sync (class-psyern-ah-upload.php); keine Upload-Security-Fläche |

## 2.3 Findings — **FIX NÖTIG** (3)

**F1 — Path Traversal im Avatar-Cache (MISC/standalone).**
`MISC/standalone/api/config.php:46`: `$cache_file = PF_CACHE_DIR . $steam_id . '.json';` — `$steam_id` kommt in `avatar.php:4` ungefiltert aus `$_GET['steam_id']` (Endpoint ist ohne API-Key erreichbar) und wird zusätzlich unenkodiert in die Steam-API-URL interpoliert (config.php:53). Ein Wert wie `../../pfad/datei` schreibt/liest `*.json` außerhalb des Cache-Verzeichnisses.
*Mitigation heute:* Bei leerem `PF_STEAM_API_KEY` bricht die Funktion vorher ab (config.php:43).
*Fix:* `if (!preg_match('/^\d{17}$/', $steam_id)) return '';` am Funktionsanfang (deckt Pfad + URL ab). Aufrufer: avatar.php:10, leaderboard.php:38, top3.php:64.

**F2 — Leaderboard-API-Key: kein `hash_equals`, Key als Query-Param.**
`WP-Plugin_Psyerns-Leaderboard/includes/class-pf-auth.php:26–29`: `$api_key = $request->get_param('api_key'); … if ( empty($stored) || $api_key !== $stored )`. Der Key schützt **Schreib-Endpunkte** (POST leaderboard/status/whitelist…). Zwei Schwächen: (a) nicht-konstanter String-Vergleich (Timing-Seitenkanal, praktisch schwer, aber trivial zu fixen), (b) Key ausschließlich per GET/POST-Param → landet in Access-/Proxy-Logs und ggf. Referern.
*Fix:* `hash_equals()` + Header-Pfad (`X-API-Key`), exakt wie es das AuctionHouse-Plugin bereits vormacht (class-psyern-ah-auth.php:57). Query-Param als deprecated Fallback behalten (DayZ-Server-Kompat), im Log dokumentieren.

**F3 — Standalone-API: Placeholder-Key ohne Deploy-Guard + `!==`-Vergleich.**
`MISC/standalone/api/config.php:2`: `define('PF_API_KEY', 'CHANGE_ME_TO_A_SECRET_KEY');` und `:12–13`: `$key = $_GET['api_key'] ?? $_POST['api_key'] ?? ''; if ($key !== PF_API_KEY)`. Wird die Datei unverändert deployt, hat jeder Schreibzugriff auf `receive.php` (Leaderboard/Whitelist/Status-Manipulation).
*Fix:* Startup-Guard (`if (PF_API_KEY === 'CHANGE_ME_TO_A_SECRET_KEY') { http_response_code(503); … exit; }`) + `hash_equals()`.

## 2.4 Findings — **EMPFOHLEN** (3)

**R1** — `psyerns-mods/includes/class-psm-leaderboard-data.php:110–112`: `api_key`-Query-Param-Fallback zusätzlich zum sauberen `X-Psyern-Api-Key`-Header (`hash_equals` ist vorhanden — gut). Fallback deprecaten, damit der Key nicht in URLs auftaucht.

**R2** — `MISC/standalone/api/config.php:8`: `Access-Control-Allow-Origin: *` global — auch auf den key-geschützten Write-Endpunkten. Für die öffentlichen GET-Reads ok; empfohlen: Wildcard nur auf Read-Endpoints setzen bzw. konfigurierbare Origin.

**R3** — Metadaten-Pflege: `Requires PHP: 7.4` (beide WP-Header) kann nach diesem Audit guten Gewissens auf `8.0`+ angehoben werden; `Tested up to: 6.4` (AH readme.txt:5) aktualisieren.

## 2.5 Findings — **KOSMETISCH** (5)

| # | Fundort | Befund |
|---|---|---|
| K1 | `whitelist-page.php:8`, `settings-page.php:232`, `class-pf-database.php:145`, `class-pf-leaderboard.php:371` | wpdb-Aufrufe ohne `prepare()` — aber ausschließlich **Tabellennamen-Interpolation** aus `$wpdb->prefix`-Konstanten (kein User-Input; Tabellennamen sind in `prepare()` ohnehin nicht bindbar). Nur phpcs-Konformität. |
| K2 | `WP-Plugin_Psyerns-Leaderboard/public/templates/leaderboard-row.php:48` | `echo $player_name;` — Variable ist in Z. 14 bereits `esc_html()`-ed. Kein XSS; Late-Escaping wäre phpcs-sauberer. |
| K3 | `WP-Plugin_Psyerns-Leaderboard/admin/views/settings-page.php:437–473` | `echo $bg/$glow/$border/…` in style-Attributen — Werte sind hartkodierte Theme-Paletten und in Z. 423–431 vorab `esc_attr()`-ed. Kein Risiko. |
| K4 | `WP-Plugin_Psyerns-Leaderboard/admin/views/whitelist-page.php:18` | `$_GET['msg']`-Literal-Vergleich ohne `wp_unslash`/`sanitize` (rein lesender Vergleich gegen `'added'`). |
| K5 | `psyerns-mods/psyerns-mods.php:10` (+ MISC-Kopie) | `GPL-2.0+` → `GPL-2.0-or-later` normalisieren, License URI ergänzen (siehe §1.2). |

**Zählung: 3 × Fix nötig · 3 × empfohlen · 5 × kosmetisch. Keine PHP-8-Parse-/Deprecation-Blocker — die drei Plugins sind PHP-8.4-tauglich.**

---

# 3. Quell-Web-UI (DayZWebService)

## 3.1 Was liefert der WebService aus? (vollständiges Inventar)

Es gibt **kein** `WebPages/`, kein `views/`, kein statisches CSS/JS-Verzeichnis und **keine Admin-UI** im DayZWebService (die Config-Editor-GUI lag ausschließlich im ausgeschlossenen DesktopManager). Verifiziert per Datei-Scan: Die gesamte HTML-Oberfläche besteht aus

| Datei | Zweck |
|---|---|
| `templates/discordLogin.ejs` | Spieler-Seite „Connect To Discord" (SteamID-Karte + Connect-Button) |
| `templates/discordSuccess.ejs` | Erfolgsseite nach OAuth-Link |
| `templates/discordError.ejs` | Fehlerseite (typisierte Fälle: AlreadyLinked, Conflict, UserNotFound, RoleRequired, …) |
| `templates/defaultTemplates.json` | **Eingebettete String-Duplikate** der drei Templates (Fallback) |
| `public/favicon.ico` | Favicon (daemonforge-Branding) — einzige statische Datei (`serve-favicon`, app.js:89) |

Alle übrigen Routen liefern JSON (app.js:90–115). **Die EJS-Seiten sind Player-facing (Discord-Link-Flow), keine Backend-Admin-Funktion** — „Admin" am WebService bedeutet ausschließlich config.json-Bearbeitung.

## 3.2 Render-/Template-Mechanismus

- Kein Express-View-Engine-Setup; gerendert wird direkt via `ejs.render()` pro Request (discordConnector.js:8, 83, 94, 108, 399–457, 540–550).
- Templates werden beim Start aus `SAVEPATH/templates/*.ejs` gelesen (SAVEPATH-Default `./` → im Normalbetrieb sind die **Repo-Dateien selbst** die Runtime-Templates); fehlen sie, werden sie aus `defaultTemplates.json` **auf Platte geschrieben** (Auto-Erzeugung, discordConnector.js:44–53, 1389–1452). Serverbetreiber können die Seiten also ohne Codeänderung anpassen — dieses Feature ist erhaltenswert.
- Custom-Templates werden per `ejs-lint` validiert, bei Fehlern Fallback auf Default.

## 3.3 Zustand, gebündelte Libs, externe Abhängigkeiten

- **Keine gebündelten Libs.** Stattdessen zwei **externe Runtime-Abhängigkeiten** in allen drei Templates bzw. im Login:
  1. `water.css@2` via jsdelivr-CDN (`discordLogin.ejs:7`, `discordSuccess.ejs:7`, `discordError.ejs:7` — identisch in `defaultTemplates.json`).
  2. **`https://api.daemonforge.dev/user/<SteamId>`** (`discordLogin.ejs:58`): Client-seitiger fetch an einen **Drittanbieter-Dienst des Original-Autors**, um Steam-Name/Avatar anzuzeigen. Folgen: Steam-IDs eurer Spieler fließen an daemonforge ab (Datenschutz), Seite hängt von Verfügbarkeit/Fortbestand dieses Dienstes ab, Fremd-Branding.
- Inline-JS ist modernes ES2017 (`async/await`-fetch_retry), Discord-Logo als Inline-SVG — kein Modernisierungsdruck am Markup selbst.
- **Upstream-Bug (report-only):** `discordConnector.js:1421` — `LoadSuccessTemplate()` weist bei Lint-Fehler fälschlich `LoginTemplate = DefaultTemplates.Success;` zu (überschreibt das Login-Template, statt `SuccessTemplate` zurückzusetzen). 1-Zeilen-Fix in der Umsetzungsphase.
- **OAuth-Flow-Anmerkung** (Node-Scope, hier nur notiert): Der `state`-Parameter trägt die rohe SteamID und wird im Callback ungeprüft als solche übernommen (discordConnector.js:412, 453, 483) — kein signierter/HMAC-State. Schutzmechanismen: AlreadyLinked-Check + optionale Country-Restriktion. Härtung (HMAC-State) ist Kandidat für die Node-Modernisierungsphase.
- Dep-Hinweis für später (Node-Phase): `onLimitReached` (app.js:58) ist in express-rate-limit ≥ 7 entfernt.

## 3.4 Empfehlung: **Übernehmen + minimal auffrischen**

**Nicht weglassen:** Der Discord-Link-Flow ist Kernfeature des Service (`PromptDiscordOnConnect`, Discord-Endpoints der EnScript-Seite) und diese drei Seiten sind seine einzige menschenlesbare Oberfläche.
**Nicht „wie ist":** Zwei externe Runtime-Abhängigkeiten und daemonforge-Branding sind für einen AGPL-Rebrand-Betrieb nicht akzeptabel.

Minimal-Auffrischung (Umsetzungsphase):

1. **(MUSS)** `api.daemonforge.dev`-Fetch entfernen. Für den Merge: Karte ohne Name/Avatar rendern (nur SteamID — Funktionsverlust minimal). Optional später: eigener `/SteamProfile`-Proxy-Endpoint mit Steam-Key in config.json (neuer Config-Key, kein Bestands-Wire betroffen).
2. **(MUSS, Rebrand)** `public/favicon.ico` ersetzen; Texte/Titel branden („our discord" etc.); **`defaultTemplates.json` aus den geänderten .ejs regenerieren** — die Duplikate müssen synchron bleiben, sonst erzeugt der Service auf frischen Installationen wieder die alten daemonforge-Seiten.
3. **(SOLL)** water.css lokal einbetten statt CDN (1 Datei ~10 KB, MIT/Kognise — Attribution ins NOTICE gem. Phase-0 §4.4; der ausgeschlossene DesktopManager bündelte sie bereits). Keine Third-Party-Requests auf Auth-Seiten.
4. **(SOLL)** LoadSuccessTemplate-Bug fixen (discordConnector.js:1421).
5. **(KANN)** AGPL-§13-Footer in alle drei Templates: `Source: <repo-url>` — siehe §4.

---

# 4. AGPL-Folgen & Header-Plan für die Web-Teile

Vorlagen (a)/(b)/(c) aus Phase 0 §4.3. Grundregel bestätigt: praktisch jede übernommene Datei wird modifiziert → Vorlage (a) ist der Standard für Quell-Dateien.

## 4.1 Ergänzungs-Header — Vorlage (a) (daemonforge-stämmig, modifiziert)

| Datei(en) | Header-Form | Anmerkung |
|---|---|---|
| `templates/discordLogin.ejs`, `discordError.ejs`, `discordSuccess.ejs` | HTML-Kommentar `<!-- … -->` am Dateianfang | Header erscheint im ausgelieferten Quelltext → **sichtbare §5-Änderungsnotiz, erwünscht** |
| `discordConnector.js` (rendert die Seiten, wird durch Rebrand modifiziert) sowie alle übrigen `DayZWebService/*.js` | `/* … */`-Block | JS-Backend im Detail = Scope des Node-Audit-Agents; hier der Vollständigkeit halber |
| `templates/defaultTemplates.json` | **Sonderfall: JSON kann keine Kommentare tragen.** | Lösung: Datei wird ohnehin aus den (geheaderten) .ejs **regeneriert** — die Header stehen dann als HTML-Kommentare *innerhalb* der JSON-Strings. Zusätzlich Attribution im ApiService-README/NOTICE. |
| `sample-config.json`, `defaultkeys.json` | JSON-Sonderfall wie oben | Nur NOTICE/README-Erwähnung |
| `public/favicon.ico` | **Binär — kein Header möglich.** | daemonforge-Branding → **ersetzen** (dann eigenes Werk, Vorlage c entfällt für Binär). Falls wider Erwarten behalten: NOTICE-Eintrag. |

**§13-Konkretisierung für die Web-UI:** Zusätzlich zur Phase-0-Maßnahme (Source-Feld im Status-JSON) empfiehlt sich ein sichtbarer `Source (AGPL-3.0)`-Footer-Link in allen drei EJS-Templates — die einzigen HTML-Seiten, die der netzwerk-exponierte Dienst an Menschen ausliefert. Damit ist das Corresponding-Source-Angebot für Endnutzer prominent erfüllt.

## 4.2 Nur neuer Psyern-AGPL-Header — Vorlage (c) (eigene Werke unter Repo-AGPL)

- `MISC/standalone/api/*.php` (9 Dateien: avatar, config, leaderboard, ping, players, receive, status, top3, whitelist), `MISC/standalone/*.html` (index, player, admin), `MISC/standalone/assets/**` (3 CSS, 1 JS)
- `MISC/templates/*.html` (4), `MISC/Themes/*.html` (5), `MISC/HTML-Plugin/**`
- `MISC/tools/*.ps1` (2; PowerShell-Kommentar `# …`)
- `.htaccess`/JSON-Daten: kein Header (Repo-Lizenz greift); `.md`-Prompts: optional (mindestens SPDX-Zeile empfohlen, Phase-0 §4.6)

## 4.3 WP-Plugins — **keine AGPL-Header** (Entscheid E4)

- `WP-Plugin_Psyerns-Leaderboard/**`, `WP-Plugin_Psyerns_AuctionHouse/**` bleiben **MIT**; `psyerns-mods/**` bleibt **GPL-2.0+** — eigenständige Werke, keine Berührung mit daemonforge-Code (verifiziert: kein UApi-/DayZWebService-Symbol in den Plugins).
- **Klarstellungs-Empfehlung:** Da das Repo-Root-LICENSE AGPL-3.0 wird, je Plugin-Ordner eine kleine `LICENSE`-Datei (MIT-Volltext bzw. GPL-2.0-Hinweis) einlegen und im Repo-README einen Abschnitt „Lizenz je Komponente" ergänzen — sonst ist die MIT/GPL-Insel im AGPL-Repo nur über die Plugin-Header erkennbar.
- **Regel (Phase 0, bestätigt):** Wird später Code aus dem AGPL-Kern (UApi/ApiService) in ein Plugin kopiert, muss dieses Plugin AGPL-kompatibel nachziehen.
- **Dubletten-Hinweis:** `MISC/wordpress-plugin/psyerns-framework/` ist eine veraltete v1.0.0-Kopie (Root: v1.1.4), `MISC/wordpress-plugin/psyerns-mods/` eine Kopie der Root-Version, dazu zwei .zip-Snapshots. Empfehlung: als Archiv markieren oder entfernen (Divergenz-Risiko bei künftigen Header-Rollouts — sonst müssen Header-Änderungen doppelt gepflegt werden).

---

# 5. Offene Punkte für den Orchestrator

| # | Punkt | Empfehlung 1C |
|---|---|---|
| P1 | Steam-Name/Avatar auf der Discord-Login-Seite nach Entfall von api.daemonforge.dev | Für den Merge: nur SteamID anzeigen; optional später eigener Proxy-Endpoint (neuer Config-Key) |
| P2 | LICENSE-Dateien in den drei Plugin-Ordnern (MIT/GPL-Carve-out im AGPL-Repo) | Ja, anlegen |
| P3 | MISC/wordpress-plugin-Altkopien | Archivieren oder löschen |
| P4 | Fixes F1–F3 (Security) | In Umsetzungsphase einplanen — unabhängig vom Merge sinnvoll |

---

*Erstellt durch Audit-Agent 1C, 2026-07-10. Datenbasis: vollständige Datei-Inventare beider Bäume, php -l (PHP 8.4.22) über 104 Dateien, Pattern-Scans + manuelle Reviews der Auth-/SQL-/Template-Pfade. Keine Code-Änderungen vorgenommen.*

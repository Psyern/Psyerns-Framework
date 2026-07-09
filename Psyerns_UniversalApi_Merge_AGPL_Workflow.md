# UniversalApi in Psyerns_Framework mergen, modernisieren & auf AGPL relizenzieren (Multi-Agent Orchestration)

## Auftrag

Nimm die alte Open-Source-Software **DayZ-UniversalApi** (Ordner `DeamonForge/`, AGPL-3.0), bringe ihren Code **auf den neuesten Stand**, integriere sie in das bestehende **Psyerns_Framework**, benenne die eingebrachten Klassen auf den Prefix **`DME_Api_`** um, und **relizenziere das gesamte Psyerns_Framework von MIT auf AGPL-3.0**. Nutze parallele Sub-Agenten.

Das Ergebnis ist **ein** kombiniertes Projekt **Psyerns_Framework unter AGPL-3.0**.

---

## Rechtlicher Rahmen (verbindlich)

Diese Entscheidung wurde bewusst getroffen: Der volle AGPL-Merge ist gewollt.

1. **Relizenzierung MIT → AGPL-3.0 ist erlaubt und Absicht.** Psyerns_Framework ist Eigentum von **Psyern / Deadmans Echo** — der Copyright-Inhaber darf sein eigenes Werk neu lizenzieren. MIT ist permissiv und mit AGPL kompatibel; MIT-Code darf in ein AGPL-Werk eingebracht werden. Das kombinierte Werk wird AGPL-3.0.
   - **Fussnote:** Bereits veroeffentlichte MIT-Versionen bleiben rueckwirkend MIT (nicht widerrufbar). Nur ab dieser Aenderung gilt AGPL. Das ist normal und kein Problem.
2. **daemonforge-Attribution bleibt erhalten.** Copyright-Vermerke des Urhebers **daemonforge** bleiben in den eingebrachten Dateien. Wir **ergaenzen** unseren Vermerk, wir **ersetzen** ihn nie.
   > `Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)`
   > `Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo`
3. **Aenderungs-Vermerke (AGPL §5)** in jeder von daemonforge stammenden, modifizierten Datei („modified by Deadmans Echo, 2026").
4. **Quelloffenlegung (AGPL §13).** Weil Psyerns_Framework auf einem Server mit Spielern laeuft (Netzwerk-Nutzung), muss der **vollstaendige Quellcode oeffentlich verfuegbar** sein. Repo offen halten; README nennt das AGPL-Angebot.
5. **Abhaengigkeits-Kompatibilitaet.** ALLE gebuendelten Fremd-Lizenzen im Psyerns_Framework muessen AGPL-kompatibel sein. MIT/BSD/Apache-2.0/permissiv = ok. **Achtung GPL-Funde** (es gibt „License: GPL"-Stellen, vermutlich das WordPress-Plugin): GPLv3 bzw. „GPLv2 or later" = AGPL-kompatibel; **reines GPLv2-only = NICHT kompatibel** → im Audit klaeren.
6. **Attribution des Ergebnisses:** README/`NOTICE` stellt klar: „Enthaelt und baut auf DayZ-UniversalApi (daemonforge), AGPL-3.0. Gesamtes Werk unter AGPL-3.0."

---

## Parameter (festgelegt)

- **Projektname:** `Psyerns_Framework` (bleibt; wird jetzt AGPL).
- **Neuer Code-Prefix fuer eingebrachten UApi-Code:** **`DME_Api_`** (ersetzt `UApi`, ~1100 Vorkommen; `UniversalApi`, 59 Vorkommen). Der Sub-Namensraum `DME_Api_` ist verbindlich, um Kollisionen mit den `DME_`-Klassen von DME-WAR zu vermeiden.
- **Bestehender Framework-Prefix `PF_` bleibt** fuer die vorhandenen Psyerns-Klassen (zwei Prefixe im Projekt sind ok).
- **Basis-Zweig:** **`DayZ-UniveralApi-1.3.2`** (festgelegt); nicht mit `-stable` mischen.

> **`DME_`-Kollisionswarnung:** Der DME-WAR-Mod nutzt bereits `DME_`-Klassen (u. a. `DME_Logger`, `DME_ConfigManager`, `DME_FactionService`, `DME_NameTagsConfig`, `DME_BossTreeChest`, `DME_TagDef`, `DME_IconDef`, `DME_PvPPolicy`). EnScript-Klassennamen sind server-global: Laufen Psyerns_Framework und DME-WAR je auf demselben Server, kollidieren gleichnamige `DME_`-Klassen → Compile-Fehler. Die eingebrachten Klassen tragen daher durchgaengig den festgelegten Sub-Namensraum **`DME_Api_`** (nicht nur `DME_`). Phase 4 prueft das.

---

## Referenz-Wissensbasis (autoritativ, EnScript-Seite)

`C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main\`
- `Tips/`, `How-To/` (RPC, Profile-Settings, Validate-Config), `Frameworks/`, `DayZGame/DayZ-1.29.161219.md`.
EnScript-Hartregeln: kein Ternary/`delete`/`GetGame()`; `g_Game` mit Null-Check; `IsDedicatedServer()` statt IsServer/IsClient; `override`+`super` zuerst; RPC-IDs >= 10000; `ref` nur auf Member; Tabs; Funktionsaufrufe einzeilig.
Web-Seite: aktuelle Stable-Versionen (Node LTS, PHP 8.x).

---

## Quelle & Ziel

- **Quelle:** `DeamonForge/DayZ-UniveralApi-1.3.2/` — EnScript-Mod `_UniversalApi/scripts/` (`UApi*`, Token-Auth), Node-Webservice `DayZWebService/` (Discord-Bot, MongoDB, JWT, Currency, Toxicity, Translate, Server-Query), `DesktopManager/`, `Schemas/`, `uapi.gproj`. Veraltete Node-Deps: `express@4.18`, `discord.js@13.7` (→ v14), `mongodb@4.6` (→ v6), `jsonwebtoken@8.5`, `node-fetch@2.6` (→ natives fetch), `@tensorflow/*@3.11`, `greenlock-express@4`, `express-rate-limit@6`.
- **Ziel:** `Psyerns_Framework/Psyerns_Framework/` — EnScript `scripts/3_Game|4_World|5_Mission/` (`PF_*`-Idiom, `PF_WebClient` RestApi, `PF_Discord*`, `PF_AlertSystem`, `PF_CB_*`), Web `WP-Plugin_Psyerns-Leaderboard/` + `psyerns-mods/` (PHP), `MISC/`. Lizenz-Datei `Psyerns_Framework/LICENSE` (aktuell MIT → wird AGPL).

---

## PHASE 0 — Baseline, Relicense-Plan & Rename-Landkarte (1 Agent)

1. Basis-Zweig waehlen; Komponenten + Dateizahlen inventarisieren.
2. **Rename-Landkarte** `UApi`→`DME_...` / `UniversalApi`→ passender Name; alle Artefakte (`uapi.gproj`, PBO-Prefix, Config-Keys, Routen).
3. **Kollisions-Liste:** alle `DME_`-Klassennamen, die DME-WAR bereits nutzt, sammeln → Zielnamen der eingebrachten Klassen dagegen absichern (eindeutiger Sub-Namensraum).
4. **Relicense-Plan:** `Psyerns_Framework/LICENSE` (MIT) → AGPL-3.0; Ergaenzungs-Header + §5-Notiz-Vorlage; daemonforge-Copyright-Erhalt festhalten.
5. **Dep-Lizenz-Scan-Plan:** alle gebuendelten Fremd-Lizenzen (inkl. der „GPL"-Funde/WP-Plugin) auf AGPL-Kompatibilitaet pruefen.

**Output:** `Psyerns_Framework/_Merge/00_Baseline_Rename_Relicense.md`.
**HITL-Checkpoint 1:** Mensch bestaetigt Basis-Zweig, Namensschema (Kollisionsvermeidung), Relicense.

---

## PHASE 1 — Modernisierungs- & Kompatibilitaets-Audit (parallele Agenten, nur reporten)

- **1A — EnScript (UApi-Mod):** 1.29-Inkompatibilitaeten (Datei/Zeile), veraltete RestApi/JSON-APIs, was bei Integration ins `PF_`-Framework auf vorhandene Infrastruktur (z. B. `PF_WebClient`) gehoben werden kann statt doppelt.
- **1B — Node-Webservice:** Dep-Upgrades + Breaking-Changes (discord.js 13→14: Intents/Partials/ChannelType/PermissionsBitField/Events; mongodb 4→6; jwt; node-fetch→fetch). TensorFlow-Toxicity: behalten/aktualisieren oder optional/streichen — begruenden.
- **1C — PHP/WordPress & Desktop/EJS-UI:** Modernisierung + **Lizenz-Kompatibilitaet** der WP-Plugin-Lizenz (GPL-Version pruefen!) mit AGPL.
- **1D — Schemas/Config/Wire-Format:** was Interop-bewahrend bleibt (Endpunkt-Pfade, Auth-Token-Format, Feldnamen), was modernisiert wird.

**Output:** `_Merge/01_Audit_<Komponente>.md` je Agent.
**HITL-Checkpoint 2:** Falls ein Fremd-Bestandteil AGPL-inkompatibel ist (z. B. GPLv2-only) → Mensch entscheidet (ersetzen/entfernen), BEVOR gemergt wird.

---

## PHASE 2 — Modernisieren (parallele Agenten, aendern)

Verhalten/Contracts bewahren, nur Implementierung/Deps erneuern.
- **EnScript:** 1.29-konform; wo sinnvoll auf `PF_WebClient`/vorhandene Framework-Infrastruktur stuetzen statt eigener HTTP-Schicht.
- **Node:** Deps auf aktuelle Stable; Migrations-Umbauten (discord.js v14, mongodb v6, natives fetch, jwt); `package.json`/Lockfile + Node-LTS-`engines`.
- **PHP/WP/Desktop/UI:** aktuelle Standards.
- §5-Aenderungsnotizen in jede beruehrte daemonforge-Datei.

**Worktree-Isolation empfohlen.**
**Output:** modernisierter Stand + `_Merge/02_Modernization_Notes.md`.

---

## PHASE 3 — Integration, Rebrand (`DME_`) & Relicense (parallele Agenten)

1. **Integration:** eingebrachten Code an den passenden Ort im Psyerns_Framework-Baum (`scripts/3_Game|4_World|5_Mission/` fuer EnScript; Web-Teile passend). Ein einziges Init/Bootstrapping — kein zweites paralleles System. Config in die vorhandene `PsyernsFrameworkConfig`-Struktur einordnen (`PsyernsFrameworkConfig.example.json`).
2. **Rebrand:** `UApi`→`DME_...`/eindeutiger Sub-Namensraum konsistent (EnScript-Klassen, Node-Module, Config-Keys). **Interop-kritische** Endpunkt-Pfade/Wire-Feldnamen nur bewusst aendern. `uapi.gproj`/PBO-Prefix/Modname anpassen.
3. **Relicense:** `Psyerns_Framework/LICENSE` durch **AGPL-3.0** ersetzen; Ergaenzungs-Header (Regel 2) in allen eingebrachten/modernisierten Dateien; README/`NOTICE` um daemonforge-Attribution + AGPL-Hinweis + §13-Quelltextangebot ergaenzen.

**Output:** integriertes AGPL-Psyerns_Framework + `_Merge/03_Integration_Rebrand_Report.md`.

---

## PHASE 4 — Verifikation & AGPL-Compliance (adversariell, Gate)

1. **Lizenz:** `Psyerns_Framework/LICENSE` = AGPL-3.0. daemonforge-Copyright in Original-Dateien erhalten. §5-Notizen vorhanden. README nennt Herkunft + AGPL + §13. ✔
2. **Dep-Kompatibilitaet:** kein gebuendelter Fremd-Bestandteil AGPL-inkompatibel (GPL-Funde final geklaert). ✔
3. **Namens-Kollision:** grep — kein eingebrachter `DME_`-Klassenname kollidiert mit DME-WAR-`DME_`-Klassen (`DME_Logger`, `DME_ConfigManager`, `DME_FactionService`, `DME_NameTagsConfig`, `DME_BossTreeChest`, `DME_TagDef`, `DME_IconDef`, `DME_PvPPolicy` u. a.). ✔
4. **Rename-Vollstaendigkeit:** keine gebrochenen `UApi`-Referenzen; nur bewusst belassene Interop-Strings uebrig.
5. **EnScript-Trockenlauf:** Verbotsliste ueber geaenderte `.c`.
6. **Node/PHP-Sanity:** `npm install` loest auf, keine offenen High-Severity-Advisories (oder dokumentiert), Start-Skripte laufen an.

**Output:** `_Merge/04_Compliance_and_Verification.md`.
**HITL-Checkpoint 3:** Freigabe erst nach bestandenem Compliance-Check.

---

## Nach Abschluss aller Agenten (Orchestrator-Synthese)

1. Konsistenz Audit ↔ Modernisierung ↔ Integration/Rebrand abgleichen; Reste schliessen.
2. Bestaetigen: gesamtes Psyerns_Framework AGPL-3.0, Attribution vollstaendig, keine Namens-/Lizenz-Kollision.
3. Abschlussbericht: geaenderte/eingebrachte/umbenannte Dateien, Dep-Upgrades + Breaking-Change-Umbauten, entfernte Features (begruendet), Lizenz-Status je Bestandteil, Compliance-Ergebnis, empfohlene Tests (DayZ-Servertest + Webservice-Smoke-Test).
4. **KEIN** Deployment, **KEIN** Git-Push ohne Freigabe.

---

## Wichtige Hinweise (fuer alle Agenten)

- **Ganzes Psyerns_Framework wird AGPL-3.0.** LICENSE ersetzen; daemonforge-Copyright nie loeschen, eigenen Vermerk nur ergaenzen.
- **`DME_`-Namen eindeutig halten** (Kollision mit DME-WAR vermeiden — eigener Sub-Namensraum).
- **Contracts/Interop bewahren:** Endpunkt-Pfade, Wire-Feldnamen, Auth-Token-Format nur bewusst aendern.
- **Ein System, kein Parallel-Init:** eingebrachten Code in die vorhandene Framework-Struktur/Config einfuegen.
- **Fremd-Lizenzen pruefen:** alles muss AGPL-kompatibel sein (GPLv2-only ist es NICHT).
- **EnScript-Regeln + Wissensbasis** strikt.
- **Keine Rechtsberatung** — setzt die getroffene Lizenz-Entscheidung technisch um; bei kommerzieller Verwertung juristisch gegenlesen.

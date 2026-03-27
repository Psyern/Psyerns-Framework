# Prompt: Leaderboard Systeme zusammenführen — WAR/Hardline Daten in v2 Theme System

## Problem

Es laufen ZWEI Leaderboard-Render-Systeme parallel im WordPress Plugin:

**System A — v2 (aktiv, Theme-basiert):**
- JS: `public/js/psyern-leaderboard.js`
- PHP Template: `public/templates/leaderboard-base.php`
- CSS: `public/css/psyern-leaderboard.css` + `psyern-theme-*.css` (6 Themes)
- Hat: Kills, Deaths, K/D, Playtime, Avatare, Top 3
- **Fehlt:** war_faction, war_level, war_boss_kills, war_alignment, hardline_reputation, globalEastPoints/WestPoints

**System B — PF Shortcodes (Legacy):**
- JS: `public/assets/js/pf-templates.js`
- CSS: `public/assets/css/pf-common.css` + `pf-dark.css` + `pf-light.css`
- Hat: Alles inkl. WAR/Hardline Felder, Faction Badges, Faction Bar, Boss Kills, Reputation
- **Problem:** Nutzt nicht die 6 Themes, hat Layout-Probleme

## Ziel

Die WAR/Hardline Features aus System B in System A einbauen. System B bleibt als Fallback aber System A ist das primäre.

## Arbeitsverzeichnis
`C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\`

## Lies zuerst ALLE diese Dateien vollständig:
- `js/psyern-leaderboard.js` — v2 JS (MUSS ERWEITERT WERDEN)
- `templates/leaderboard-base.php` — v2 PHP Template (MUSS ERWEITERT WERDEN)
- `assets/js/pf-templates.js` — PF JS (REFERENZ für die neuen Features)
- `css/psyern-leaderboard.css` — Base CSS (evtl. neue Klassen nötig)
- `css/psyern-theme-neon.css` — Beispiel Theme (Farben für neue Elemente)

## WordPress API Response Format

Die API liefert bereits alle Daten. Beispiel pro Spieler:
```json
{
    "steam_id": "76561198000000010",
    "player_name": "WildCard",
    "pve_points": 68895,
    "pvp_points": 0,
    "kills": 4259,
    "deaths": 62,
    "ai_kills": 234,
    "pve_deaths": 45,
    "pvp_deaths": 0,
    "longest_shot": 312,
    "playtime": 0,
    "is_online": 0,
    "last_login": "2026-03-23 19:00:00",
    "category_kills": {"PlayersBased": 2, "ZombiesBased": 3456, "AIBased": 234, "AnimalsBased": 567},
    "category_deaths": {...},
    "category_longest_ranges": {...},
    "avatar_url": "https://avatars.steamstatic.com/...",
    "war_faction": "EAST",
    "war_alignment": 2800,
    "war_level": 2,
    "war_boss_kills": 1,
    "hardline_reputation": 0
}
```

Meta-Daten:
```json
{
    "generatedAt": "2026-03-26 01:37:18",
    "playerOnlineCounter": 0,
    "totalPlayers": 16,
    "globalEastPoints": 0,
    "globalWestPoints": 0,
    "players": [...]
}
```

## Was in psyern-leaderboard.js ergänzt werden muss

### 1. Faction Badge neben Spielernamen
- Zeige `war_faction` als farbiges Badge neben dem Namen
- EAST = rot, WEST = blau, NEUTRAL = grau, leer = kein Badge
- HTML: `<span class="psyern-lb__faction psyern-lb__faction--east">EAST</span>`

### 2. Boss Kills Spalte in der Tabelle
- Neue Spalte "Boss" in der Tabelle
- Zeigt `war_boss_kills` Wert
- Wenn 0: leer lassen
- CSS Klasse: `psyern-lb__boss-kills`

### 3. Reputation Spalte in der Tabelle
- Neue Spalte "Rep" in der Tabelle
- Zeigt `hardline_reputation` Wert
- Wenn 0: leer lassen
- CSS Klasse: `psyern-lb__reputation`

### 4. Faction Bar im Header
- Visueller Balken der EAST vs WEST Punkte zeigt
- Nutze `globalEastPoints` und `globalWestPoints` aus den Meta-Daten
- EAST rot links, WEST blau rechts, proportional
- Nur anzeigen wenn beide > 0
- CSS Klassen: `psyern-lb__faction-bar`, `psyern-lb__faction-bar-track`, `psyern-lb__faction-bar-east`, `psyern-lb__faction-bar-west`

### 5. Top 3 Boss Kills
- Shortcode `[pf_top3_bosskills]` existiert bereits in der Settings-Seite
- Muss in `class-pf-shortcodes.php` registriert werden
- Sortierung: nach `war_boss_kills` DESC
- Nutze den gleichen Top 3 Card-Style wie deadliest/monthly

### 6. Player Card erweitern
- Faction Badge
- War Level
- Boss Kills
- Hardline Reputation

## Was in leaderboard-base.php ergänzt werden muss

- Neue `<th>` Spalten: Boss, Rep
- Neue `<td>` Zellen mit den Werten
- Faction Badge neben dem Namen

## Was in psyern-leaderboard.css ergänzt werden muss

- `.psyern-lb__faction` — Badge-Basis (kleine Pill, inline)
- `.psyern-lb__faction--east` — Rot (#e74c3c)
- `.psyern-lb__faction--west` — Blau (#3498db)
- `.psyern-lb__faction--neutral` — Grau (#95a5a6)
- `.psyern-lb__faction-bar` — Container für den Balken
- `.psyern-lb__faction-bar-track` — Hintergrund
- `.psyern-lb__faction-bar-east` — Roter Anteil
- `.psyern-lb__faction-bar-west` — Blauer Anteil
- `.psyern-lb__boss-kills` — Boss Kill Zelle (KEIN Emoji, nur Zahl)
- `.psyern-lb__reputation` — Rep Zelle (KEIN Emoji, nur Zahl)

Diese Klassen existieren teilweise schon — prüfe was da ist und ergänze was fehlt.

## Was in den 6 Theme-CSS Dateien ergänzt werden muss

Jedes Theme braucht Farben für:
- Faction Badge (EAST/WEST/NEUTRAL)
- Faction Bar
- Boss Kills und Reputation Spalte

Themes: military, ash, ops, outbreak, cyberpunk, inferno, stalker

## WICHTIG: Emojis entfernen

In `psyern-leaderboard.css` existieren aktuell CSS Regeln die Emojis per `::before` einfügen:
```css
.psyern-lb__boss-kills::before { content: '\2620 '; }
.psyern-lb__reputation::before { content: '\2605 '; }
```
Diese MÜSSEN ENTFERNT werden. Boss und Rep Spalten zeigen NUR die Zahl, kein Emoji/Icon.

Prüfe auch `pf-templates.js` ob dort Emojis für Boss/Rep Spalten gerendert werden — falls ja, auch entfernen.

## Regeln

- **Enforce Script Pitfalls** gelten nicht hier — das ist PHP/JS/CSS
- **Minimale Änderungen** am bestehenden v2 Code
- **Keine neuen Dateien** — nur bestehende erweitern
- **Backward-kompatibel** — wenn war_faction leer/undefined ist, nichts anzeigen
- **Alle 6 Themes** müssen die neuen Elemente korrekt darstellen
- **pf-templates.js als Referenz** nutzen — dort sind die Render-Funktionen für Faction Badge, Faction Bar etc. bereits implementiert. Kopiere die Logik in das v2 System.

## Agent-Orchestrierung (3 Agents, sequentiell)

### Agent 1: Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Lies `psyern-leaderboard.js` und `leaderboard-base.php` vollständig. Dokumentiere:
1. Wie wird die Tabelle gerendert? (JS dynamisch oder PHP server-side?)
2. Wo werden die Spalten definiert?
3. Wo werden die Top 3 Cards gerendert?
4. Wo werden die Player Cards gerendert?
5. Welche CSS-Klassen werden bereits für die neuen Features genutzt?
6. Wie greift das JS auf die API Daten zu?

### Agent 2: Implementation
**Typ:** Schreibend
**Aufgabe:** Implementiere alle 6 Features in das v2 System.
**Dateien zu ändern:**
- `js/psyern-leaderboard.js` — Faction Badge, Boss Kills, Reputation, Faction Bar
- `templates/leaderboard-base.php` — Neue Spalten (falls PHP-gerendert)
- `css/psyern-leaderboard.css` — Neue CSS-Klassen
- `css/psyern-theme-military.css` — Theme-spezifische Farben
- `css/psyern-theme-ash.css` — Theme-spezifische Farben
- `css/psyern-theme-ops.css` — Theme-spezifische Farben
- `css/psyern-theme-outbreak.css` — Theme-spezifische Farben
- `css/psyern-theme-cyberpunk.css` — Theme-spezifische Farben
- `css/psyern-theme-inferno.css` — Theme-spezifische Farben
- `css/psyern-theme-stalker.css` — Theme-spezifische Farben
- `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\class-pf-shortcodes.php` — `[pf_top3_bosskills]` Shortcode registrieren

### Agent 3: Verifizierung
**Typ:** Explore (read-only)
**Aufgabe:** Prüfe alle geänderten Dateien auf:
- JS Syntax korrekt
- CSS Selektoren matchen HTML-Klassen
- Alle 7 Themes haben die neuen Farben (military, ash, ops, outbreak, cyberpunk, inferno, stalker)
- KEINE Emojis (☠ ★) in Boss/Rep Spalten — weder per CSS ::before noch per JS
- Backward-kompatibel (leere/undefined Werte zeigen nichts an)
- Shortcode registriert und funktionsfähig

## Bonus: Inferno Theme Upgrade

Das Inferno Theme (`psyern-theme-inferno.css`) ist zu langsam und hat nicht genug Feuer-Effekte. Verbessere es:

### Performance
- Die bestehenden CSS-Animationen (`psyern-inferno-haze`, `psyern-inferno-blaze`, `psyern-inferno-ember-badge`, `psyern-inferno-flameline`, `psyern-inferno-nameshift`) nutzen teure Properties wie `filter`, `box-shadow`, `background-size`. Optimiere:
  - Nutze `will-change: transform, opacity` auf animierten Elementen
  - Ersetze `filter: drop-shadow` durch `text-shadow` wo möglich
  - Nutze `transform: translateZ(0)` für GPU-Beschleunigung

### Mehr Feuer-Effekte
- **Rising Ember Particles:** Erstelle ein leichtgewichtiges CSS-only Partikel-System mit `@keyframes` + `::before`/`::after` Pseudo-Elementen. 5-8 kleine orangene/gelbe Punkte die von unten nach oben schweben und verblassen. Kein JS Canvas nötig — pure CSS.
- **Flicker-Effekt auf dem Titel:** Der Titel soll unregelmäßig flackern wie eine Flamme. Nutze eine `@keyframes` Animation mit zufälligen Opacity-Werten (0.85, 1.0, 0.9, 1.0, 0.8, 1.0) über 2-3 Sekunden.
- **Heat Haze Distortion auf den Top 3 Cards:** Subtiler `skewX` oder `scaleY` Wobble auf den Cards, als würde Hitze die Luft verzerren. Sehr dezent (±0.3deg über 4 Sekunden).
- **Glowing Ember Border auf der Tabelle:** Die Tabelle bekommt einen subtilen pulsierenden Glow am unteren Rand, als würde darunter Glut glimmen.
- **Smoke Vignette:** Der bestehende `::after` Gradient (Zeile 17-26) soll dezent animiert werden — leichtes Pulsieren der Opacity.

### Wichtig
- Alle Effekte müssen per `prefers-reduced-motion: reduce` abschaltbar sein
- Keine JS-Abhängigkeit — alles pure CSS
- Die Seite darf nicht spürbar langsamer werden als andere Themes

## Ausführungsreihenfolge

```
Agent 1 (Analyse) → Agent 2 (Implementation) → Agent 3 (Verifizierung)
```

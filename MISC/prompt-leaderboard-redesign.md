# 🎮 Claude Prompt — Deadmans-Echo Leaderboard Theme Redesign

---

## VERWENDUNG
Diesen Prompt direkt als **erste Nachricht** in einem neuen Claude-Chat senden.
Dann im zweiten Schritt das gewünschte Theme angeben (siehe unten).

---

## SYSTEM PROMPT

```
Du bist ein spezialisierter Frontend-Developer für das DayZ-Community-Projekt Deadmans-Echo.
Du erhältst Aufgaben zur CSS/JS-Überarbeitung des Leaderboards auf https://deadmans-echo.de/leaderboard/

TECHNISCHER KONTEXT:
- WordPress-basierte Seite
- Leaderboard wird via PHP-Shortcode gerendert
- CSS-Themes als separate Dateien: public/css/psyern-theme-[name].css
- PHP-Templates: public/templates/leaderboard-[name].php
- Kein Framework — reines HTML/CSS/JS, kein React/Vue
- Bestehende Klassen-Struktur beibehalten: .leaderboard-container, .leaderboard-table, th, td, tr
- Neue Effekte als eigene Klassen ergänzen, niemals bestehende Selektoren überschreiben

DESIGN-BIBLIOTHEK (liegt als crt-military-terminal-styles.md vor):
Alle CSS Custom Properties, Keyframe-Animationen, Partikel-Systeme und Klassen
aus dieser Datei sind als Referenz zu verwenden. Nicht neu erfinden — direkt einsetzen.

QUALITÄTSANSPRUCH:
- Production-ready, kein Prototype-Look
- Jede Animation muss einen purpose haben (kein Effekt um des Effekts willen)
- Performance: will-change, pointer-events: none und position: fixed für Overlays
- Alle Overlays bekommen z-index-Hierarchie: Content > UI > Partikel > Hintergrund
- Google Fonts via <link> einbinden, kein @import in CSS

AUSGABE-FORMAT:
Liefere immer:
1. Die vollständige CSS-Datei (kommentiert, mit Section-Headings)
2. Einen JS-Block für Partikel/Canvas-Effekte (falls vorhanden) als separates <script>
3. Einen kurzen Einbindungs-Guide (welche Klassen wo ans HTML)
4. Eine Klassen-Übersicht als kompakte Tabelle
```

---

## THEME-PROMPTS — EINER DAVON NACH DEM SYSTEM PROMPT SENDEN

---

### 🔥 THEME 1: INFERNO

```
Überarbeite das Leaderboard-Theme zu: INFERNO — Fire & Flames

AESTHETIC: Blazing hellfire. Scorched earth. Ein Terminal das brennt.

FARBPALETTE (aus Design-Bibliothek):
--fire-core: #fff0a0  (heißester Kern)
--fire-yellow: #ffcc00
--fire-orange: #ff6600
--fire-red: #cc1100
--fire-ember: #ff3300
--inferno-bg: #0a0200
--inferno-bg-dark: #050000

PFLICHT-EFFEKTE (alle aus der Design-Bibliothek übernehmen):
1. Rahmen-Glow: .inferno-container mit @keyframes infernoPulse (3-Phasen: dim → normal → aufflackern)
2. Brennende Ränder: .burning-border via ::before/::after — oben/unten gegenläufig animiert
3. Überschriften: .inferno-title mit @keyframes flameTitle (4-Phasen weiß-gelb–orange–rot)
4. Ember Partikel: createEmberSystem() — 55 Partikel, zufällige Größe/Drift/Farbe/Dauer
5. Lava Ground: .lava-ground — Gegenlicht von unten + .scorched-overlay (3 Radial-Gradients)
6. Vignette: .inferno-vignette-flicker — verkohlte Ränder, atmend flackernd
7. Tabelle: table.inferno-table — Top 1 weiß-gelb (Kernhitze), Top 2 orange, Top 3 rot

FONTS:
<link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@400;700&family=Share+Tech+Mono&family=Orbitron:wght@400;700&display=swap" rel="stylesheet">
- Headings: Cinzel 700
- Daten/Tabelle: Share Tech Mono
- Labels: Orbitron 400

EXTRA-WUNSCH:
- Der Hover-Effekt auf Tabellenzeilen soll sich anfühlen wie eine Glut die kurz aufleuchtet
- Top-1-Zeile bekommt einen leichten permanenten Flacker via animation
- Section-Trennlinien als gradient von orange nach transparent (nicht als border)
- Ausgabe als: public/css/psyern-theme-inferno.css + inline <script> Block
```

---

### ☢️ THEME 2: STALKER

```
Überarbeite das Leaderboard-Theme zu: STALKER — Radioactive Zone
S.T.A.L.K.E.R. inspired. Die Zone lebt. Alles strahlt.

FARBPALETTE (aus Design-Bibliothek):
--rad-yellow: #ffe600
--rad-orange: #ff7b00
--rad-orange-dim: #b85500
--rad-green: #7fff00  (Giftgrün-Akzent)
--rad-bg: #0a0800
--rad-bg-dark: #050400

PFLICHT-EFFEKTE (alle aus der Design-Bibliothek übernehmen):
1. Rahmen-Glow: .stalker-container mit @keyframes radPulse (3-Phasen, unregelmäßig)
2. Radioaktive Symbole: createBiohazardBg() mit ☢ — 6 Symbole, unterschiedliche Rotation/Richtung
3. Überschriften: .stalker-title mit @keyframes titleGlimmer (Atem-Effekt gelb→weiß-gelb)
4. Subheadings: @keyframes subtitlePulse auf .stalker-subtitle
5. Chromatic Aberration: .chroma-glitch mit @keyframes chromaGlitch (gelegentlicher RGB-Split)
6. Film Grain: createGrainCanvas() — dynamisches Canvas-Grain alle 80ms
7. Geiger Counter: .geiger-display mit geigerEffect(el) — unregelmäßiger visueller Tick
8. Vignette: .rad-vignette-ground — warmes Orange an den Rändern

FONTS:
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=VT323&family=Orbitron:wght@400;700&display=swap" rel="stylesheet">
- Hauptheadings: VT323 (Pixel-Terminal-Charakter)
- Daten/Tabelle: Share Tech Mono
- Zone-Labels: Orbitron

EXTRA-WUNSCH:
- Kleine Geiger-Counter-Anzeige oben rechts im Container: zeigt "RAD: [Wert]μSv/h"
  Der Wert soll per JS zufällig zwischen 140 und 890 fluktuieren (Geiger-Rhythmus)
- Jede Tabellenzeile bekommt beim Hover einen kurzen chromaGlitch-Flash
- Die ☢-Symbole im Hintergrund: untere Hälfte des Screens etwas dichter (Zone-Atmosphäre am Boden)
- Ausgabe als: public/css/psyern-theme-stalker.css + inline <script> Block
```

---

### 🖥️ THEME 3: OPS — CRT MILITARY TERMINAL

```
Überarbeite das Leaderboard-Theme zu: OPS — CRT Military Terminal
Classified. Phosphor green. Alles ist ein Befehl.

FARBPALETTE (aus Design-Bibliothek):
--crt-green: #00ff41
--crt-green-dim: #00b32c
--crt-green-glow: #00ff4188
--crt-amber: #ffb000  (für Warnungen/Highlights)
--crt-bg: #000a00
--crt-bg-dark: #000500

PFLICHT-EFFEKTE (alle aus der Design-Bibliothek übernehmen):
1. Rahmen-Glow: .terminal-container mit @keyframes borderPulse
2. Scanlines: ::before Methode A (statisches Raster) + .crt-scanline-move (scrollende Linie)
3. Flicker: body::after @keyframes crtFlicker + .screen-flicker @keyframes hardFlicker
4. Vignette: .crt-vignette-strong
5. Bildschirmwölbung: .crt-screen mit perspective(1000px) rotateX(0.3deg)
6. Matrix-Regen: Canvas-Script — Katakana + Sonderzeichen, opacity: 0.12
7. Text-Glow: .glow-text auf allen Datenwerten
8. Tabelle: table.terminal-table — Top 3 Gold/Silber/Bronze, Hover leuchtet auf

FONTS:
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=VT323&display=swap" rel="stylesheet">
- Alles: Share Tech Mono (Konsistenz ist Militär-Prinzip)
- Nur Haupttitel optional: VT323

EXTRA-WUNSCH:
- Jeder Spielername bekommt ein vorangestelltes "> " (Cursor-Style)
- Status-Zeile unter dem Leaderboard: "// LAST UPDATED: [Zeit] — SIGNAL: STRONG — OPERATIVES: [Anzahl]"
  Diese Zeile aktualisiert sich jede Sekunde per JS (Uhrzeit live)
- Rang-Nummern als militärische Bezeichnungen: #1 = "ALPHA-1", #2 = "ALPHA-2" etc.
- Der Container-Titel soll als Tipp-Animation erscheinen beim Laden (typewriter effect, 60ms pro Zeichen)
- Ausgabe als: public/css/psyern-theme-ops.css + inline <script> Block
```

---

## PODIUM-LAYOUT — GILT FÜR ALLE DREI THEMES

```
AUFGABE:
Die bestehenden Top-3-Karten werden von untereinander auf nebeneinander umgestellt.
Layout: 2. Platz links · 1. Platz Mitte · 3. Platz rechts

AVATAR-GRÖSSEN (nicht verhandelbar):
  Platz 1 — 120px  (dominant, größter Avatar)
  Platz 2 —  90px  (mittelgroß, links)
  Platz 3 —  70px  (kleinster, rechts)

HTML-STRUKTUR (DOM-Reihenfolge immer: 2 → 1 → 3):
<div class="podium-wrapper">
  <div class="podium-card podium-silver" data-rank="2"> ... </div>
  <div class="podium-card podium-gold"   data-rank="1"> ... </div>
  <div class="podium-card podium-bronze" data-rank="3"> ... </div>
</div>

CSS-KERN:
  display: flex + align-items: flex-end → Podium-Treppchen-Effekt
  Platz 1 bekommt .podium-crown (👑) mit Float-Animation
  Platz 1 Avatar bekommt pulsierenden Glow in Theme-Farbe
  Podium-Stufen (.podium-stand--1/2/3): Höhen 60px / 40px / 24px

THEME-FARBEN für Avatar-Ringe und Stufen:
  Inferno: Gold-Kern weiß-gelb #fff0a0, Stufen in Glutrot-Verlauf
  Stalker: Gold-Kern #ffe600, Stufen in Toxisch-Orange-Verlauf
  Ops:     Gold-Kern #00ff41, Stufen in Phosphorgrün-Verlauf

RESPONSIVE:
  768px: Avatar-Größen reduzieren (90 / 70 / 55px)
  480px: Stats ausblenden, nur Avatar + Name, Stufen flacher

Avatar-Fallback immer einbauen:
  onerror="this.src='/assets/default-avatar.png'"
```

---

## ALLGEMEINE HINWEISE FÜR ALLE THEMES

```
IMMER beachten:
- z-index Hierarchie strikt einhalten:
  9999+ = UI/Scanlines/Flicker
  9990–9998 = Partikel-Overlays
  9980–9989 = Hintergrund-Effekte (Vignette, Grain)
  1–10 = Hintergrund-Symbole (rotierende Icons)
  -1 = Canvas-Hintergrund (Matrix, etc.)

- Alle fixed-position Overlays: pointer-events: none

- Performance-Pflicht:
  will-change: transform, opacity  → auf alle animierten Partikel
  transform: translateZ(0)         → auf Canvas-Elemente
  backface-visibility: hidden      → auf Flip-Animationen

- Partikel-Systeme immer mit Cleanup:
  setTimeout(() => { el.remove(); spawnNew(); }, duration * 1000)

- Kein !important verwenden

- CSS-Datei-Struktur (Kommentar-Sections):
  /* === 1. CUSTOM PROPERTIES === */
  /* === 2. BASE & BODY === */
  /* === 3. CONTAINER & BORDER === */
  /* === 4. OVERLAYS & EFFECTS === */
  /* === 5. TYPOGRAPHY & HEADINGS === */
  /* === 6. TABLE === */
  /* === 7. ANIMATIONS (Keyframes) === */
  /* === 8. PARTICLES (JS-Klassen) === */
```

---

*// DEADMANS-ECHO — DESIGN SYSTEM v2.0 //*
*// THEMES: INFERNO · STALKER · OPS //*

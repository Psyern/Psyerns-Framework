# Prompt: CSS/Layout Fixes v2 — Psyerns Framework WordPress Plugin

## Arbeitsverzeichnis
`C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\`

## Lies zuerst ALLE diese Dateien vollständig:
- `css/psyern-leaderboard.css` — Base Styles
- `css/psyern-theme-neon.css` — Neon Theme
- `css/psyern-theme-echo.css` — Echo Theme
- `css/psyern-theme-military.css` — Military Theme (funktioniert als Referenz)
- `assets/css/pf-common.css` — PF Legacy Base
- `assets/css/pf-dark.css` — PF Legacy Dark
- `assets/js/pf-templates.js` — PF JS Renderer (rendert Leaderboard, Top 3, Player Card, Status)
- `class-pf-shortcodes.php` — Shortcode PHP (setzt Container-Klassen)
- `js/psyern-leaderboard.js` — v2 JS Renderer
- `templates/leaderboard-base.php` — v2 PHP Template

---

## Analyse der 3 Probleme

### Problem 1: Neon/Echo Theme zeigt keine Farben

**Ursache:** Es laufen ZWEI Render-Systeme parallel:

**System A (v2):** `psyern-leaderboard.js` + `leaderboard-base.php`
- Container: `<div class="psyern-lb psyern-lb--neon">`
- CSS-Klassen: `.psyern-lb__title`, `.psyern-lb__table`, `.psyern-lb__top3-card`
- Theme-CSS: `psyern-theme-neon.css` mit Selektoren `.psyern-lb--neon .psyern-lb__title`

**System B (PF Shortcodes):** `pf-templates.js` + `class-pf-shortcodes.php`
- Container: `<div id="pf-lb-123" class="psyern-lb psyern-lb--neon">`
- Aber `pf-templates.js` rendert innerHTML mit Klassen wie `psyern-lb__title`, `psyern-lb__table` etc.
- PROBLEM: `pf-common.css` Zeile 1 hat einen Global Reset: `*, *::before, *::after { margin: 0; padding: 0; }` der alles überschreibt
- PROBLEM: `pf-dark.css` setzt `.pf-dark` Styles die mit dem Neon-Theme kollidieren könnten

**Fix:**
1. In `pf-common.css`: Den Global Reset `*` auf `.pf-container *` scopen statt auf `*`
2. In `pf-dark.css`: Die Klasse `.pf-dark` wird nicht mehr gesetzt (wurde bereits entfernt), aber die CSS-Datei wird noch geladen. Entweder entfernen oder sicherstellen dass sie keine Styles auf `psyern-lb` Elemente anwendet.
3. Prüfe ob der `pf-templates.js` Renderer auch die Theme-Klasse richtig durchreicht

### Problem 2: Leaderboard rechts abgeschnitten / kein Scroll auf Desktop

**Ursache:** Mehrere Faktoren:
1. WordPress-Theme setzt `overflow: hidden` auf einem Parent-Element (z.B. `.entry-content`, `.post-content`, `.elementor-widget-container`)
2. Der `table-wrap` Div existiert im PF-JS-gerenderten HTML (wurde hinzugefügt), ABER der Sticky-Column CSS kann mit dem Scroll kollidieren
3. `pf-common.css` hat `.pf-table-wrapper { overflow-x: auto }` aber das ist ein anderer Wrapper als `.psyern-lb__table-wrap`

**Fix:**
1. In `psyern-leaderboard.css` `.psyern-lb__table-wrap`: Stelle sicher `overflow-x: auto !important` und `display: block !important`
2. Füge hinzu: `.psyern-lb { overflow: visible !important; }` damit der Parent den Scroll nicht blockt
3. Die Sticky Columns (Rang, Avatar, Name) brauchen korrekte `left` Werte und `background: var(--psyern-bg)` damit sie beim Scrollen sichtbar bleiben
4. Füge einen CSS-Override hinzu der gängige WordPress-Theme-Wrapper überschreibt:
```css
.entry-content .psyern-lb__table-wrap,
.post-content .psyern-lb__table-wrap,
.elementor-widget-container .psyern-lb__table-wrap,
.wp-block-shortcode .psyern-lb__table-wrap {
    overflow-x: auto !important;
    overflow-y: visible !important;
}
```

### Problem 3: Top 3 Cards vertikal statt Grid / Avatare verschoben

**Ursache:** `pf-templates.js` → `renderTop3Cards()` rendert die Cards in der Reihenfolge [1, 0, 2] (Platz 2 zuerst, dann 1, dann 3) und nutzt die Klasse `.psyern-lb__top3` für das Grid. ABER:
1. Der Grid-CSS hat `grid-template-columns: 1fr 1.15fr 1fr` nur in `psyern-leaderboard.css` — prüfe ob die Regel aktiv ist
2. `pf-common.css` Global Reset könnte das Grid brechen
3. Die Avatare haben keine feste Größe im PF-Renderer — `renderTop3Cards` setzt `<img class="psyern-lb__top3-avatar">` aber die CSS-Regel `.psyern-lb__top3-avatar` hat `width: 56px; height: 56px` — prüfe ob das WordPress-Theme `img` Styles überschreibt (z.B. `max-width: 100%`)

**Fix:**
1. Stelle sicher dass `.psyern-lb__top3` Grid nicht von einem Parent oder Reset gebrochen wird
2. Avatar-Bilder: Setze explizit `width`, `height`, `object-fit: cover`, `display: block` mit `!important` falls nötig
3. Prüfe ob das v2 System (PHP Template) und PF System (JS) verschiedene Top-3 HTML-Strukturen erzeugen — vereinheitlichen

---

## Agent-Orchestrierung (3 Agents, sequentiell)

### Agent 1: Deep Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Lies ALLE CSS + JS + PHP Dateien und erstelle eine exakte Diagnose.

Für jedes Problem dokumentiere:
- Welche CSS-Datei verursacht den Konflikt (mit Zeilennummer)
- Welcher CSS-Selektor überschreibt was (Specificity-Vergleich)
- Welches HTML wird tatsächlich gerendert (aus JS oder PHP)
- Welche Container-Klassen sind auf dem Element

Prüfe auch:
- Gibt es doppelte Render-Aufrufe? (Rendert sowohl `psyern-leaderboard.js` ALS AUCH `pf-templates.js` in den gleichen Container?)
- Wie wird entschieden welches System rendert?
- Werden beide CSS-Systeme (`pf-*` und `psyern-lb*`) gleichzeitig geladen?

### Agent 2: Fix
**Typ:** Schreibend
**Aufgabe:** Fixe alle 3 Probleme basierend auf Agent 1's Analyse.

**Prioritäten:**
1. `pf-common.css` Global Reset entschärfen (auf `.pf-container` scopen oder entfernen)
2. Sicherstellen dass `.psyern-lb--neon` Theme-Styles greifen (Specificity erhöhen wenn nötig)
3. `.psyern-lb__table-wrap` Scroll auf Desktop erzwingen (WordPress-Theme-Overrides breaken)
4. Top 3 Grid fixen (3 Spalten nebeneinander, Platz 1 mitte + größer)
5. Avatar-Bilder korrekt positionieren (feste Größe, rund, zentriert)
6. Sticky Columns korrekt (Rang + Avatar + Name fixiert links beim Scrollen)

**Regeln:**
- Minimale Änderungen
- Nutze `!important` nur wo WordPress-Themes überschrieben werden müssen
- Teste mental ob die Änderung alle 6 Themes betrifft oder nur einzelne
- Keine neuen Dateien

### Agent 3: Verifizierung
**Typ:** Explore (read-only)
**Aufgabe:** Prüfe alle von Agent 2 geänderten Dateien.

Checkliste:
- [ ] Global Reset in `pf-common.css` ist entschärft
- [ ] `.psyern-lb--neon .psyern-lb__title` zeigt gelben Glow
- [ ] `.psyern-lb--neon .psyern-lb__rank` zeigt gelbe Farbe
- [ ] `.psyern-lb--neon .psyern-lb__table th` zeigt gelbe Header
- [ ] `.psyern-lb--echo` Theme-Farben greifen
- [ ] Top 3 Grid: 3 Spalten auf Desktop, 1 Spalte auf Mobile
- [ ] Avatare: 56px, rund, zentriert in Card
- [ ] Tabelle: Horizontal scrollbar auf Desktop mit sichtbarem Scrollbar
- [ ] Sticky Columns: Rang + Avatar + Name bleiben links fixiert
- [ ] Shadow-Gradient am rechten Rand der sticky Spalte
- [ ] Kein CSS-Konflikt zwischen `pf-*` und `psyern-lb*` Klassen
- [ ] Alle 6 Themes getestet (mental walkthrough der Selektoren)

---

## Ausführungsreihenfolge

```
Agent 1 (Deep Analyse) → Agent 2 (Fix) → Agent 3 (Verifizierung)
```

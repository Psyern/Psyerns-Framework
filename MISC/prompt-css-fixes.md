# Prompt: CSS/Layout Fixes für Psyerns Framework WordPress Plugin

## Kontext

Das WordPress Plugin zeigt ein Leaderboard mit 6 Design-Themes. Es gibt 3 Probleme die gefixt werden müssen.

**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\`

**Lies zuerst alle relevanten Dateien:**
- `css/psyern-leaderboard.css` — Basis-Styles (Tabelle, Top 3, Sticky Columns)
- `css/psyern-theme-neon.css` — Neon Theme (Cyberpunk Noir)
- `css/psyern-theme-echo.css` — Echo Theme
- `css/psyern-theme-military.css` — Military Theme (Referenz, funktioniert)
- `css/psyern-theme-ash.css` — Ash Theme
- `css/psyern-theme-ops.css` — Ops Theme
- `css/psyern-theme-outbreak.css` — Outbreak Theme
- `js/psyern-leaderboard.js` — v2 Leaderboard JS (rendert Top 3 + Tabelle)
- `templates/leaderboard-base.php` — v2 PHP Template
- `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\assets\js\pf-templates.js` — PF Shortcode JS (rendert Top 3 + Tabelle)
- `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\assets\css\pf-common.css` — PF Legacy Styles

---

## Problem 1: Top 3 Cards Layout kaputt

**Symptom:** Die Top 3 Cards werden vertikal gestapelt statt nebeneinander (3er-Grid). Platz 1 sollte in der Mitte und größer sein. Siehe: Aktuell zeigt es Card 1 oben, Card 2 darunter, Card 3 ganz unten.

**Erwartetes Layout:**
```
    [#2 Silber]   [#1 Gold (größer)]   [#3 Bronze]
```

**Fix-Ansatz:**
- Prüfe ob `.psyern-lb__top3` ein `display: grid; grid-template-columns: 1fr 1.15fr 1fr;` hat
- Prüfe ob das JS die Cards in der richtigen Reihenfolge rendert (Order: 2, 1, 3 — Platz 2 links, Platz 1 mitte, Platz 3 rechts)
- Prüfe ob ein Theme oder ein Parent-Container das Grid überschreibt
- Prüfe ob `pf-templates.js` und `psyern-leaderboard.js` beide Top 3 rendern und sich gegenseitig stören
- Stelle sicher dass die Grid-Regel auf allen Viewports greift (nicht nur Mobile)

---

## Problem 2: Neon & Echo Themes haben keine Farben

**Symptom:** Neon-Theme zeigt keinen Neon-Glow, kein Gelb, keine Animationen. Echo-Theme zeigt keine Theme-spezifischen Farben. Beide sehen aus wie das Basis-Dark-Theme.

**Diagnose-Schritte:**
1. Prüfe ob die Theme-CSS-Dateien korrekt geladen werden — schaue in `class-pf-shortcodes.php` wie Themes enqueued werden
2. Prüfe ob die Theme-Klasse (`.psyern-lb--neon`, `.psyern-lb--echo`) auf dem Container gesetzt wird
3. Prüfe ob die CSS-Selektoren in den Theme-Dateien mit den tatsächlichen HTML-Klassen matchen
4. Prüfe ob ein anderes CSS die Theme-Farben überschreibt (Specificity-Problem)
5. Prüfe ob `pf-templates.js` den Container mit `pf-dark`/`pf-light` Klasse erstellt statt `psyern-lb--neon`

**Wahrscheinliche Ursache:** Die PF-Shortcodes (`pf-templates.js`) nutzen `pf-dark`/`pf-light` Klassen, aber die Theme-CSS-Dateien targeten `.psyern-lb--neon`. Die Klassen matchen nicht.

**Fix-Ansatz:**
- In `pf-templates.js` muss der Container die richtige Theme-Klasse bekommen (z.B. `psyern-lb psyern-lb--neon`)
- ODER die Theme-CSS-Regeln müssen auch auf `.pf-dark`/`.pf-light` Container wirken
- Am besten: `pf-templates.js` ändert die Container-Klasse auf das aktive Theme aus `psyernConfig.theme` oder `pf_config.theme`

---

## Problem 3: Horizontales Scrollen funktioniert nicht am Desktop

**Symptom:** Die Tabelle wird rechts abgeschnitten. Am Handy (Touch-Scroll) funktioniert es, am Desktop nicht. Kein sichtbarer Scrollbar.

**Diagnose-Schritte:**
1. Prüfe ob `.psyern-lb__table-wrap` auf dem tatsächlichen HTML-Element liegt (evtl. fehlt der Wrapper)
2. Prüfe ob ein Parent-Element `overflow: hidden` setzt und den Scroll blockt
3. Prüfe ob die Tabelle tatsächlich breiter ist als der Container (`width: max-content`)
4. Prüfe ob Sticky Columns korrekt funktionieren — `position: sticky` braucht einen scrollbaren Parent
5. Prüfe ob das WordPress Theme (Elementor, Divi, etc.) einen `overflow: hidden` auf einem Wrapper setzt

**Fix-Ansatz:**
- Füge `!important` zu `overflow-x: auto` auf `.psyern-lb__table-wrap` hinzu um Theme-Overrides zu brechen
- Füge `overflow: visible !important` auf `.psyern-lb` hinzu damit der Parent nicht clippt
- Teste ob das PF-Shortcode-System (`pf-templates.js`) den `table-wrap` Div auch generiert
- Stelle sicher dass `border-collapse: separate` gesetzt ist (nötig für sticky)

---

## Agent-Orchestrierung (3 Agents, sequentiell)

### Agent 1: Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Lies ALLE relevanten CSS, JS, PHP Dateien und identifiziere die exakten Ursachen der 3 Probleme.

**Prüfe:**
1. Welche HTML-Klassen werden tatsächlich gerendert? (Lese JS + PHP Templates)
2. Welche CSS-Selektoren sind aktiv? (Vergleiche Klassen in HTML vs CSS)
3. Gibt es Konflikte zwischen dem v2 System (`psyern-lb`) und dem PF System (`pf-container`)?
4. Welcher Code rendert die Top 3? (JS oder PHP?)
5. Welcher Code rendert die Tabelle? (JS oder PHP?)
6. Wird der `table-wrap` Div tatsächlich generiert?
7. Werden die Theme-CSS-Dateien geladen?

**Output:** Für jedes der 3 Probleme: Exakte Ursache mit Dateiname + Zeilennummer + was falsch ist.

### Agent 2: Fix
**Typ:** Schreibend
**Aufgabe:** Fixe alle 3 Probleme basierend auf Agent 1's Analyse.

**Regeln:**
- Minimale Änderungen — nur fixen was kaputt ist
- Keine neuen Dateien erstellen
- Beide Render-Systeme (v2 PHP + PF JS) müssen funktionieren
- Alle 6 Themes müssen korrekt dargestellt werden
- Sticky Columns: Rang, Avatar, Name fixiert — Rest scrollt
- Top 3: 3er-Grid, Platz 1 mitte+größer, Platz 2 links, Platz 3 rechts
- Scrollbar sichtbar auf Desktop

### Agent 3: Verifizierung
**Typ:** Explore (read-only)
**Aufgabe:** Prüfe alle von Agent 2 geänderten Dateien auf:

1. **CSS Syntax** — Keine fehlenden Klammern, Semikolons
2. **Selector-Matching** — Alle CSS-Selektoren matchen tatsächliche HTML-Klassen
3. **Theme-Konsistenz** — Alle 6 Themes haben die gleichen Basis-Klassen
4. **Sticky Columns** — `position: sticky` + `left` Werte + `z-index` korrekt
5. **Grid Layout** — Top 3 Grid funktioniert auf Desktop (3 Spalten) und Mobile (1 Spalte)
6. **Overflow** — `overflow-x: auto` nicht durch Parent blockiert
7. **Kein Konflikt** — PF-System und v2-System stören sich nicht gegenseitig

**Output:** Liste aller gefundenen Probleme. Falls alles OK: "Verified — no issues found."

---

## Ausführungsreihenfolge

```
Agent 1 (Analyse) → Agent 2 (Fix) → Agent 3 (Verifizierung)
```

Sequentiell — jeder Agent wartet auf den vorherigen.

# Prompt: Inferno Theme — Hover Fix + Partikel sichtbar machen

## Arbeitsverzeichnis
`C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\public\`

## Lies zuerst diese Dateien vollständig:
- `css/psyern-theme-inferno.css` — Das Inferno Theme (komplett lesen)
- `css/psyern-leaderboard.css` — Base Styles (Sticky Columns, Table, z-index Werte)
- `js/psyern-leaderboard.js` — v2 JS Renderer (wie HTML gerendert wird)

---

## Problem 1: Zeilen verschieben sich beim Hover

**Symptom:** Wenn man über eine Tabellenzeile hovert, springt/verschiebt sich die Zeile.

**Wahrscheinliche Ursache:** Der Hover-Effekt in Inferno CSS fügt ein `::before` Pseudo-Element auf die `<tr>` ein (feurige linke Kante). Dieses Element verändert den Layout-Flow — entweder durch:
- `position: absolute` fehlt oder wirkt nicht korrekt
- Die `<tr>` hat kein `position: relative`
- Das `::before` beeinflusst die Zeilenhöhe
- `border`, `padding`, oder `margin` ändert sich beim Hover
- `transform` auf der Zeile verschiebt sie

**Fix-Ansatz:**
- Stelle sicher dass `<tr>` KEIN `transform` beim Hover bekommt
- Das `::before` muss `position: absolute` haben und die Zeile muss `position: relative` sein
- KEIN `border` oder `padding` Änderung beim Hover — nur `background` und `box-shadow`
- Teste: Entferne temporär alle Hover-Effekte auf `tbody tr:hover` und füge sie einzeln wieder hinzu um den Verursacher zu finden

---

## Problem 2: Partikel-Effekte verschwinden wenn Tabelle geladen wird

**Symptom:** Partikel (Ember Glow, Smoke Vignette) sind sichtbar während "Loading..." angezeigt wird. Sobald die Tabelle gerendert wird, verschwinden sie.

**Ursache:** Die Partikel sind `::before` und `::after` Pseudo-Elemente auf `.psyern-lb--inferno` mit `z-index: 10`. ABER:
1. Die Tabelle und ihre Inhalte haben eigene `background` Farben die die Partikel verdecken
2. Die Sticky-Column Cells haben `background: var(--psyern-bg)` (solid, nicht transparent) — das überdeckt die Partikel komplett
3. Die `::before`/`::after` Pseudo-Elemente liegen per `position: absolute` über dem Content, aber der Content ist opak

**Das Kernproblem:** CSS Pseudo-Elemente können nicht "durch" opake Hintergründe scheinen. Egal welcher z-index — wenn die Tabellenzellen einen soliden Hintergrund haben, verdecken sie die Partikel darunter.

**Lösung:** Die Partikel-Effekte ÜBER dem Content rendern (hoher z-index + pointer-events: none) UND die Effekte müssen selbst leuchtend/glowing sein statt subtile Gradienten. Außerdem:

### A) Partikel ÜBER dem Content (Overlay-Ansatz)
- `z-index: 100` statt 10 auf den Pseudo-Elementen
- `pointer-events: none` ist schon gesetzt
- Die Effekte müssen visuell stark genug sein um über dem dunklen Content sichtbar zu sein
- Nutze `mix-blend-mode: screen` oder `mix-blend-mode: lighten` — das blendet die hellen Partikel über den dunklen Content ohne ihn zu verdecken

### B) Rising Ember Particles (CSS-only, über dem Content)
Erstelle 6-8 kleine Partikel die von unten nach oben schweben. Nutze `box-shadow` auf einem `::before` Element mit multiplen Schatten für mehrere Partikel gleichzeitig:

```css
.psyern-lb--inferno .psyern-lb__table-wrap::before {
    content: '';
    position: absolute;
    bottom: 0;
    left: 0;
    right: 0;
    height: 100%;
    z-index: 100;
    pointer-events: none;
    /* Multiple ember particles as box-shadows */
    box-shadow:
        20px -20px 0 0 rgba(255, 69, 0, 0),
        50% -10% 0 0 rgba(255, 140, 0, 0),
        80% -5% 0 0 rgba(255, 200, 0, 0);
    animation: psyern-rising-embers 4s ease-in infinite;
}
```

Oder besser: Nutze `background: radial-gradient()` mit animierter `background-position` für aufsteigende Punkte.

Oder am besten: Mehrere `<span>` Elemente die per CSS animiert werden — aber das braucht JS. Da wir kein neues JS wollen, nutze den Multi-Box-Shadow Trick auf einem Pseudo-Element.

### C) Rahmen-Flackern
- Der äußere Rahmen des Containers soll unregelmäßig flackern
- Nutze `border-color` Animation oder `box-shadow` Animation
- Wechsle zwischen verschiedenen Intensitäten von Orange/Rot
- Unregelmäßiges Timing (nicht linear) — nutze `cubic-bezier` oder `steps()`

```css
@keyframes psyern-inferno-border-flicker {
    0%, 18%, 22%, 25%, 53%, 57%, 100% {
        border-color: #3d1800;
        box-shadow: 0 0 5px rgba(255, 69, 0, 0.1);
    }
    20%, 24%, 55% {
        border-color: #ff4500;
        box-shadow: 0 0 20px rgba(255, 69, 0, 0.4), 0 0 40px rgba(255, 140, 0, 0.2);
    }
}
```

### D) Partikel auch AUSSERHALB des Containers
- Nutze `overflow: visible` auf `.psyern-lb--inferno` (statt hidden)
- Die Pseudo-Elemente können dann über die Container-Grenzen hinaus gehen
- Ember-Partikel die über den oberen Rand hinaus schweben
- ACHTUNG: Das darf das WordPress-Theme-Layout nicht brechen — nutze `clip-path` oder begrenze auf max 30px Überhang

---

## Zusammenfassung der zu ändernden Dateien

1. **`css/psyern-theme-inferno.css`** — Hauptarbeit:
   - Hover-Fix auf Tabellenzeilen
   - Partikel z-index + mix-blend-mode
   - Rising ember particles
   - Border-Flacker-Animation
   - Overflow für externe Partikel
   - `prefers-reduced-motion` für alle neuen Animationen

2. **`css/psyern-leaderboard.css`** — Eventuell:
   - Prüfe ob base-styles die Inferno-Effekte blockieren (overflow, z-index)

---

## Agent-Orchestrierung (2 Agents, sequentiell)

### Agent 1: Analyse + Fix
**Typ:** Schreibend
**Aufgabe:**
1. Lies alle 3 Dateien
2. Identifiziere den exakten Hover-Shift Verursacher (welche CSS-Regel auf welcher Zeile)
3. Identifiziere warum Partikel bei gefüllter Tabelle nicht sichtbar sind
4. Fixe beide Probleme
5. Füge Rising Embers, Border-Flicker, und externe Partikel hinzu
6. Alle neuen Animationen mit `prefers-reduced-motion: reduce` abschaltbar

### Agent 2: Verifizierung
**Typ:** Explore (read-only)
**Aufgabe:** Prüfe die geänderte `psyern-theme-inferno.css`:
- [ ] Kein `transform` auf `tbody tr:hover` (verursacht Shift)
- [ ] Pseudo-Elemente haben `pointer-events: none`
- [ ] Partikel haben `z-index: 100` oder höher
- [ ] `mix-blend-mode: screen` oder `lighten` auf Overlay-Effekten
- [ ] Keine Layout-Shifts bei Hover (kein border/padding/margin Änderung)
- [ ] `prefers-reduced-motion` deaktiviert alle Animationen
- [ ] Overflow ist so gesetzt dass externe Partikel sichtbar sind aber Layout nicht bricht
- [ ] Kein Konflikt mit Sticky Columns (z-index Hierarchie: Partikel > Sticky > Content)

## Ausführungsreihenfolge
```
Agent 1 (Fix) → Agent 2 (Verifizierung)
```

# Multi-Agent Orchestration Prompt — Theme Color Audit & Fix
## Psyerns Framework Leaderboard Plugin
### Datum: 28.03.2026 | Repo: Psyern/Ninjin_Leaderboard

---

## KONTEXT & PROBLEM

Das WordPress-Plugin `psyerns-framework` hat 9 Leaderboard-Themes:
`military`, `ash`, `ops`, `outbreak`, `cyberpunk`, `stalker`, `inferno`, `frostbite`, `bubblegum`

**Symptom:** Kills, Deaths, Faction-Badges, KD-Ratio und andere Textelemente zeigen in fast ALLEN Themes **pinke Farben** (`#ff69b4`, `#ffb6da`, `#d090b0`) — obwohl diese nur im `bubblegum`-Theme korrekt sind.

**Plugin-Pfad (lokal):**
```
C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\
```

**Bereits analysiert & bekannte Fakten:**
1. `psyern-theme-bubblegum.css` — vollständig scoped, alle Regeln unter `.psyern-lb--bubblegum` ✅
2. `psyern-theme-cyberpunk.css` — vollständig scoped, `#ff00ff` nur unter `.psyern-lb--cyberpunk` ✅
3. `psyern-leaderboard.css` (Base) — pinke Farben NUR in `.psyern-lb--bubblegum`-scoped Regeln ✅
4. `psyern-theme-effects.js` — `createBubbleSystem()` nur aufgerufen wenn `cl.contains('psyern-lb--bubblegum')` ✅
5. `psyern-leaderboard.js` — keine hardcodierten Farben ✅
6. **`class-pf-shortcodes.php`** — `frostbite` und `bubblegum` fehlten in `$all_themes` → BEREITS GEFIXT ✅
7. **`psyern-leaderboard.css`** — `color: inherit` zu `th`/`td` hinzugefügt → BEREITS GEFIXT ✅

**Aber:** Das Problem besteht weiterhin in ALLEN Themes. Die pinke Farbe erscheint bei Kills, Deaths, Faction-Text.

---

## ARCHITEKTUR-ÜBERSICHT

```
public/
  css/
    psyern-leaderboard.css          ← Base-CSS, immer geladen
    psyern-theme-military.css       ← .psyern-lb--military scoped
    psyern-theme-ash.css
    psyern-theme-ops.css
    psyern-theme-outbreak.css
    psyern-theme-cyberpunk.css
    psyern-theme-stalker.css
    psyern-theme-inferno.css
    psyern-theme-frostbite.css
    psyern-theme-bubblegum.css
  js/
    psyern-leaderboard.js           ← Render-JS, kein Farb-Code
    psyern-theme-effects.js         ← Canvas-Effekte, theme-gated
  assets/js/
    pf-templates.js                 ← renderTop3, renderServerStatus etc.
  templates/
    leaderboard-base.php            ← HTML-Template
public/class-pf-shortcodes.php     ← Shortcode + Asset-Enqueue
```

**Theme-Klasse wird gesetzt als:**
```html
<div class="psyern-lb psyern-lb--military">
```

**CSS Kaskade:**
1. `psyern-leaderboard.css` → Base-Styles (color: inherit auf td/th)
2. `psyern-theme-military.css` → `.psyern-lb--military { color: #4ade80; }` (grün)
3. WordPress-Site-Theme CSS → **HIER KANN ÜBERSCHREIBUNG PASSIEREN**

---

## BEKANNTE FARBPALETTEN PRO THEME

| Theme      | Accent-Farbe  | Text-Farbe   | Kills/Deaths erbt |
|------------|---------------|--------------|-------------------|
| military   | `#4ade80`     | `#4ade80`    | Grün              |
| ash        | `#c8392b`     | `#c8b89a`    | Creme/Beige       |
| ops        | `#39ff14`     | `#39ff14`    | Neon-Grün         |
| outbreak   | `#f59e0b`     | `#fde68a`    | Amber             |
| cyberpunk  | `#00ff88`     | `#e0e0e0`    | Silber/Grün       |
| stalker    | `#ff8c00`     | `#d4c8a0`    | Orange/Sand       |
| inferno    | `#ff4500`     | `#f0d8b0`    | Orange/Rot        |
| frostbite  | `#5ba8e0`     | `#c8ddf0`    | Eisblau           |
| bubblegum  | `#ff69b4`     | `#d8ccd2`    | Pink ← KORREKT    |

---

## AGENT AUFGABEN

### AGENT 1 — CSS Scope Validator

**Aufgabe:** Verifiziere dass JEDE Farb-Deklaration in allen Theme-CSS-Dateien korrekt unter dem jeweiligen `.psyern-lb--{theme}`-Selektor scoped ist.

**Zu prüfen in jeder `psyern-theme-*.css`:**
```
1. Gibt es Selektoren die NICHT mit .psyern-lb--{theme} beginnen?
2. Gibt es @keyframes die bubblegum-Farben (#ff69b4, #ffb6da, #d090b0) enthalten?
3. Gibt es ::before / ::after Pseudo-Elemente ohne Theme-Scope?
4. Gibt es Selektoren wie .psyern-lb__name, .psyern-lb__rank (ohne --theme) mit Farbwerten?
```

**Prüf-Script:**
```powershell
$cssDir = "...\public\css"
foreach ($theme in @("military","ash","ops","outbreak","cyberpunk","stalker","inferno","frostbite")) {
    $file = "$cssDir\psyern-theme-$theme.css"
    $lines = Get-Content $file
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $l = $lines[$i].Trim()
        # Selektor der NICHT theme-scoped ist aber Farbe hat
        if ($l -match "^\.(psyern|pf)-" -and $l -notmatch "psyern-lb--$theme" -and $l -match "color|background") {
            Write-Host "[$theme] UNSCOPED L$($i+1): $l"
        }
    }
}
```

**Erwartetes Ergebnis:** Keine unscoped Regeln mit Farbwerten außer in `bubblegum`.

**Wenn Fehler gefunden:** Füge den fehlenden `.psyern-lb--{theme}`-Prefix hinzu.

---

### AGENT 2 — WordPress Theme Isolation

**Aufgabe:** Das WordPress-Site-Theme (nicht das Leaderboard-Theme) könnte CSS-Regeln haben die in das Leaderboard hineinbluten. Erstelle einen Isolation-Fix.

**Verdächtige WP-Theme-Regeln:**
```css
/* WP-Themes setzen oft sowas: */
table td { color: #xyz; }           /* überschreibt td color */
.entry-content * { color: #xyz; }   /* globaler override */
a { color: #ff69b4; }               /* Link-Farbe die vererbt wird */
p, span { color: #xyz; }            /* Text-Farbe */
```

**Fix in `psyern-leaderboard.css` — stärkere Isolation:**
```css
/* Am Anfang der Datei, nach .psyern-lb { ... } */

/* === WP-THEME ISOLATION === */
/* Verhindert dass das WordPress-Site-Theme Farben ins Leaderboard injiziert */
.psyern-lb,
.psyern-lb * {
    /* Kein color-Reset — Theme-CSS muss das setzen */
}

/* Tabellen-Zellen explizit vom WP-Theme isolieren */
.psyern-lb .psyern-lb__table,
.psyern-lb .psyern-lb__table thead,
.psyern-lb .psyern-lb__table tbody,
.psyern-lb .psyern-lb__table tfoot,
.psyern-lb .psyern-lb__table tr,
.psyern-lb .psyern-lb__table th,
.psyern-lb .psyern-lb__table td {
    color: inherit;
    font-family: inherit;
    font-size: inherit;
    line-height: inherit;
    border-color: inherit;
}

/* Links innerhalb des Leaderboards erben Theme-Farbe */
.psyern-lb a,
.psyern-lb a:link,
.psyern-lb a:visited,
.psyern-lb a:hover,
.psyern-lb a:active {
    color: inherit;
    text-decoration: none;
}

/* Spans/Paragraphen die WP-Themes gerne überschreiben */
.psyern-lb span,
.psyern-lb p,
.psyern-lb div {
    color: inherit;
}
```

**Wichtig:** Diese Regeln MÜSSEN vor den `.psyern-lb--{theme}`-Regeln stehen damit Theme-spezifische Overrides (höhere Spezifizität) trotzdem gewinnen.

---

### AGENT 3 — Theme-CSS Completeness Checker

**Aufgabe:** Jedes Theme-CSS muss für ALLE Leaderboard-Elemente eine explizite Farbdefinition haben. Prüfe ob Elemente fehlen.

**Pflicht-Elemente pro Theme (mit expliziter `color`-Deklaration):**
```
.psyern-lb--{theme}                        ← Haupt-Textfarbe (vererbt sich)
.psyern-lb--{theme} .psyern-lb__title      ← Titel
.psyern-lb--{theme} .psyern-lb__meta       ← Meta-Info
.psyern-lb--{theme} .psyern-lb__rank       ← Rang-Nummer
.psyern-lb--{theme} .psyern-lb__name       ← Spielername (kann erben)
.psyern-lb--{theme} .psyern-lb__kd         ← KD-Ratio
.psyern-lb--{theme} .psyern-lb__playtime   ← Spielzeit
.psyern-lb--{theme} .psyern-lb__boss-kills ← Boss-Kills
.psyern-lb--{theme} .psyern-lb__reputation ← Reputation
.psyern-lb--{theme} .psyern-lb__loading    ← Loading-Text
.psyern-lb--{theme} .psyern-lb__table th   ← Tabellen-Header
.psyern-lb--{theme} .psyern-lb__table td   ← Tabellen-Zellen
.psyern-lb--{theme} .psyern-lb__faction--east   ← Faction EAST
.psyern-lb--{theme} .psyern-lb__faction--west   ← Faction WEST
.psyern-lb--{theme} .psyern-lb__faction--neutral ← Faction NEUTRAL
```

**Prüf-Script:**
```powershell
$required = @(
    "psyern-lb__rank",
    "psyern-lb__kd",
    "psyern-lb__playtime",
    "psyern-lb__loading",
    "psyern-lb__table th",
    "psyern-lb__faction--east",
    "psyern-lb__faction--west"
)
foreach ($theme in @("military","ash","ops","outbreak","cyberpunk","stalker","inferno","frostbite","bubblegum")) {
    $file = Get-Content "...\psyern-theme-$theme.css" -Raw
    foreach ($el in $required) {
        if ($file -notmatch [regex]::Escape($el)) {
            Write-Host "[$theme] FEHLT: $el"
        }
    }
}
```

**Wenn Elemente fehlen:** Füge sie mit der Theme-Farbe hinzu. Beispiel für `military` (falls `table td` fehlt):
```css
.psyern-lb--military .psyern-lb__table td {
    color: #a0d080; /* military text dim */
}
```

---

### AGENT 4 — Spezifizitäts-Konflikt Analyzer

**Aufgabe:** CSS-Spezifizität-Konflikte finden. Die Base-CSS (`psyern-leaderboard.css`) und Theme-CSS werden alle geladen. Wenn Base-CSS höhere Spezifizität hat als Theme-CSS für dasselbe Element, gewinnt Base.

**Spezifizitäts-Tabelle:**
```
.psyern-lb__faction--east { color: red; }          = 0,1,0 (10)  ← BASE
.psyern-lb--military .psyern-lb__faction--east { } = 0,2,0 (20)  ← THEME gewinnt ✅
.psyern-lb .psyern-lb__table td { }               = 0,2,0 (20)  ← BASE
.psyern-lb--military .psyern-lb__table td { }     = 0,2,0 (20)  ← GLEICHSTAND → Reihenfolge!
```

**Problem:** Bei Gleichstand gewinnt die LETZTE Deklaration in der CSS-Ladereihenfolge.

**Ladereihenfolge in `class-pf-shortcodes.php`:**
```php
// 1. Base CSS
wp_enqueue_style('psyern-leaderboard', .../psyern-leaderboard.css);
// 2. Alle Themes (military zuerst, bubblegum zuletzt)
wp_enqueue_style('psyern-theme-military', ...);
// ...
wp_enqueue_style('psyern-theme-bubblegum', ...); ← LETZTE!
```

**KRITISCHER BUG:** `psyern-theme-bubblegum.css` wird ALS LETZTES geladen!

Wenn `psyern-theme-bubblegum.css` eine Regel hat mit **gleicher Spezifizität** wie `psyern-theme-military.css` für ein Element das BEIDE definieren, gewinnt bubblegum — und damit die pinke Farbe!

**Beispiel-Konflikt:**
```css
/* psyern-theme-military.css */
.psyern-lb--military .psyern-lb__table td { border-bottom: 1px solid rgba(42,74,42,0.3); }
/* Spez: 0,2,0 */

/* psyern-leaderboard.css Base + color: inherit */
.psyern-lb .psyern-lb__table td { color: inherit; }
/* Spez: 0,2,0 — GLEICHSTAND mit bubblegum unten! */

/* psyern-theme-bubblegum.css */
.psyern-lb--bubblegum .psyern-lb__faction--east { color: #ff69b4; }
/* Spez: 0,2,0 — aber NACH military geladen! */
```

**ABER:** Da der Selektor `.psyern-lb--bubblegum` heißt und military `.psyern-lb--military` setzt, sollten diese Regeln sich NICHT gegenseitig beeinflussen — außer...

**Der echte Fallback-Bug:** Wenn kein Theme-Selektor matcht (z.B. Theme-CSS nicht geladen oder falsche Klasse), greift der Base-Fallback. Die Base-CSS setzt:
```css
/* psyern-leaderboard.css Zeile 394 */
.psyern-lb__faction-label-east { color: #e74c3c; }  /* Rot — kein scope! */
.psyern-lb__faction-label-west { color: #3498db; }  /* Blau — kein scope! */
```

**Diese sind GLOBAL — kein `.psyern-lb--{theme}` Prefix!**

Prüfe ob es weitere globale (unscoped) Farbregeln in `psyern-leaderboard.css` gibt.

---

### AGENT 5 — PHP Template & JS Render Auditor

**Aufgabe:** Prüfe ob `pf-templates.js` oder `psyern-leaderboard.js` inline `style`-Attribute mit Farben setzen, die das Theme-CSS überschreiben.

**Bekannte inline-styles in `pf-templates.js` (bereits gefunden):**
```javascript
// Zeile 229: style="text-align:center;padding:20px 12px;..." — KEIN color ✅
// Zeile 289: style="width:10px;height:10px" — KEIN color ✅
// Zeile 301: style="font-size:1rem;font-weight:700" — KEIN color ✅
```

**Keine hardcodierten Farben in JS — sauber.** ✅

**Aber prüfe:** Hat `pf-templates.js` Render-Funktionen die CSS-Klassen FALSCH setzen?

```javascript
// Prüfe in pf-templates.js: Werden Klassen korrekt gesetzt?
// Beispiel-Bug: wenn psyern-lb__faction--neutral statt --east/--west gesetzt wird,
// bekommt EAST die neutrale Grau-Farbe statt Rot
```

**Script:**
```powershell
$tpl = Get-Content "...\pf-templates.js" -Raw
# Faction-Badge Funktion finden
$faction = [regex]::Match($tpl, "factionBadge[\s\S]{0,500}?return")
Write-Host $faction.Value
```

---

### AGENT 6 — Live-Site CSS-Override Detector

**Aufgabe:** Da das Plugin auf `deadmans-echo.de` läuft, muss das WordPress-Site-Theme auf CSS-Interferenz geprüft werden.

**Diagnose-Schritte (manuell im Browser):**
1. Öffne `https://deadmans-echo.de/leaderboard/` in Chrome/Firefox
2. Öffne DevTools → Inspector → Klicke auf ein "pinkes" Element (z.B. Kill-Zahl)
3. Im **Styles-Panel** siehst du die CSS-Kaskade von oben (spezifischste) nach unten (allgemeinste)
4. Suche nach Regeln die **NICHT** `psyern-` prefixed sind — diese kommen vom WP-Site-Theme

**Häufige WP-Theme-Interferenzen:**
```css
/* Divi / Elementor / Avada etc. setzen oft: */
.et_pb_text td, .et_pb_text th { color: inherit; }  /* OK */
body .entry-content a { color: #ff69b4; }            /* PINK LINK-FARBE! */
table { color: var(--accent-color); }                /* CSS-Variable vom WP-Theme */
```

**Fix wenn WP-Theme-Interferenz gefunden:** Füge in `psyern-leaderboard.css` einen spezifischeren Override ein:
```css
/* Nuclear option — nur wenn WP-Theme !important nutzt */
.psyern-lb--military,
.psyern-lb--military * {
    /* NICHT color hier setzen — Theme-CSS muss das tun */
}

/* Stattdessen: Höhere Spezifizität für alle Theme-Selektoren */
/* Präfix alle Theme-CSS Regeln mit body oder html: */
body .psyern-lb--military .psyern-lb__table td { color: #a0d080; }
```

---

### AGENT 7 — Comprehensive Fix Executor

**Aufgabe:** Implementiere alle gefundenen Fixes.

#### Fix 1: Base-CSS `psyern-leaderboard.css` — Alle unscoped Faction-Farben scopebounded machen

**Problem:** Diese Regeln in Base-CSS sind unscoped:
```css
/* Zeile ~394 */
.psyern-lb__faction-label-east { color: #e74c3c; }   /* ROT, GLOBAL */
.psyern-lb__faction-label-west { color: #3498db; }   /* BLAU, GLOBAL */
.psyern-lb__faction--east { color: #e74c3c; ... }    /* ROT, GLOBAL */
.psyern-lb__faction--west { color: #3498db; ... }    /* BLAU, GLOBAL */
.psyern-lb__faction--neutral { color: #95a5a6; ... } /* GRAU, GLOBAL */
```

Diese globalen Defaults sind zwar nicht pink — aber wenn ein WP-Theme-Plugin oder eine CSS-Variable sie überschreibt, gibt es kein spezifisches Fallback.

**Prüfe zuerst** ob diese Regeln tatsächlich das Problem verursachen indem du sie im Browser temporär deaktivierst.

#### Fix 2: Theme-CSS Spezifizitäts-Erhöhung

Falls WP-Theme-CSS mit `!important` oder body-scoped Regeln interferiert, erhöhe die Spezifizität aller Theme-Regeln:

**In JEDER `psyern-theme-{name}.css`** — ändere den Root-Selektor:
```css
/* VORHER: */
.psyern-lb--military { color: #4ade80; ... }

/* NACHHER (höhere Spezifizität): */
.psyern-lb.psyern-lb--military { color: #4ade80; ... }
/* Spez: 0,2,0 statt 0,1,0 */
```

**Script zum automatischen Refactoring:**
```powershell
foreach ($theme in @("military","ash","ops","outbreak","cyberpunk","stalker","inferno","frostbite","bubblegum")) {
    $file = "...\psyern-theme-$theme.css"
    $content = Get-Content $file -Raw
    # Ändere .psyern-lb--theme zu .psyern-lb.psyern-lb--theme
    $fixed = $content -replace "\.psyern-lb--$theme", ".psyern-lb.psyern-lb--$theme"
    Set-Content $file $fixed -NoNewline
    Write-Host "Fixed: psyern-theme-$theme.css"
}
```

⚠️ **Vorher Backup machen!**

#### Fix 3: `psyern-leaderboard.css` — table td mit höherer Spezifizität

```css
/* AKTUELL (Spez 0,2,0): */
.psyern-lb .psyern-lb__table td { color: inherit; }

/* BESSER (Spez 0,3,0 — gewinnt gegen WP-Theme body td-Regeln): */
.psyern-lb .psyern-lb__table tbody td,
.psyern-lb .psyern-lb__table thead th {
    color: inherit !important; /* nur wenn WP-Theme !important nutzt */
    font-family: inherit;
}
```

---

## AUSFÜHRUNGSREIHENFOLGE

```
Phase 1 (AGENT 1): CSS-Scope-Scan aller Theme-Dateien
Phase 2 (AGENT 3): Vollständigkeit prüfen — fehlen Elemente?
Phase 3 (AGENT 4): Spezifizitäts-Analyse — was gewinnt die Kaskade?
Phase 4 (AGENT 6): Browser-Diagnose live auf deadmans-echo.de
Phase 5 (AGENT 2): Base-CSS Isolation-Fix implementieren
Phase 6 (AGENT 7): Falls nötig — Spezifizitäts-Erhöhung in Theme-CSS
Phase 7 (AGENT 5): JS-Render-Verifikation
```

---

## SOFORT-TEST NACH FIX

Nach jedem Fix: Öffne das Leaderboard mit jedem Theme und prüfe:

```
✅ Military  → Grün (#4ade80) bei Kills, Deaths, KD
✅ Ash       → Creme/Beige (#c8b89a) bei Text
✅ Ops       → Neon-Grün (#39ff14) bei Text
✅ Outbreak  → Amber (#fde68a) bei Text
✅ Cyberpunk → Silber (#e0e0e0) + Cyan/Magenta-Akzente
✅ Stalker   → Orange/Sand (#d4c8a0) bei Text
✅ Inferno   → Orange/Rot (#f0d8b0) bei Text
✅ Frostbite → Eisblau (#c8ddf0) bei Text
✅ Bubblegum → Pink (#d8ccd2 base, #ff69b4 accent) ← Einziges mit Pink
```

---

## DATEIEN DIE BEARBEITET WERDEN DÜRFEN

```
public/css/psyern-leaderboard.css           ← Base-CSS
public/css/psyern-theme-military.css
public/css/psyern-theme-ash.css
public/css/psyern-theme-ops.css
public/css/psyern-theme-outbreak.css
public/css/psyern-theme-cyberpunk.css
public/css/psyern-theme-stalker.css
public/css/psyern-theme-inferno.css
public/css/psyern-theme-frostbite.css
public/css/psyern-theme-bubblegum.css
public/class-pf-shortcodes.php              ← Asset-Enqueue
```

## DATEIEN DIE NICHT VERÄNDERT WERDEN DÜRFEN

```
public/js/psyern-leaderboard.js             ← Render-Logic clean
public/js/psyern-theme-effects.js           ← Effects clean + gated
public/assets/js/pf-templates.js            ← Templates clean
public/templates/leaderboard-base.php       ← HTML korrekt
```

---

## GIT WORKFLOW NACH FIXES

```powershell
cd "C:\Users\Administrator\Desktop\Psyerns_Framework\MISC"
git add wordpress-plugin/psyerns-framework/public/css/
git add wordpress-plugin/psyerns-framework/public/class-pf-shortcodes.php
git commit -m "fix(themes): resolve pink color bleed — scope isolation + specificity fixes"
```

Dann Plugin-Dateien per FTP/SFTP auf den Server hochladen:
```
/wp-content/plugins/psyerns-framework/public/css/
/wp-content/plugins/psyerns-framework/public/class-pf-shortcodes.php
```

Danach: WordPress Admin → Einstellungen → Plugin-Cache leeren (falls LiteSpeed/W3TC/WP Rocket aktiv)

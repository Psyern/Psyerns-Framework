# Multi-Agent Orchestration Prompt — WordPress Plugin: "Psyerns Mods Showreel"

> **Ziel:** Entwickle ein vollständiges WordPress-Plugin, das ein Showreel für Steam-Workshop-Mods ("Psyerns Mods") darstellt und nahtlos zum Design der Webseite https://deadmans-echo.de/ passt.

---

## CONTEXT FÜR ALLE AGENTEN

```
Website:        https://deadmans-echo.de/
Design-Style:   Dark Military/Survival Gaming — dunkel, atmosphärisch, DayZ-Ästhetik
Farben:         Schwarz/Dunkelgrau als Basis, gedämpfte Akzente (Grün/Khaki, Orange-Rot für CTAs)
Typografie:     Bold, kapitalisierte Headlines; serifenlos, modern-militärisch
Atmosphäre:     Immersiv, post-apokalyptisch, Community-fokussiert
Plugin-Name:    Psyerns Mods Showreel
Shortcode:      [psyerns_mods]
Steam-Beispiel: https://steamcommunity.com/sharedfiles/filedetails/?id=3623510671
Discord-Server: https://discord.gg/CuAVVdTtvv
```

---

## AGENT 1 — ANALYZER

**Rolle:** Analysiere alle Anforderungen und erstelle eine vollständige Spezifikation.

```
Du bist Agent 1 (Analyzer). Analysiere die folgende Anforderung gründlich und erstelle eine strukturierte technische Spezifikation im JSON-Format.

### Anforderungen:

**Plugin-Funktion:**
- WordPress Admin-Panel zum Verwalten von Steam Workshop Items
- Für jedes Item können folgende Daten hinterlegt werden:
  1. Steam Workshop URL (z.B. https://steamcommunity.com/sharedfiles/filedetails/?id=3623510671)
  2. Automatische Darstellung des Workshop-Items (Titel, Vorschaubild, Beschreibung via Steam Web API oder oEmbed/OpenGraph)
  3. Discord-Link (optional pro Item oder global)
  4. Preis-Sektion:
     - Checkbox "Kostenlos" (wenn aktiv → zeigt "FREE" Badge, deaktiviert Preisfeld)
     - Preisfeld in EUR (wenn nicht kostenlos)
  5. Repack-Option:
     - Toggle Ja/Nein "Repack verfügbar"
     - Wenn Ja: Repack-Preis in EUR
  6. Source-File-Option:
     - Toggle Ja/Nein "Source File verfügbar"
     - Wenn Ja: Source-Preis in EUR
  7. Reihenfolge / Sortierung (Drag & Drop oder Nummerfeld)
  8. Sichtbarkeit: Veröffentlicht / Entwurf

**Frontend (via Shortcode [psyerns_mods]):**
- Grid/Card-Layout — responsive
- Jede Karte zeigt:
  - Workshop-Vorschaubild (automatisch abgerufen)
  - Mod-Name / Titel
  - Kurzbeschreibung (aus Steam oder manuell)
  - Preisinfo (FREE oder EUR-Betrag)
  - Repack-Badge + Preis (wenn aktiv)
  - Source-Badge + Preis (wenn aktiv)
  - CTA-Button → Link zum Steam Workshop Item
  - Discord-Button → Link zum Discord
- Dark-Theme passend zu deadmans-echo.de

**Technisch:**
- WordPress 6.x kompatibel
- Eigene Custom Post Type: psyern_mod
- Custom Meta Fields via WordPress Options API / postmeta
- Steam Web API Integration für Thumbnail + Titel (fallback: OpenGraph Scraping)
- Admin: Custom Post List mit Vorschauspalte
- Shortcode-Output gecacht (Transients API, 12h)
- Kein Page Builder erforderlich — reines PHP/CSS/JS Plugin

Erstelle daraus:
1. Plugin-Dateistruktur (vollständiges Verzeichnisbaum)
2. Custom Post Type Definition
3. Meta-Felder Schema (Name, Typ, Default, Validierung)
4. Steam API Integration Plan (Endpoint, Felder, Fallback)
5. Shortcode Output Schema
6. Admin UI Sektionen
7. Caching-Strategie

Gib die Ausgabe als strukturierten Plan zurück, den Agent 2 (Designer) und Agent 5 (Coder) direkt verwenden können.
```

---

## AGENT 2 — DESIGNER

**Rolle:** Definiere das komplette Design-System für Plugin-Frontend und Admin-UI.

```
Du bist Agent 2 (Designer). Basierend auf dem Output von Agent 1 und dem Design der Webseite https://deadmans-echo.de/ (Dark Military/Survival Gaming Ästhetik, DayZ-Community) erstelle ein vollständiges Design-System für das WordPress-Plugin "Psyerns Mods Showreel".

### Design-Referenz deadmans-echo.de:
- Hintergrund: Tief-schwarz (#0a0a0a) bis Dunkelgrau (#111111, #1a1a1a)
- Akzentfarbe: Militärisch-Grün/Khaki oder Bernstein/Orange-Rot für CTAs
- Typografie: Stark, kapitalisiert, serifenlose Bold-Fonts (z.B. Rajdhani, Bebas Neue, Oswald)
- Atmosphäre: Post-apokalyptisch, immersiv, rau — keine cleanen SaaS-Looks
- Karten: Dunkle Oberflächen mit subtilen Borders/Glows, keine weißen Hintergründe
- Buttons: Solid mit Hover-Glow oder Outline mit Fill-on-Hover

### Aufgaben:

1. **Farb-Tokens definieren:**
   ```
   --pf-bg-base:        #0a0a0a
   --pf-bg-card:        #111827   (Dark Blue-Gray)
   --pf-bg-card-hover:  #1f2937
   --pf-border:         #374151
   --pf-border-glow:    rgba(255,165,0,0.3)  (Orange-Glow für Hover)
   --pf-accent:         #f97316   (Orange — CTA Steam Button)
   --pf-accent-discord: #5865f2   (Discord Blau)
   --pf-free-badge:     #22c55e   (Grün)
   --pf-price-badge:    #eab308   (Gelb/Gold)
   --pf-repack-badge:   #3b82f6   (Blau)
   --pf-source-badge:   #8b5cf6   (Lila)
   --pf-text-primary:   #f9fafb
   --pf-text-secondary: #9ca3af
   --pf-text-muted:     #6b7280
   ```

2. **Karten-Design spezifizieren:**
   - Karte: `background: var(--pf-bg-card)` + `border: 1px solid var(--pf-border)`
   - Hover: Border-Glow (`box-shadow: 0 0 20px var(--pf-border-glow)`) + leichte Elevation
   - Bild-Ratio: **1:1 (quadratisch)** — `aspect-ratio: 1 / 1`, `object-fit: cover`, `width: 100%`
   - Content-Bereich: 16px Padding
   - Badge-Leiste unter dem Bild: Flex-Row mit Gap 8px

3. **Badge-Design:**
   - FREE: Grüner Hintergrund, weißer Text, Capslock, `font-size: 11px`, `border-radius: 4px`, `padding: 2px 8px`
   - Preisangabe: Gold/Gelb, gleiches System
   - Repack: Blau + "REPACK" Label + Preis
   - Source: Lila + "SOURCE" Label + Preis

4. **Button-Design:**
   - Steam-Button: Orange (#f97316), weiße Schrift, Hover → Aufhellen, Steam-Icon (SVG)
   - Discord-Button: Discord-Blau (#5865f2), weiße Schrift, Discord-Icon (SVG)
   - Beide: `border-radius: 6px`, `padding: 10px 20px`, `font-weight: 700`, `text-transform: uppercase`, `font-size: 13px`
   - Button-Gruppe: Flex-Row mit 8px Gap, full-width auf Mobile

5. **Grid-Layout:**
   - Desktop (≥1024px): 3 Spalten
   - Tablet (768–1023px): 2 Spalten
   - Mobile (<768px): 1 Spalte
   - Gap: 24px
   - Container: max-width 1200px, auto-margin, 24px Side-Padding

6. **Admin-UI Design:**
   - WordPress native Meta Boxes, aber styled mit Plugin-eigenen CSS-Klassen
   - Section-Header: Dunkelgrau mit Akzent-Borderline links
   - Toggle-Switches (CSS only, kein JS-Framework)
   - Preisfelder: Disabled (grayed out) wenn kostenlos oder Option deaktiviert
   - Preview-Thumbnail in der Admin-Liste: 80×45px

7. **Hover-States & Animationen:**
   - Kartenübergang: `transition: all 200ms ease-out`
   - Hover-Translate: `transform: translateY(-4px)`
   - Bild-Zoom: `transform: scale(1.05)` auf Hover (overflow hidden auf Wrapper)
   - Badge-Hover: kein State-Change (statisch)

8. **Typografie:**
   - Mod-Name: `font-size: 18px`, `font-weight: 700`, `text-transform: uppercase`, `letter-spacing: 0.05em`
   - Beschreibung: `font-size: 14px`, `line-height: 1.6`, `color: var(--pf-text-secondary)`
   - Preis: `font-size: 13px`, `font-weight: 600`
   - Google Font Empfehlung: "Rajdhani" (Headings) + "Inter" (Body) oder bestehende Theme-Fonts übernehmen

Gib das Design-System als kommentierte CSS-Variable-Liste + beschreibende Spezifikation zurück, die Agent 4 (CSS) direkt implementieren kann.
```

---

## AGENT 3 — REVIEWER

**Rolle:** Prüfe Spezifikation und Design auf Vollständigkeit, Fehler und WordPress-Standards.

```
Du bist Agent 3 (Reviewer). Prüfe den Output von Agent 1 (Analyzer) und Agent 2 (Designer) auf folgende Punkte und erstelle einen Bericht mit Korrekturen und Freigabe:

### Checkliste WordPress-Plugin-Standards:
- [ ] Plugin-Header vollständig (Plugin Name, Version, Author, Text Domain, License)
- [ ] Kein direkter Datei-Zugriff ohne `defined('ABSPATH')` Guard
- [ ] Alle User-Inputs werden sanitized (`sanitize_text_field`, `esc_url`, `absint`)
- [ ] Alle Outputs werden escaped (`esc_html`, `esc_url`, `esc_attr`)
- [ ] Nonces für alle Forms und AJAX-Requests
- [ ] Capabilities geprüft (`current_user_can('manage_options')`)
- [ ] Custom Post Type korrekt registriert (`register_post_type`)
- [ ] `register_activation_hook` / `register_deactivation_hook` vorhanden
- [ ] Assets nur auf notwendigen Seiten geladen (`wp_enqueue_scripts` mit Conditional)
- [ ] Transients-Cache korrekt geleert bei Post-Save (`save_post` hook)
- [ ] Textdomain für Übersetzungen geladen

### Checkliste Design & UX:
- [ ] Kontrast ≥4.5:1 für alle Text/Hintergrund-Paare (WCAG AA)
- [ ] Touch-Targets ≥44×44px für alle Buttons
- [ ] Hover-Zustände definiert
- [ ] Dark Mode konsistent (kein weißes Element ohne Absicht)
- [ ] Responsive Breakpoints vollständig (mobile / tablet / desktop)
- [ ] Plugin-CSS hat ausreichend Spezifität ohne `!important`-Overuse
- [ ] Steam-Bild Fallback definiert (wenn API nicht erreichbar)
- [ ] Leerer State definiert ("Noch keine Mods vorhanden")

### Checkliste Steam API:
- [ ] Steam Web API Endpoint korrekt: `https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/`
- [ ] Workshop-ID korrekt aus URL extrahiert (regex: `/id=(\d+)/`)
- [ ] API Key Handling (optional - funktioniert auch ohne für public items)
- [ ] Fallback auf OpenGraph/oEmbed wenn API nicht antwortet
- [ ] Rate Limiting bedacht (Caching als Schutz)
- [ ] Error Handling bei fehlgeschlagenen API-Calls

### Sicherheitscheck:
- [ ] SQL Injection: Kein direktes `$wpdb->query` mit unescapten Inputs
- [ ] XSS: Alle Outputs escaped
- [ ] CSRF: Nonces überall
- [ ] Remote Code Execution: Kein `eval()`, kein `system()`
- [ ] Steam URL Validierung (nur steam community URLs erlaubt)

Erstelle:
1. Liste aller gefundenen Probleme mit Schweregrad (KRITISCH / WARNUNG / INFO)
2. Korrekturvorschläge
3. Freigabe-Entscheidung (FREIGEGEBEN / ÜBERARBEITUNG ERFORDERLICH)
4. Finale Spezifikation mit integrierten Korrekturen für Agent 4 und Agent 5
```

---

## AGENT 4 — CSS AUTHOR

**Rolle:** Schreibe die vollständige CSS-Datei des Plugins.

```
Du bist Agent 4 (CSS Author). Basierend auf dem Design-System von Agent 2 und der finalen Freigabe von Agent 3, schreibe die vollständige CSS-Datei für das WordPress-Plugin "Psyerns Mods Showreel".

### Datei: `psyerns-mods/public/css/psyerns-mods-public.css`

### Anforderungen:

1. **CSS-Variablen am :root** — alle Design-Tokens aus Agent 2 definieren

2. **Container & Grid:**
```css
.psyerns-mods-grid { ... }       /* Outer wrapper */
.psyerns-mods-container { ... }  /* max-width, padding */
.psyerns-mods-item { ... }       /* Einzelne Karte */
```

3. **Karten-Komponenten:**
```css
.psm-card { ... }                /* Basis-Karte */
.psm-card:hover { ... }          /* Hover State */
.psm-card__image-wrap { ... }    /* Bild-Container: aspect-ratio: 1/1; overflow: hidden; width: 100% */
.psm-card__image { ... }         /* Das Bild: width:100%; height:100%; object-fit:cover; display:block */
.psm-card:hover .psm-card__image { transform: scale(1.05); } /* Zoom auf Hover */
.psm-card__body { ... }          /* Content Bereich */
.psm-card__title { ... }         /* Mod-Name */
.psm-card__description { ... }   /* Kurzbeschreibung — IMMER bereinigter Plaintext, kein BBCode */
.psm-card__badges { ... }        /* Badge-Zeile */
.psm-card__actions { ... }       /* Button-Gruppe */
```

4. **Badges:**
```css
.psm-badge { ... }               /* Basis-Badge */
.psm-badge--free { ... }         /* Kostenlos */
.psm-badge--price { ... }        /* Bezahlpflichtig */
.psm-badge--repack { ... }       /* Repack */
.psm-badge--source { ... }       /* Source File */
```

5. **Buttons:**
```css
.psm-btn { ... }                 /* Basis-Button */
.psm-btn--steam { ... }          /* Steam CTA */
.psm-btn--steam:hover { ... }
.psm-btn--discord { ... }        /* Discord */
.psm-btn--discord:hover { ... }
```

6. **Icons (Inline-SVG via CSS content oder data-URI):**
   - Steam Icon (SVG path) in .psm-btn--steam::before
   - Discord Icon (SVG path) in .psm-btn--discord::before

7. **Empty State:**
```css
.psm-empty-state { ... }
.psm-empty-state__icon { ... }
.psm-empty-state__text { ... }
```

8. **Admin CSS:** `psyerns-mods/admin/css/psyerns-mods-admin.css`
   - Meta Box Styling
   - Toggle Switch Styling
   - Disabled State für Preisfelder
   - Section Headers

9. **Responsive:**
```css
@media (max-width: 767px) { ... }
@media (min-width: 768px) and (max-width: 1023px) { ... }
@media (min-width: 1024px) { ... }
```

10. **Accessibility:**
    - `.psm-btn:focus-visible` mit sichtbarem Outline (2px solid, Akzentfarbe)
    - Kein `outline: none` ohne Ersatz
    - `prefers-reduced-motion` Media Query für Hover-Animationen

Schreibe vollständige, produktionsreife CSS — keine Platzhalter. Alle Selektoren müssen mit `.psyerns-mods` oder `.psm-` geprefixed sein, um Theme-Konflikte zu vermeiden.
```

---

## AGENT 5 — CODER

**Rolle:** Schreibe den vollständigen PHP/JS Code des WordPress-Plugins.

```
Du bist Agent 5 (Coder). Schreibe das vollständige WordPress-Plugin "Psyerns Mods Showreel" basierend auf der finalen Spezifikation von Agent 3. Verwende das CSS von Agent 4.

### Plugin-Struktur:
```
psyerns-mods/
├── psyerns-mods.php              ← Plugin-Hauptdatei (Header + Init)
├── uninstall.php                 ← Cleanup bei Deinstallation
├── admin/
│   ├── class-psm-admin.php       ← Admin-Klasse (CPT Registration, Meta Boxes)
│   ├── css/
│   │   └── psyerns-mods-admin.css
│   └── js/
│       └── psyerns-mods-admin.js ← Toggle-Logik für Admin-Fields
├── includes/
│   ├── class-psm-post-type.php   ← Custom Post Type
│   ├── class-psm-meta.php        ← Meta Fields Save/Get
│   ├── class-psm-steam-api.php   ← Steam API Integration
│   └── class-psm-cache.php       ← Transients Wrapper
└── public/
    ├── class-psm-shortcode.php   ← Shortcode [psyerns_mods]
    ├── css/
    │   └── psyerns-mods-public.css
    └── js/
        └── psyerns-mods-public.js
```

### Schreibe folgende Dateien vollständig aus:

---

#### 1. `psyerns-mods.php` — Plugin-Header & Bootstrap
```php
<?php
/**
 * Plugin Name: Psyerns Mods Showreel
 * Plugin URI:  https://deadmans-echo.de/
 * Description: Showreel für Steam Workshop Mods von Psyern. Verwalte Mods mit Preisen, Repack- und Source-Optionen.
 * Version:     1.0.0
 * Author:      Psyern
 * Author URI:  https://deadmans-echo.de/
 * Text Domain: psyerns-mods
 * License:     GPL-2.0+
 */
```
- `defined('ABSPATH') || exit;`
- Autoload aller Klassen
- `register_activation_hook` → Flush Rewrite Rules
- `register_deactivation_hook` → Flush Rewrite Rules
- Hook-Registration für Admin und Public

---

#### 2. `includes/class-psm-post-type.php` — Custom Post Type
```php
register_post_type('psyern_mod', [
    'labels'      => [...],  // Deutsch
    'public'      => true,
    'has_archive' => false,
    'menu_icon'   => 'dashicons-games',
    'supports'    => ['title', 'thumbnail'],
    'show_in_rest'=> false,
]);
```

---

#### 3. `includes/class-psm-meta.php` — Meta Felder
Meta-Felder (alle sanitized beim Speichern):
```
_psm_steam_url       string  Steam Workshop URL
_psm_discord_url     string  Discord Einladungslink
_psm_is_free         bool    Kostenlos-Checkbox
_psm_price           float   Preis in EUR
_psm_has_repack      bool    Repack verfügbar
_psm_repack_price    float   Repack-Preis
_psm_has_source      bool    Source File verfügbar
_psm_source_price    float   Source-Preis
_psm_steam_data      string  Gecachter Steam API Response (JSON)
_psm_order           int     Sortierreihenfolge
```
- `save_post` Hook mit Nonce-Check + Capability-Check
- `get_psm_meta($post_id, $key)` Hilfsfunktion

---

#### 4. `includes/class-psm-steam-api.php` — Steam API
```php
class PSM_Steam_API {
    const API_URL = 'https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/';
    
    public static function get_item_data($steam_url) {
        // 1. Workshop-ID extrahieren via Regex /id=(\d+)/
        // 2. POST-Request an Steam API
        // 3. Ergebnis: ['title', 'description', 'preview_url', 'workshop_id']
        // 4. Fallback: wp_remote_get($steam_url) + OpenGraph Parsing
        // 5. Return: Array mit Daten oder WP_Error
    }
    
    public static function extract_workshop_id($url) { ... }
    public static function fetch_from_api($workshop_id) { ... }
    public static function fetch_opengraph_fallback($url) { ... }
    
    /**
     * BBCode Bereinigung — PFLICHT
     * Steam-Beschreibungen enthalten BBCode-Tags die NICHT roh angezeigt werden dürfen.
     * Implementiere strip_bbcode() als statische Methode:
     *
     * Entferne folgende BBCode-Tags vollständig (inkl. Inhalt wo sinnvoll):
     *   [img]...[/img]        → komplett entfernen (Bild-URLs nicht anzeigen)
     *   [url=...]...[/url]    → nur Linktext behalten
     *   [h1]...[/h1]         → Inhalt behalten, Tags entfernen
     *   [h2]...[/h2]         → Inhalt behalten, Tags entfernen
     *   [h3]...[/h3]         → Inhalt behalten, Tags entfernen
     *   [b]...[/b]           → Inhalt behalten, Tags entfernen
     *   [i]...[/i]           → Inhalt behalten, Tags entfernen
     *   [u]...[/u]           → Inhalt behalten, Tags entfernen
     *   [strike]...[/strike] → Inhalt behalten, Tags entfernen
     *   [list]...[/list]     → Inhalt behalten, Tags entfernen
     *   [*]                  → mit "• " ersetzen
     *   [hr]                 → komplett entfernen
     *   [quote]...[/quote]   → Inhalt behalten, Tags entfernen
     *   [code]...[/code]     → Inhalt behalten, Tags entfernen
     *   [noparse]...[/noparse] → Inhalt behalten, Tags entfernen
     *   [table]...[/table]   → komplett entfernen
     *   [tr]...[/tr]         → komplett entfernen
     *   [td]...[/td]         → komplett entfernen
     *   [th]...[/th]         → komplett entfernen
     *   [previewyoutube=...][/previewyoutube] → komplett entfernen
     *
     * Implementierung via preg_replace():
     *   // Bilder komplett entfernen
     *   $text = preg_replace('/\[img\].*?\[\/img\]/si', '', $text);
     *   // URL-Tags: nur Linktext behalten
     *   $text = preg_replace('/\[url=[^\]]*\](.*?)\[\/url\]/si', '$1', $text);
     *   // Alle übrigen bekannten Tags: Tags entfernen, Inhalt behalten
     *   $text = preg_replace('/\[\/?(h[1-6]|b|i|u|strike|list|quote|code|noparse|hr)\]/si', '', $text);
     *   // [*] als Listenpunkt
     *   $text = preg_replace('/\[\*\]/', '• ', $text);
     *   // YouTube/Table Blöcke komplett entfernen
     *   $text = preg_replace('/\[previewyoutube[^\]]*\].*?\[\/previewyoutube\]/si', '', $text);
     *   $text = preg_replace('/\[table\].*?\[\/table\]/si', '', $text);
     *   // Alle verbliebenen unbekannten [tag] Reste entfernen
     *   $text = preg_replace('/\[[^\]]{1,30}\]/', '', $text);
     *   // Mehrfache Leerzeilen auf max. 2 reduzieren
     *   $text = preg_replace('/\n{3,}/', "\n\n", $text);
     *   return trim($text);
     *
     * Aufruf: Wende strip_bbcode() auf das 'description'-Feld aus der Steam API an
     * BEVOR es gespeichert oder im Shortcode ausgegeben wird.
     * Die gecachten Steam-Daten (_psm_steam_data) speichern bereits die BEREINIGTE Version.
     */
    public static function strip_bbcode($text) { ... }
}
```

---

#### 5. `includes/class-psm-cache.php` — Caching
```php
class PSM_Cache {
    const CACHE_PREFIX = 'psm_steam_';
    const CACHE_TTL    = 43200; // 12 Stunden
    
    public static function get($key) { ... }
    public static function set($key, $data) { ... }
    public static function delete($key) { ... }
    public static function flush_on_save($post_id) { ... } // save_post Hook
}
```

---

#### 6. `admin/class-psm-admin.php` — Admin UI
Admin Meta Box mit folgenden Feldern:

**Sektion: Steam Integration**
- Textfeld: Steam Workshop URL
- Button: "Daten laden" (AJAX → lädt Titel + Bild aus Steam)
- Preview: Vorschaubild + Titel (readonly)

**Sektion: Verfügbarkeit & Preise**
- Checkbox: ☑ Kostenlos
- Preisfeld: EUR ____ (disabled wenn Kostenlos aktiv)

**Sektion: Repack**
- Toggle: Repack verfügbar (Ja/Nein)
- Preisfeld: Repack-Preis EUR ____ (disabled wenn Toggle = Nein)

**Sektion: Source File**
- Toggle: Source File verfügbar (Ja/Nein)
- Preisfeld: Source-Preis EUR ____ (disabled wenn Toggle = Nein)

**Sektion: Links**
- Textfeld: Discord URL (Optional — überschreibt globale Einstellung)

**Sektion: Sortierung**
- Zahlenfeld: Reihenfolge (Standard: 0)

---

#### 7. `admin/js/psyerns-mods-admin.js`
```javascript
// Toggle-Logik: Preisfeld enable/disable
// Kostenlos-Checkbox → Preisfeld togglen
// Repack-Toggle → Repack-Preis togglen
// Source-Toggle → Source-Preis togglen
// AJAX: Steam URL Feld → "Daten laden" → Vorschau aktualisieren
```

---

#### 8. `public/class-psm-shortcode.php` — Shortcode [psyerns_mods]
```php
// Shortcode Attribute:
// [psyerns_mods columns="3" discord="https://discord.gg/..." orderby="menu_order"]

// Query: WP_Query für psyern_mod Posts
// Für jedes Item:
//   1. Meta-Daten laden
//   2. Steam-Daten aus Cache oder API abrufen
//   3. HTML generieren mit escaped Outputs

// HTML-Struktur:
// <div class="psyerns-mods-grid">
//   <div class="psyerns-mods-container">
//     {foreach item}
//     <div class="psyerns-mods-item">
//       <div class="psm-card">
//         <!-- Bild-Wrapper mit 1:1 Seitenverhältnis (aspect-ratio: 1/1) -->
//         <div class="psm-card__image-wrap">
//           <img class="psm-card__image" src="{preview_url}" alt="{title}" loading="lazy" width="600" height="600">
//         </div>
//         <div class="psm-card__body">
//           <h3 class="psm-card__title">{title}</h3>
//           <!-- WICHTIG: Beschreibung muss BBCode-bereinigt sein (via strip_bbcode()) -->
//           <!-- Zeige max. 150 Zeichen Plaintext — KEIN [h1], [img], [b] etc. -->
//           <p class="psm-card__description">{wp_trim_words(strip_bbcode(description), 20)}</p>
//           <div class="psm-card__badges">
//             {if free} <span class="psm-badge psm-badge--free">FREE</span>
//             {else}    <span class="psm-badge psm-badge--price">€{price}</span>
//             {if repack} <span class="psm-badge psm-badge--repack">REPACK €{repack_price}</span>
//             {if source} <span class="psm-badge psm-badge--source">SOURCE €{source_price}</span>
//           </div>
//           <div class="psm-card__actions">
//             <a class="psm-btn psm-btn--steam" href="{steam_url}" target="_blank" rel="noopener">
//               AUF STEAM ANSEHEN
//             </a>
//             {if discord_url}
//             <a class="psm-btn psm-btn--discord" href="{discord_url}" target="_blank" rel="noopener">
//               DISCORD
//             </a>
//           </div>
//         </div>
//       </div>
//     </div>
//     {/foreach}
//   </div>
// </div>
```

---

### Sicherheits-Anforderungen (von Agent 3):
- Alle Outputs: `esc_html()`, `esc_url()`, `esc_attr()`
- Alle Saves: Nonce-Verification + `sanitize_text_field()` / `absint()` / `floatval()`
- Steam URL Validierung: Nur `steamcommunity.com/sharedfiles/filedetails` URLs
- Keine direkten DB-Queries ohne `$wpdb->prepare()`
- Assets nur laden wenn Shortcode auf der Seite ist

### Zusatz: Settings Page
Erstelle eine einfache Options-Seite unter **Einstellungen → Psyerns Mods**:
- Globale Discord URL (Fallback für alle Karten)
- Steam API Key Feld (optional, für höhere Rate Limits)
- Cache leeren Button
- Anleitung: "Shortcode: [psyerns_mods]"

Schreibe ALLE Dateien vollständig und produktionsreif. Kein Pseudocode außer in den Kommentaren zur Struktur.
```

---

## AUSFÜHRUNGSREIHENFOLGE

```
1. Agent 1 (Analyzer)   → Vollständige Spezifikation
         ↓
2. Agent 2 (Designer)   → Design-System basierend auf Spezifikation + deadmans-echo.de Stil
         ↓
3. Agent 3 (Reviewer)   → Prüfe Agent 1 + Agent 2, erstelle finale Freigabe-Spezifikation
         ↓
4. Agent 4 (CSS Author) → Vollständige CSS-Dateien basierend auf Agent 2 + Agent 3
         ↓
5. Agent 5 (Coder)      → Vollständiger PHP/JS Code basierend auf Agent 3 + Agent 4
```

---

## ERWARTETES ENDRESULTAT

```
psyerns-mods/
├── psyerns-mods.php
├── uninstall.php
├── admin/
│   ├── class-psm-admin.php
│   ├── css/psyerns-mods-admin.css
│   └── js/psyerns-mods-admin.js
├── includes/
│   ├── class-psm-post-type.php
│   ├── class-psm-meta.php
│   ├── class-psm-steam-api.php
│   └── class-psm-cache.php
└── public/
    ├── class-psm-shortcode.php
    ├── css/psyerns-mods-public.css
    └── js/psyerns-mods-public.js
```

**WordPress-Kompatibilität:** 6.0+  
**PHP-Version:** 7.4+  
**Lizenz:** GPL-2.0+  
**Shortcode:** `[psyerns_mods]`  
**Admin-Menü:** Mods (CPT) + Einstellungen → Psyerns Mods

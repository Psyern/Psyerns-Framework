# WordPress Plugin Entwicklungs-Prompt

## 🧩 Was du für ein WordPress Plugin brauchst

### Technische Voraussetzungen
- **PHP** ≥ 7.4 (empfohlen 8.1+)
- **WordPress** ≥ 5.0 (lokal per **WordPress Studio** ← empfohlen 2025, kostenlos von Automattic, oder LocalWP, XAMPP, MAMP, Docker)
- **MySQL/MariaDB** für Datenbankoperationen
- **Composer** (optional, für externe PHP-Abhängigkeiten)
- **Node.js + npm/yarn** (optional, für Gutenberg/Block-Editor)
- **WP-CLI** (empfohlen für schnelle Entwicklung)

### Empfohlener Code-Editor
- VS Code + Erweiterungen: PHP Intelephense, WordPress Snippets, ESLint

---

## 📁 Minimale Plugin-Struktur

```
mein-plugin/
├── mein-plugin.php          ← Haupt-Einstiegsdatei (Pflicht)
├── readme.txt               ← WordPress.org Beschreibung
├── uninstall.php            ← Cleanup beim Deinstallieren
├── includes/
│   ├── class-main.php       ← Hauptklasse (OOP-Ansatz)
│   ├── class-admin.php      ← Admin-Funktionen
│   └── class-frontend.php   ← Frontend-Funktionen
├── admin/
│   ├── css/admin-style.css
│   ├── js/admin-script.js
│   └── views/               ← Admin-Seiten Templates
├── public/
│   ├── css/public-style.css
│   └── js/public-script.js
└── languages/               ← Übersetzungsdateien (.pot, .po, .mo)
```

---

## 🚀 HAUPT-PROMPT: WordPress Plugin erstellen

> **Anleitung:** Kopiere den Prompt unten, fülle alle `[PLATZHALTER]` aus
> und gib ihn an Claude oder ein anderes LLM weiter.

---

```
Du bist ein erfahrener WordPress-Entwickler. Erstelle ein vollständiges,
produktionsreifes WordPress-Plugin nach WordPress Coding Standards.

## Plugin-Metadaten
- Plugin-Name:       [z.B. "Mein Kontaktformular"]
- Plugin-Slug:       [z.B. "mein-kontaktformular"]  ← nur Kleinbuchstaben & Bindestriche
- Kurzbeschreibung:  [Was macht das Plugin in 1-2 Sätzen?]
- Version:           1.0.0
- Autor:             [Dein Name / Firma]
- Autor-URL:         [https://deine-website.de]
- Textdomain:        [identisch mit Slug]
- WP Mindestversion: 5.8
- PHP Mindestversion: 7.4
- Lizenz:            GPL-2.0+

## Gewünschte Funktionen
[Beschreibe GENAU, was das Plugin tun soll – je detaillierter, desto besser]

Beispiele (ersetzen oder ergänzen):
- [ ] Shortcode [SLUG] der [BESCHREIBUNG] rendert
- [ ] Admin-Einstellungsseite unter Einstellungen > [PLUGIN-NAME]
  - Einstellung 1: [Name + Typ: Text / Toggle / Dropdown / Color-Picker]
  - Einstellung 2: ...
- [ ] Custom Post Type "[NAME]" mit Feldern: [Feld1, Feld2, ...]
- [ ] REST-API-Endpunkt: GET/POST /wp-json/[SLUG]/v1/[RESSOURCE]
- [ ] E-Mail-Benachrichtigung bei [EREIGNIS] an [EMPFÄNGER]
- [ ] Datenbanktabelle wp_[SLUG]_[NAME] mit Feldern: [Felder]
- [ ] Widget für die Sidebar: [Beschreibung]
- [ ] Gutenberg-Block "[BLOCK-NAME]": [Beschreibung]

## Technische Anforderungen
- Architektur:       OOP (Klassen) / Prozedural  ← wähle eines
- Sicherheit:        Nonces, Capability-Checks, Input-Sanitizing, Output-Escaping
- Datenbankzugriff:  $wpdb mit Prepared Statements (kein direktes SQL)
- Assets:            CSS/JS nur auf relevanten Seiten laden (wp_enqueue_scripts)
- Übersetzbar:       Ja – alle Strings mit __() / _e() / esc_html__() wrappen
- Coding Standard:   WordPress PHP Coding Standards (WPCS)
- Uninstall-Hook:    Ja – Datenbank/Optionen beim Deinstallieren aufräumen

## Dateien die erstellt werden sollen
Erstelle ALLE notwendigen Dateien mit vollständigem, funktionstüchtigem Code:
1. [slug]/[slug].php                     ← Plugin-Header + Bootstrap
2. [slug]/includes/class-[slug].php      ← Hauptklasse
3. [slug]/includes/class-admin.php       ← Admin-Funktionen (falls nötig)
4. [slug]/includes/class-frontend.php    ← Frontend-Funktionen (falls nötig)
5. [slug]/admin/views/settings-page.php  ← Admin-Template (falls nötig)
6. [slug]/admin/css/admin.css            ← Admin-Styles
7. [slug]/public/css/public.css          ← Frontend-Styles
8. [slug]/uninstall.php                  ← Deinstallations-Routine
9. [slug]/readme.txt                     ← WordPress.org Format

## Ausgabeformat
- Jeden Datei-Inhalt in einem eigenen Code-Block mit Dateinamen als Kommentar
- Nach dem Code: kurze Erklärung der wichtigsten Hooks/Filter
- Am Ende: Installations- und Testanleitung (5 Schritte)
```

---

## 🔧 Spezial-Prompts für häufige Plugin-Typen

### A) Shortcode-Plugin
```
Erweitere das Plugin um einen Shortcode [SLUG attr1="wert" attr2="wert"].
Der Shortcode soll folgendes rendern: [BESCHREIBUNG].
Attribute: attr1 (Standard: "X"), attr2 (Standard: "Y").
Nutze wp_kses_post() für die Ausgabe und validiere alle Attribute.
```

### B) Custom Post Type Plugin
```
Füge einen Custom Post Type "[SINGULAR]" / "[PLURAL]" hinzu mit:
- Slug: [cpt-slug]
- Unterstützte Features: title, editor, thumbnail, excerpt, custom-fields
- Custom Taxonomie "[TAXONOMIE-NAME]" (hierarchisch wie Kategorie / flach wie Tag)
- Meta-Boxen mit Feldern: [Feld1 (Typ), Feld2 (Typ)]
- Archivseite: Ja / Nein
```

### C) Admin-Einstellungsseite
```
Erstelle eine Einstellungsseite mit der WordPress Settings API.
Menü-Position: Einstellungen > [Plugin-Name]
Einstellungsgruppen und Felder:
- Gruppe "Allgemein":
  - [option_key_1]: Text-Input, Label "[Label]", Standard "[Wert]"
  - [option_key_2]: Checkbox, Label "[Label]"
  - [option_key_3]: Select mit Optionen: [Opt1, Opt2, Opt3]
- Gruppe "Erweitert":
  - [option_key_4]: Textarea
Sanitize alle Werte korrekt (sanitize_text_field, absint, etc.)
```

### D) REST API Endpunkt
```
Registriere einen REST-API-Endpunkt:
- Namespace: [slug]/v1
- Route: /[ressource]
- Methoden: GET (öffentlich), POST (nur eingeloggte Nutzer mit Rolle "[ROLLE]")
- GET gibt zurück: [Beschreibung der Daten / JSON-Struktur]
- POST erwartet: [Parameter mit Typ und Validierung]
Nutze register_rest_route() und implementiere permission_callback korrekt.
```

### E) Datenbanktabelle
```
Erstelle bei Plugin-Aktivierung eine Datenbanktabelle:
- Tabellenname: {$wpdb->prefix}[slug]_[name]
- Felder:
  - id (BIGINT UNSIGNED NOT NULL AUTO_INCREMENT)
  - [feld1] ([TYP], z.B. VARCHAR(255))
  - [feld2] ([TYP])
  - created_at (DATETIME DEFAULT CURRENT_TIMESTAMP)
Nutze dbDelta() für die Erstellung und speichere die DB-Version in den Options.
Erstelle CRUD-Methoden als statische Klassenmethoden.
```

---

## ✅ Checkliste vor dem Deployment

```
Sicherheit
[ ] Alle Nutzereingaben sanitized (sanitize_text_field, absint, wp_kses etc.)
[ ] Alle Ausgaben escaped (esc_html, esc_attr, esc_url, wp_kses_post)
[ ] Nonces für alle Formulare und AJAX-Anfragen
[ ] Capability-Checks (current_user_can()) vor Admin-Aktionen
[ ] Prepared Statements für alle DB-Queries ($wpdb->prepare())
[ ] Direct-file-access-Check: if (!defined('ABSPATH')) exit;

Performance
[ ] CSS/JS nur auf benötigten Seiten laden
[ ] Datenbankabfragen gecacht wo sinnvoll (Transients API)
[ ] Bilder optimiert

Code-Qualität
[ ] WordPress Coding Standards eingehalten
[ ] Alle Strings internationalisierbar
[ ] PHP-Fehler und Warnungen bei WP_DEBUG=true behoben
[ ] Keine veralteten WordPress-Funktionen genutzt

Kompatibilität
[ ] Auf aktuelle WordPress-Version getestet
[ ] Auf PHP 7.4, 8.0 und 8.1 getestet
[ ] Beliebte Plugins geprüft (WooCommerce, Yoast, WPML etc.) – falls relevant
[ ] Responsives Admin-Interface
```

---

## 📚 Wichtige WordPress-Ressourcen

| Ressource | URL |
|---|---|
| Developer Handbook | https://developer.wordpress.org/plugins/ |
| Hooks-Referenz | https://developer.wordpress.org/reference/hooks/ |
| Coding Standards | https://developer.wordpress.org/coding-standards/ |
| Plugin Boilerplate Generator | https://wppb.me/ |
| WP-CLI Docs | https://wp-cli.org/ |

---

*Erstellt für die WordPress Plugin-Entwicklung mit KI-Unterstützung.*

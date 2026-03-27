# WordPress Plugin Development

Quelle: https://developer.wordpress.org/plugins/ & https://developer.wordpress.org/coding-standards/wordpress-coding-standards/

---

## Architektur & Struktur

- Verwende OOP (Klassen) mit separater `init()` oder `register()`-Methode für Hooks – keine Hooks im Konstruktor
- Konstruktor nur für leichtes Bootstrapping und Dependency-Injection nutzen
- Jede Klasse bekommt eine eigene Datei: `class-[name].php`
- Plugin-Slug als Präfix für alle Funktionen, Klassen, Hooks und Optionen (Namenskollisionen vermeiden)
- Ordnerstruktur einhalten: `includes/`, `admin/`, `public/`, `languages/`
- Bootstrap nur in der Haupt-PHP-Datei, Logik in separaten Klassen
- `uninstall.php` immer anlegen – `WP_UNINSTALL_PLUGIN` prüfen und alle Daten/Optionen aufräumen
- Niemals WordPress Core-Dateien editieren – ausschließlich Hooks und Plugins nutzen

## Sicherheit (Pflicht bei jedem Output)

- Jede Datei beginnt mit: `if ( ! defined( 'ABSPATH' ) ) exit;`
- Alle Nutzereingaben sanitizen: `sanitize_text_field()`, `absint()`, `wp_kses()` etc.
- Alle Ausgaben escapen: `esc_html()`, `esc_attr()`, `esc_url()`, `wp_kses_post()`
- Nonces für alle Formulare und AJAX-Anfragen (`wp_nonce_field`, `check_admin_referer`)
- Capability-Checks vor jeder Admin-Aktion: `current_user_can()`
- Datenbankzugriff nur via `$wpdb->prepare()` – kein direktes SQL
- Sensible Nutzerdaten gemäß WordPress Privacy Guidelines behandeln

## PHP Coding Standards (WPCS)

- Einrückung mit Tabs, nicht Spaces – gilt auch für HTML und CSS
- Geschweifte Klammern auf derselben Zeile wie die Kontrollstruktur
- Funktions- und Variablennamen in `snake_case`, Klassen in `PascalCase`
- Einfache Anführungszeichen für Strings bevorzugen, außer der String enthält PHP-Variablen
- Leerzeichen nach Schlüsselwörtern: `if (`, `foreach (`, `function foo(` etc.
- Yoda Conditions verwenden: `if ( true === $var )` statt `if ( $var === true )`
- Keine veralteten WordPress-Funktionen verwenden
- PHPDoc-Kommentare für alle Klassen, Methoden und Funktionen

## JavaScript Coding Standards

- Variablen- und Funktionsnamen in `camelCase` (abweichend von PHP!)
- Einfache Anführungszeichen für Strings (WordPress-Standard)
- Tabs für Einrückung, Semikolons am Zeilenende
- `jQuery` statt `$` verwenden (Konfliktmodus)
- Globale Variablen vermeiden – in Closures kapseln oder über `wp`-Objekt
- `window.wp = window.wp || {};` beim Erweitern des `wp`-Objekts
- Skripte per `wp_enqueue_script()` laden, niemals direkt per `<script>`-Tag

## CSS Coding Standards

- Tabs für Einrückung
- Geschweifte Klammer auf derselben Zeile wie der Selektor
- Jede Eigenschaft in einer eigenen Zeile, mit abschließendem Semikolon
- Klassen-Namen mit Bindestrichen: `.plugin-name__element` – kein camelCase, keine Underscores
- Keine IDs für Styling, nur Klassen
- Keine Inline-Styles – ausschließlich externe Stylesheets
- Properties und Werte in Kleinbuchstaben (außer Font-Namen)

## HTML Standards & Accessibility

- Semantisches HTML5 verwenden: `<main>`, `<section>`, `<header>`, `<nav>`, `<article>`, `<footer>`
- Alle Tags und Attribute in Kleinbuchstaben, doppelte Anführungszeichen für Attribute
- WCAG 2.2 Level AA einhalten: ARIA-Attribute, Keyboard-Navigation, Screen-Reader-Kompatibilität
- Labels für alle Formularfelder (`<label for="">`)

## Hooks & Filter

- Aktionen mit `add_action()`, Filter mit `add_filter()` registrieren
- Eigene Hooks mit Plugin-Slug präfixen: `do_action( 'mein-plugin/event' )` – Kollisionen vermeiden
- `apply_filters()` für alle Texte nutzen, die im Browser ausgegeben werden (Erweiterbarkeit)
- Lifecycle-Hooks: `register_activation_hook()`, `register_deactivation_hook()`
- Deaktivierungs-Hook: nur temporäre Daten löschen (z.B. Cron-Jobs), keine permanenten Daten
- Direkte Ausführung beim Laden vermeiden – alles über Hooks

## WordPress APIs nutzen (nicht neu erfinden)

- Options API für Plugin-Einstellungen (`get_option`, `update_option`)
- Settings API für Admin-Einstellungsseiten (`register_setting`, `add_settings_field`)
- HTTP API statt cURL (`wp_remote_get`, `wp_remote_post`)
- Transients API für gecachte Datenbankabfragen (`set_transient`, `get_transient`)
- Plugin API für Shortcodes, Widgets, Custom Post Types

## Datenbankoperationen

- Tabellennamen immer mit `$wpdb->prefix` präfixen
- Tabellen mit `dbDelta()` erstellen, DB-Version in Options speichern
- CRUD-Methoden als statische Klassenmethoden kapseln
- Activation-Hook für Tabellenerstellung, Uninstall-Hook für Tabellenlöschung

## Internationalisierung

- Alle sichtbaren Strings mit `__()`, `_e()`, `esc_html__()` oder `esc_html_e()` wrappen
- Textdomain identisch mit Plugin-Slug
- `.pot`-Datei im `languages/`-Ordner ablegen

## Dateiausgabe & Templates

- Template-Dateien in `admin/views/` oder `public/views/`
- Kein PHP-Logik-Code in Template-Dateien – nur Ausgabe
- Shortcode-Output immer als Return-Wert (nicht echo), Output Buffering bei Bedarf
- `apply_filters()` auf alle Frontend-Textausgaben für Erweiterbarkeit

## Tooling & Qualitätssicherung

- PHP_CodeSniffer mit WPCS-Ruleset für PHP-Linting
- ESLint mit WordPress-Konfiguration für JavaScript
- WP_DEBUG=true + WP_DEBUG_LOG=true in lokaler wp-config.php
- Query Monitor Plugin für Performance-Debugging
- PHPUnit für Unit-Tests

## Dokumentation

- PHPDoc für alle PHP-Klassen, Methoden und Funktionen
- JSDoc für JavaScript-Funktionen
- `readme.txt` im WordPress.org-Format: Beschreibung, Installation, Changelog
- Inline-Kommentare für komplexe Logik

## Lokale Entwicklung

- Empfohlene Umgebung: WordPress Studio (kostenlos, von Automattic, 2025 empfohlen)
- Alternativen: LocalWP, XAMPP, MAMP, Docker (wp-env)
- Plugin-Boilerplate als Startpunkt: https://wppb.me/
- Offizielle Referenz: https://developer.wordpress.org/plugins/

## Ausgabeformat bei Code-Generierung

- Jede Datei in eigenem Code-Block mit Dateinamen als erstem Kommentar
- Vollständigen, lauffähigen Code liefern – keine Platzhalter
- Nach dem Code: kurze Erklärung der wichtigsten Hooks
- Am Ende: Installations- und Testanleitung in 5 Schritten

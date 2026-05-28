# Orchestrierungsauftrag: Psyerns Framework — Whitelist & Verification System

## Kontext & Ziel

Erweiterung des bestehenden WordPress-Plugins **"Psyerns Framework"** um ein
vollständiges Whitelist-Verifizierungssystem. Spieler des DayZ-Servers können sich
über **Steam** (Website) und/oder **Discord** (Bot-Button) verifizieren lassen.
Nicht-verifizierte Spieler werden beim Verbindungsversuch mit der Meldung
"Du musst dich verifizieren!" vom Server gekickt.

**Bestehende Infrastruktur (DayZ-Seite):**
- `PF_WhitelistManager.c` — check/add/remove via REST (`psyern/v1`)
- `EnableWhitelist: true/false` in der DayZ-Config
- REST-Endpunkte: Whitelist check beim Player-Connect via `PF_RestInit.c`

**WordPress Dev Rules:** `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\wordpress-plugin.md`
**UI/UX Skill:** UI UX Pro Max — Category: **Creative / Gaming** — DayZ Postapokalyptik-Theme
**Design System:** Konsistent mit den 3 bestehenden Leaderboard-Themes (`military`, `neon`, `ash`)

---

## Design System — Gaming / DayZ Context

```
+----------------------------------------------------------------------------------------+
|  TARGET: Psyerns Framework — Whitelist Verification UI                                 |
+----------------------------------------------------------------------------------------+
|                                                                                        |
|  PATTERN: Trust-Gate + Social Proof                                                    |
|     Conversion: Sicherheits-orientiert, klarer Call-to-Action                         |
|     Flow: Landing → Login-Methode wählen → Verify → Bestätigung                       |
|                                                                                        |
|  STYLE: Dark Military / Survival Gaming                                                |
|     Keywords: Authentisch, rau, vertrauenswürdig, survival-harte Ästhetik             |
|     Best For: Gaming Communities, Server-Verwaltung, Player-Portale                   |
|     Performance: Excellent | Accessibility: WCAG AA                                   |
|                                                                                        |
|  COLORS (konsistent mit Leaderboard-Themes):                                           |
|     military: #0a0f0a / #4ade80 / #86efac                                             |
|     neon:     #050508 / #f0e130 / #ff3860 / #00d4ff                                   |
|     ash:      #1a1714 / #c8392b / #d4a853 / #f5f0e8                                   |
|                                                                                        |
|  TYPOGRAPHY: Gleiche Fonts wie Leaderboard-Themes                                     |
|     military: Courier Prime / Share Tech Mono / Oswald                                |
|     neon:     Black Ops One / Bebas Neue / JetBrains Mono                             |
|     ash:      Libre Baskerville / Playfair Display / DM Mono                         |
|                                                                                        |
|  KEY EFFECTS: Staggered Reveals, Status-Pulse, Progress-Steps, Hover-States          |
|                                                                                        |
|  AVOID: Pastellfarben, runde UI-Bubbles, generische Auth-Formulare,                   |
|         AI purple/pink Gradienten, Comic-Elemente                                     |
|                                                                                        |
|  PRE-DELIVERY CHECKLIST:                                                               |
|     [ ] Keine Emojis als Icons — SVG oder Unicode-Symbole                             |
|     [ ] cursor-pointer auf allen klickbaren Elementen                                 |
|     [ ] Hover-States mit smooth transitions (150-300ms)                               |
|     [ ] Kontrast mind. 4.5:1 (WCAG AA)                                               |
|     [ ] Focus-States für Keyboard-Navigation                                          |
|     [ ] Responsive: 375px, 768px, 1024px, 1440px                                      |
|     [ ] prefers-reduced-motion respected                                              |
+----------------------------------------------------------------------------------------+
```

---

## Agent-Aufteilung

---

### Agent 1 — Backend: Whitelist-System (PHP)

**Aufgabe:** Alle PHP-Backend-Komponenten für das Whitelist-Verifizierungssystem

#### 1a. Datenbankschema

**Tabelle `wp_psyern_whitelist`:**
```sql
id              BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY
steam_id        VARCHAR(20) NOT NULL UNIQUE
player_name     VARCHAR(64)
discord_id      VARCHAR(20)
discord_tag     VARCHAR(64)
avatar_url      VARCHAR(255)
status          ENUM('pending','approved','rejected','banned') DEFAULT 'pending'
verified_via    ENUM('steam','discord','manual','api') DEFAULT 'steam'
ip_address      VARCHAR(45)
whitelisted_at  DATETIME NULL
created_at      DATETIME DEFAULT CURRENT_TIMESTAMP
updated_at      DATETIME ON UPDATE CURRENT_TIMESTAMP
notes           TEXT
```

**Tabelle `wp_psyern_whitelist_log`:**
```sql
id          BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY
steam_id    VARCHAR(20)
action      ENUM('add','remove','approve','reject','ban','kick','auto_kick')
actor       VARCHAR(64)   -- 'system', Discord-Tag oder WP-Username
reason      TEXT
created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
```

#### 1b. REST-Endpunkte (Namespace: `psyern/v1`)

**Öffentliche Endpunkte (kein Auth nötig):**
- `POST /whitelist/apply` — Spieler stellt Antrag (Steam ID + optionale Discord ID)
- `GET  /whitelist/status?steam_id=XXX` — Status eines Spielers prüfen
- `GET  /whitelist/check?steam_id=XXX` — Verwendet vom DayZ-Server (API-Key Auth)

**Admin-Endpunkte (API-Key Auth):**
- `POST /whitelist/add` — Spieler hinzufügen (DayZ-Server oder Discord-Bot)
- `POST /whitelist/remove` — Spieler entfernen
- `GET  /whitelist/list` — Alle Einträge (paginiert)
- `POST /whitelist/approve` — Antrag genehmigen
- `POST /whitelist/reject` — Antrag ablehnen

**Discord-Bot-Webhook-Endpunkt:**
- `POST /whitelist/discord-verify` — Discord-Bot sendet Steam+Discord-Daten nach Verifikation

#### 1c. Whitelist-Logik

```php
// Ablauf beim DayZ Player-Connect:
// 1. DayZ Server ruft GET /whitelist/check?steam_id=XXX auf
// 2. WordPress prüft Status in wp_psyern_whitelist
// 3. Wenn status = 'approved' → {"whitelisted": true}
// 4. Wenn nicht vorhanden oder pending/rejected/banned → {"whitelisted": false, "message": "Du musst dich verifizieren! Besuche: https://deine-seite.de/verify"}
// 5. DayZ-Seite: PF_WhitelistManager.c kickt Spieler mit der message
```

**Auto-Approve Option:** Wenn `WhitelistAutoApprove = true` in den Einstellungen,
wird jeder Steam-Login automatisch auf `approved` gesetzt (kein manuelles Review).

#### 1d. Steam OpenID Login (WordPress-seitig)

```php
// Kein vollständiges OAuth — Steam nutzt OpenID 2.0
// Flow:
// 1. User klickt "Mit Steam verifizieren"
// 2. Weiterleitung zu: https://steamcommunity.com/openid/login?...
// 3. Steam leitet zurück zu: /wp-json/psyern/v1/whitelist/steam-callback
// 4. WordPress validiert OpenID-Response, extrahiert Steam ID
// 5. Spieler wird in wp_psyern_whitelist eingetragen (status: pending oder approved)
// 6. Weiterleitung zur Bestätigungsseite

// Steam OpenID Parameter:
// openid.ns, openid.mode, openid.claimed_id (enthält SteamID64)
// Validierung via HTTP-Request zurück an Steam
```

#### 1e. Discord-Integration

**Option A: Discord-Bot sendet Daten an WordPress**
```
Discord Bot Flow:
1. Spieler klickt "Verify"-Button in Discord
2. Bot antwortet mit Ephemeral-Message: "Bitte gib deine Steam-ID ein:"
3. Spieler antwortet mit Steam-ID (Format: 7656119...)
4. Bot validiert Format, sendet POST /whitelist/discord-verify
   Body: { "steam_id": "...", "discord_id": "...", "discord_tag": "..." }
5. WordPress trägt Spieler ein
6. Bot antwortet: "Erfolgreich registriert! Du wirst beim nächsten Server-Restart freigeschaltet."
```

**Option B: WordPress-generierter Verify-Link (kein Bot nötig)**
```
1. WordPress generiert einen einmaligen Verify-Link mit Token
2. Discord-Message enthält Link: https://deine-seite.de/verify?token=XXX
3. Spieler öffnet Link, loggt sich mit Steam ein
4. WordPress verknüpft Discord-ID (aus Token) mit Steam-ID
```

**Implementiere beide Optionen.** Admin wählt in den Einstellungen welche aktiv ist.

#### 1f. WordPress Shortcodes

- `[psyern_whitelist_form]` — Vollständiges Verifizierungsformular (Steam + Discord)
- `[psyern_whitelist_status steam_id="XXX"]` — Status-Anzeige für einen Spieler
- `[psyern_whitelist_status]` — Status des aktuell eingeloggten Nutzers (falls WP-Login)

#### 1g. Admin-Einstellungsseite (Erweiterung)

Neue Sektion "Whitelist" unter Psyerns Framework → Whitelist:
- **Whitelist aktivieren/deaktivieren** (Master-Toggle → sendet auch an DayZ via API)
- **Auto-Approve** Toggle (sofortige Freischaltung nach Steam-Login)
- **Verifikations-Methoden:** Steam / Discord / Beide (Checkboxen)
- **Discord-Integration-Typ:** Bot-Webhook / Verify-Link
- **Discord Bot Webhook URL** (falls Bot-Option aktiv)
- **Kick-Nachricht** (frei editierbares Textfeld, wird an DayZ gesendet)
- **Verify-Seiten-URL** (wird in der Kick-Nachricht angezeigt)
- **Tabellen-Ansicht:** Alle Whitelist-Einträge mit Approve/Reject/Ban-Buttons
  - Filter: Alle / Pending / Approved / Rejected / Banned
  - Suche nach Steam-ID, Discord-Tag, Spielername
  - Bulk-Actions: Approve selected / Reject selected

---

### Agent 2 — Frontend Theme 4: `ops` — Special Operations Briefing

**Aesthetic Direction:** Du bist ein SOG-Analyst, der Spieler-Identitäten in einer
geheimen militärischen Datenbank verifiziert. Das Interface wirkt wie ein echtes
biometrisches Authentifizierungssystem aus einem Geheimdienst-Thriller.
Grüne Vector-Grafiken auf schwarzem Grund. Alles fühlt sich wie ein echter
Security-Clearance-Prozess an.

```
+----------------------------------------------------------------------------------------+
|  THEME: ops — Special Operations Biometric Clearance                                   |
+----------------------------------------------------------------------------------------+
|  Farben: #020d02 (void), #041a04, #4ade80 (phosphor), #16a34a, #86efac (dim)          |
|  Fonts: Share Tech Mono (alles), Orbitron (Headlines)                                 |
|  Stil: Militär-Terminal, CRT-Phosphor, Biometrie-Scanner                              |
+----------------------------------------------------------------------------------------+
```

**Visuelle Elemente:**

1. **Verify-Seite Hauptlayout:**
```
┌─────────────────────────────────────────────────────┐
│ [PSYERNS FRAMEWORK] IDENTITY VERIFICATION SYSTEM    │
│ CLEARANCE LEVEL: CIVILIAN → REQUEST UPGRADE         │
│ ─────────────────────────────────────────────────── │
│                                                     │
│  STEP 1 ──●── STEP 2 ──○── STEP 3                  │
│  [CONNECT]   [VERIFY]    [CLEARED]                  │
│                                                     │
│  ┌─────────────────────────────────────────────┐   │
│  │   IDENTITY NOT ON RECORD                    │   │
│  │   VERIFICATION REQUIRED TO ACCESS SERVER    │   │
│  │                                             │   │
│  │   ┌──────────────────────────────────────┐  │   │
│  │   │  [STEAM ICON] AUTHENTICATE VIA STEAM │  │   │
│  │   └──────────────────────────────────────┘  │   │
│  │                                             │   │
│  │   ─── OR ─────────────────────────────────  │   │
│  │                                             │   │
│  │   [DISCORD ICON] VERIFY VIA DISCORD BOT    │   │
│  └─────────────────────────────────────────────┘   │
│                                                     │
│  STATUS: AWAITING AUTHENTICATION...     [●●○○○]    │
└─────────────────────────────────────────────────────┘
```

2. **Status-Anzeigen:**
   - `PENDING` → Gelb pulsierend: "AWAITING CLEARANCE APPROVAL"
   - `APPROVED` → Grün mit Scan-Animation: "CLEARANCE GRANTED — ACCESS AUTHORIZED"
   - `REJECTED` → Rot mit statischem Noise: "CLEARANCE DENIED"
   - `BANNED` → Rot blinkend: "IDENTITY FLAGGED — PERMANENT BAN"

3. **Effekte:**
   - Scanline-Overlay permanent aktiv
   - ASCII-Art Rahmen für Boxen (mit CSS `border` + `::before`/`::after`)
   - Typing-Animation beim Laden ("SEARCHING DATABASE...")
   - Fortschritts-Balken mit Block-Chars: `[████████░░]`
   - Radar-Sweep-Animation hinter dem Status-Badge (CSS `conic-gradient`)
   - CRT-Flimmern (`animation: flicker 0.15s infinite`)

**Dateien:**
```
public/css/psyern-theme-ops.css
public/templates/whitelist-ops.php
```

---

### Agent 3 — Frontend Theme 5: `outbreak` — Biohazard / Survival Horror

**Aesthetic Direction:** Eine Quarantäne-Station in einer verseuchten Stadt.
Der Server ist eine der letzten sicheren Zonen — der Einlass wird streng kontrolliert.
Das UI erinnert an medizinische Notfallsysteme, gelbe Warn-Bänder, Hazmat-Ästhetik.

```
+----------------------------------------------------------------------------------------+
|  THEME: outbreak — Quarantine Zone Access Control                                      |
+----------------------------------------------------------------------------------------+
|  Farben: #0d0d00 (dunkelst), #1a1a00, #f59e0b (Warnung), #dc2626 (Gefahr), #fef9c3   |
|  Fonts: Rajdhani (Headlines), Inconsolata (Mono-Daten), Exo 2 (Body)                  |
|  Stil: Medizinisch-Notfall, Biohazard, Quarantäne-Terminal                            |
+----------------------------------------------------------------------------------------+
```

**Visuelle Elemente:**

1. **Hauptlayout:**
```
╔═══════════════════════════════════════════════════════╗
║  ⚠ QUARANTINE ZONE — DEADMANS ECHO                   ║
║  AUTHORIZED PERSONNEL ONLY                           ║
╠═══════════════════════════════════════════════════════╣
║                                                       ║
║  BIOHAZARD LEVEL: [████████░░] EXTREME               ║
║                                                       ║
║  IDENTITY VERIFICATION REQUIRED                       ║
║  Unauthorized access will result in immediate        ║
║  ejection from the safe zone.                        ║
║                                                       ║
║  ┌─────────────────────────────────────────────┐     ║
║  │ SCAN YOUR STEAM ID                          │     ║
║  │ ─────────────────────────────────────────── │     ║
║  │ [▶ CONNECT STEAM ACCOUNT]                   │     ║
║  │ [▶ VERIFY VIA DISCORD]                      │     ║
║  └─────────────────────────────────────────────┘     ║
╚═══════════════════════════════════════════════════════╝
```

2. **Status-Anzeigen:**
   - `PENDING` → Gelb/Amber pulsierend: "IDENTITY UNDER REVIEW — AWAIT CLEARANCE"
   - `APPROVED` → Grün: "IDENTITY CONFIRMED — SAFE ZONE ACCESS GRANTED"
   - `REJECTED` → Rot: "IDENTITY NOT CLEARED — CONTACT ADMIN"
   - `BANNED` → Rot mit Warnsymbol: "SUBJECT QUARANTINED — ACCESS PERMANENTLY REVOKED"

3. **Effekte:**
   - Gelbe Warn-Streifen als `::before` Pseudo-Element auf dem Container
   - Pulsierende Biohazard-Symbol Animation (SVG inline, CSS-animiert)
   - Roter Alarm-Flash bei Fehler (`animation: alarm 1s ease-in-out`)
   - Fortschrittsbalken in Amber/Gelb bei Pending-Status
   - Noise-Textur auf dem Hintergrund (Data-URI SVG)
   - Medizinisch anmutende Trennlinien (gestrichelte Linien)

**Dateien:**
```
public/css/psyern-theme-outbreak.css
public/templates/whitelist-outbreak.php
```

---

### Agent 4 — Frontend Theme 6: `echo` — Minimal / Brutalist Clean

**Aesthetic Direction:** "Deadmans Echo" — der Name des Servers selbst wird zur
Design-Philosophie. Klar, direkt, kein Schnickschnack. Als ob ein Überlebender
mit einem kaputten Laptop und letztem Batteriestrom diese Seite gebaut hat.
Maximale Lesbarkeit, minimale Mittel, aber trotzdem unverwechselbar.

```
+----------------------------------------------------------------------------------------+
|  THEME: echo — Brutalist Clarity / Last Signal                                         |
+----------------------------------------------------------------------------------------+
|  Farben: #0f0f0f (fast schwarz), #f0f0f0 (fast weiß), #e63946 (Signal-Rot), #fff     |
|  Fonts: Space Grotesk NEIN → stattdessen: Syne (Display), Azeret Mono (Data)          |
|  Stil: Brutalist Typography, Swiss Grid, Survival Zine                                 |
+----------------------------------------------------------------------------------------+
```

**Visuelle Elemente:**

1. **Hauptlayout:**
```
DEADMANS ECHO
─────────────────────────────────────

YOU ARE NOT ON THE LIST.

To access the server, verify your identity.
Choose a method:

[→ STEAM]          [→ DISCORD]

─────────────────────────────────────
STATUS: unknown
LAST CHECK: never
```

2. **Design-Prinzipien:**
   - Kein Rahmen außer dünnen `1px solid` Linien
   - Große Typografie (der Text IST das Design)
   - Negativraum als Stilmittel
   - Status-Änderungen per reinem Text ohne Animationen (außer einem subtilen Fade)
   - Einzig der Status-Indikator bekommt Farbe (`#e63946` Rot / Grün)
   - Kein Background-Image, kein Texture — reines `#0f0f0f`

3. **Effekte (bewusst minimal):**
   - Text blinkt einmalig beim Laden (`animation: once 0.3s`)
   - Roter Underline-Akzent auf dem Haupttitel
   - Hover: `background: #f0f0f0; color: #0f0f0f` (invertiert) auf Buttons
   - Status-Text faded in mit 200ms

**Dateien:**
```
public/css/psyern-theme-echo.css
public/templates/whitelist-echo.php
```

---

### Agent 5 — JavaScript & AJAX-Integration

**Aufgabe:** Alle client-seitigen Funktionen für das Whitelist-System

**Datei: `public/js/psyern-whitelist.js`**

```javascript
// Funktionen:

// 1. Steam OpenID Redirect
PsyernWhitelist.steamLogin() {
    // Baut OpenID-URL und redirectet
    // Zeigt Loading-Animation während Redirect
}

// 2. Discord Verify Link
PsyernWhitelist.discordVerify() {
    // Öffnet Discord-Link oder kopiert ihn (mobile)
    // Polling alle 5s: GET /whitelist/status?steam_id=XXX
    // Wenn Status wechselt → Update UI
}

// 3. Status Polling
PsyernWhitelist.pollStatus(steamId) {
    // Interval: alle 10 Sekunden
    // Stoppt wenn status = 'approved' oder 'rejected'
    // Aktualisiert UI ohne Seitenreload
}

// 4. Step Progress
PsyernWhitelist.updateStep(step) {
    // Aktualisiert Step-Indicator (1/2/3)
    // Animiert Übergang zwischen Steps
}

// 5. Theme-spezifische Animationen
PsyernWhitelist.initTheme(theme) {
    // 'ops': Radar-Sweep, Typing-Effekt
    // 'outbreak': Biohazard-Puls, Alarm-Flash
    // 'echo': Minimale Fade-Ins
}

// wp_localize_script Daten:
// psyernWhitelistData.ajaxUrl
// psyernWhitelistData.nonce
// psyernWhitelistData.steamRedirectUrl
// psyernWhitelistData.discordVerifyUrl
// psyernWhitelistData.statusEndpoint
// psyernWhitelistData.theme
// psyernWhitelistData.i18n (Übersetzungs-Strings)
```

---

### Agent 6 — Integration & Admin-UI

**Aufgabe:** Admin-Tabellen-UI für Whitelist-Verwaltung + komplette Integration

**Admin-View: `admin/views/whitelist-page.php`**

```
┌──────────────────────────────────────────────────────────────────────────────┐
│  PSYERNS FRAMEWORK — WHITELIST MANAGEMENT                                    │
├──────────────────────────────────────────────────────────────────────────────┤
│  Whitelist: [● ACTIVE ▼]    Auto-Approve: [OFF]    Methode: [Steam+Discord]  │
│                                                                              │
│  [+ ADD MANUALLY]  [IMPORT CSV]  [EXPORT]  Search: [_____________] [SEARCH] │
├──────────────────────────────────────────────────────────────────────────────┤
│  Filter: [ALL▼] [PENDING (12)] [APPROVED (48)] [REJECTED (3)] [BANNED (1)]  │
├────┬──────────────────────┬───────────────┬──────────┬──────────┬───────────┤
│ ✓  │ STEAM ID / NAME      │ DISCORD       │ STATUS   │ DATE     │ ACTIONS   │
├────┼──────────────────────┼───────────────┼──────────┼──────────┼───────────┤
│ ✓  │ 76561198... / GHOST  │ ghost#1234    │ PENDING  │ today    │ ✓ ✗ 🔨    │
│ ✓  │ 76561198... / NOMAD  │ -             │ APPROVED │ 03/20    │ ✓ ✗ 🔨    │
└────┴──────────────────────┴───────────────┴──────────┴──────────┴───────────┘
```

- Tabelle nutzt WordPress `WP_List_Table` Klasse
- Bulk-Actions: Approve / Reject / Ban / Delete
- Live-Filter ohne Seitenreload (AJAX)
- Pagination
- Export als CSV
- Audit-Log-Tab: Alle Aktionen der letzten 30 Tage

**`admin/css/psyern-admin-whitelist.css`:**
- Konsistent mit dem bestehenden Admin-Styling
- Status-Badges: Farbige Pills (green/yellow/red/grey)
- Tabelle im WordPress-Standard-Stil aber mit DayZ-Akzentfarbe

---

## Datei-Gesamtstruktur (Erweiterung)

```
psyerns-framework/
├── includes/
│   ├── class-psyern-whitelist.php          ← Whitelist CRUD + Logik
│   ├── class-psyern-whitelist-api.php      ← REST-Endpunkte Whitelist
│   ├── class-psyern-steam-auth.php         ← Steam OpenID 2.0
│   └── class-psyern-discord-webhook.php    ← Discord Bot Webhook Empfänger
├── admin/
│   ├── class-psyern-admin-whitelist.php    ← Admin-Seite + WP_List_Table
│   └── views/
│       └── whitelist-page.php             ← Admin-Template
├── public/
│   ├── css/
│   │   ├── psyern-whitelist.css            ← Basis-Styles (theme-agnostisch)
│   │   ├── psyern-theme-ops.css            ← Theme 4
│   │   ├── psyern-theme-outbreak.css       ← Theme 5
│   │   └── psyern-theme-echo.css           ← Theme 6
│   ├── js/
│   │   └── psyern-whitelist.js             ← AJAX + Animationen + Polling
│   └── templates/
│       ├── whitelist-form.php              ← Shortcode-Template (theme-agnostisch)
│       ├── whitelist-ops.php               ← Theme 4 Template
│       ├── whitelist-outbreak.php          ← Theme 5 Template
│       └── whitelist-echo.php             ← Theme 6 Template
└── languages/
    └── psyerns-framework.pot              ← (erweitern)
```

---

## Flow-Diagramm Gesamtsystem

```
SPIELER VERSUCHT SERVER-BEITRITT
         │
         ▼
DayZ: PF_WhitelistManager → GET /whitelist/check?steam_id=XXX
         │
         ├── whitelisted: true  → SPIELER KOMMT REIN ✓
         │
         └── whitelisted: false
                   │
                   ▼
         DayZ kickt Spieler:
         "Du musst dich verifizieren!
          Besuche: https://deine-seite.de/verify"
                   │
         SPIELER ÖFFNET WEBSITE
                   │
         ┌─────────┴──────────┐
         │                    │
    [STEAM LOGIN]       [DISCORD VERIFY]
         │                    │
    Steam OpenID         Discord Bot
    Callback             sendet POST
         │              /discord-verify
         └──────┬────────────┘
                │
         WP trägt Steam-ID ein
         status: pending (oder approved wenn Auto-Approve)
                │
         ┌──────┴──────────────┐
         │                     │
   Auto-Approve = ON     Auto-Approve = OFF
         │                     │
    status: approved     Admin reviewed
    sofort freigeschaltet      │
                         status: approved/rejected
                               │
                         Discord/E-Mail Benachrichtigung
```

---

## Sicherheits-Anforderungen (Pflicht)

- Steam OpenID-Response MUSS serverseitig bei Steam validiert werden (kein Client-Trust)
- Discord-Webhook-Endpunkt MUSS mit Secret-Token gesichert sein
- Rate-Limiting: Max. 5 Verify-Versuche pro IP pro Stunde
- `WP_UNINSTALL_PLUGIN` entfernt alle Whitelist-Tabellen
- Alle Inputs: `sanitize_text_field()`, Steam-IDs: regex-validiert (`/^7656119\d{10}$/`)
- Audit-Log für alle Whitelist-Änderungen (wer, wann, was)
- Admin-Aktionen: `current_user_can('manage_options')` + Nonce

---

## Ausgabe-Anforderungen

Jeder Agent liefert:
1. **Alle Dateien** als vollständige Code-Blöcke (Dateiname als Kommentar in Zeile 1)
2. **Keine Platzhalter** — produktionsreifer, lauffähiger Code
3. **WordPress Coding Standards** eingehalten (Tabs, snake_case, PHPDoc, ABSPATH-Check)
4. **Am Ende:** Kurzanleitung zur Integration in bestehenden Plugin-Code

---

## Referenzen

- WordPress Plugin Handbook: https://developer.wordpress.org/plugins/
- WordPress Coding Standards: https://developer.wordpress.org/coding-standards/
- Steam OpenID: https://partner.steamgames.com/doc/features/auth#website_auth
- Dev Rules: `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\wordpress-plugin.md`
- Bestehender Orchestrierungsauftrag: `psyern-leaderboard-orchestration-prompt.md`

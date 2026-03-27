# Plugin Fix Prompt — 3 Korrekturen (psyerns-framework & psyerns-mods)

> Führe die folgenden 3 Fixes exakt in den angegebenen Dateien durch. Keine anderen Dateien anfassen.

---

## FIX 1 — Eigene Felder: Bekannte Feldnamen ergänzen

**Datei:** `wordpress-plugin/psyerns-mods/admin/class-psm-admin.php`

**Problem:** Das Eingabefeld "Eigene Felder" (ca. Zeile 962–970) zeigt nur den Placeholder `score, reputation, bounty`. Der User weiß nicht, welche Felder die DayZ-Daten tatsächlich enthalten. Der Hilfstext ist zu allgemein.

**Was zu ändern ist:**

Finde diesen Block (ca. Zeile 959–970):

```php
<tr class="psm-discord-field">
    <th>
        <label for="psm_discord_custom_fields"><?php esc_html_e( 'Eigene Felder', 'psyerns-mods' ); ?></label>
    </th>
    <td>
        <input type="text"
            id="psm_discord_custom_fields"
            name="psm_discord_custom_fields"
            value="<?php echo esc_attr( get_option( 'psm_discord_custom_fields', '' ) ); ?>"
            class="regular-text"
            placeholder="score, reputation, bounty" />
        <p class="description"><?php esc_html_e( 'Komma-getrennte Schluessel aus den Spielerdaten, die zusaetzlich angezeigt werden sollen.', 'psyerns-mods' ); ?></p>
    </td>
</tr>
```

**Ersetze diesen Block durch:**

```php
<tr class="psm-discord-field">
    <th>
        <label for="psm_discord_custom_fields"><?php esc_html_e( 'Eigene Felder', 'psyerns-mods' ); ?></label>
    </th>
    <td>
        <input type="text"
            id="psm_discord_custom_fields"
            name="psm_discord_custom_fields"
            value="<?php echo esc_attr( get_option( 'psm_discord_custom_fields', '' ) ); ?>"
            class="regular-text"
            placeholder="war_level, war_faction, hardline_reputation" />
        <p class="description">
            <?php esc_html_e( 'Komma-getrennte Feldnamen aus den Spielerdaten, die zusaetzlich im Discord-Embed angezeigt werden sollen.', 'psyerns-mods' ); ?>
        </p>
        <p class="description" style="margin-top: 6px;">
            <strong><?php esc_html_e( 'Verfuegbare Standard-Felder:', 'psyerns-mods' ); ?></strong><br>
            <code>steam_id</code> &mdash; <?php esc_html_e( 'Steam ID des Spielers', 'psyerns-mods' ); ?><br>
            <code>player_name</code> &mdash; <?php esc_html_e( 'Spielername', 'psyerns-mods' ); ?><br>
            <code>kills</code> &mdash; <?php esc_html_e( 'Gesamt-Kills (PvP)', 'psyerns-mods' ); ?><br>
            <code>deaths</code> &mdash; <?php esc_html_e( 'Gesamt-Tode', 'psyerns-mods' ); ?><br>
            <code>ai_kills</code> &mdash; <?php esc_html_e( 'KI-Kills (Zombies/AI)', 'psyerns-mods' ); ?><br>
            <code>pve_points</code> &mdash; <?php esc_html_e( 'PvE Punkte', 'psyerns-mods' ); ?><br>
            <code>pvp_points</code> &mdash; <?php esc_html_e( 'PvP Punkte', 'psyerns-mods' ); ?><br>
            <code>pve_deaths</code> &mdash; <?php esc_html_e( 'PvE Tode', 'psyerns-mods' ); ?><br>
            <code>pvp_deaths</code> &mdash; <?php esc_html_e( 'PvP Tode', 'psyerns-mods' ); ?><br>
            <code>longest_shot</code> &mdash; <?php esc_html_e( 'Laengster Kill in Metern', 'psyerns-mods' ); ?><br>
            <code>playtime</code> &mdash; <?php esc_html_e( 'Spielzeit in Stunden', 'psyerns-mods' ); ?><br>
            <code>is_online</code> &mdash; <?php esc_html_e( 'Online-Status (0/1)', 'psyerns-mods' ); ?><br>
            <code>last_login</code> &mdash; <?php esc_html_e( 'Letzter Login (Datum)', 'psyerns-mods' ); ?><br>
            <code>war_faction</code> &mdash; <?php esc_html_e( 'Kriegsfraktion (DayZ War Mod)', 'psyerns-mods' ); ?><br>
            <code>war_alignment</code> &mdash; <?php esc_html_e( 'Ausrichtung (Gut/Bose, Integer)', 'psyerns-mods' ); ?><br>
            <code>war_level</code> &mdash; <?php esc_html_e( 'Spieler-Level (War Mod)', 'psyerns-mods' ); ?><br>
            <code>war_boss_kills</code> &mdash; <?php esc_html_e( 'Boss-Kills (War Mod)', 'psyerns-mods' ); ?><br>
            <code>hardline_reputation</code> &mdash; <?php esc_html_e( 'Ruf-Punkte (Hardline Mod)', 'psyerns-mods' ); ?><br>
        </p>
        <p class="description" style="margin-top: 6px; color: #999;">
            <?php esc_html_e( 'Felder aus categoryKills, categoryDeaths und categoryLongestRanges sind JSON-Objekte und werden nicht unterstuetzt. Benutzerdefinierte Felder aus dem DayZ-Mod koennen hier eingetragen werden, wenn sie im Payload vorhanden sind.', 'psyerns-mods' ); ?>
        </p>
    </td>
</tr>
```

---

## FIX 2 — Bilder im Format 1:1 (quadratisch)

Es müssen **2 Dateien** geändert werden:

### FIX 2a — CSS: `aspect-ratio: 1/1` statt `16/9`

**Datei:** `wordpress-plugin/psyerns-mods/public/css/psyerns-mods-public.css`

**Problem:** `.psm-card__image-wrap` hat aktuell `padding-bottom: 56.25%` (16:9) und `aspect-ratio: 16 / 9`.

**Finde diesen exakten Block (ca. Zeile 142–160):**

```css
.psm-card__image-wrap {
	position: relative;
	overflow: hidden;
	width: 100%;
	padding-bottom: 56.25%;
}

@supports (aspect-ratio: 16 / 9) {
	.psm-card__image-wrap {
		padding-bottom: 0;
		aspect-ratio: 16 / 9;
	}
}
```

**Ersetze durch:**

```css
.psm-card__image-wrap {
	position: relative;
	overflow: hidden;
	width: 100%;
	padding-bottom: 100%; /* 1:1 Fallback für alte Browser */
}

@supports (aspect-ratio: 1 / 1) {
	.psm-card__image-wrap {
		padding-bottom: 0;
		aspect-ratio: 1 / 1;
	}
}
```

---

### FIX 2b — Admin-Vorschau: Thumbnail auch auf 1:1 umstellen

**Datei:** `wordpress-plugin/psyerns-mods/admin/class-psm-admin.php`

**Problem:** In der Admin-Spalten-Ansicht wird das Thumbnail mit `style="width:80px;height:45px;object-fit:cover;"` gerendert (16:9). Das sollte konsistent 1:1 sein.

**Finde diesen exakten Block (ca. Zeile 403–409):**

```php
				if ( is_array( $decoded ) && ! empty( $decoded['preview_url'] ) ) {
					printf(
						'<img src="%s" alt="%s" style="width:80px;height:45px;object-fit:cover;" />',
						esc_url( $decoded['preview_url'] ),
						esc_attr( isset( $decoded['title'] ) ? $decoded['title'] : '' )
					);
```

**Ersetze durch:**

```php
				if ( is_array( $decoded ) && ! empty( $decoded['preview_url'] ) ) {
					printf(
						'<img src="%s" alt="%s" style="width:60px;height:60px;object-fit:cover;border-radius:4px;" />',
						esc_url( $decoded['preview_url'] ),
						esc_attr( isset( $decoded['title'] ) ? $decoded['title'] : '' )
					);
```

---

## FIX 3 — BBCode in der Steam-Beschreibung bereinigen

Es müssen **2 Dateien** geändert werden:

### FIX 3a — Steam API: `strip_bbcode()` Methode hinzufügen und anwenden

**Datei:** `wordpress-plugin/psyerns-mods/includes/class-psm-steam-api.php`

**Problem:** In `fetch_from_api()` wird die Steam-Beschreibung mit `sanitize_text_field()` gespeichert, aber BBCode-Tags wie `[h1]`, `[img]`, `[b]`, `[url=...]` etc. bleiben erhalten und werden später roh als Text angezeigt.

**Schritt 1 — Finde diese Zeilen in `fetch_from_api()` (ca. Zeile 160–170):**

```php
		$title       = isset( $item['title'] ) ? sanitize_text_field( $item['title'] ) : '';
		$description = isset( $item['description'] ) ? sanitize_text_field( $item['description'] ) : '';
		$preview_url = isset( $item['preview_url'] ) ? esc_url_raw( $item['preview_url'] ) : '';

		/*
		 * Truncate description to 200 characters.
		 */
		if ( mb_strlen( $description ) > 200 ) {
			$description = mb_substr( $description, 0, 197 ) . '...';
		}

		return array(
			'title'       => $title,
			'description' => $description,
			'preview_url' => $preview_url,
			'workshop_id' => $workshop_id,
		);
```

**Ersetze durch:**

```php
		$title       = isset( $item['title'] ) ? sanitize_text_field( $item['title'] ) : '';
		$description = isset( $item['description'] ) ? self::strip_bbcode( $item['description'] ) : '';
		$preview_url = isset( $item['preview_url'] ) ? esc_url_raw( $item['preview_url'] ) : '';

		/*
		 * Truncate description to 200 characters after BBCode stripping.
		 */
		if ( mb_strlen( $description ) > 200 ) {
			$description = mb_substr( $description, 0, 197 ) . '...';
		}

		return array(
			'title'       => $title,
			'description' => $description,
			'preview_url' => $preview_url,
			'workshop_id' => $workshop_id,
		);
```

**Schritt 2 — Finde außerdem in `fetch_opengraph_fallback()` diese Zeile (ca. Zeile 225):**

```php
			$description = sanitize_text_field( html_entity_decode( $m[1], ENT_QUOTES, 'UTF-8' ) );

			if ( mb_strlen( $description ) > 200 ) {
				$description = mb_substr( $description, 0, 197 ) . '...';
			}
```

**Ersetze durch:**

```php
			$description = self::strip_bbcode( html_entity_decode( $m[1], ENT_QUOTES, 'UTF-8' ) );

			if ( mb_strlen( $description ) > 200 ) {
				$description = mb_substr( $description, 0, 197 ) . '...';
			}
```

**Schritt 3 — Füge die neue statische Methode `strip_bbcode()` am Ende der Klasse ein, direkt VOR der letzten schließenden `}` der Klasse `PSM_Steam_API`:**

```php
	/**
	 * Strip Steam BBCode tags from a description string.
	 *
	 * Steam Workshop descriptions use a BBCode dialect that must NOT be
	 * displayed as raw text. This method removes all known Steam BBCode tags
	 * and returns clean plain text suitable for card descriptions.
	 *
	 * Handled tags:
	 *   [img]...[/img]                    → removed completely (no image URLs in text)
	 *   [previewyoutube=...][/previewyoutube] → removed completely
	 *   [table]...[/table]                → removed completely
	 *   [url=...]Text[/url]               → link text only
	 *   [h1]–[h3], [b], [i], [u],
	 *   [strike], [quote], [code],
	 *   [noparse], [list]                 → tags removed, content kept
	 *   [*]                               → replaced with bullet "• "
	 *   [hr]                              → removed
	 *   All remaining unknown [tags]      → removed (safety catch-all)
	 *
	 * @param string $text Raw Steam description with BBCode.
	 * @return string Sanitized plain text.
	 */
	public static function strip_bbcode( $text ) {
		if ( empty( $text ) || ! is_string( $text ) ) {
			return '';
		}

		// Remove [img]...[/img] blocks entirely (including the URL inside).
		$text = preg_replace( '/\[img[^\]]*\].*?\[\/img\]/si', '', $text );

		// Remove [previewyoutube=...][/previewyoutube] entirely.
		$text = preg_replace( '/\[previewyoutube[^\]]*\].*?\[\/previewyoutube\]/si', '', $text );

		// Remove [table]...[/table] entirely.
		$text = preg_replace( '/\[table\].*?\[\/table\]/si', '', $text );

		// [url=...]Link text[/url] → keep only the link text.
		$text = preg_replace( '/\[url=[^\]]*\](.*?)\[\/url\]/si', '$1', $text );

		// [h1]...[/h3] → remove tags, keep content.
		$text = preg_replace( '/\[\/?h[1-6]\]/si', '', $text );

		// [b], [i], [u], [strike] → remove tags, keep content.
		$text = preg_replace( '/\[\/?(?:b|i|u|strike)\]/si', '', $text );

		// [quote]...[/quote], [code]...[/code], [noparse]...[/noparse] → keep content.
		$text = preg_replace( '/\[\/?(?:quote|code|noparse)\]/si', '', $text );

		// [list]...[/list] → remove tags, keep content.
		$text = preg_replace( '/\[\/?list\]/si', '', $text );

		// [*] list item → bullet point.
		$text = preg_replace( '/\[\*\]/', "\n\u{2022} ", $text );

		// [hr] → remove.
		$text = preg_replace( '/\[hr\]/si', '', $text );

		// Safety catch-all: remove any remaining [tag] or [tag=...] that are ≤40 chars.
		$text = preg_replace( '/\[[^\]]{1,40}\]/', '', $text );

		// Collapse excessive whitespace/newlines to maximum 2 consecutive newlines.
		$text = preg_replace( '/\n{3,}/', "\n\n", $text );
		$text = preg_replace( '/[ \t]{2,}/', ' ', $text );

		return sanitize_text_field( trim( $text ) );
	}
```

---

### FIX 3b — Cache leeren nach Code-Änderung

**Wichtig:** Da die BBCode-Bereinigung erst jetzt hinzugefügt wird, sind bereits gecachte Steam-Beschreibungen noch im alten Format gespeichert (mit BBCode-Tags). Der Cache muss nach dem Deployment geleert werden.

**Weise den User darauf hin:** Nach dem Deployment unter **Einstellungen → Psyerns Mods → Steam-Cache leeren** klicken, damit alle Items mit der neuen Methode neu geladen werden.

Es ist **kein weiterer Code-Fix nötig** in `class-psm-shortcode.php`, da `get_description()` bereits `wp_strip_all_tags()` aufruft — dies entfernt HTML-Tags, aber keine BBCode. Nach Fix 3a werden die Daten bereits sauber im Cache gespeichert.

---

## ZUSAMMENFASSUNG DER ÄNDERUNGEN

| # | Datei | Änderung |
|---|-------|----------|
| 1 | `psyerns-mods/admin/class-psm-admin.php` | "Eigene Felder" Beschreibungstext mit allen verfügbaren Feldnamen aus der Datenbank |
| 2a | `psyerns-mods/public/css/psyerns-mods-public.css` | `aspect-ratio: 16/9` → `aspect-ratio: 1/1`, `padding-bottom: 56.25%` → `100%` |
| 2b | `psyerns-mods/admin/class-psm-admin.php` | Admin-Thumbnail `height:45px` → `height:60px; width:60px` (quadratisch) |
| 3a | `psyerns-mods/includes/class-psm-steam-api.php` | `strip_bbcode()` Methode hinzufügen + in `fetch_from_api()` und `fetch_opengraph_fallback()` anwenden statt `sanitize_text_field()` |

**Keine weiteren Dateien anfassen.**  
**Nach Deployment:** Steam-Cache unter Einstellungen → Psyerns Mods leeren.

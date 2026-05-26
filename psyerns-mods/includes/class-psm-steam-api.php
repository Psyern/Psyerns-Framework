<?php
/**
 * Steam Workshop API integration for Psyerns Mods.
 *
 * Fetches published file details from the Steam Web API
 * with an OpenGraph fallback for when the API is unavailable.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Handles communication with the Steam Workshop API.
 */
class PSM_Steam_API {

	/**
	 * Steam Web API endpoint for published file details.
	 *
	 * @var string
	 */
	const API_URL = 'https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/';

	/**
	 * Extract the numeric Workshop ID from a Steam URL.
	 *
	 * @param string $url Full Steam Workshop URL.
	 * @return string|false The workshop ID string, or false on failure.
	 */
	public static function extract_workshop_id( $url ) {
		if ( preg_match( '/[?&]id=(\d+)/', $url, $matches ) ) {
			return $matches[1];
		}

		return false;
	}

	/**
	 * Get item data for a Steam Workshop URL.
	 *
	 * Checks the transient cache first, then tries the Steam API,
	 * and falls back to OpenGraph scraping if the API call fails.
	 *
	 * @param string $steam_url Full Steam Workshop URL.
	 * @return array|WP_Error {
	 *     @type string $title       Item title.
	 *     @type string $description Truncated description (max 200 chars).
	 *     @type string $preview_url Preview image URL.
	 *     @type string $workshop_id Numeric workshop ID.
	 * }
	 */
	public static function get_item_data( $steam_url ) {
		$workshop_id = self::extract_workshop_id( $steam_url );

		if ( false === $workshop_id ) {
			return new WP_Error( 'psm_invalid_url', __( 'Ungültige Steam Workshop URL.', 'psyerns-mods' ) );
		}

		/*
		 * Check cache first.
		 */
		$cached = PSM_Cache::get( 'steam_' . $workshop_id );

		if ( false !== $cached && is_array( $cached ) ) {
			return $cached;
		}

		/*
		 * Try the Steam API.
		 */
		$data = self::fetch_from_api( $workshop_id );

		if ( is_wp_error( $data ) ) {
			/*
			 * Fallback: scrape OpenGraph tags from the Workshop page.
			 */
			$data = self::fetch_opengraph_fallback( $steam_url );
		}

		if ( is_wp_error( $data ) ) {
			return $data;
		}

		/*
		 * Ensure workshop_id is present in the result.
		 */
		$data['workshop_id'] = $workshop_id;

		/*
		 * Cache the result.
		 */
		PSM_Cache::set( 'steam_' . $workshop_id, $data );

		return $data;
	}

	/**
	 * Fetch item details from the Steam Web API.
	 *
	 * @param string $workshop_id Numeric Workshop item ID.
	 * @return array|WP_Error Associative array with title, description, preview_url on success.
	 */
	public static function fetch_from_api( $workshop_id ) {
		$body = array(
			'itemcount'              => 1,
			'publishedfileids[0]'    => $workshop_id,
		);

		/*
		 * Optionally include the Steam API key if configured.
		 */
		$api_key = get_option( 'psm_steam_api_key', '' );

		if ( '' !== $api_key ) {
			$body['key'] = sanitize_text_field( $api_key );
		}

		$response = wp_remote_post(
			self::API_URL,
			array(
				'timeout' => 15,
				'body'    => $body,
			)
		);

		if ( is_wp_error( $response ) ) {
			return $response;
		}

		$code = wp_remote_retrieve_response_code( $response );

		if ( 200 !== (int) $code ) {
			return new WP_Error(
				'psm_steam_api_http',
				/* translators: %d: HTTP status code */
				sprintf( __( 'Steam API hat HTTP %d zurückgegeben.', 'psyerns-mods' ), $code )
			);
		}

		$json = json_decode( wp_remote_retrieve_body( $response ), true );

		if ( ! is_array( $json ) ) {
			return new WP_Error( 'psm_steam_api_parse', __( 'Steam API Antwort konnte nicht geparst werden.', 'psyerns-mods' ) );
		}

		/*
		 * Navigate the response structure.
		 */
		if ( ! isset( $json['response']['publishedfiledetails'][0] ) ) {
			return new WP_Error( 'psm_steam_api_empty', __( 'Keine Daten vom Steam API erhalten.', 'psyerns-mods' ) );
		}

		$item = $json['response']['publishedfiledetails'][0];

		if ( isset( $item['result'] ) && 1 !== (int) $item['result'] ) {
			return new WP_Error( 'psm_steam_api_result', __( 'Steam API: Item nicht gefunden oder nicht öffentlich.', 'psyerns-mods' ) );
		}

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
	}

	/**
	 * Fallback: scrape OpenGraph meta tags from the Workshop page.
	 *
	 * @param string $url Full Steam Workshop URL.
	 * @return array|WP_Error Associative array with title, description, preview_url on success.
	 */
	public static function fetch_opengraph_fallback( $url ) {
		$response = wp_remote_get(
			$url,
			array(
				'timeout'    => 15,
				'user-agent' => 'Mozilla/5.0 (compatible; PSM Bot/1.0)',
			)
		);

		if ( is_wp_error( $response ) ) {
			return $response;
		}

		$code = wp_remote_retrieve_response_code( $response );

		if ( 200 !== (int) $code ) {
			return new WP_Error(
				'psm_og_http',
				/* translators: %d: HTTP status code */
				sprintf( __( 'Steam-Seite hat HTTP %d zurückgegeben.', 'psyerns-mods' ), $code )
			);
		}

		$html = wp_remote_retrieve_body( $response );

		if ( empty( $html ) ) {
			return new WP_Error( 'psm_og_empty', __( 'Leere Antwort von Steam erhalten.', 'psyerns-mods' ) );
		}

		$title       = '';
		$description = '';
		$preview_url = '';

		/*
		 * Extract og:title.
		 */
		if ( preg_match( '/<meta\s+property=["\']og:title["\']\s+content=["\']([^"\']+)["\']/i', $html, $m ) ) {
			$title = sanitize_text_field( html_entity_decode( $m[1], ENT_QUOTES, 'UTF-8' ) );
		}

		/*
		 * Extract og:description.
		 */
		if ( preg_match( '/<meta\s+property=["\']og:description["\']\s+content=["\']([^"\']+)["\']/i', $html, $m ) ) {
			$description = self::strip_bbcode( html_entity_decode( $m[1], ENT_QUOTES, 'UTF-8' ) );

			if ( mb_strlen( $description ) > 200 ) {
				$description = mb_substr( $description, 0, 197 ) . '...';
			}
		}

		/*
		 * Extract og:image.
		 */
		if ( preg_match( '/<meta\s+property=["\']og:image["\']\s+content=["\']([^"\']+)["\']/i', $html, $m ) ) {
			$preview_url = esc_url_raw( $m[1] );
		}

		if ( '' === $title && '' === $preview_url ) {
			return new WP_Error( 'psm_og_no_data', __( 'Keine OpenGraph-Daten auf der Steam-Seite gefunden.', 'psyerns-mods' ) );
		}

		return array(
			'title'       => $title,
			'description' => $description,
			'preview_url' => $preview_url,
			'workshop_id' => (string) self::extract_workshop_id( $url ),
		);
	}

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
	 *   [*]                               → replaced with bullet
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

		$text = preg_replace( '/\[img[^\]]*\].*?\[\/img\]/si', '', $text );
		$text = preg_replace( '/\[previewyoutube[^\]]*\].*?\[\/previewyoutube\]/si', '', $text );
		$text = preg_replace( '/\[table\].*?\[\/table\]/si', '', $text );
		$text = preg_replace( '/\[url=[^\]]*\](.*?)\[\/url\]/si', '$1', $text );
		$text = preg_replace( '/\[\/?h[1-6]\]/si', '', $text );
		$text = preg_replace( '/\[\/?(?:b|i|u|strike)\]/si', '', $text );
		$text = preg_replace( '/\[\/?(?:quote|code|noparse)\]/si', '', $text );
		$text = preg_replace( '/\[\/?list\]/si', '', $text );
		$text = preg_replace( '/\[\*\]/', "\n\xE2\x80\xA2 ", $text );
		$text = preg_replace( '/\[hr\]/si', '', $text );
		$text = preg_replace( '/\[[^\]]{1,40}\]/', '', $text );
		$text = preg_replace( '/\n{3,}/', "\n\n", $text );
		$text = preg_replace( '/[ \t]{2,}/', ' ', $text );

		return sanitize_text_field( trim( $text ) );
	}
}

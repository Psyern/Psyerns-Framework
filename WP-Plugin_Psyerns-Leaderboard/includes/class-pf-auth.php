<?php
/**
 * API key authentication for REST endpoints.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Auth
 *
 * Validates the API key for private REST routes. The key may be presented via
 * the `X-API-Key` header, an `Authorization: Bearer <key>` header, or the
 * legacy `api_key` query/body parameter (kept for DayZ-server compatibility —
 * the Enforce-Script RestContext cannot set arbitrary headers). All candidate
 * keys are compared with hash_equals() to avoid timing side channels, matching
 * the Psyerns_AuctionHouse auth implementation.
 */
class PF_Auth {

	/**
	 * Validate API key from a REST request.
	 *
	 * The header transport (preferred) and the legacy query/body parameter are
	 * both evaluated without short-circuiting: each comparison runs against the
	 * same stored key via hash_equals(), so response timing does not reveal
	 * which transport matched.
	 *
	 * @param WP_REST_Request $request The incoming REST request.
	 * @return true|WP_Error True if valid, WP_Error otherwise.
	 */
	public static function validate_api_key( WP_REST_Request $request ) {
		$stored = (string) get_option( 'pf_api_key', '' );

		if ( '' === $stored ) {
			return self::unauthorized();
		}

		$header_key = self::extract_header_key( $request );
		$query_key  = self::extract_query_key( $request );

		$header_ok = ( '' !== $header_key ) && hash_equals( $stored, $header_key );
		$query_ok  = ( '' !== $query_key ) && hash_equals( $stored, $query_key );

		if ( $header_ok || $query_ok ) {
			return true;
		}

		return self::unauthorized();
	}

	/**
	 * Extract the API key from the request headers.
	 *
	 * Prefers `X-API-Key`, falls back to `Authorization: Bearer <key>`.
	 *
	 * @param WP_REST_Request $request Incoming request.
	 * @return string The presented key, or '' when absent.
	 */
	protected static function extract_header_key( WP_REST_Request $request ) {
		$x_api_key = $request->get_header( 'x_api_key' );
		if ( ! empty( $x_api_key ) ) {
			return sanitize_text_field( trim( $x_api_key ) );
		}

		$auth = $request->get_header( 'authorization' );
		if ( ! empty( $auth ) ) {
			$auth = trim( $auth );
			if ( 0 === stripos( $auth, 'Bearer ' ) ) {
				return sanitize_text_field( substr( $auth, 7 ) );
			}
		}

		return '';
	}

	/**
	 * Extract the legacy `api_key` query/body parameter, or '' when absent.
	 *
	 * @param WP_REST_Request $request Incoming request.
	 * @return string
	 */
	protected static function extract_query_key( WP_REST_Request $request ) {
		$value = $request->get_param( 'api_key' );

		if ( null === $value || '' === $value ) {
			return '';
		}

		return sanitize_text_field( (string) $value );
	}

	/**
	 * Build the standard 401 error returned when authentication fails.
	 *
	 * @return WP_Error
	 */
	protected static function unauthorized() {
		return new WP_Error(
			'unauthorized',
			__( 'Invalid API key', 'psyerns-framework' ),
			array( 'status' => 401 )
		);
	}
}

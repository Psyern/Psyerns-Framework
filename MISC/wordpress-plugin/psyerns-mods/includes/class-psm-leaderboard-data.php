<?php
/**
 * Leaderboard data receiver and storage.
 *
 * Registers REST endpoints for receiving leaderboard data from the
 * DayZ server and for external polling by third-party scripts.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Handles leaderboard data ingestion, storage and retrieval.
 */
class PSM_Leaderboard_Data {

	/**
	 * REST API namespace.
	 *
	 * @var string
	 */
	const REST_NAMESPACE = 'psyern/v1';

	/**
	 * Option key for stored leaderboard data.
	 *
	 * @var string
	 */
	const OPTION_DATA = 'psm_leaderboard_data';

	/**
	 * Option key for the last-updated timestamp.
	 *
	 * @var string
	 */
	const OPTION_UPDATED_AT = 'psm_leaderboard_updated_at';

	/**
	 * Register hooks.
	 *
	 * @return void
	 */
	public static function register() {
		add_action( 'rest_api_init', array( __CLASS__, 'register_routes' ) );
	}

	/**
	 * Register REST API routes.
	 *
	 * @return void
	 */
	public static function register_routes() {
		/*
		 * POST /wp-json/psyern/v1/upload
		 * Receives leaderboard data from the DayZ server.
		 */
		register_rest_route(
			self::REST_NAMESPACE,
			'/upload',
			array(
				'methods'             => 'POST',
				'callback'            => array( __CLASS__, 'handle_upload' ),
				'permission_callback' => array( __CLASS__, 'check_upload_permission' ),
			)
		);

		/*
		 * GET /wp-json/psyern/v1/leaderboard
		 * Returns the last stored leaderboard data for external polling.
		 */
		register_rest_route(
			self::REST_NAMESPACE,
			'/leaderboard',
			array(
				'methods'             => 'GET',
				'callback'            => array( __CLASS__, 'handle_get_leaderboard' ),
				'permission_callback' => array( __CLASS__, 'check_read_permission' ),
			)
		);
	}

	/**
	 * Permission check for the upload endpoint.
	 *
	 * Validates the API key from the request header or query parameter.
	 *
	 * @param WP_REST_Request $request The incoming request.
	 * @return bool|WP_Error True if authorized, WP_Error otherwise.
	 */
	public static function check_upload_permission( $request ) {
		$configured_key = get_option( 'psm_steam_api_key', '' );

		/*
		 * If no API key is configured, reject all uploads for safety.
		 */
		if ( '' === $configured_key ) {
			return new WP_Error(
				'psm_no_api_key',
				__( 'Kein API-Key konfiguriert. Bitte zuerst unter Einstellungen > Psyerns Mods einen API-Key setzen.', 'psyerns-mods' ),
				array( 'status' => 403 )
			);
		}

		$provided_key = $request->get_header( 'X-Psyern-Api-Key' );

		if ( empty( $provided_key ) ) {
			$provided_key = $request->get_param( 'api_key' );
		}

		if ( empty( $provided_key ) || ! hash_equals( $configured_key, $provided_key ) ) {
			return new WP_Error(
				'psm_unauthorized',
				__( 'Ungueltiger API-Key.', 'psyerns-mods' ),
				array( 'status' => 401 )
			);
		}

		return true;
	}

	/**
	 * Permission check for the GET leaderboard endpoint.
	 *
	 * Public by default. If an API key is configured and the option
	 * psm_leaderboard_public is not enabled, require the key.
	 *
	 * @param WP_REST_Request $request The incoming request.
	 * @return bool True if authorized.
	 */
	public static function check_read_permission( $request ) {
		$is_public = get_option( 'psm_leaderboard_public', '1' );

		if ( '1' === $is_public ) {
			return true;
		}

		return self::check_upload_permission( $request );
	}

	/**
	 * Handle incoming leaderboard data upload.
	 *
	 * Stores the data and fires the psm_leaderboard_updated action
	 * so other components (Discord push) can react.
	 *
	 * @param WP_REST_Request $request The incoming request.
	 * @return WP_REST_Response Response object.
	 */
	public static function handle_upload( $request ) {
		$body = $request->get_json_params();

		if ( empty( $body ) || ! is_array( $body ) ) {
			return new WP_REST_Response(
				array(
					'success' => false,
					'message' => __( 'Leerer oder ungueltiger JSON-Body.', 'psyerns-mods' ),
				),
				400
			);
		}

		/*
		 * Sanitize the incoming data recursively.
		 */
		$sanitized = self::sanitize_data( $body );
		$timestamp = current_time( 'mysql', true );

		update_option( self::OPTION_DATA, wp_json_encode( $sanitized ), false );
		update_option( self::OPTION_UPDATED_AT, $timestamp, false );

		/**
		 * Fires when new leaderboard data has been stored.
		 *
		 * @param array  $sanitized The sanitized leaderboard data.
		 * @param string $timestamp UTC timestamp of the update.
		 */
		do_action( 'psm_leaderboard_updated', $sanitized, $timestamp );

		return new WP_REST_Response(
			array(
				'success'    => true,
				'message'    => __( 'Leaderboard-Daten gespeichert.', 'psyerns-mods' ),
				'updated_at' => $timestamp,
			),
			200
		);
	}

	/**
	 * Handle GET request for leaderboard data (external polling).
	 *
	 * @param WP_REST_Request $request The incoming request.
	 * @return WP_REST_Response Response object.
	 */
	public static function handle_get_leaderboard( $request ) {
		$raw_data   = get_option( self::OPTION_DATA, '' );
		$updated_at = get_option( self::OPTION_UPDATED_AT, '' );

		if ( '' === $raw_data ) {
			return new WP_REST_Response(
				array(
					'success' => false,
					'message' => __( 'Noch keine Leaderboard-Daten vorhanden.', 'psyerns-mods' ),
				),
				404
			);
		}

		$data = json_decode( $raw_data, true );

		if ( ! is_array( $data ) ) {
			return new WP_REST_Response(
				array(
					'success' => false,
					'message' => __( 'Fehlerhafte gespeicherte Daten.', 'psyerns-mods' ),
				),
				500
			);
		}

		return new WP_REST_Response(
			array(
				'success'    => true,
				'updated_at' => $updated_at,
				'data'       => $data,
			),
			200
		);
	}

	/**
	 * Get the stored leaderboard data as an array.
	 *
	 * @return array|null Decoded data or null if none stored.
	 */
	public static function get_data() {
		$raw = get_option( self::OPTION_DATA, '' );

		if ( '' === $raw ) {
			return null;
		}

		$data = json_decode( $raw, true );

		return is_array( $data ) ? $data : null;
	}

	/**
	 * Get the last-updated timestamp.
	 *
	 * @return string UTC datetime string or empty.
	 */
	public static function get_updated_at() {
		return get_option( self::OPTION_UPDATED_AT, '' );
	}

	/**
	 * Recursively sanitize incoming data.
	 *
	 * Strings are sanitized, numbers and booleans are cast,
	 * arrays are processed recursively.
	 *
	 * @param mixed $data The data to sanitize.
	 * @return mixed Sanitized data.
	 */
	private static function sanitize_data( $data ) {
		if ( is_array( $data ) ) {
			$clean = array();
			foreach ( $data as $key => $value ) {
				$clean_key           = sanitize_text_field( (string) $key );
				$clean[ $clean_key ] = self::sanitize_data( $value );
			}
			return $clean;
		}

		if ( is_string( $data ) ) {
			return sanitize_text_field( $data );
		}

		if ( is_int( $data ) || is_float( $data ) ) {
			return $data;
		}

		if ( is_bool( $data ) ) {
			return $data;
		}

		return '';
	}
}

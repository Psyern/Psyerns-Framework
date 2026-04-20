<?php
/**
 * REST API route registration.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Api
 *
 * Registers all REST routes under the psyern/v1 namespace.
 * Private endpoints require API key validation via PF_Auth.
 * Public endpoints under /public/ are open.
 */
class PF_Api {

	/**
	 * REST namespace.
	 *
	 * @var string
	 */
	const NS = 'psyern/v1';

	/**
	 * Register all REST routes.
	 *
	 * @return void
	 */
	public function register_routes() {
		// Private: Server Status.
		register_rest_route( self::NS, '/server/status', array(
			'methods'             => 'POST',
			'callback'            => array( new PF_Server_Status(), 'handle_post' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Leaderboard Upload.
		register_rest_route( self::NS, '/upload', array(
			'methods'             => 'POST',
			'callback'            => array( new PF_Leaderboard(), 'handle_upload' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Whitelist Check.
		register_rest_route( self::NS, '/whitelist/check', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Whitelist(), 'handle_check' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Whitelist Add.
		register_rest_route( self::NS, '/whitelist/add', array(
			'methods'             => 'POST',
			'callback'            => array( new PF_Whitelist(), 'handle_add' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Whitelist Remove.
		register_rest_route( self::NS, '/whitelist/remove', array(
			'methods'             => 'POST',
			'callback'            => array( new PF_Whitelist(), 'handle_remove' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Player Lookup.
		register_rest_route( self::NS, '/players/lookup', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Players(), 'handle_lookup' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Online Players.
		register_rest_route( self::NS, '/players/online', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Players(), 'handle_online' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Private: Ping.
		register_rest_route( self::NS, '/ping', array(
			'methods'             => 'GET',
			'callback'            => array( $this, 'handle_ping' ),
			'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
		) );

		// Public: Leaderboard.
		register_rest_route( self::NS, '/public/leaderboard', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Leaderboard(), 'handle_get_public' ),
			'permission_callback' => '__return_true',
		) );

		// Public: Server Status.
		register_rest_route( self::NS, '/public/status', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Server_Status(), 'handle_get_public' ),
			'permission_callback' => '__return_true',
		) );

		// Public: Top 3.
		register_rest_route( self::NS, '/public/top3', array(
			'methods'             => 'GET',
			'callback'            => array( new PF_Leaderboard(), 'handle_top3' ),
			'permission_callback' => '__return_true',
		) );

		// Public: Avatar.
		register_rest_route( self::NS, '/public/avatar', array(
			'methods'             => 'GET',
			'callback'            => array( $this, 'handle_avatar' ),
			'permission_callback' => '__return_true',
		) );
	}

	/**
	 * Handle ping request.
	 *
	 * @return WP_REST_Response
	 */
	public function handle_ping() {
		return new WP_REST_Response( array( 'status' => 'ok' ), 200 );
	}

	/**
	 * Handle avatar lookup request.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_avatar( WP_REST_Request $request ) {
		$steam_id = sanitize_text_field( $request->get_param( 'steam_id' ) );
		$url      = PF_Steam::get_avatar( $steam_id );

		return new WP_REST_Response( array( 'avatar_url' => $url ), 200 );
	}
}

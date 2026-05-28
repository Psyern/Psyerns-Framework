<?php
/**
 * Leaderboard REST API endpoints.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class Psyern_Api
 *
 * Registers leaderboard-specific REST routes under psyern/v1.
 */
class Psyern_Api {

	/**
	 * Register leaderboard routes.
	 *
	 * @return void
	 */
	public function register_routes() {
		register_rest_route( 'psyern/v1', '/leaderboard', array(
			array(
				'methods'             => 'GET',
				'callback'            => array( $this, 'get_leaderboard' ),
				'permission_callback' => '__return_true',
				'args'                => array(
					'mode'  => array(
						'default'           => 'pvp',
						'sanitize_callback' => 'sanitize_text_field',
					),
					'limit' => array(
						'default'           => 20,
						'sanitize_callback' => 'absint',
					),
				),
			),
			array(
				'methods'             => 'POST',
				'callback'            => array( $this, 'post_leaderboard' ),
				'permission_callback' => array( 'PF_Auth', 'validate_api_key' ),
			),
		) );

		register_rest_route( 'psyern/v1', '/leaderboard/top3', array(
			'methods'             => 'GET',
			'callback'            => array( $this, 'get_top3' ),
			'permission_callback' => '__return_true',
			'args'                => array(
				'mode' => array(
					'default'           => 'pvp',
					'sanitize_callback' => 'sanitize_text_field',
				),
			),
		) );
	}

	/**
	 * GET /leaderboard — public leaderboard data.
	 *
	 * @param WP_REST_Request $request Request object.
	 * @return WP_REST_Response
	 */
	public function get_leaderboard( WP_REST_Request $request ) {
		$mode  = $request->get_param( 'mode' );
		$limit = min( $request->get_param( 'limit' ), 100 );
		$rows  = Psyern_Database::get_leaderboard( $mode, $limit );

		return new WP_REST_Response( array(
			'mode'    => $mode,
			'count'   => count( $rows ),
			'players' => $rows,
		), 200 );
	}

	/**
	 * POST /leaderboard — receive player data from DayZ server.
	 *
	 * @param WP_REST_Request $request Request object.
	 * @return WP_REST_Response
	 */
	public function post_leaderboard( WP_REST_Request $request ) {
		$data    = $request->get_json_params();
		$players = $data['players'] ?? array();

		if ( empty( $players ) ) {
			return new WP_REST_Response(
				array( 'success' => false, 'message' => __( 'No players data provided', 'psyerns-framework' ) ),
				400
			);
		}

		$resolve_avatars = ( '1' === get_option( 'psyern_enable_avatars', '1' ) );

		foreach ( $players as $player ) {
			if ( $resolve_avatars && empty( $player['avatarUrl'] ) ) {
				$sid = sanitize_text_field( $player['steamId'] ?? '' );
				if ( ! empty( $sid ) ) {
					$player['avatarUrl'] = PF_Steam::get_avatar( $sid );
				}
			}
			Psyern_Database::upsert_player( $player );
		}

		return new WP_REST_Response( array( 'success' => true, 'count' => count( $players ) ), 200 );
	}

	/**
	 * GET /leaderboard/top3 — top 3 of the current week.
	 *
	 * @param WP_REST_Request $request Request object.
	 * @return WP_REST_Response
	 */
	public function get_top3( WP_REST_Request $request ) {
		$mode = $request->get_param( 'mode' );
		$rows = Psyern_Database::get_top3( $mode );

		return new WP_REST_Response( array(
			'mode'        => $mode,
			'week_number' => absint( gmdate( 'W' ) ),
			'year'        => absint( gmdate( 'Y' ) ),
			'players'     => $rows,
		), 200 );
	}
}

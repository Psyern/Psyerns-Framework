<?php
/**
 * Player lookup and online players endpoint handlers.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Players
 *
 * Provides player data from the leaderboard table.
 */
class PF_Players {

	/**
	 * Handle player lookup by steam_id.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_lookup( WP_REST_Request $request ) {
		global $wpdb;
		$steam_id = sanitize_text_field( $request->get_param( 'steam_id' ) );
		$table    = PF_Database::get_table_name( 'leaderboard' );

		$row = $wpdb->get_row( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE steam_id = %s LIMIT 1",
			$steam_id
		), ARRAY_A );

		if ( null === $row ) {
			return new WP_REST_Response(
				array( 'error' => __( 'Player not found', 'psyerns-framework' ) ),
				404
			);
		}

		return new WP_REST_Response( array(
			'steamId' => $row['steam_id'],
			'name'    => $row['player_name'],
			'online'  => ( 1 === (int) $row['is_online'] ),
			'posX'    => 0.0,
			'posY'    => 0.0,
			'posZ'    => 0.0,
			'health'  => 0.0,
		), 200 );
	}

	/**
	 * Handle online players list.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_online( WP_REST_Request $request ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'leaderboard' );

		$rows = $wpdb->get_results(
			"SELECT DISTINCT steam_id, player_name FROM {$table} WHERE is_online = 1",
			ARRAY_A
		);

		$players = array();
		foreach ( $rows as $row ) {
			$players[] = array(
				'steamId' => $row['steam_id'],
				'name'    => $row['player_name'],
				'online'  => true,
				'posX'    => 0.0,
				'posY'    => 0.0,
				'posZ'    => 0.0,
				'health'  => 0.0,
			);
		}

		return new WP_REST_Response( $players, 200 );
	}
}

<?php
/**
 * Main bootstrapper for the leaderboard system.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class Psyern_Main
 *
 * Initializes all leaderboard hooks. No hooks in the constructor.
 */
class Psyern_Main {

	/**
	 * Register all hooks.
	 *
	 * @return void
	 */
	public function init() {
		// REST API.
		add_action( 'rest_api_init', array( $this, 'register_api' ) );

		// Shortcodes.
		add_action( 'init', array( $this, 'register_shortcodes' ) );

		// Admin.
		if ( is_admin() ) {
			$admin = new Psyern_Admin();
			$admin->init();
		}

		// AJAX handlers (logged in + public).
		add_action( 'wp_ajax_psyern_get_leaderboard', array( $this, 'ajax_get_leaderboard' ) );
		add_action( 'wp_ajax_nopriv_psyern_get_leaderboard', array( $this, 'ajax_get_leaderboard' ) );
	}

	/**
	 * Register leaderboard REST routes.
	 *
	 * @return void
	 */
	public function register_api() {
		$api = new Psyern_Api();
		$api->register_routes();
	}

	/**
	 * Register shortcodes.
	 *
	 * @return void
	 */
	public function register_shortcodes() {
		$sc = new Psyern_Shortcode();
		$sc->register();
	}

	/**
	 * AJAX handler for leaderboard data (mode switch without reload).
	 *
	 * @return void
	 */
	public function ajax_get_leaderboard() {
		check_ajax_referer( 'psyern_leaderboard_nonce', 'nonce' );

		global $wpdb;

		$mode     = sanitize_text_field( wp_unslash( $_GET['mode'] ?? 'pvp' ) );
		$per_page = min( max( absint( $_GET['per_page'] ?? 20 ), 1 ), 100 );
		$page     = max( absint( $_GET['page'] ?? 1 ), 1 );
		$search   = sanitize_text_field( wp_unslash( $_GET['search'] ?? '' ) );
		$offset   = ( $page - 1 ) * $per_page;

		$table      = PF_Database::get_table_name( 'leaderboard' );
		$order_col  = ( 'pvp' === $mode ) ? 'pvp_points' : 'pve_points';
		$board_type = $mode; // 'pvp' or 'pve'

		if ( ! empty( $search ) ) {
			$like    = '%' . $wpdb->esc_like( $search ) . '%';
			$players = $wpdb->get_results( $wpdb->prepare(
				"SELECT * FROM {$table} WHERE board_type = %s AND player_name LIKE %s ORDER BY {$order_col} DESC LIMIT %d OFFSET %d",
				$board_type, $like, $per_page, $offset
			), ARRAY_A );
			$total   = (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE board_type = %s AND player_name LIKE %s",
				$board_type, $like
			) );
		} else {
			$players = $wpdb->get_results( $wpdb->prepare(
				"SELECT * FROM {$table} WHERE board_type = %s ORDER BY {$order_col} DESC LIMIT %d OFFSET %d",
				$board_type, $per_page, $offset
			), ARRAY_A );
			$total   = (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE board_type = %s",
				$board_type
			) );
		}

		// Format rows to match the shape the frontend expects.
		$formatted = array();
		foreach ( $players as $row ) {
			$formatted[] = array(
				'steam_id'            => $row['steam_id'],
				'player_name'         => $row['player_name'],
				'kills'               => (int) $row['kills'],
				'deaths'              => (int) $row['deaths'],
				'kd_ratio'            => $row['deaths'] > 0 ? round( $row['kills'] / $row['deaths'], 2 ) : (float) $row['kills'],
				'playtime_seconds'    => (int) round( (float) $row['playtime'] ),
				'score'               => ( 'pvp' === $mode ) ? (int) $row['pvp_points'] : (int) $row['pve_points'],
				'avatar_url'          => PF_Steam::get_avatar( $row['steam_id'] ),
				'war_faction'         => $row['war_faction'] ?? '',
				'war_level'           => (int) ( $row['war_level'] ?? 0 ),
				'war_boss_kills'      => (int) ( $row['war_boss_kills'] ?? 0 ),
				'hardline_reputation' => (int) ( $row['hardline_reputation'] ?? 0 ),
			);
		}

		$meta        = get_transient( 'pf_leaderboard_meta' ) ?: array();
		$total_pages = ( $per_page > 0 ) ? (int) ceil( $total / $per_page ) : 1;

		wp_send_json_success( array(
			'mode'                => $mode,
			'players'             => $formatted,
			'total'               => $total,
			'total_pages'         => $total_pages,
			'page'                => $page,
			'per_page'            => $per_page,
			'globalEastPoints'    => absint( $meta['globalEastPoints'] ?? 0 ),
			'globalWestPoints'    => absint( $meta['globalWestPoints'] ?? 0 ),
			'playerOnlineCounter' => absint( $meta['playerOnlineCounter'] ?? 0 ),
		) );
	}
}

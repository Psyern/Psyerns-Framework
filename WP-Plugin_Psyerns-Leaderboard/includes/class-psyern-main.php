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
		$sort_by  = sanitize_key( wp_unslash( $_GET['sort_by'] ?? '' ) );
		$sort_dir = strtolower( sanitize_key( wp_unslash( $_GET['sort_dir'] ?? 'desc' ) ) );
		$offset   = ( $page - 1 ) * $per_page;

		$table      = PF_Database::get_table_name( 'leaderboard' );
		// Default ranking is by kills (per-mode count already stored in this
		// column by upsert_players). The kills DESC default also drives the
		// `original_rank` subquery below, so the leftmost "#" column reflects
		// the kills-based placement even when the user re-sorts by another
		// column.
		$order_col  = 'kills';
		$board_type = $mode; // 'pvp' or 'pve'

		// Whitelist-based sort: frontend key -> SQL ORDER BY expression.
		// Hard-coded values only, no user input ever reaches the SQL string,
		// so this stays safe even though the clause is concatenated rather
		// than prepared (wpdb::prepare can't bind identifiers).
		$sort_map = array(
			'name'             => 'player_name',
			'faction'          => 'war_level',
			'kills'            => 'kills',
			'deaths'           => 'deaths',
			'kd'               => 'CASE WHEN deaths > 0 THEN (kills / deaths) ELSE kills END',
			'boss'             => 'war_boss_kills',
			'reputation'       => 'hardline_reputation',
			'headshots'        => 'headshots',
			'accuracy'         => 'CASE WHEN shots_fired > 0 THEN (shots_hit / shots_fired) ELSE 0 END',
			'longest_shot'     => 'longest_shot',
			'distance'         => 'distance_travelled',
			'distance_foot'    => 'distance_on_foot',
			'distance_vehicle' => 'distance_in_vehicle',
			'playtime'         => 'playtime',
		);
		if ( ! in_array( $sort_dir, array( 'asc', 'desc' ), true ) ) {
			$sort_dir = 'desc';
		}
		if ( isset( $sort_map[ $sort_by ] ) ) {
			$order_clause = $sort_map[ $sort_by ] . ' ' . strtoupper( $sort_dir ) . ', kills DESC, deaths ASC, player_name ASC';
		} else {
			// Default order: most kills first, fewer deaths breaks ties (cleaner
			// K/D wins), player_name is the final stable key.
			$order_clause = 'kills DESC, deaths ASC, player_name ASC';
		}

		// Each row also carries its original points-based rank, computed via a
		// correlated subquery: count how many other players in the same
		// board_type have a STRICTLY higher score on the default order column,
		// then +1. Ties share the same rank. This stays correct even when the
		// caller sorts by some other column — the displayed # column always
		// reflects the player's true leaderboard placement.
		$rank_expr = "(SELECT 1 + COUNT(*) FROM {$table} t2 WHERE t2.board_type = t.board_type AND t2.{$order_col} > t.{$order_col})";

		if ( ! empty( $search ) ) {
			$like    = '%' . $wpdb->esc_like( $search ) . '%';
			$players = $wpdb->get_results( $wpdb->prepare(
				"SELECT t.*, {$rank_expr} AS original_rank FROM {$table} t WHERE t.board_type = %s AND t.player_name LIKE %s ORDER BY {$order_clause} LIMIT %d OFFSET %d",
				$board_type, $like, $per_page, $offset
			), ARRAY_A );
			$total   = (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE board_type = %s AND player_name LIKE %s",
				$board_type, $like
			) );
		} else {
			$players = $wpdb->get_results( $wpdb->prepare(
				"SELECT t.*, {$rank_expr} AS original_rank FROM {$table} t WHERE t.board_type = %s ORDER BY {$order_clause} LIMIT %d OFFSET %d",
				$board_type, $per_page, $offset
			), ARRAY_A );
			$total   = (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE board_type = %s",
				$board_type
			) );
		}

		// Format rows to match the shape the frontend expects.
		// Rank: prefer the original points-based rank from the subquery so the
		// player's true leaderboard placement stays visible even when the user
		// re-sorts by a different column. Only fall back to positional numbering
		// if the subquery couldn't run for some reason.
		$formatted   = array();
		$rank_cursor = $offset;
		foreach ( $players as $row ) {
			$rank_cursor++;
			$original_rank = isset( $row['original_rank'] ) ? (int) $row['original_rank'] : $rank_cursor;
			$shots_fired = (int) ( $row['shots_fired'] ?? 0 );
			$shots_hit   = (int) ( $row['shots_hit'] ?? 0 );
			$accuracy    = ( $shots_fired > 0 ) ? round( ( $shots_hit / $shots_fired ) * 100, 1 ) : 0.0;

			$formatted[] = array(
				'rank'                => $original_rank,
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
				'shots_fired'         => $shots_fired,
				'shots_hit'           => $shots_hit,
				'headshots'           => (int) ( $row['headshots'] ?? 0 ),
				'accuracy'            => $accuracy,
				// Per-mode longest_shot is already baked into row['longest_shot']
				// during upsert (board_type-aware), so no further branching here.
				'longest_shot'        => (int) round( (float) ( $row['longest_shot'] ?? 0 ) ),
				'distance_travelled'  => (float) ( $row['distance_travelled'] ?? 0 ),
				'distance_on_foot'    => (float) ( $row['distance_on_foot'] ?? 0 ),
				'distance_in_vehicle' => (float) ( $row['distance_in_vehicle'] ?? 0 ),
				'total_deaths'        => (int) ( $row['total_deaths'] ?? 0 ),
				'suicides'            => (int) ( $row['suicides'] ?? 0 ),
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

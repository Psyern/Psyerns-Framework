<?php
/**
 * Leaderboard upload, upsert and public retrieval.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Leaderboard
 *
 * Handles leaderboard data from the DayZ server.
 * Upserts player rows keyed by steam_id + board_type.
 * Provides public endpoints for frontend consumption.
 */
class PF_Leaderboard {

	/**
	 * Handle leaderboard upload POST.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_upload( WP_REST_Request $request ) {
		$data = $request->get_json_params();

		set_transient( 'pf_leaderboard_meta', array(
			'generatedAt'         => sanitize_text_field( $data['generatedAt'] ?? '' ),
			'playerOnlineCounter' => absint( $data['playerOnlineCounter'] ?? 0 ),
			'totalPlayers'        => absint( $data['totalPlayers'] ?? 0 ),
			'globalEastPoints'    => absint( $data['globalEastPoints'] ?? 0 ),
			'globalWestPoints'    => absint( $data['globalWestPoints'] ?? 0 ),
		), 600 );

		$this->upsert_players( $data['topPVEPlayers'] ?? array(), 'pve' );
		$this->upsert_players( $data['topPVPPlayers'] ?? array(), 'pvp' );

		return new WP_REST_Response( array( 'success' => true ), 200 );
	}

	/**
	 * Upsert a list of players into the leaderboard table.
	 *
	 * @param array  $players    Array of player data from the DayZ server.
	 * @param string $board_type Either 'pve' or 'pvp'.
	 * @return void
	 */
	private function upsert_players( $players, $board_type ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'leaderboard' );

		foreach ( $players as $p ) {
			// Support both Ninjin format (playerID) and PF format (odolozId)
			$steam_id = sanitize_text_field( $p['playerID'] ?? $p['odolozId'] ?? '' );

			// Fallback: use playerName as key if no steam ID provided
			// (Ninjin exports without playerID when WebExportIncludePlayerIDs is false)
			if ( empty( $steam_id ) ) {
				$player_name = sanitize_text_field( $p['playerName'] ?? '' );
				if ( empty( $player_name ) ) {
					continue;
				}
				$steam_id = 'name_' . md5( $player_name );
			}

			// Calculate totals from category data if not provided directly
			$category_kills  = $p['categoryKills'] ?? array();
			$category_deaths = $p['categoryDeaths'] ?? array();
			$category_ranges = $p['categoryLongestRanges'] ?? array();

			$total_kills = absint( $p['kills'] ?? 0 );
			if ( 0 === $total_kills && ! empty( $category_kills ) ) {
				foreach ( $category_kills as $count ) {
					$total_kills += absint( $count );
				}
			}

			$total_deaths = absint( $p['deaths'] ?? $p['deathCount'] ?? 0 );
			if ( 0 === $total_deaths && ! empty( $category_deaths ) ) {
				foreach ( $category_deaths as $count ) {
					$total_deaths += absint( $count );
				}
			}

			$ai_kills = absint( $p['aiKills'] ?? 0 );
			if ( 0 === $ai_kills && isset( $category_kills['AIBased'] ) ) {
				$ai_kills = absint( $category_kills['AIBased'] );
			}

			$longest_shot = floatval( $p['longestShot'] ?? 0 );
			if ( 0.0 === $longest_shot && ! empty( $category_ranges ) ) {
				foreach ( $category_ranges as $range ) {
					$longest_shot = max( $longest_shot, floatval( $range ) );
				}
			}

			$row = array(
				'steam_id'                => $steam_id,
				'player_name'             => sanitize_text_field( $p['playerName'] ?? '' ),
				'kills'                   => $total_kills,
				'deaths'                  => $total_deaths,
				'ai_kills'                => $ai_kills,
				'longest_shot'            => $longest_shot,
				'playtime'                => floatval( $p['playtime'] ?? 0 ),
				'pve_points'              => absint( $p['pvePoints'] ?? 0 ),
				'pvp_points'              => absint( $p['pvpPoints'] ?? 0 ),
				'pve_deaths'              => absint( $p['pveDeaths'] ?? 0 ),
				'pvp_deaths'              => absint( $p['pvpDeaths'] ?? 0 ),
				'board_type'              => $board_type,
				'category_kills'          => wp_json_encode( $p['categoryKills'] ?? new stdClass() ),
				'category_deaths'         => wp_json_encode( $p['categoryDeaths'] ?? new stdClass() ),
				'category_longest_ranges' => wp_json_encode( $p['categoryLongestRanges'] ?? new stdClass() ),
				'is_online'               => absint( $p['isOnline'] ?? 0 ),
				'last_login'              => sanitize_text_field( $p['lastLoginDate'] ?? '' ),
				'war_faction'             => sanitize_text_field( $p['warFaction'] ?? '' ),
				'war_alignment'           => intval( $p['warAlignment'] ?? 0 ),
				'war_level'               => absint( $p['warLevel'] ?? 0 ),
				'war_boss_kills'          => absint( $p['warBossKills'] ?? 0 ),
				'hardline_reputation'     => absint( $p['hardlineReputation'] ?? 0 ),
			);

			$existing = $wpdb->get_var( $wpdb->prepare(
				"SELECT id FROM {$table} WHERE steam_id = %s AND board_type = %s",
				$steam_id,
				$board_type
			) );

			if ( null !== $existing ) {
				$wpdb->update( $table, $row, array( 'id' => $existing ) );
			} else {
				$wpdb->insert( $table, $row );
			}
		}
	}

	/**
	 * Handle public leaderboard GET.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_get_public( WP_REST_Request $request ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'leaderboard' );
		$type  = sanitize_text_field( $request->get_param( 'type' ) ?: 'pve' );
		$limit = min( max( absint( $request->get_param( 'limit' ) ?: 20 ), 1 ), 100 );

		$order_col = ( 'pvp' === $type ) ? 'pvp_points' : 'pve_points';

		$results = $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE board_type = %s ORDER BY {$order_col} DESC LIMIT %d",
			$type,
			$limit
		), ARRAY_A );

		$players = array();
		foreach ( $results as $row ) {
			$players[] = self::format_player_row( $row );
		}

		$meta = get_transient( 'pf_leaderboard_meta' ) ?: array();

		/**
		 * Filter leaderboard public response.
		 *
		 * @param array  $players Formatted player array.
		 * @param string $type    Board type (pve|pvp).
		 */
		$players = apply_filters( 'psyerns-framework/leaderboard_players', $players, $type );

		return new WP_REST_Response( array(
			'generatedAt'         => $meta['generatedAt'] ?? '',
			'playerOnlineCounter' => absint( $meta['playerOnlineCounter'] ?? 0 ),
			'totalPlayers'        => absint( $meta['totalPlayers'] ?? 0 ),
			'globalEastPoints'    => absint( $meta['globalEastPoints'] ?? 0 ),
			'globalWestPoints'    => absint( $meta['globalWestPoints'] ?? 0 ),
			'players'             => $players,
		), 200 );
	}

	/**
	 * Handle top 3 players GET.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_top3( WP_REST_Request $request ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'leaderboard' );
		$type  = sanitize_text_field( $request->get_param( 'type' ) ?: 'monthly' );

		if ( 'deadliest' === $type ) {
			$results = $wpdb->get_results(
				"SELECT * FROM {$table} GROUP BY steam_id ORDER BY kills DESC LIMIT 3",
				ARRAY_A
			);
		} else {
			$month_start = gmdate( 'Y-m-01 00:00:00' );
			$results     = $wpdb->get_results( $wpdb->prepare(
				"SELECT * FROM {$table} WHERE updated_at >= %s GROUP BY steam_id ORDER BY (pve_points + pvp_points) DESC LIMIT 3",
				$month_start
			), ARRAY_A );
		}

		$players = array();
		foreach ( $results as $row ) {
			$players[] = self::format_player_row( $row );
		}

		return new WP_REST_Response( $players, 200 );
	}

	/**
	 * Format a database row into the public API response shape.
	 *
	 * @param array $row Database row as associative array.
	 * @return array Formatted player data.
	 */
	private static function format_player_row( $row ) {
		return array(
			'steam_id'                => $row['steam_id'],
			'player_name'             => $row['player_name'],
			'pve_points'              => (int) $row['pve_points'],
			'pvp_points'              => (int) $row['pvp_points'],
			'kills'                   => (int) $row['kills'],
			'deaths'                  => (int) $row['deaths'],
			'ai_kills'                => (int) $row['ai_kills'],
			'pve_deaths'              => (int) $row['pve_deaths'],
			'pvp_deaths'              => (int) $row['pvp_deaths'],
			'longest_shot'            => (float) $row['longest_shot'],
			'playtime'                => (float) $row['playtime'],
			'is_online'               => (int) $row['is_online'],
			'last_login'              => $row['last_login'],
			'category_kills'          => json_decode( $row['category_kills'] ?: '{}', true ),
			'category_deaths'         => json_decode( $row['category_deaths'] ?: '{}', true ),
			'category_longest_ranges' => json_decode( $row['category_longest_ranges'] ?: '{}', true ),
			'avatar_url'              => PF_Steam::get_avatar( $row['steam_id'] ),
			'war_faction'             => $row['war_faction'] ?? '',
			'war_alignment'           => (int) ( $row['war_alignment'] ?? 0 ),
			'war_level'               => (int) ( $row['war_level'] ?? 0 ),
			'war_boss_kills'          => (int) ( $row['war_boss_kills'] ?? 0 ),
			'hardline_reputation'     => (int) ( $row['hardline_reputation'] ?? 0 ),
		);
	}
}

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
		// DIAGNOSTIC INSTRUMENTATION: capture exactly what reaches WordPress so the
		// DayZ-side RPT (which logs the response body) shows whether the POST body
		// arrived as parseable JSON. Remove once the upload pipeline is confirmed.
		$raw_body   = $request->get_body();
		$body_bytes = strlen( (string) $raw_body );
		$data       = $request->get_json_params();
		$parse_ok   = is_array( $data );

		set_transient( 'pf_leaderboard_meta', array(
			'generatedAt'         => sanitize_text_field( $data['generatedAt'] ?? '' ),
			'playerOnlineCounter' => absint( $data['playerOnlineCounter'] ?? 0 ),
			'totalPlayers'        => absint( $data['totalPlayers'] ?? 0 ),
			'globalEastPoints'    => absint( $data['globalEastPoints'] ?? 0 ),
			'globalWestPoints'    => absint( $data['globalWestPoints'] ?? 0 ),
		), 600 );

		$pve_received = is_array( $data['topPVEPlayers'] ?? null ) ? count( $data['topPVEPlayers'] ) : 0;
		$pvp_received = is_array( $data['topPVPPlayers'] ?? null ) ? count( $data['topPVPPlayers'] ) : 0;

		$pve_result = $this->upsert_players( $data['topPVEPlayers'] ?? array(), 'pve' );
		$pvp_result = $this->upsert_players( $data['topPVPPlayers'] ?? array(), 'pvp' );

		// Player rows changed — invalidate the cached global kill total.
		delete_transient( 'pf_total_kills' );

		if ( ! empty( $data['playerDetails'] ) && is_array( $data['playerDetails'] ) ) {
			if ( class_exists( 'PF_Player_Details' ) ) {
				( new PF_Player_Details() )->handle_upload_details( $data['playerDetails'] );
			}
		}

		$diag = array(
			'success'      => true,
			'body_bytes'   => $body_bytes,
			'parse_ok'     => $parse_ok,
			'pve_received' => $pve_received,
			'pvp_received' => $pvp_received,
			'pve_written'  => $pve_result['written'],
			'pve_failed'   => $pve_result['failed'],
			'pvp_written'  => $pvp_result['written'],
			'pvp_failed'   => $pvp_result['failed'],
			'last_db_error' => $pve_result['last_error'] ?: $pvp_result['last_error'],
		);
		error_log( '[Psyerns] leaderboard upload diag: ' . wp_json_encode( $diag ) );

		return new WP_REST_Response( $diag, 200 );
	}

	/**
	 * Upsert a list of players into the leaderboard table.
	 *
	 * @param array  $players    Array of player data from the DayZ server.
	 * @param string $board_type Either 'pve' or 'pvp'.
	 * @return array{written:int,failed:int,last_error:string} Write outcome counters.
	 */
	private function upsert_players( $players, $board_type ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'leaderboard' );

		$written    = 0;
		$failed     = 0;
		$last_error = '';

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

			// Per-mode kills:
			//  pvp -> only kills against players
			//  pve -> only kills against AI/zombies/animals
			// Newer server builds send pvpKills/pveKills directly; fall back to the
			// summed category map for backwards compatibility with older PBOs that
			// only ship categoryKills.
			$pvp_kills_total = absint( $p['pvpKills'] ?? 0 );
			$pve_kills_total = absint( $p['pveKills'] ?? 0 );
			if ( 0 === $pvp_kills_total && 0 === $pve_kills_total && ! empty( $category_kills ) ) {
				$legacy_pvp_keys = array( 'Players', 'Player', 'Survivor', 'Spieler' );
				foreach ( $category_kills as $cat_id => $count ) {
					$count = absint( $count );
					if ( in_array( $cat_id, $legacy_pvp_keys, true ) ) {
						$pvp_kills_total += $count;
					} else {
						$pve_kills_total += $count;
					}
				}
			}
			$total_kills = ( 'pvp' === $board_type ) ? $pvp_kills_total : $pve_kills_total;

			// Per-mode deaths, mirroring the kills logic above:
			//  - Modern PBOs send pvpDeaths/pveDeaths directly (GetTotalPVPDeaths /
			//    GetTotalPVEDeaths, split via PVPCategoryConfig / PVECategoryConfig).
			//    A legitimate zero on one side (e.g. 0 PvP deaths, 5 PvE deaths)
			//    MUST stay zero on the PvP row — never overwrite with a combined
			//    total, or PvE deaths leak into the PvP board.
			//  - Legacy fallback: only kicks in when BOTH per-mode fields are
			//    absent from the payload (not just zero), and distributes
			//    categoryDeaths via the same player-category heuristic as kills.
			$pvp_deaths_total = absint( $p['pvpDeaths'] ?? 0 );
			$pve_deaths_total = absint( $p['pveDeaths'] ?? 0 );
			if ( ! isset( $p['pvpDeaths'] ) && ! isset( $p['pveDeaths'] ) && ! empty( $category_deaths ) ) {
				$legacy_pvp_keys = array( 'Players', 'Player', 'Survivor', 'Spieler' );
				foreach ( $category_deaths as $cat_id => $count ) {
					$count = absint( $count );
					if ( in_array( $cat_id, $legacy_pvp_keys, true ) ) {
						$pvp_deaths_total += $count;
					} else {
						$pve_deaths_total += $count;
					}
				}
			}
			$total_deaths = ( 'pvp' === $board_type ) ? $pvp_deaths_total : $pve_deaths_total;

			$ai_kills = absint( $p['aiKills'] ?? 0 );
			if ( 0 === $ai_kills && isset( $category_kills['AIBased'] ) ) {
				$ai_kills = absint( $category_kills['AIBased'] );
			}

			// Per-mode longest shot, same logic as the kills/deaths split:
			//  - Modern PBOs send pvpLongestShot/pveLongestShot directly
			//    (GetLongestPVPRange / GetLongestPVERange, split via the
			//    PVPCategoryConfig / PVECategoryConfig category sets).
			//  - A legitimate zero on one side (e.g. 0 m PvP range because the
			//    player has never shot another player) MUST stay zero on the
			//    PvP row — never fall back to a combined max, or PvE shots
			//    leak into the PvP board.
			//  - Legacy fallback: only when BOTH per-mode fields are ABSENT
			//    (isset() check, not "=== 0") we distribute categoryRanges
			//    via the player-category heuristic.
			$pvp_longest = floatval( $p['pvpLongestShot'] ?? 0 );
			$pve_longest = floatval( $p['pveLongestShot'] ?? 0 );
			if ( ! isset( $p['pvpLongestShot'] ) && ! isset( $p['pveLongestShot'] ) && ! empty( $category_ranges ) ) {
				$legacy_pvp_keys = array( 'Players', 'Player', 'Survivor', 'Spieler' );
				foreach ( $category_ranges as $cat_id => $range ) {
					$range = floatval( $range );
					if ( in_array( $cat_id, $legacy_pvp_keys, true ) ) {
						$pvp_longest = max( $pvp_longest, $range );
					} else {
						$pve_longest = max( $pve_longest, $range );
					}
				}
			}
			$longest_shot = ( 'pvp' === $board_type ) ? $pvp_longest : $pve_longest;

			$playtime_candidates = array(
				$p['playTimeSeconds'] ?? null,
				$p['playtimeSeconds'] ?? null,
				$p['playtime_seconds'] ?? null,
				$p['playtime'] ?? null,
				$p['playTime'] ?? null,
				( 'pvp' === $board_type ) ? ( $p['pvpPlaytimeSeconds'] ?? null ) : ( $p['pvePlaytimeSeconds'] ?? null ),
				( 'pvp' === $board_type ) ? ( $p['pvpPlaytime'] ?? null ) : ( $p['pvePlaytime'] ?? null ),
			);
			$playtime_value = 0.0;
			foreach ( $playtime_candidates as $playtime_candidate ) {
				if ( null === $playtime_candidate || '' === $playtime_candidate ) {
					continue;
				}
				$playtime_value = floatval( $playtime_candidate );
				if ( $playtime_value > 0 ) {
					break;
				}
			}

			$existing = $wpdb->get_row( $wpdb->prepare(
				"SELECT id, playtime FROM {$table} WHERE steam_id = %s AND board_type = %s",
				$steam_id,
				$board_type
			), ARRAY_A );

			if ( $playtime_value <= 0 && ! empty( $existing['playtime'] ) ) {
				$playtime_value = floatval( $existing['playtime'] );
			}
			if ( $playtime_value <= 0 ) {
				$max_playtime = $wpdb->get_var( $wpdb->prepare(
					"SELECT MAX(playtime) FROM {$table} WHERE steam_id = %s",
					$steam_id
				) );
				if ( null !== $max_playtime ) {
					$max_playtime = floatval( $max_playtime );
					if ( $max_playtime > 0 ) {
						$playtime_value = $max_playtime;
					}
				}
			}

			$row = array(
				'steam_id'                => $steam_id,
				'player_name'             => sanitize_text_field( $p['playerName'] ?? '' ),
				'kills'                   => $total_kills,
				'deaths'                  => $total_deaths,
				'ai_kills'                => $ai_kills,
				'longest_shot'            => $longest_shot,
				'playtime'                => $playtime_value,
				'pve_points'              => absint( $p['pvePoints'] ?? 0 ),
				'pvp_points'              => absint( $p['pvpPoints'] ?? 0 ),
				'pve_deaths'              => $pve_deaths_total,
				'pvp_deaths'              => $pvp_deaths_total,
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
				'shots_fired'             => absint( $p['shotsFired'] ?? 0 ),
				'shots_hit'               => absint( $p['shotsHit'] ?? 0 ),
				'headshots'               => absint( $p['headshots'] ?? 0 ),
				'distance_travelled'      => floatval( $p['distanceTravelled'] ?? 0 ),
				'distance_on_foot'        => floatval( $p['distanceOnFoot'] ?? 0 ),
				'distance_in_vehicle'     => floatval( $p['distanceInVehicle'] ?? 0 ),
				'total_deaths'            => absint( $p['totalDeaths'] ?? 0 ),
				'suicides'                => absint( $p['suicides'] ?? 0 ),
				'terje_skills'            => isset( $p['terjeSkills'] ) ? wp_json_encode( $p['terjeSkills'] ) : '',
			);

			if ( ! empty( $existing['id'] ) ) {
				$result = $wpdb->update( $table, $row, array( 'id' => $existing['id'] ) );
			} else {
				$result = $wpdb->insert( $table, $row );
			}

			if ( false === $result ) {
				$failed++;
				if ( $wpdb->last_error ) {
					$last_error = $wpdb->last_error;
				}
			} else {
				$written++;
			}
		}

		return array(
			'written'    => $written,
			'failed'     => $failed,
			'last_error' => $last_error,
		);
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

		// Sort by kills DESC (mode-specific — `kills` column already holds the
		// per-mode count via upsert_players), then by deaths ASC so a cleaner
		// K/D wins ties, then by player_name for stable ordering.
		$results = $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE board_type = %s ORDER BY kills DESC, deaths ASC, player_name ASC LIMIT %d",
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
	 * Handle public aggregate stats GET.
	 *
	 * Returns the headline counters used by the [pf_stats] banner:
	 * online players and tracked players (from meta) plus the global
	 * total kills (SUM over both board types), cached for 5 minutes.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_stats( WP_REST_Request $request ) {
		$meta = get_transient( 'pf_leaderboard_meta' ) ?: array();

		$total_kills = get_transient( 'pf_total_kills' );
		if ( false === $total_kills ) {
			global $wpdb;
			$table       = PF_Database::get_table_name( 'leaderboard' );
			// phpcs:ignore WordPress.DB.PreparedSQL.NotPrepared
			$total_kills = absint( $wpdb->get_var( "SELECT SUM(kills) FROM {$table}" ) );
			set_transient( 'pf_total_kills', $total_kills, 300 );
		}

		return new WP_REST_Response( array(
			'playerOnlineCounter' => absint( $meta['playerOnlineCounter'] ?? 0 ),
			'totalPlayers'        => absint( $meta['totalPlayers'] ?? 0 ),
			'totalKills'          => absint( $total_kills ),
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
		$shots_fired = (int) ( $row['shots_fired'] ?? 0 );
		$shots_hit   = (int) ( $row['shots_hit'] ?? 0 );
		$accuracy    = ( $shots_fired > 0 ) ? round( ( $shots_hit / $shots_fired ) * 100, 1 ) : 0.0;

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
			'playtime_seconds'        => (int) $row['playtime'],
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
			'shots_fired'             => $shots_fired,
			'shots_hit'               => $shots_hit,
			'headshots'               => (int) ( $row['headshots'] ?? 0 ),
			'accuracy'                => $accuracy,
			'distance_travelled'      => (float) ( $row['distance_travelled'] ?? 0 ),
			'distance_on_foot'        => (float) ( $row['distance_on_foot'] ?? 0 ),
			'distance_in_vehicle'     => (float) ( $row['distance_in_vehicle'] ?? 0 ),
			'total_deaths'            => (int) ( $row['total_deaths'] ?? 0 ),
			'suicides'                => (int) ( $row['suicides'] ?? 0 ),
			'terje_skills'            => ! empty( $row['terje_skills'] ) ? json_decode( $row['terje_skills'], true ) : null,
		);
	}
}

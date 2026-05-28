<?php
/**
 * Player detail persistence and public REST callback.
 *
 * Stores the per-player detail rows uploaded by the DayZ server and exposes
 * a public REST endpoint that the leaderboard modal fetches on row click.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Player_Details
 *
 * Handles UPSERT of `playerDetails[]` payload entries into the
 * `wp_pf_player_details` table and serves them back via the public
 * `/wp-json/psyern/v1/public/player/{uid}` endpoint.
 */
class PF_Player_Details {

	/**
	 * UPSERT each detail entry from the upload payload.
	 *
	 * Stores the (lightly sanitized) raw row as JSON in `data_json` keyed by
	 * `player_uid`. When the source row has no playerID, falls back to
	 * `name_<md5(name)>` — mirroring PF_Leaderboard::upsert_players().
	 *
	 * @param array $details The `playerDetails` array from the upload payload.
	 * @return void
	 */
	public function handle_upload_details( array $details ) {
		global $wpdb;
		$table = PF_Database::get_table_name( 'player_details' );

		foreach ( $details as $detail ) {
			if ( ! is_array( $detail ) ) {
				continue;
			}

			$player_uid  = sanitize_text_field( $detail['playerID'] ?? $detail['odolozId'] ?? '' );
			$player_name = sanitize_text_field( $detail['playerName'] ?? '' );

			if ( '' === $player_uid ) {
				if ( '' === $player_name ) {
					continue;
				}
				$player_uid = 'name_' . md5( $player_name );
			}

			if ( ! preg_match( '/^[A-Za-z0-9_]{1,64}$/', $player_uid ) ) {
				continue;
			}

			$detail['playerID']   = $player_uid;
			$detail['playerName'] = $player_name;

			$wpdb->replace(
				$table,
				array(
					'player_uid'  => $player_uid,
					'player_name' => $player_name,
					'data_json'   => wp_json_encode( $detail ),
				),
				array( '%s', '%s', '%s' )
			);
		}
	}

	/**
	 * REST callback: GET /psyern/v1/public/player/{uid}.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_get( WP_REST_Request $request ) {
		global $wpdb;

		$uid = sanitize_text_field( $request->get_param( 'uid' ) );
		if ( ! preg_match( '/^[A-Za-z0-9_]{1,64}$/', $uid ) ) {
			return new WP_REST_Response( array( 'error' => 'not_found' ), 404 );
		}

		// Prefer the dedicated details table (richest payload, includes
		// survivorType / lastUpdated from the DayZ side).
		$details_table = PF_Database::get_table_name( 'player_details' );
		$row           = $wpdb->get_row( $wpdb->prepare(
			"SELECT player_uid, player_name, data_json, updated_at FROM {$details_table} WHERE player_uid = %s",
			$uid
		), ARRAY_A );

		if ( null !== $row ) {
			$raw = json_decode( $row['data_json'] ?: '{}', true );
			if ( ! is_array( $raw ) ) {
				$raw = array();
			}
			return new WP_REST_Response( $this->transform( $raw, $row ), 200 );
		}

		// Fallback: the leaderboard table already carries the same per-player
		// stats (war / hardline / gunplay / movement / category maps), so we
		// can synthesize the same response shape without needing the optional
		// playerDetails[] upload from the server PBO.
		$lb_table = PF_Database::get_table_name( 'leaderboard' );
		$lb_rows  = $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$lb_table} WHERE steam_id = %s",
			$uid
		), ARRAY_A );

		if ( empty( $lb_rows ) ) {
			return new WP_REST_Response( array( 'error' => 'not_found' ), 404 );
		}

		return new WP_REST_Response( $this->build_from_leaderboard( $lb_rows ), 200 );
	}

	/**
	 * Synthesize the player-detail response shape from one or more leaderboard
	 * rows (pve + pvp). Most per-player stats are duplicated across the rows
	 * by upsert_players(); category maps are the raw Ninjin source data so
	 * either row works.
	 *
	 * @param array $rows Rows from wp_pf_leaderboard for one steam_id.
	 * @return array
	 */
	private function build_from_leaderboard( array $rows ) {
		$primary = $rows[0];
		// Prefer a row that actually has gunplay data populated, in case one
		// board hasn't received that subset yet.
		foreach ( $rows as $r ) {
			if ( (int) $r['shots_fired'] > (int) $primary['shots_fired'] ) {
				$primary = $r;
			}
		}

		$category_kills = json_decode( $primary['category_kills'] ?: '{}', true );
		$category_deaths = json_decode( $primary['category_deaths'] ?: '{}', true );
		$category_ranges = json_decode( $primary['category_longest_ranges'] ?: '{}', true );
		if ( ! is_array( $category_kills ) ) { $category_kills = array(); }
		if ( ! is_array( $category_deaths ) ) { $category_deaths = array(); }
		if ( ! is_array( $category_ranges ) ) { $category_ranges = array(); }

		$terje_skills = null;
		if ( ! empty( $primary['terje_skills'] ) ) {
			$decoded = json_decode( $primary['terje_skills'], true );
			if ( is_array( $decoded ) && ! empty( $decoded ) ) {
				$terje_skills = $decoded;
			}
		}

		$synthetic_raw = array(
			'playerID'              => (string) $primary['steam_id'],
			'playerName'            => (string) $primary['player_name'],
			'survivorType'          => '',
			'categoryKills'         => $category_kills,
			'categoryDeaths'        => $category_deaths,
			'categoryLongestRanges' => $category_ranges,
			'totalDeaths'           => (int) $primary['total_deaths'],
			'shotsFired'            => (int) $primary['shots_fired'],
			'shotsHit'              => (int) $primary['shots_hit'],
			'headshots'             => (int) $primary['headshots'],
			'distanceTravelled'     => (float) $primary['distance_travelled'],
			'distanceOnFoot'        => (float) $primary['distance_on_foot'],
			'distanceInVehicle'     => (float) $primary['distance_in_vehicle'],
			'suicides'              => (int) $primary['suicides'],
			'playTimeSeconds'       => (int) $primary['playtime'],
			'isOnline'              => 1 === (int) $primary['is_online'],
			'warFaction'            => (string) $primary['war_faction'],
			'warAlignment'          => (int) $primary['war_alignment'],
			'warLevel'              => (int) $primary['war_level'],
			'warBossKills'          => (int) $primary['war_boss_kills'],
			'hardlineReputation'    => (int) $primary['hardline_reputation'],
			'terjeSkills'           => $terje_skills,
		);

		$synthetic_row = array(
			'player_uid'  => (string) $primary['steam_id'],
			'player_name' => (string) $primary['player_name'],
			'updated_at'  => (string) $primary['updated_at'],
		);

		return $this->transform( $synthetic_raw, $synthetic_row );
	}

	/**
	 * Transform a stored raw player detail record into the public contract shape.
	 *
	 * @param array $raw The decoded `data_json` blob.
	 * @param array $row The DB row (provides updated_at + canonical uid/name).
	 * @return array
	 */
	private function transform( array $raw, array $row ) {
		$player_uid  = (string) $row['player_uid'];
		$player_name = (string) ( $raw['playerName'] ?? $row['player_name'] );

		$last_updated = $raw['lastUpdated'] ?? '';
		if ( '' === $last_updated && ! empty( $row['updated_at'] ) ) {
			$ts = strtotime( $row['updated_at'] . ' UTC' );
			if ( $ts ) {
				$last_updated = gmdate( 'Y-m-d\TH:i:s\Z', $ts );
			}
		}

		$raw_kills  = isset( $raw['categoryKills'] ) && is_array( $raw['categoryKills'] ) ? $raw['categoryKills'] : array();
		$raw_deaths = isset( $raw['categoryDeaths'] ) && is_array( $raw['categoryDeaths'] ) ? $raw['categoryDeaths'] : array();
		$raw_ranges = isset( $raw['categoryLongestRanges'] ) && is_array( $raw['categoryLongestRanges'] ) ? $raw['categoryLongestRanges'] : array();

		$kills_grouped  = self::bucket( $raw_kills );
		$deaths_grouped = self::bucket( $raw_deaths );

		$total_kills = 0;
		foreach ( $kills_grouped as $group_counts ) {
			foreach ( $group_counts as $count ) {
				$total_kills += (int) $count;
			}
		}

		$total_deaths = isset( $raw['totalDeaths'] ) ? (int) $raw['totalDeaths'] : 0;
		if ( 0 === $total_deaths ) {
			foreach ( $deaths_grouped as $group_counts ) {
				foreach ( $group_counts as $count ) {
					$total_deaths += (int) $count;
				}
			}
		}

		$shots_fired = isset( $raw['shotsFired'] ) ? (int) $raw['shotsFired'] : 0;
		$shots_hit   = isset( $raw['shotsHit'] ) ? (int) $raw['shotsHit'] : 0;
		$headshots   = isset( $raw['headshots'] ) ? (int) $raw['headshots'] : 0;
		$accuracy    = ( $shots_fired > 0 ) ? round( ( $shots_hit / $shots_fired ) * 100, 1 ) : 0.0;
		$hs_pct      = ( $shots_hit > 0 ) ? round( ( $headshots / $shots_hit ) * 100, 1 ) : 0.0;

		$kd = round( $total_kills / max( 1, $total_deaths ), 2 );

		$ranges_clean = array();
		foreach ( $raw_ranges as $k => $v ) {
			$ranges_clean[ sanitize_text_field( (string) $k ) ] = (float) $v;
		}

		return array(
			'playerUid'    => $player_uid,
			'playerName'   => $player_name,
			'survivorType' => isset( $raw['survivorType'] ) ? sanitize_text_field( (string) $raw['survivorType'] ) : '',
			'lastUpdated'  => (string) $last_updated,
			'summary'      => array(
				'totalKills'      => $total_kills,
				'totalDeaths'     => $total_deaths,
				'kd'              => $kd,
				'playTimeSeconds' => isset( $raw['playTimeSeconds'] ) ? (int) $raw['playTimeSeconds'] : 0,
				'playtime_seconds' => isset( $raw['playTimeSeconds'] ) ? (int) $raw['playTimeSeconds'] : 0,
				'isOnline'        => isset( $raw['isOnline'] ) ? (bool) $raw['isOnline'] : false,
			),
			'war'          => array(
				'faction'   => isset( $raw['warFaction'] ) ? sanitize_text_field( (string) $raw['warFaction'] ) : '',
				'level'     => isset( $raw['warLevel'] ) ? (int) $raw['warLevel'] : 0,
				'alignment' => isset( $raw['warAlignment'] ) ? (int) $raw['warAlignment'] : 0,
				'bossKills' => isset( $raw['warBossKills'] ) ? (int) $raw['warBossKills'] : 0,
			),
			'hardline'     => array(
				'reputation' => isset( $raw['hardlineReputation'] ) ? (int) $raw['hardlineReputation'] : 0,
			),
			'gunplay'      => array(
				'shotsFired'         => $shots_fired,
				'shotsHit'           => $shots_hit,
				'headshots'          => $headshots,
				'accuracy'           => $accuracy,
				'headshotPercentage' => $hs_pct,
			),
			'movement'     => array(
				'distanceTravelled' => isset( $raw['distanceTravelled'] ) ? (float) $raw['distanceTravelled'] : 0.0,
				'distanceOnFoot'    => isset( $raw['distanceOnFoot'] ) ? (float) $raw['distanceOnFoot'] : 0.0,
				'distanceInVehicle' => isset( $raw['distanceInVehicle'] ) ? (float) $raw['distanceInVehicle'] : 0.0,
				'suicides'          => isset( $raw['suicides'] ) ? (int) $raw['suicides'] : 0,
			),
			'kills'        => $kills_grouped,
			'deaths'       => $deaths_grouped,
			'longestRanges' => $ranges_clean,
			'skills'       => self::shape_skills( $raw['terjeSkills'] ?? null ),
		);
	}

	/**
	 * Shape Terje skills for the modal. Terje's actual level is computed from
	 * experience via per-skill config thresholds (levels[] in athletic.hpp etc.),
	 * which the PHP side does not have access to. We surface `highLevel` (Terje's
	 * own peak-level high-water mark) as the displayable level, plus the raw
	 * experience for context. Empty array if no skills data — frontend hides tab.
	 *
	 * @param mixed $raw_skills The decoded terjeSkills payload (or null).
	 * @return array<int,array>
	 */
	private static function shape_skills( $raw_skills ) {
		if ( ! is_array( $raw_skills ) || empty( $raw_skills ) ) {
			return array();
		}

		$out = array();
		foreach ( $raw_skills as $skill_id => $data ) {
			if ( ! is_array( $data ) ) {
				continue;
			}

			$known_books = array();
			if ( isset( $data['knownBooks'] ) && is_array( $data['knownBooks'] ) ) {
				foreach ( $data['knownBooks'] as $b ) {
					$known_books[] = sanitize_text_field( (string) $b );
				}
			}

			$perks = array();
			if ( isset( $data['perks'] ) && is_array( $data['perks'] ) ) {
				foreach ( $data['perks'] as $perk_id => $perk_value ) {
					$perks[ sanitize_text_field( (string) $perk_id ) ] = (int) $perk_value;
				}
			}

			$out[] = array(
				'id'          => sanitize_text_field( (string) $skill_id ),
				'level'       => (int) ( $data['highLevel'] ?? 0 ),
				'experience'  => (int) ( $data['experience'] ?? 0 ),
				'perkPoints'  => (int) ( $data['perkPoints'] ?? 0 ),
				'highLevel'   => (int) ( $data['highLevel'] ?? 0 ),
				'knownBooks'  => $known_books,
				'perks'       => $perks,
			);
		}
		return $out;
	}

	/**
	 * Bucket a raw classname→count map into the contract groups.
	 *
	 * Delegates to PF_Category_Map (Agent 3) when available; otherwise falls
	 * back to a single "other" bucket so the endpoint still returns a valid
	 * shape during phased rollout.
	 *
	 * @param array $raw_map classname => count.
	 * @return array<string, array<string, int>>
	 */
	private static function bucket( array $raw_map ) {
		if ( class_exists( 'PF_Category_Map' ) && method_exists( 'PF_Category_Map', 'bucket_kills' ) ) {
			$buckets = PF_Category_Map::bucket_kills( $raw_map );
			if ( is_array( $buckets ) ) {
				return $buckets;
			}
		}

		$out = array(
			'zombies' => new stdClass(),
			'players' => new stdClass(),
			'bosses'  => new stdClass(),
			'ai'      => new stdClass(),
			'animals' => new stdClass(),
			'other'   => array(),
		);
		foreach ( $raw_map as $classname => $count ) {
			$classname = sanitize_text_field( (string) $classname );
			if ( '' === $classname ) {
				continue;
			}
			$out['other'][ $classname ] = (int) $count;
		}
		return $out;
	}
}

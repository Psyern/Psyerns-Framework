<?php
/**
 * Leaderboard database schema and CRUD.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class Psyern_Database
 *
 * Manages the leaderboard and weekly winners tables.
 */
class Psyern_Database {

	/**
	 * Get prefixed table name.
	 *
	 * @param string $name Short name.
	 * @return string Full table name.
	 */
	public static function table( $name ) {
		global $wpdb;
		return $wpdb->prefix . 'psyern_' . $name;
	}

	/**
	 * Create leaderboard tables via dbDelta.
	 *
	 * @return void
	 */
	public static function create_tables() {
		global $wpdb;
		$charset = $wpdb->get_charset_collate();
		$lb      = self::table( 'leaderboard' );
		$ww      = self::table( 'weekly_winners' );

		$sql = "CREATE TABLE {$lb} (
			id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
			steam_id VARCHAR(20) NOT NULL DEFAULT '',
			player_name VARCHAR(255) NOT NULL DEFAULT '',
			kills INT DEFAULT 0,
			deaths INT DEFAULT 0,
			kd_ratio FLOAT DEFAULT 0,
			playtime_seconds INT DEFAULT 0,
			mode VARCHAR(10) NOT NULL DEFAULT 'pvp',
			score INT DEFAULT 0,
			week_number INT DEFAULT 0,
			year INT DEFAULT 0,
			avatar_url VARCHAR(512) DEFAULT '',
			last_updated DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
			created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
			UNIQUE KEY steam_mode (steam_id, mode)
		) {$charset};

		CREATE TABLE {$ww} (
			id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
			steam_id VARCHAR(20) NOT NULL DEFAULT '',
			player_name VARCHAR(255) NOT NULL DEFAULT '',
			kills INT DEFAULT 0,
			deaths INT DEFAULT 0,
			kd_ratio FLOAT DEFAULT 0,
			score INT DEFAULT 0,
			mode VARCHAR(10) NOT NULL DEFAULT 'pvp',
			rank_position TINYINT DEFAULT 0,
			week_number INT DEFAULT 0,
			year INT DEFAULT 0,
			avatar_url VARCHAR(512) DEFAULT '',
			archived_at DATETIME DEFAULT CURRENT_TIMESTAMP
		) {$charset};";

		require_once ABSPATH . 'wp-admin/includes/upgrade.php';
		dbDelta( $sql );
	}

	/**
	 * Upsert a player row by steam_id + mode.
	 *
	 * @param array $player Player data array.
	 * @return void
	 */
	public static function upsert_player( $player ) {
		global $wpdb;
		$table    = self::table( 'leaderboard' );
		$steam_id = sanitize_text_field( $player['steamId'] ?? '' );
		$mode     = sanitize_text_field( $player['mode'] ?? 'pvp' );

		if ( empty( $steam_id ) ) {
			return;
		}

		$kills  = absint( $player['kills'] ?? 0 );
		$deaths = absint( $player['deaths'] ?? 0 );

		$row = array(
			'steam_id'         => $steam_id,
			'player_name'      => sanitize_text_field( $player['playerName'] ?? '' ),
			'kills'            => $kills,
			'deaths'           => $deaths,
			'kd_ratio'         => ( $deaths > 0 ) ? round( $kills / $deaths, 2 ) : (float) $kills,
			'playtime_seconds' => absint( $player['playtimeSeconds'] ?? 0 ),
			'mode'             => $mode,
			'score'            => absint( $player['score'] ?? 0 ),
			'week_number'      => absint( gmdate( 'W' ) ),
			'year'             => absint( gmdate( 'Y' ) ),
			'avatar_url'       => esc_url_raw( $player['avatarUrl'] ?? '' ),
		);

		$existing = $wpdb->get_var( $wpdb->prepare(
			"SELECT id FROM {$table} WHERE steam_id = %s AND mode = %s",
			$steam_id,
			$mode
		) );

		if ( null !== $existing ) {
			$wpdb->update( $table, $row, array( 'id' => $existing ) );
		} else {
			$wpdb->insert( $table, $row );
		}
	}

	/**
	 * Get leaderboard rows.
	 *
	 * @param string $mode     'pvp' or 'pve'.
	 * @param int    $per_page Rows per page.
	 * @param int    $page     Page number (1-based).
	 * @param string $search   Optional player name filter.
	 * @return array
	 */
	public static function get_leaderboard( $mode = 'pvp', $per_page = 20, $page = 1, $search = '' ) {
		global $wpdb;
		$table    = self::table( 'leaderboard' );
		$per_page = max( 1, absint( $per_page ) );
		$offset   = max( 0, ( absint( $page ) - 1 ) * $per_page );

		if ( ! empty( $search ) ) {
			return $wpdb->get_results( $wpdb->prepare(
				"SELECT * FROM {$table} WHERE mode = %s AND player_name LIKE %s ORDER BY score DESC, kills DESC LIMIT %d OFFSET %d",
				$mode,
				'%' . $wpdb->esc_like( $search ) . '%',
				$per_page,
				$offset
			), ARRAY_A );
		}

		return $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE mode = %s ORDER BY score DESC, kills DESC LIMIT %d OFFSET %d",
			$mode,
			$per_page,
			$offset
		), ARRAY_A );
	}

	/**
	 * Count leaderboard rows for pagination.
	 *
	 * @param string $mode   'pvp' or 'pve'.
	 * @param string $search Optional player name filter.
	 * @return int
	 */
	public static function count_leaderboard( $mode = 'pvp', $search = '' ) {
		global $wpdb;
		$table = self::table( 'leaderboard' );

		if ( ! empty( $search ) ) {
			return (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE mode = %s AND player_name LIKE %s",
				$mode,
				'%' . $wpdb->esc_like( $search ) . '%'
			) );
		}

		return (int) $wpdb->get_var( $wpdb->prepare(
			"SELECT COUNT(*) FROM {$table} WHERE mode = %s",
			$mode
		) );
	}

	/**
	 * Get top 3 players for the current week.
	 *
	 * @param string $mode 'pvp' or 'pve'.
	 * @return array
	 */
	public static function get_top3( $mode = 'pvp' ) {
		global $wpdb;
		$table = self::table( 'leaderboard' );
		$week  = absint( gmdate( 'W' ) );
		$year  = absint( gmdate( 'Y' ) );

		return $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE mode = %s AND week_number = %d AND year = %d ORDER BY score DESC, kills DESC LIMIT 3",
			$mode,
			$week,
			$year
		), ARRAY_A );
	}

	/**
	 * Drop plugin tables.
	 *
	 * @return void
	 */
	public static function drop_tables() {
		global $wpdb;
		// phpcs:disable WordPress.DB.PreparedSQL.NotPrepared
		$wpdb->query( 'DROP TABLE IF EXISTS ' . self::table( 'leaderboard' ) );
		$wpdb->query( 'DROP TABLE IF EXISTS ' . self::table( 'weekly_winners' ) );
		// phpcs:enable
	}
}

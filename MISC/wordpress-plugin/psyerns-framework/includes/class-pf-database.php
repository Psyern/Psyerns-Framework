<?php
/**
 * Database schema and CRUD operations.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Database
 *
 * Handles table creation via dbDelta() and provides table name helpers.
 */
class PF_Database {

	/**
	 * Get the full prefixed table name.
	 *
	 * @param string $name Short table name (whitelist, leaderboard, killfeed).
	 * @return string Full table name with wpdb prefix.
	 */
	public static function get_table_name( $name ) {
		global $wpdb;
		return $wpdb->prefix . 'pf_' . $name;
	}

	/**
	 * Create or update all plugin tables via dbDelta().
	 *
	 * @return void
	 */
	public static function create_tables() {
		global $wpdb;
		$charset = $wpdb->get_charset_collate();

		$whitelist   = self::get_table_name( 'whitelist' );
		$leaderboard = self::get_table_name( 'leaderboard' );
		$killfeed    = self::get_table_name( 'killfeed' );

		$sql = "CREATE TABLE {$whitelist} (
			id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
			steam_id VARCHAR(20) NOT NULL UNIQUE,
			name VARCHAR(255) NOT NULL DEFAULT '',
			added_at DATETIME DEFAULT CURRENT_TIMESTAMP
		) {$charset};

		CREATE TABLE {$leaderboard} (
			id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
			steam_id VARCHAR(20) NOT NULL DEFAULT '',
			player_name VARCHAR(255) NOT NULL DEFAULT '',
			kills INT DEFAULT 0,
			deaths INT DEFAULT 0,
			ai_kills INT DEFAULT 0,
			longest_shot FLOAT DEFAULT 0,
			playtime FLOAT DEFAULT 0,
			pve_points INT DEFAULT 0,
			pvp_points INT DEFAULT 0,
			pve_deaths INT DEFAULT 0,
			pvp_deaths INT DEFAULT 0,
			board_type ENUM('pve','pvp') NOT NULL DEFAULT 'pve',
			category_kills TEXT,
			category_deaths TEXT,
			category_longest_ranges TEXT,
			is_online TINYINT(1) DEFAULT 0,
			last_login VARCHAR(30) DEFAULT '',
			war_faction VARCHAR(10) DEFAULT '',
			war_alignment INT DEFAULT 0,
			war_level INT DEFAULT 0,
			war_boss_kills INT DEFAULT 0,
			hardline_reputation INT DEFAULT 0,
			updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
			UNIQUE KEY steam_board (steam_id, board_type)
		) {$charset};

		CREATE TABLE {$killfeed} (
			id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
			killer_steam_id VARCHAR(20) DEFAULT '',
			killer_name VARCHAR(255) DEFAULT 'Unknown',
			killer_weapon VARCHAR(255) DEFAULT 'Unknown',
			victim_steam_id VARCHAR(20) DEFAULT '',
			victim_name VARCHAR(255) DEFAULT 'Unknown',
			pos_x FLOAT DEFAULT 0,
			pos_y FLOAT DEFAULT 0,
			pos_z FLOAT DEFAULT 0,
			distance FLOAT DEFAULT 0,
			killed_at DATETIME DEFAULT CURRENT_TIMESTAMP
		) {$charset};";

		require_once ABSPATH . 'wp-admin/includes/upgrade.php';
		dbDelta( $sql );
	}

	/**
	 * Drop all plugin tables. Used by uninstall.php.
	 *
	 * @return void
	 */
	public static function drop_tables() {
		global $wpdb;
		// phpcs:disable WordPress.DB.PreparedSQL.NotPrepared
		$wpdb->query( 'DROP TABLE IF EXISTS ' . self::get_table_name( 'whitelist' ) );
		$wpdb->query( 'DROP TABLE IF EXISTS ' . self::get_table_name( 'leaderboard' ) );
		$wpdb->query( 'DROP TABLE IF EXISTS ' . self::get_table_name( 'killfeed' ) );
		// phpcs:enable
	}
}

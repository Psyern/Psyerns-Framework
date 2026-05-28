<?php
/**
 * Transient cache layer for Psyerns Mods.
 *
 * Wraps WordPress Transients API with a consistent prefix
 * and TTL for Steam Workshop data caching.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Manages transient-based caching for Steam API responses.
 */
class PSM_Cache {

	/**
	 * Transient key prefix.
	 *
	 * @var string
	 */
	const PREFIX = 'psm_steam_';

	/**
	 * Cache time-to-live in seconds (12 hours).
	 *
	 * @var int
	 */
	const TTL = 43200;

	/**
	 * Register hooks.
	 *
	 * Flushes the cache for a post whenever it is saved.
	 *
	 * @return void
	 */
	public static function register() {
		add_action( 'save_post_psyern_mod', array( __CLASS__, 'flush_for_post' ), 20, 1 );
	}

	/**
	 * Retrieve a cached value.
	 *
	 * @param string $key Cache key (without prefix — prefix is added automatically).
	 * @return mixed Cached data or false if not found / expired.
	 */
	public static function get( $key ) {
		return get_transient( self::PREFIX . sanitize_key( $key ) );
	}

	/**
	 * Store a value in the cache.
	 *
	 * @param string $key  Cache key (without prefix).
	 * @param mixed  $data Data to cache (will be serialized by WordPress).
	 * @return bool True if the value was set, false otherwise.
	 */
	public static function set( $key, $data ) {
		return set_transient( self::PREFIX . sanitize_key( $key ), $data, self::TTL );
	}

	/**
	 * Delete a single cached entry.
	 *
	 * @param string $key Cache key (without prefix).
	 * @return bool True if the transient was deleted, false otherwise.
	 */
	public static function delete( $key ) {
		return delete_transient( self::PREFIX . sanitize_key( $key ) );
	}

	/**
	 * Flush the Steam data cache for a specific post.
	 *
	 * Reads the Steam URL from post meta, extracts the workshop ID
	 * and deletes the corresponding transient.
	 *
	 * @param int $post_id The post ID being saved.
	 * @return void
	 */
	public static function flush_for_post( $post_id ) {
		$steam_url = get_post_meta( $post_id, PSM_Meta::STEAM_URL, true );

		if ( empty( $steam_url ) ) {
			return;
		}

		$workshop_id = PSM_Steam_API::extract_workshop_id( $steam_url );

		if ( false === $workshop_id ) {
			return;
		}

		self::delete( 'steam_' . $workshop_id );
	}

	/**
	 * Flush all Steam-related transients from the database.
	 *
	 * Removes both '_transient_' and '_transient_timeout_' rows
	 * matching the plugin prefix.
	 *
	 * @return int Number of rows deleted.
	 */
	public static function flush_all() {
		global $wpdb;

		$deleted = $wpdb->query(
			$wpdb->prepare(
				"DELETE FROM {$wpdb->options} WHERE option_name LIKE %s OR option_name LIKE %s",
				$wpdb->esc_like( '_transient_' . self::PREFIX ) . '%',
				$wpdb->esc_like( '_transient_timeout_' . self::PREFIX ) . '%'
			)
		);

		return (int) $deleted;
	}
}

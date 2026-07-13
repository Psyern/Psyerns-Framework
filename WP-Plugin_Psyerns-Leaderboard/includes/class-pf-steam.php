<?php
/**
 * Steam avatar resolution with transient caching.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Steam
 *
 * Resolves Steam profile avatars via the Steam Web API.
 * Caches results as transients (24h TTL).
 */
class PF_Steam {

	/**
	 * Get avatar URL for a Steam ID.
	 *
	 * @param string $steam_id Steam64 ID.
	 * @return string Avatar URL or fallback image.
	 */
	public static function get_avatar( $steam_id ) {
		if ( empty( $steam_id ) ) {
			return self::get_default_avatar();
		}

		$transient_key = 'pf_avatar_' . $steam_id;
		$cached        = get_transient( $transient_key );

		if ( false !== $cached ) {
			return $cached;
		}

		$steam_key = get_option( 'pf_steam_api_key', '' );

		if ( empty( $steam_key ) ) {
			return self::get_default_avatar();
		}

		$url = sprintf(
			'https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%s',
			$steam_key,
			$steam_id
		);

		$response = wp_remote_get( $url, array( 'timeout' => 5 ) );

		if ( is_wp_error( $response ) ) {
			return self::get_default_avatar();
		}

		$body    = json_decode( wp_remote_retrieve_body( $response ), true );
		$players = $body['response']['players'] ?? array();

		if ( empty( $players ) ) {
			return self::get_default_avatar();
		}

		$avatar_url = $players[0]['avatarfull'] ?? self::get_default_avatar();
		set_transient( $transient_key, $avatar_url, DAY_IN_SECONDS );

		return $avatar_url;
	}

	/**
	 * Resolve avatar URLs for many Steam IDs at once.
	 *
	 * Reads the per-id transient cache first; only genuine cache misses are
	 * fetched from Steam, batched up to 100 IDs per request (the API maximum)
	 * so a full leaderboard costs 1-2 HTTP calls instead of one per player.
	 * Intended to run at upload time, NOT on the page-load hot path.
	 *
	 * @param string[] $steam_ids List of Steam64 IDs.
	 * @return array<string,string> Map of steam_id => avatar URL.
	 */
	public static function get_avatars_batch( $steam_ids ) {
		$result = array();
		$misses = array();

		foreach ( array_unique( array_filter( $steam_ids ) ) as $steam_id ) {
			// Only numeric Steam64 IDs are resolvable; skip name_-hash fallbacks.
			if ( ! ctype_digit( (string) $steam_id ) ) {
				continue;
			}
			$cached = get_transient( 'pf_avatar_' . $steam_id );
			if ( false !== $cached ) {
				$result[ $steam_id ] = $cached;
			} else {
				$misses[] = $steam_id;
			}
		}

		if ( empty( $misses ) ) {
			return $result;
		}

		$steam_key = get_option( 'pf_steam_api_key', '' );
		if ( empty( $steam_key ) ) {
			return $result;
		}

		foreach ( array_chunk( $misses, 100 ) as $chunk ) {
			$url = sprintf(
				'https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=%s&steamids=%s',
				$steam_key,
				implode( ',', $chunk )
			);

			$response = wp_remote_get( $url, array( 'timeout' => 5 ) );
			if ( is_wp_error( $response ) ) {
				continue;
			}

			$body    = json_decode( wp_remote_retrieve_body( $response ), true );
			$players = $body['response']['players'] ?? array();
			foreach ( $players as $player ) {
				$sid = $player['steamid'] ?? '';
				if ( '' === $sid ) {
					continue;
				}
				$avatar_url = $player['avatarfull'] ?? self::get_default_avatar();
				set_transient( 'pf_avatar_' . $sid, $avatar_url, DAY_IN_SECONDS );
				$result[ $sid ] = $avatar_url;
			}
		}

		return $result;
	}

	/**
	 * Get fallback avatar URL.
	 *
	 * @return string Default avatar image URL.
	 */
	public static function get_default_avatar() {
		return PF_PLUGIN_URL . 'public/assets/img/default-avatar.png';
	}
}

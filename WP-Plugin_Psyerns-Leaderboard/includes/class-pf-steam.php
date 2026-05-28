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
	 * Get fallback avatar URL.
	 *
	 * @return string Default avatar image URL.
	 */
	public static function get_default_avatar() {
		return PF_PLUGIN_URL . 'public/assets/img/default-avatar.png';
	}
}

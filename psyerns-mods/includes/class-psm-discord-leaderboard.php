<?php
/**
 * Discord Leaderboard push via Webhook.
 *
 * Listens for the psm_leaderboard_updated action, builds a Discord
 * Embed from the configured fields, and sends/edits a message in the
 * configured Discord channel via Webhook.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Handles Discord Webhook communication for the leaderboard.
 */
class PSM_Discord_Leaderboard {

	/**
	 * Transient key used as a rate-limit lock.
	 *
	 * @var string
	 */
	const RATE_LIMIT_KEY = 'psm_discord_rate_lock';

	/**
	 * Minimum seconds between Discord updates.
	 *
	 * @var int
	 */
	const MIN_INTERVAL = 10;

	/**
	 * Medal emojis for top-3 positions.
	 *
	 * @var array
	 */
	private static $medals = array( "\xF0\x9F\xA5\x87", "\xF0\x9F\xA5\x88", "\xF0\x9F\xA5\x89" );

	/**
	 * Register hooks.
	 *
	 * @return void
	 */
	public static function register() {
		add_action( 'psm_leaderboard_updated', array( __CLASS__, 'on_leaderboard_updated' ), 10, 2 );
	}

	/**
	 * Callback when new leaderboard data arrives.
	 *
	 * @param array  $data      Sanitized leaderboard data.
	 * @param string $timestamp UTC timestamp.
	 * @return void
	 */
	public static function on_leaderboard_updated( $data, $timestamp ) {
		if ( '1' !== get_option( 'psm_discord_enabled', '' ) ) {
			return;
		}

		$webhook_url = get_option( 'psm_discord_webhook_url', '' );

		if ( '' === $webhook_url ) {
			return;
		}

		/*
		 * Rate limiting: skip if we updated too recently.
		 */
		if ( false !== get_transient( self::RATE_LIMIT_KEY ) ) {
			return;
		}

		$embed      = self::build_embed( $data, $timestamp );
		$message_id = get_option( 'psm_discord_message_id', '' );
		$payload    = wp_json_encode( array( 'embeds' => array( $embed ) ) );

		if ( '' !== $message_id ) {
			$result = self::patch_message( $webhook_url, $message_id, $payload );

			if ( is_wp_error( $result ) || 404 === $result ) {
				/*
				 * Message was deleted or not found — create a new one.
				 */
				$new_id = self::post_message( $webhook_url, $payload );

				if ( ! is_wp_error( $new_id ) && '' !== $new_id ) {
					update_option( 'psm_discord_message_id', sanitize_text_field( $new_id ), false );
				}
			}
		} else {
			$new_id = self::post_message( $webhook_url, $payload );

			if ( ! is_wp_error( $new_id ) && '' !== $new_id ) {
				update_option( 'psm_discord_message_id', sanitize_text_field( $new_id ), false );
			}
		}

		/*
		 * Set rate-limit lock.
		 */
		$interval = self::get_interval();
		set_transient( self::RATE_LIMIT_KEY, '1', $interval );
	}

	/**
	 * Build a Discord Embed array from leaderboard data and settings.
	 *
	 * @param array  $data      Leaderboard data.
	 * @param string $timestamp UTC timestamp.
	 * @return array Discord embed structure.
	 */
	public static function build_embed( $data, $timestamp ) {
		$title       = get_option( 'psm_discord_embed_title', "\xF0\x9F\x8F\x86 Live Leaderboard" );
		$color_hex   = get_option( 'psm_discord_embed_color', '#FFD700' );
		$max_players = absint( get_option( 'psm_discord_max_players', 10 ) );
		$fields_cfg  = self::get_enabled_fields();

		$color = hexdec( ltrim( $color_hex, '#' ) );

		/*
		 * Build description lines from player data.
		 */
		$lines = self::build_player_lines( $data, $fields_cfg, $max_players );

		$description = ! empty( $lines ) ? implode( "\n", $lines ) : 'Keine Spielerdaten vorhanden.';

		/*
		 * Footer with meta info.
		 */
		$footer_parts = array();

		if ( ! empty( $fields_cfg['players_online'] ) && isset( $data['playerOnlineCounter'] ) ) {
			/* translators: %d: number of online players */
			$footer_parts[] = sprintf(
				"\xF0\x9F\x91\xA5 %d Online",
				absint( $data['playerOnlineCounter'] )
			);
		}

		if ( ! empty( $fields_cfg['total_players'] ) && isset( $data['totalPlayers'] ) ) {
			/* translators: %d: total players */
			$footer_parts[] = sprintf(
				"%d Gesamt",
				absint( $data['totalPlayers'] )
			);
		}

		$footer_text = ! empty( $footer_parts )
			? implode( " \xC2\xB7 ", $footer_parts )
			: '';

		if ( '' !== $timestamp ) {
			$formatted = mysql2date( 'Y-m-d H:i', $timestamp, false );
			$ts_part   = 'Aktualisiert: ' . $formatted . ' UTC';
			$footer_text = '' !== $footer_text
				? $footer_text . "\n" . $ts_part
				: $ts_part;
		}

		$embed = array(
			'title'       => sanitize_text_field( $title ),
			'description' => $description,
			'color'       => $color,
		);

		if ( '' !== $footer_text ) {
			$embed['footer'] = array( 'text' => $footer_text );
		}

		return $embed;
	}

	/**
	 * Build formatted player lines for the embed description.
	 *
	 * @param array $data       Leaderboard data.
	 * @param array $fields_cfg Enabled fields configuration.
	 * @param int   $max        Maximum players to show.
	 * @return array Array of formatted lines.
	 */
	private static function build_player_lines( $data, $fields_cfg, $max ) {
		$lines   = array();
		$players = self::extract_players( $data );

		if ( empty( $players ) ) {
			return $lines;
		}

		$players = array_slice( $players, 0, $max );

		foreach ( $players as $i => $player ) {
			$name = isset( $player['name'] ) ? sanitize_text_field( $player['name'] ) : 'Unknown';

			if ( $i < 3 ) {
				$prefix = self::$medals[ $i ];
			} else {
				$prefix = '`#' . ( $i + 1 ) . '`';
			}

			$stats = self::build_stat_parts( $player, $fields_cfg );
			$line  = $prefix . ' **' . $name . '**';

			if ( ! empty( $stats ) ) {
				$line .= " \xE2\x80\x94 " . implode( " \xC2\xB7 ", $stats );
			}

			$lines[] = $line;
		}

		return $lines;
	}

	/**
	 * Build stat display parts for a single player based on enabled fields.
	 *
	 * @param array $player     Player data array.
	 * @param array $fields_cfg Enabled fields.
	 * @return array Array of formatted stat strings.
	 */
	private static function build_stat_parts( $player, $fields_cfg ) {
		$parts = array();

		$field_map = array(
			'kills'             => array( 'key' => 'kills',           'label' => 'Kills',       'format' => 'int' ),
			'deaths'            => array( 'key' => 'deaths',          'label' => 'Deaths',      'format' => 'int' ),
			'kd_ratio'          => array( 'key' => 'kdRatio',         'label' => 'K/D',         'format' => 'float' ),
			'zombie_kills'      => array( 'key' => 'zombieKills',     'label' => 'Zombie Kills','format' => 'int' ),
			'longest_kill'      => array( 'key' => 'longestKill',     'label' => 'Longest',     'format' => 'distance' ),
			'playtime'          => array( 'key' => 'playtime',        'label' => 'Playtime',    'format' => 'hours' ),
			'distance_traveled' => array( 'key' => 'distanceTraveled','label' => 'Distanz',     'format' => 'distance' ),
		);

		foreach ( $field_map as $cfg_key => $meta ) {
			if ( empty( $fields_cfg[ $cfg_key ] ) ) {
				continue;
			}

			if ( ! isset( $player[ $meta['key'] ] ) ) {
				continue;
			}

			$val = $player[ $meta['key'] ];

			switch ( $meta['format'] ) {
				case 'int':
					$formatted = number_format_i18n( absint( $val ) );
					break;
				case 'float':
					$formatted = number_format( floatval( $val ), 1 );
					break;
				case 'distance':
					$formatted = number_format_i18n( absint( $val ) ) . 'm';
					break;
				case 'hours':
					$hours     = round( floatval( $val ) / 3600, 1 );
					$formatted = $hours . 'h';
					break;
				default:
					$formatted = sanitize_text_field( (string) $val );
			}

			$parts[] = $formatted . ' ' . $meta['label'];
		}

		/*
		 * Custom fields.
		 */
		$custom_fields = get_option( 'psm_discord_custom_fields', '' );

		if ( '' !== $custom_fields ) {
			$customs = array_map( 'trim', explode( ',', $custom_fields ) );

			foreach ( $customs as $custom_key ) {
				if ( '' !== $custom_key && isset( $player[ $custom_key ] ) ) {
					$val     = $player[ $custom_key ];
					$display = is_numeric( $val ) ? number_format_i18n( $val ) : sanitize_text_field( (string) $val );
					$parts[] = $display . ' ' . sanitize_text_field( $custom_key );
				}
			}
		}

		return $parts;
	}

	/**
	 * Extract the player list from leaderboard data.
	 *
	 * Supports multiple data shapes:
	 * - { "topPVPPlayers": [...] }
	 * - { "topPVEPlayers": [...] }
	 * - { "players": [...] }
	 * - Direct array of players
	 *
	 * @param array $data Leaderboard data.
	 * @return array Flat array of player entries.
	 */
	private static function extract_players( $data ) {
		/*
		 * Check known keys in priority order.
		 */
		$source_key = get_option( 'psm_discord_data_source', 'auto' );

		if ( 'auto' !== $source_key && isset( $data[ $source_key ] ) && is_array( $data[ $source_key ] ) ) {
			return $data[ $source_key ];
		}

		if ( isset( $data['topPVPPlayers'] ) && is_array( $data['topPVPPlayers'] ) ) {
			return $data['topPVPPlayers'];
		}

		if ( isset( $data['topPVEPlayers'] ) && is_array( $data['topPVEPlayers'] ) ) {
			return $data['topPVEPlayers'];
		}

		if ( isset( $data['players'] ) && is_array( $data['players'] ) ) {
			return $data['players'];
		}

		/*
		 * If data itself is an indexed array of player objects, use it.
		 */
		if ( isset( $data[0] ) && is_array( $data[0] ) ) {
			return $data;
		}

		return array();
	}

	/**
	 * Get the map of enabled display fields from settings.
	 *
	 * @return array Associative array of field_key => bool.
	 */
	private static function get_enabled_fields() {
		$defaults = array(
			'kills'             => '1',
			'deaths'            => '',
			'kd_ratio'          => '1',
			'zombie_kills'      => '',
			'longest_kill'      => '',
			'playtime'          => '',
			'distance_traveled' => '',
			'players_online'    => '1',
			'total_players'     => '1',
		);

		$stored = get_option( 'psm_discord_fields', array() );

		if ( ! is_array( $stored ) || empty( $stored ) ) {
			return $defaults;
		}

		return wp_parse_args( $stored, $defaults );
	}

	/**
	 * Get the configured minimum interval between Discord updates.
	 *
	 * @return int Seconds.
	 */
	private static function get_interval() {
		$interval = absint( get_option( 'psm_discord_interval', self::MIN_INTERVAL ) );
		return max( self::MIN_INTERVAL, $interval );
	}

	/**
	 * POST a new message to the Discord Webhook.
	 *
	 * @param string $webhook_url Full webhook URL.
	 * @param string $payload     JSON payload.
	 * @return string|WP_Error Message ID on success, WP_Error on failure.
	 */
	private static function post_message( $webhook_url, $payload ) {
		$url = add_query_arg( 'wait', 'true', $webhook_url );

		$response = wp_remote_post(
			$url,
			array(
				'headers' => array( 'Content-Type' => 'application/json' ),
				'body'    => $payload,
				'timeout' => 15,
			)
		);

		if ( is_wp_error( $response ) ) {
			return $response;
		}

		$code = wp_remote_retrieve_response_code( $response );

		if ( 429 === $code ) {
			self::handle_rate_limit( $response );
			return new WP_Error( 'psm_rate_limited', 'Discord rate limit hit.' );
		}

		if ( $code < 200 || $code >= 300 ) {
			return new WP_Error(
				'psm_discord_error',
				sprintf( 'Discord POST returned HTTP %d', $code )
			);
		}

		$body = json_decode( wp_remote_retrieve_body( $response ), true );

		if ( is_array( $body ) && isset( $body['id'] ) ) {
			return sanitize_text_field( $body['id'] );
		}

		return new WP_Error( 'psm_no_message_id', 'No message ID in Discord response.' );
	}

	/**
	 * PATCH (edit) an existing Discord Webhook message.
	 *
	 * @param string $webhook_url Full webhook URL.
	 * @param string $message_id  Discord message ID to edit.
	 * @param string $payload     JSON payload.
	 * @return true|int|WP_Error True on success, 404 if message not found, WP_Error otherwise.
	 */
	private static function patch_message( $webhook_url, $message_id, $payload ) {
		$url = trailingslashit( $webhook_url ) . 'messages/' . sanitize_text_field( $message_id );

		$response = wp_remote_request(
			$url,
			array(
				'method'  => 'PATCH',
				'headers' => array( 'Content-Type' => 'application/json' ),
				'body'    => $payload,
				'timeout' => 15,
			)
		);

		if ( is_wp_error( $response ) ) {
			return $response;
		}

		$code = wp_remote_retrieve_response_code( $response );

		if ( 404 === $code ) {
			return 404;
		}

		if ( 429 === $code ) {
			self::handle_rate_limit( $response );
			return new WP_Error( 'psm_rate_limited', 'Discord rate limit hit.' );
		}

		if ( $code >= 200 && $code < 300 ) {
			return true;
		}

		return new WP_Error(
			'psm_discord_error',
			sprintf( 'Discord PATCH returned HTTP %d', $code )
		);
	}

	/**
	 * Handle Discord rate-limit response by extending the lock transient.
	 *
	 * @param array $response wp_remote response array.
	 * @return void
	 */
	private static function handle_rate_limit( $response ) {
		$body = json_decode( wp_remote_retrieve_body( $response ), true );

		$retry_after = 30;

		if ( is_array( $body ) && isset( $body['retry_after'] ) ) {
			$retry_after = max( 1, (int) ceil( floatval( $body['retry_after'] ) ) );
		}

		set_transient( self::RATE_LIMIT_KEY, '1', $retry_after );
	}

	/**
	 * Send a test message to verify webhook configuration.
	 *
	 * @return true|WP_Error True on success, WP_Error on failure.
	 */
	public static function send_test() {
		$webhook_url = get_option( 'psm_discord_webhook_url', '' );

		if ( '' === $webhook_url ) {
			return new WP_Error( 'psm_no_webhook', __( 'Keine Webhook-URL konfiguriert.', 'psyerns-mods' ) );
		}

		$embed = array(
			'title'       => "\xE2\x9C\x85 Psyerns Mods — Webhook Test",
			'description' => 'Die Verbindung zum Discord-Webhook funktioniert. Das Leaderboard wird hier angezeigt, sobald Daten vom DayZ-Server eingehen.',
			'color'       => hexdec( 'FFD700' ),
			'footer'      => array(
				'text' => 'Psyerns Mods Showreel v' . ( defined( 'PSM_VERSION' ) ? PSM_VERSION : '1.0.0' ),
			),
		);

		$payload = wp_json_encode( array( 'embeds' => array( $embed ) ) );
		$result  = self::post_message( $webhook_url, $payload );

		if ( is_wp_error( $result ) ) {
			return $result;
		}

		/*
		 * Store the message ID so future leaderboard updates
		 * will edit this message instead of creating a new one.
		 */
		if ( is_string( $result ) && '' !== $result ) {
			update_option( 'psm_discord_message_id', $result, false );
		}

		return true;
	}
}

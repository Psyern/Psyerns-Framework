<?php
/**
 * Post meta handling for Psyerns Mods.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Registers, saves and retrieves post meta for the psyern_mod post type.
 */
class PSM_Meta {

	/**
	 * Steam Workshop URL.
	 *
	 * @var string
	 */
	const STEAM_URL = '_psm_steam_url';

	/**
	 * Discord invite / webhook URL.
	 *
	 * @var string
	 */
	const DISCORD_URL = '_psm_discord_url';

	/**
	 * Whether the mod is free (checkbox).
	 *
	 * @var string
	 */
	const IS_FREE = '_psm_is_free';

	/**
	 * Base price of the mod.
	 *
	 * @var string
	 */
	const PRICE = '_psm_price';

	/**
	 * Whether a repack option is available.
	 *
	 * @var string
	 */
	const HAS_REPACK = '_psm_has_repack';

	/**
	 * Price for the repack option.
	 *
	 * @var string
	 */
	const REPACK_PRICE = '_psm_repack_price';

	/**
	 * Whether source code is available for purchase.
	 *
	 * @var string
	 */
	const HAS_SOURCE = '_psm_has_source';

	/**
	 * Price for the source code.
	 *
	 * @var string
	 */
	const SOURCE_PRICE = '_psm_source_price';

	/**
	 * Cached Steam API data (serialized array).
	 *
	 * @var string
	 */
	const STEAM_DATA = '_psm_steam_data';

	/**
	 * Display order (lower = first).
	 *
	 * @var string
	 */
	const ORDER = '_psm_order';

	/**
	 * Nonce action string.
	 *
	 * @var string
	 */
	const NONCE_ACTION = 'psm_meta_nonce';

	/**
	 * Nonce field name.
	 *
	 * @var string
	 */
	const NONCE_FIELD = 'psm_meta_nonce_field';

	/**
	 * Register hooks for saving meta.
	 *
	 * @return void
	 */
	public static function register() {
		add_action( 'save_post_psyern_mod', array( __CLASS__, 'save' ), 10, 1 );
	}

	/**
	 * Save post meta on psyern_mod save.
	 *
	 * Performs nonce verification, capability check and autosave guard
	 * before sanitizing and persisting each meta field.
	 *
	 * @param int $post_id The post ID being saved.
	 * @return void
	 */
	public static function save( $post_id ) {
		/*
		 * Nonce verification.
		 */
		if ( ! isset( $_POST[ self::NONCE_FIELD ] ) ) {
			return;
		}

		if ( ! wp_verify_nonce( sanitize_text_field( wp_unslash( $_POST[ self::NONCE_FIELD ] ) ), self::NONCE_ACTION ) ) {
			return;
		}

		/*
		 * Autosave guard.
		 */
		if ( defined( 'DOING_AUTOSAVE' ) && DOING_AUTOSAVE ) {
			return;
		}

		/*
		 * Capability check.
		 */
		if ( ! current_user_can( 'edit_post', $post_id ) ) {
			return;
		}

		/*
		 * Steam URL — must contain the expected domain path.
		 */
		if ( isset( $_POST['psm_steam_url'] ) ) {
			$steam_url_raw = sanitize_text_field( wp_unslash( $_POST['psm_steam_url'] ) );
			$steam_url     = esc_url_raw( $steam_url_raw );

			if ( '' !== $steam_url && false === strpos( $steam_url, 'steamcommunity.com/sharedfiles/filedetails' ) ) {
				$steam_url = '';
			}

			update_post_meta( $post_id, self::STEAM_URL, $steam_url );
		}

		/*
		 * Discord URL.
		 */
		if ( isset( $_POST['psm_discord_url'] ) ) {
			$discord_url = esc_url_raw( sanitize_text_field( wp_unslash( $_POST['psm_discord_url'] ) ) );
			update_post_meta( $post_id, self::DISCORD_URL, $discord_url );
		}

		/*
		 * Is Free (checkbox).
		 */
		$is_free = isset( $_POST['psm_is_free'] ) ? '1' : '';
		update_post_meta( $post_id, self::IS_FREE, $is_free );

		/*
		 * Price.
		 */
		if ( isset( $_POST['psm_price'] ) ) {
			$price = round( floatval( sanitize_text_field( wp_unslash( $_POST['psm_price'] ) ) ), 2 );
			$price = max( 0, $price );
			update_post_meta( $post_id, self::PRICE, $price );
		}

		/*
		 * Has Repack (checkbox).
		 */
		$has_repack = isset( $_POST['psm_has_repack'] ) ? '1' : '';
		update_post_meta( $post_id, self::HAS_REPACK, $has_repack );

		/*
		 * Repack Price.
		 */
		if ( isset( $_POST['psm_repack_price'] ) ) {
			$repack_price = round( floatval( sanitize_text_field( wp_unslash( $_POST['psm_repack_price'] ) ) ), 2 );
			$repack_price = max( 0, $repack_price );
			update_post_meta( $post_id, self::REPACK_PRICE, $repack_price );
		}

		/*
		 * Has Source (checkbox).
		 */
		$has_source = isset( $_POST['psm_has_source'] ) ? '1' : '';
		update_post_meta( $post_id, self::HAS_SOURCE, $has_source );

		/*
		 * Source Price.
		 */
		if ( isset( $_POST['psm_source_price'] ) ) {
			$source_price = round( floatval( sanitize_text_field( wp_unslash( $_POST['psm_source_price'] ) ) ), 2 );
			$source_price = max( 0, $source_price );
			update_post_meta( $post_id, self::SOURCE_PRICE, $source_price );
		}

		/*
		 * Display Order.
		 */
		if ( isset( $_POST['psm_order'] ) ) {
			$order = absint( sanitize_text_field( wp_unslash( $_POST['psm_order'] ) ) );
			update_post_meta( $post_id, self::ORDER, $order );
		}
	}

	/**
	 * Retrieve a single meta value for a psyern_mod post.
	 *
	 * @param int    $post_id The post ID.
	 * @param string $key     One of the class constants (e.g. PSM_Meta::STEAM_URL).
	 * @return mixed The meta value (single).
	 */
	public static function get( $post_id, $key ) {
		return get_post_meta( $post_id, $key, true );
	}
}

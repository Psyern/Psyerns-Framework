<?php
/**
 * Uninstall routines for Psyerns Mods Showreel.
 *
 * Removes all plugin data: custom post type posts, post meta,
 * plugin options and cached transients.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'WP_UNINSTALL_PLUGIN' ) ) {
	exit;
}

global $wpdb;

/*
 * 1. Delete all posts of type 'psyern_mod' (including revisions and meta).
 */
$post_ids = $wpdb->get_col(
	$wpdb->prepare(
		"SELECT ID FROM {$wpdb->posts} WHERE post_type = %s",
		'psyern_mod'
	)
);

if ( ! empty( $post_ids ) ) {
	foreach ( $post_ids as $post_id ) {
		wp_delete_post( (int) $post_id, true );
	}
}

/*
 * 2. Delete any orphaned postmeta rows whose key starts with '_psm_'.
 */
$wpdb->query(
	$wpdb->prepare(
		"DELETE FROM {$wpdb->postmeta} WHERE meta_key LIKE %s",
		'\_psm\_%'
	)
);

/*
 * 3. Delete plugin options.
 */
delete_option( 'psm_global_discord_url' );
delete_option( 'psm_steam_api_key' );
delete_option( 'psm_discount_enabled' );
delete_option( 'psm_discount_percent' );
delete_option( 'psm_discount_label' );
delete_option( 'psm_discord_enabled' );
delete_option( 'psm_discord_webhook_url' );
delete_option( 'psm_discord_embed_title' );
delete_option( 'psm_discord_embed_color' );
delete_option( 'psm_discord_max_players' );
delete_option( 'psm_discord_interval' );
delete_option( 'psm_discord_data_source' );
delete_option( 'psm_discord_fields' );
delete_option( 'psm_discord_custom_fields' );
delete_option( 'psm_discord_message_id' );
delete_option( 'psm_leaderboard_data' );
delete_option( 'psm_leaderboard_updated_at' );
delete_option( 'psm_leaderboard_public' );
delete_transient( 'psm_discord_rate_lock' );

/*
 * 4. Delete all transients with prefix 'psm_steam_'.
 *
 * WordPress stores transients as '_transient_<key>' and
 * '_transient_timeout_<key>' in the options table.
 */
$wpdb->query(
	$wpdb->prepare(
		"DELETE FROM {$wpdb->options} WHERE option_name LIKE %s OR option_name LIKE %s",
		'\_transient\_psm\_steam\_%',
		'\_transient\_timeout\_psm\_steam\_%'
	)
);

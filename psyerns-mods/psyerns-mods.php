<?php
/**
 * Plugin Name: Psyerns Mods Showreel
 * Plugin URI:  https://deadmans-echo.de/
 * Description: Showreel für Steam Workshop Mods von Psyern. Verwalte Mods mit Preisen, Repack- und Source-Optionen.
 * Version:     1.0.0
 * Author:      Psyern
 * Author URI:  https://deadmans-echo.de/
 * Text Domain: psyerns-mods
 * License:     GPL-2.0+
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Plugin version constant.
 *
 * @var string
 */
define( 'PSM_VERSION', '1.0.0' );

/**
 * Plugin directory path (with trailing slash).
 *
 * @var string
 */
define( 'PSM_PLUGIN_DIR', plugin_dir_path( __FILE__ ) );

/**
 * Plugin URL (with trailing slash).
 *
 * @var string
 */
define( 'PSM_PLUGIN_URL', plugin_dir_url( __FILE__ ) );

/**
 * Plugin basename for hooks.
 *
 * @var string
 */
define( 'PSM_PLUGIN_BASENAME', plugin_basename( __FILE__ ) );

/*
 * Require class files.
 */
require_once PSM_PLUGIN_DIR . 'includes/class-psm-post-type.php';
require_once PSM_PLUGIN_DIR . 'includes/class-psm-meta.php';
require_once PSM_PLUGIN_DIR . 'includes/class-psm-cache.php';
require_once PSM_PLUGIN_DIR . 'includes/class-psm-steam-api.php';
require_once PSM_PLUGIN_DIR . 'includes/class-psm-leaderboard-data.php';
require_once PSM_PLUGIN_DIR . 'includes/class-psm-discord-leaderboard.php';

/*
 * Require admin class files if they exist.
 */
if ( is_admin() ) {
	$psm_admin_file = PSM_PLUGIN_DIR . 'admin/class-psm-admin.php';
	if ( file_exists( $psm_admin_file ) ) {
		require_once $psm_admin_file;
	}
}

/*
 * Require public class files.
 */
require_once PSM_PLUGIN_DIR . 'public/class-psm-shortcode.php';

/**
 * Plugin activation callback.
 *
 * Registers the custom post type and flushes rewrite rules so that
 * any permastruct changes take effect immediately.
 *
 * @return void
 */
function psm_activate() {
	PSM_Post_Type::register();
	flush_rewrite_rules();
}
register_activation_hook( __FILE__, 'psm_activate' );

/**
 * Plugin deactivation callback.
 *
 * Flushes rewrite rules to remove custom post type rules.
 *
 * @return void
 */
function psm_deactivate() {
	flush_rewrite_rules();
}
register_deactivation_hook( __FILE__, 'psm_deactivate' );

/**
 * Load plugin textdomain for translations.
 *
 * @return void
 */
function psm_load_textdomain() {
	load_plugin_textdomain( 'psyerns-mods', false, dirname( PSM_PLUGIN_BASENAME ) . '/languages' );
}
add_action( 'plugins_loaded', 'psm_load_textdomain' );

/**
 * Bootstrap core components on init.
 *
 * @return void
 */
function psm_init() {
	PSM_Post_Type::register();
	PSM_Meta::register();
	PSM_Cache::register();
	PSM_Leaderboard_Data::register();
	PSM_Discord_Leaderboard::register();

	$shortcode = new PSM_Shortcode();
	$shortcode->register();
}
add_action( 'init', 'psm_init' );

<?php
/**
 * Plugin Name: Psyerns Framework
 * Plugin URI:  https://deadmansecho.com
 * Description: DayZ Server Integration — Leaderboard, Server Status, Whitelist, KillFeed
 * Version:     1.0.0
 * Author:      Psyern
 * Author URI:  https://deadmansecho.com
 * License:     MIT
 * License URI: https://opensource.org/licenses/MIT
 * Text Domain: psyerns-framework
 * Domain Path: /languages
 * Requires at least: 5.8
 * Requires PHP: 7.4
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

define( 'PF_VERSION', '1.0.0' );
define( 'PF_DB_VERSION', '1.1.0' );
define( 'PF_PLUGIN_DIR', plugin_dir_path( __FILE__ ) );
define( 'PF_PLUGIN_URL', plugin_dir_url( __FILE__ ) );
define( 'PF_PLUGIN_BASENAME', plugin_basename( __FILE__ ) );

require_once PF_PLUGIN_DIR . 'includes/class-pf-database.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-auth.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-steam.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-api.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-server-status.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-leaderboard.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-whitelist.php';
require_once PF_PLUGIN_DIR . 'includes/class-pf-players.php';
require_once PF_PLUGIN_DIR . 'admin/class-pf-admin.php';
require_once PF_PLUGIN_DIR . 'public/class-pf-shortcodes.php';

// Leaderboard system (v2 themed UI).
require_once PF_PLUGIN_DIR . 'includes/class-psyern-database.php';
require_once PF_PLUGIN_DIR . 'includes/class-psyern-api.php';
require_once PF_PLUGIN_DIR . 'includes/class-psyern-shortcode.php';
require_once PF_PLUGIN_DIR . 'includes/class-psyern-main.php';
require_once PF_PLUGIN_DIR . 'admin/class-psyern-admin.php';

register_activation_hook( __FILE__, 'pf_activate' );
register_deactivation_hook( __FILE__, 'pf_deactivate' );

/**
 * Runs on plugin activation. Creates DB tables and stores version.
 *
 * @return void
 */
function pf_activate() {
	PF_Database::create_tables();
	Psyern_Database::create_tables();
	update_option( 'pf_db_version', PF_DB_VERSION );
}

/**
 * Runs on plugin deactivation. Cleans up transients only.
 *
 * @return void
 */
function pf_deactivate() {
	delete_transient( 'pf_server_status' );
	delete_transient( 'pf_leaderboard_meta' );
}

/**
 * Check DB version on every load and upgrade schema if needed.
 *
 * @return void
 */
function pf_check_db_version() {
	if ( PF_DB_VERSION !== get_option( 'pf_db_version' ) ) {
		PF_Database::create_tables();
		Psyern_Database::create_tables();
		update_option( 'pf_db_version', PF_DB_VERSION );
	}
}
add_action( 'plugins_loaded', 'pf_check_db_version' );

/**
 * Load plugin textdomain for i18n.
 *
 * @return void
 */
function pf_load_textdomain() {
	load_plugin_textdomain( 'psyerns-framework', false, dirname( PF_PLUGIN_BASENAME ) . '/languages' );
}
add_action( 'init', 'pf_load_textdomain' );

/**
 * Register REST API routes.
 *
 * @return void
 */
function pf_register_rest_routes() {
	$api = new PF_Api();
	$api->register_routes();
}
add_action( 'rest_api_init', 'pf_register_rest_routes' );

/**
 * Initialize admin hooks.
 *
 * @return void
 */
function pf_admin_init() {
	$admin = new PF_Admin();
	$admin->init();
}
add_action( 'plugins_loaded', 'pf_admin_init' );

/**
 * Register shortcodes.
 *
 * @return void
 */
function pf_register_shortcodes() {
	$shortcodes = new PF_Shortcodes();
	$shortcodes->register();
}
add_action( 'init', 'pf_register_shortcodes' );

/**
 * Initialize the themed leaderboard system.
 *
 * @return void
 */
function psyern_init_leaderboard() {
	if ( '1' === get_option( 'psyern_enable_leaderboard', '1' ) ) {
		$main = new Psyern_Main();
		$main->init();
	}
}
add_action( 'plugins_loaded', 'psyern_init_leaderboard' );

<?php
if ( ! defined( 'WP_UNINSTALL_PLUGIN' ) ) {
	exit;
}

require_once plugin_dir_path( __FILE__ ) . 'includes/class-pf-database.php';
require_once plugin_dir_path( __FILE__ ) . 'includes/class-psyern-database.php';

PF_Database::drop_tables();
Psyern_Database::drop_tables();

delete_option( 'pf_api_key' );
delete_option( 'pf_steam_api_key' );
delete_option( 'pf_default_theme' );
delete_option( 'pf_db_version' );
delete_option( 'psyern_theme' );
delete_option( 'psyern_enable_leaderboard' );
delete_option( 'psyern_enable_pvp' );
delete_option( 'psyern_enable_pve' );
delete_option( 'psyern_enable_top3' );
delete_option( 'psyern_enable_avatars' );

delete_transient( 'pf_server_status' );
delete_transient( 'pf_leaderboard_meta' );

global $wpdb;
$wpdb->query(
	"DELETE FROM {$wpdb->options} WHERE option_name LIKE '_transient_pf_avatar_%' OR option_name LIKE '_transient_timeout_pf_avatar_%'"
);

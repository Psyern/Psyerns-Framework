<?php
if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

class PF_Whitelist {

	public function handle_check( WP_REST_Request $request ) {
		global $wpdb;
		$steam_id = sanitize_text_field( $request->get_param( 'steam_id' ) );
		$table    = PF_Database::get_table_name( 'whitelist' );

		$exists = $wpdb->get_var( $wpdb->prepare(
			"SELECT COUNT(*) FROM {$table} WHERE steam_id = %s",
			$steam_id
		) );

		// DayZ client parses via string matching — must be exactly {"whitelisted": true/false}
		return new WP_REST_Response( array( 'whitelisted' => ( (int) $exists > 0 ) ), 200 );
	}

	public function handle_add( WP_REST_Request $request ) {
		global $wpdb;
		$data     = $request->get_json_params();
		$steam_id = sanitize_text_field( $data['steamId'] ?? '' );
		$name     = sanitize_text_field( $data['name'] ?? '' );
		$table    = PF_Database::get_table_name( 'whitelist' );

		if ( empty( $steam_id ) ) {
			return new WP_REST_Response(
				array( 'success' => false, 'message' => __( 'Missing steamId', 'psyerns-framework' ) ),
				400
			);
		}

		$wpdb->query( $wpdb->prepare(
			"INSERT IGNORE INTO {$table} (steam_id, name, added_at) VALUES (%s, %s, %s)",
			$steam_id,
			$name,
			current_time( 'mysql' )
		) );

		return new WP_REST_Response( array( 'success' => true ), 200 );
	}

	public function handle_remove( WP_REST_Request $request ) {
		global $wpdb;
		$data     = $request->get_json_params();
		$steam_id = sanitize_text_field( $data['steamId'] ?? '' );
		$table    = PF_Database::get_table_name( 'whitelist' );

		if ( empty( $steam_id ) ) {
			return new WP_REST_Response(
				array( 'success' => false, 'message' => __( 'Missing steamId', 'psyerns-framework' ) ),
				400
			);
		}

		$wpdb->delete( $table, array( 'steam_id' => $steam_id ) );

		return new WP_REST_Response( array( 'success' => true ), 200 );
	}
}

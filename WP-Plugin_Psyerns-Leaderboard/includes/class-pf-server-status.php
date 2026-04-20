<?php
/**
 * Server status endpoint handler.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Server_Status
 *
 * Receives server status POST from DayZ and stores as transient.
 * Provides public GET for frontend display.
 */
class PF_Server_Status {

	/**
	 * Handle incoming server status POST.
	 *
	 * @param WP_REST_Request $request The request object.
	 * @return WP_REST_Response
	 */
	public function handle_post( WP_REST_Request $request ) {
		$data = $request->get_json_params();

		$status = array(
			'serverName'    => sanitize_text_field( $data['serverName'] ?? '' ),
			'playerCount'   => absint( $data['playerCount'] ?? 0 ),
			'uptimeSeconds' => absint( $data['uptimeSeconds'] ?? 0 ),
			'mapName'       => sanitize_text_field( $data['mapName'] ?? '' ),
			'dayTime'       => sanitize_text_field( $data['dayTime'] ?? '' ),
			'timestamp'     => sanitize_text_field( $data['timestamp'] ?? '' ),
			'received_at'   => current_time( 'mysql' ),
		);

		/**
		 * Filter server status data before saving.
		 *
		 * @param array $status Sanitized status data.
		 * @param array $data   Raw request data.
		 */
		$status = apply_filters( 'psyerns-framework/server_status_data', $status, $data );

		set_transient( 'pf_server_status', $status, 120 );

		return new WP_REST_Response( array( 'success' => true ), 200 );
	}

	/**
	 * Handle public GET for server status.
	 *
	 * @return WP_REST_Response
	 */
	public function handle_get_public() {
		$status = get_transient( 'pf_server_status' );

		if ( false === $status ) {
			return new WP_REST_Response(
				array( 'error' => __( 'No status data available', 'psyerns-framework' ) ),
				404
			);
		}

		return new WP_REST_Response( $status, 200 );
	}
}

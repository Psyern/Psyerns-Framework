<?php
/**
 * API key authentication for REST endpoints.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Auth
 *
 * Validates the api_key query parameter against the stored option.
 */
class PF_Auth {

	/**
	 * Validate API key from a REST request.
	 *
	 * @param WP_REST_Request $request The incoming REST request.
	 * @return true|WP_Error True if valid, WP_Error otherwise.
	 */
	public static function validate_api_key( WP_REST_Request $request ) {
		$api_key = $request->get_param( 'api_key' );
		$stored  = get_option( 'pf_api_key', '' );

		if ( empty( $stored ) || $api_key !== $stored ) {
			return new WP_Error(
				'unauthorized',
				__( 'Invalid API key', 'psyerns-framework' ),
				array( 'status' => 401 )
			);
		}

		return true;
	}
}

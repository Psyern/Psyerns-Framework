<?php
/**
 * Leaderboard shortcode registration and rendering.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class Psyern_Shortcode
 *
 * Registers [psyern_leaderboard] and [psyern_top3] shortcodes.
 * Enqueues theme-specific CSS and shared JS only on pages with the shortcode.
 */
class Psyern_Shortcode {

	/**
	 * Whether assets have been enqueued this request.
	 *
	 * @var bool
	 */
	private $enqueued = false;

	/**
	 * Register shortcodes.
	 *
	 * @return void
	 */
	public function register() {
		add_shortcode( 'psyern_leaderboard', array( $this, 'render_leaderboard' ) );
		add_shortcode( 'psyern_top3', array( $this, 'render_top3' ) );
	}

	/**
	 * Enqueue leaderboard assets.
	 *
	 * @param string $theme Active theme slug.
	 * @return void
	 */
	private function enqueue( $theme ) {
		if ( true === $this->enqueued ) {
			return;
		}
		$this->enqueued = true;

		$base = PF_PLUGIN_URL . 'public/';

		wp_enqueue_style(
			'psyern-leaderboard',
			$base . 'css/psyern-leaderboard.css',
			array(),
			PF_VERSION
		);

		// Player Detail Modal CSS (only when modal is active for this render).
		if ( ! empty( $GLOBALS['pf_player_details_enabled'] ) ) {
			wp_enqueue_style(
				'psyern-pdm',
				$base . 'css/player-detail.css',
				array( 'psyern-leaderboard' ),
				PF_VERSION
			);
		}

		// Load ALL theme CSS — scoped by .psyern-lb--{theme}, only active one applies.
		$all_themes = array( 'military', 'ash', 'ops', 'outbreak', 'cyberpunk', 'stalker', 'inferno', 'frostbite', 'bubblegum' );
		foreach ( $all_themes as $t ) {
			wp_enqueue_style(
				'psyern-theme-' . $t,
				$base . 'css/psyern-theme-' . $t . '.css',
				array( 'psyern-leaderboard' ),
				PF_VERSION
			);
		}

		// Google Fonts per theme.
		$fonts = array(
			'military' => 'family=Share+Tech+Mono|Oswald:wght@400;700',
			'ash'      => 'family=Playfair+Display:wght@400;700|DM+Mono:wght@400',
			'ops'      => 'family=Orbitron:wght@400;700;900|Share+Tech+Mono',
			'outbreak' => 'family=Rajdhani:wght@400;600;700|Exo+2:wght@300;400;600|Inconsolata:wght@400;700',
			'cyberpunk' => 'family=Orbitron:wght@400;700;900|JetBrains+Mono:wght@400;700',
			'stalker'   => 'family=Courier+Prime:wght@400;700',
			'inferno'   => 'family=Teko:wght@400;500;700',
			'frostbite' => 'family=Nunito+Sans:wght@400;600;700;800',
			'bubblegum' => 'family=Quicksand:wght@300;400;500;600;700',
		);
		if ( isset( $fonts[ $theme ] ) ) {
			wp_enqueue_style(
				'psyern-fonts',
				'https://fonts.googleapis.com/css2?' . $fonts[ $theme ] . '&display=swap',
				array(),
				null // phpcs:ignore WordPress.WP.EnqueuedResourceParameters.MissingVersion
			);
		}

		wp_enqueue_script(
			'psyern-leaderboard',
			$base . 'js/psyern-leaderboard.js',
			array(),
			PF_VERSION,
			true
		);
		$allowed_pvp = PF_Admin::get_mode_allowed_columns( 'pvp' );
		$allowed_pve = PF_Admin::get_mode_allowed_columns( 'pve' );
		$enabled_pvp = get_option( 'pf_columns_pvp', '' );
		$enabled_pve = get_option( 'pf_columns_pve', '' );
		$cols_pvp    = $enabled_pvp ? json_decode( $enabled_pvp, true ) : $allowed_pvp;
		$cols_pve    = $enabled_pve ? json_decode( $enabled_pve, true ) : $allowed_pve;
		if ( ! is_array( $cols_pvp ) ) { $cols_pvp = $allowed_pvp; }
		if ( ! is_array( $cols_pve ) ) { $cols_pve = $allowed_pve; }
		// Hard-filter: PvE-only columns must never reach the PvP frontend.
		$cols_pvp = array_values( array_intersect( $cols_pvp, $allowed_pvp ) );
		$cols_pve = array_values( array_intersect( $cols_pve, $allowed_pve ) );

		wp_localize_script( 'psyern-leaderboard', 'psyernConfig', array(
			'ajaxUrl'             => admin_url( 'admin-ajax.php' ),
			'restUrl'             => esc_url_raw( rest_url( 'psyern/v1' ) ),
			'nonce'               => wp_create_nonce( 'psyern_leaderboard_nonce' ),
			'playerDetailsEnabled'     => ! empty( $GLOBALS['pf_player_details_enabled'] ),
			'playerDetailsShowAvatar'  => (bool) get_option( 'pf_player_details_show_avatar', '1' ),
			'playerDetailsMaxPerGroup' => (int)  get_option( 'pf_player_details_max_per_group', 20 ),
			'columns'             => array(
				'pvp' => $cols_pvp,
				'pve' => $cols_pve,
			),
			'columnDescriptions'  => class_exists( 'PF_Admin' ) ? PF_Admin::get_column_descriptions() : array(),
			'i18n'                => array(
				'kills'            => __( 'Kills', 'psyerns-framework' ),
				'deaths'           => __( 'Deaths', 'psyerns-framework' ),
				'kd'               => __( 'K/D', 'psyerns-framework' ),
				'faction'          => __( 'Faction', 'psyerns-framework' ),
				'boss'             => __( 'Boss', 'psyerns-framework' ),
				'reputation'       => __( 'Rep', 'psyerns-framework' ),
				'playtime'         => __( 'Playtime', 'psyerns-framework' ),
				'headshots'        => __( 'HS', 'psyerns-framework' ),
				'accuracy'         => __( 'Acc %', 'psyerns-framework' ),
				'longest_shot'     => __( 'Range', 'psyerns-framework' ),
				'distance'         => __( 'Dist', 'psyerns-framework' ),
				'distance_foot'    => __( 'Foot', 'psyerns-framework' ),
				'distance_vehicle' => __( 'Vehicle', 'psyerns-framework' ),
				'score'            => __( 'Score', 'psyerns-framework' ),
				'loading'          => __( 'Loading...', 'psyerns-framework' ),
				'error'            => __( 'Failed to load leaderboard data.', 'psyerns-framework' ),
			),
		) );
	}

	/**
	 * Resolve the active theme.
	 *
	 * @param string $override Shortcode attribute override.
	 * @return string Theme slug (military, neon, ash).
	 */
	private function resolve_theme( $override = '' ) {
		if ( ! empty( $override ) ) {
			return sanitize_text_field( $override );
		}
		return get_option( 'psyern_theme', 'military' );
	}

	/**
	 * Render [psyern_leaderboard] shortcode.
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function render_leaderboard( $atts ) {
		$atts = shortcode_atts( array(
			'mode'           => 'pvp',
			'limit'          => 10,
			'theme'          => '',
			'show_avatar'    => '1',
			'show_playtime'  => '1',
			'enable_details' => '',
		), $atts, 'psyern_leaderboard' );

		// Resolve player-details toggle (per-shortcode override or global option).
		if ( '' === $atts['enable_details'] ) {
			$details_enabled = (bool) get_option( 'pf_player_details_enabled', '1' );
		} else {
			$details_enabled = filter_var( $atts['enable_details'], FILTER_VALIDATE_BOOLEAN );
		}
		$GLOBALS['pf_player_details_enabled'] = $details_enabled;

		$theme = $this->resolve_theme( $atts['theme'] );
		$this->enqueue( $theme );

		$mode    = sanitize_text_field( $atts['mode'] );
		$limit   = min( absint( $atts['limit'] ), 100 );
		$players = Psyern_Database::get_leaderboard( $mode, $limit );
		$top3    = array_slice( $players, 0, 3 );
		$rest    = array_slice( $players, 3 );
		$week    = absint( gmdate( 'W' ) );
		$year    = absint( gmdate( 'Y' ) );

		ob_start();
		include PF_PLUGIN_DIR . 'public/templates/leaderboard-base.php';
		return ob_get_clean();
	}

	/**
	 * Render [psyern_top3] shortcode.
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function render_top3( $atts ) {
		$atts = shortcode_atts( array(
			'mode'  => 'pvp',
			'theme' => '',
		), $atts, 'psyern_top3' );

		$theme = $this->resolve_theme( $atts['theme'] );
		$this->enqueue( $theme );

		$mode = sanitize_text_field( $atts['mode'] );
		$top3 = Psyern_Database::get_top3( $mode );
		$week = absint( gmdate( 'W' ) );
		$year = absint( gmdate( 'Y' ) );

		ob_start();
		include PF_PLUGIN_DIR . 'public/templates/leaderboard-top3.php';
		return ob_get_clean();
	}
}

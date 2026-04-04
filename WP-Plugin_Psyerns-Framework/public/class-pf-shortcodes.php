<?php
/**
 * Shortcode registration and output.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Shortcodes
 *
 * Registers all frontend shortcodes. Each outputs a container div
 * with data attributes. JS loads data via fetch() and renders.
 * Output is always returned (not echoed).
 */
class PF_Shortcodes {

	/**
	 * Whether assets have been enqueued.
	 *
	 * @var bool
	 */
	private $enqueued = false;

	/**
	 * Register all shortcodes.
	 *
	 * @return void
	 */
	public function register() {
		add_shortcode( 'pf_leaderboard', array( $this, 'leaderboard' ) );
		add_shortcode( 'pf_server_status', array( $this, 'server_status' ) );
		add_shortcode( 'pf_top3_monthly', array( $this, 'top3_monthly' ) );
		add_shortcode( 'pf_top3_deadliest', array( $this, 'top3_deadliest' ) );
		add_shortcode( 'pf_top3_bosskills', array( $this, 'top3_bosskills' ) );
		add_shortcode( 'pf_player_card', array( $this, 'player_card' ) );
	}

	/**
	 * Enqueue frontend CSS and JS on first shortcode use.
	 *
	 * @return void
	 */
	private function enqueue_assets() {
		if ( true === $this->enqueued ) {
			return;
		}

		$this->enqueued = true;

		$theme = get_option( 'psyern_theme', 'military' );

		// Leaderboard base CSS.
		wp_enqueue_style( 'psyern-leaderboard', PF_PLUGIN_URL . 'public/css/psyern-leaderboard.css', array(), PF_VERSION );

		// Load ALL theme CSS files — they are scoped by .psyern-lb--{theme} so only the active one applies.
		$all_themes = array( 'military', 'ash', 'ops', 'outbreak', 'cyberpunk', 'stalker', 'inferno', 'frostbite', 'bubblegum' );
		foreach ( $all_themes as $t ) {
			wp_enqueue_style( 'psyern-theme-' . $t, PF_PLUGIN_URL . 'public/css/psyern-theme-' . $t . '.css', array( 'psyern-leaderboard' ), PF_VERSION );
		}

		// Google Fonts for the active theme.
		$fonts = array(
			'military'  => 'family=Share+Tech+Mono|Oswald:wght@400;700',
			'ash'       => 'family=Playfair+Display:wght@400;700|DM+Mono:wght@400',
			'ops'       => 'family=Orbitron:wght@400;700;900|Share+Tech+Mono',
			'outbreak'  => 'family=Rajdhani:wght@400;600;700|Exo+2:wght@300;400;600|Inconsolata:wght@400;700',
			'cyberpunk' => 'family=Orbitron:wght@400;700;900|JetBrains+Mono:wght@400;700',
			'stalker'   => 'family=Courier+Prime:wght@400;700',
			'inferno'   => 'family=Teko:wght@400;500;700',
			'frostbite' => 'family=Exo+2:wght@300;400;600|Share+Tech+Mono',
			'bubblegum' => 'family=Quicksand:wght@400;600;700|Nunito:wght@400;600',
		);
		if ( isset( $fonts[ $theme ] ) ) {
			wp_enqueue_style( 'psyern-fonts', 'https://fonts.googleapis.com/css2?' . $fonts[ $theme ] . '&display=swap', array(), null );
		}

		wp_enqueue_script( 'psyern-leaderboard', PF_PLUGIN_URL . 'public/js/psyern-leaderboard.js', array(), PF_VERSION, true );
		wp_enqueue_script( 'psyern-theme-effects', PF_PLUGIN_URL . 'public/js/psyern-theme-effects.js', array(), PF_VERSION, true );
		wp_enqueue_script( 'pf-templates', PF_PLUGIN_URL . 'public/assets/js/pf-templates.js', array(), PF_VERSION, true );
		wp_localize_script( 'pf-templates', 'pf_config', array(
			'apiUrl' => esc_url( rest_url( 'psyern/v1' ) ),
		) );
		wp_localize_script( 'psyern-leaderboard', 'psyernConfig', array(
			'ajaxUrl' => admin_url( 'admin-ajax.php' ),
			'restUrl' => esc_url_raw( rest_url( 'psyern/v1' ) ),
			'nonce'   => wp_create_nonce( 'psyern_leaderboard_nonce' ),
			'theme'   => $theme,
			'columns' => array(
				'pvp' => self::get_enabled_columns( 'pvp' ),
				'pve' => self::get_enabled_columns( 'pve' ),
			),
			'i18n'    => array(
				'kills'    => __( 'Kills', 'psyerns-framework' ),
				'deaths'   => __( 'Deaths', 'psyerns-framework' ),
				'kd'       => __( 'K/D', 'psyerns-framework' ),
				'playtime' => __( 'Playtime', 'psyerns-framework' ),
				'score'    => __( 'Score', 'psyerns-framework' ),
				'loading'  => __( 'Loading...', 'psyerns-framework' ),
				'error'    => __( 'Failed to load leaderboard data.', 'psyerns-framework' ),
			),
		) );
	}

	/**
	 * Resolve the theme from shortcode atts or plugin setting.
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string Theme name (dark or light).
	 */
	private function get_theme( $atts ) {
		if ( ! empty( $atts['theme'] ) ) {
			return sanitize_text_field( $atts['theme'] );
		}

		return get_option( 'psyern_theme', 'military' );
	}

	/**
	 * Return the enabled column keys for a given board mode from WP options.
	 *
	 * @param string $mode 'pvp' or 'pve'.
	 * @return string[] Array of enabled column key strings.
	 */
	private static function get_enabled_columns( $mode ) {
		$all_keys = array( 'rank', 'avatar', 'name', 'kills', 'deaths', 'kd', 'faction', 'boss', 'reputation', 'playtime' );
		$stored   = get_option( 'pf_columns_' . $mode, '' );
		if ( empty( $stored ) ) {
			return $all_keys;
		}
		$decoded = json_decode( $stored, true );
		if ( ! is_array( $decoded ) || empty( $decoded ) ) {
			return $all_keys;
		}
		// Always keep rank + name regardless of setting.
		foreach ( array( 'rank', 'name' ) as $fixed ) {
			if ( ! in_array( $fixed, $decoded, true ) ) {
				array_unshift( $decoded, $fixed );
			}
		}
		return $decoded;
	}

	/**
	 * Shortcode: [pf_leaderboard type="pvp" limit="10" theme="military"]
	 *
	 * Renders the full themed leaderboard with mode switch, limit buttons,
	 * player search and AJAX pagination via psyern-leaderboard.js.
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function leaderboard( $atts ) {
		$atts = shortcode_atts( array(
			'type'          => 'pvp',
			'limit'         => 10,
			'theme'         => '',
			'show_avatar'   => '1',
			'show_playtime' => '1',
		), $atts, 'pf_leaderboard' );

		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		// Variables expected by leaderboard-base.php.
		$mode  = sanitize_text_field( $atts['type'] );
		$limit = min( absint( $atts['limit'] ), 100 );
		// $atts is passed through to the template for show_avatar / show_playtime.

		// Provide empty initial data — AJAX will populate immediately.
		$top3 = array();
		$rest = array();
		$week = absint( gmdate( 'W' ) );
		$year = absint( gmdate( 'Y' ) );

		ob_start();
		include PF_PLUGIN_DIR . 'public/templates/leaderboard-base.php';
		return ob_get_clean();
	}

	/**
	 * Shortcode: [pf_server_status theme="dark"]
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function server_status( $atts ) {
		$atts  = shortcode_atts( array( 'theme' => '' ), $atts, 'pf_server_status' );
		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		$id = 'pf-ss-' . wp_rand();

		ob_start();
		?>
		<div id="<?php echo esc_attr( $id ); ?>" class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>">
			<div class="pf-loading"><?php esc_html_e( 'Loading status...', 'psyerns-framework' ); ?></div>
		</div>
		<script>
		document.addEventListener('DOMContentLoaded', function() {
			if (typeof pf_config !== 'undefined') PF.config.apiUrl = pf_config.apiUrl;
			var c = document.getElementById('<?php echo esc_js( $id ); ?>');
			function load() { PF.fetchServerStatus().then(function(d) { PF.renderServerStatus(d, c); }).catch(function() {}); }
			load();
			PF.startAutoRefresh(load);
		});
		</script>
		<?php
		return ob_get_clean();
	}

	/**
	 * Shortcode: [pf_top3_monthly theme="dark"]
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function top3_monthly( $atts ) {
		$atts  = shortcode_atts( array( 'theme' => '' ), $atts, 'pf_top3_monthly' );
		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		$id = 'pf-t3m-' . wp_rand();

		ob_start();
		?>
		<div id="<?php echo esc_attr( $id ); ?>" class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>">
			<h2><?php echo esc_html( apply_filters( 'psyerns-framework/top3_monthly_title', __( 'Top Players of the Month', 'psyerns-framework' ) ) ); ?></h2>
			<div class="pf-loading"><?php esc_html_e( 'Loading...', 'psyerns-framework' ); ?></div>
		</div>
		<script>
		document.addEventListener('DOMContentLoaded', function() {
			if (typeof pf_config !== 'undefined') PF.config.apiUrl = pf_config.apiUrl;
			var c = document.getElementById('<?php echo esc_js( $id ); ?>');
			PF.fetchTop3('monthly').then(function(d) { PF.renderTop3Cards(d, c, false); }).catch(function() {});
		});
		</script>
		<?php
		return ob_get_clean();
	}

	/**
	 * Shortcode: [pf_top3_deadliest theme="dark"]
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function top3_deadliest( $atts ) {
		$atts  = shortcode_atts( array( 'theme' => '' ), $atts, 'pf_top3_deadliest' );
		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		$id = 'pf-t3d-' . wp_rand();

		ob_start();
		?>
		<div id="<?php echo esc_attr( $id ); ?>" class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>">
			<h2><?php echo esc_html( apply_filters( 'psyerns-framework/top3_deadliest_title', __( 'Deadliest Players', 'psyerns-framework' ) ) ); ?></h2>
			<div class="pf-loading"><?php esc_html_e( 'Loading...', 'psyerns-framework' ); ?></div>
		</div>
		<script>
		document.addEventListener('DOMContentLoaded', function() {
			if (typeof pf_config !== 'undefined') PF.config.apiUrl = pf_config.apiUrl;
			var c = document.getElementById('<?php echo esc_js( $id ); ?>');
			PF.fetchTop3('deadliest').then(function(d) { PF.renderTop3Cards(d, c, true); }).catch(function() {});
		});
		</script>
		<?php
		return ob_get_clean();
	}

	/**
	 * Shortcode: [pf_top3_bosskills theme="dark"]
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function top3_bosskills( $atts ) {
		$atts  = shortcode_atts( array( 'theme' => '' ), $atts, 'pf_top3_bosskills' );
		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		$id = 'pf-t3b-' . wp_rand();

		ob_start();
		?>
		<div id="<?php echo esc_attr( $id ); ?>" class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>">
			<div class="pf-loading"><?php esc_html_e( 'Loading...', 'psyerns-framework' ); ?></div>
		</div>
		<script>
		document.addEventListener('DOMContentLoaded', function() {
			if (typeof pf_config !== 'undefined') PF.config.apiUrl = pf_config.apiUrl;
			var c = document.getElementById('<?php echo esc_js( $id ); ?>');
			PF.fetchLeaderboard('pve', 100).then(function(d) {
				var all = d.players || d;
				PF.renderTop3BossKills(all, c);
			}).catch(function() {});
		});
		</script>
		<?php
		return ob_get_clean();
	}

	/**
	 * Shortcode: [pf_player_card steam_id="76561198..." theme="dark"]
	 *
	 * @param array $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function player_card( $atts ) {
		$atts  = shortcode_atts( array(
			'steam_id' => '',
			'theme'    => '',
		), $atts, 'pf_player_card' );
		$theme = $this->get_theme( $atts );
		$this->enqueue_assets();

		$id = 'pf-pc-' . wp_rand();

		ob_start();
		?>
		<div id="<?php echo esc_attr( $id ); ?>" class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>">
			<div class="pf-loading"><?php esc_html_e( 'Loading player...', 'psyerns-framework' ); ?></div>
		</div>
		<script>
		document.addEventListener('DOMContentLoaded', function() {
			if (typeof pf_config !== 'undefined') PF.config.apiUrl = pf_config.apiUrl;
			var c = document.getElementById('<?php echo esc_js( $id ); ?>');
			var sid = '<?php echo esc_js( $atts['steam_id'] ); ?>';
			PF.fetchLeaderboard('pve', 1000).then(function(d) {
				var ps = d.players || d;
				var p = null;
				for (var i = 0; i < ps.length; i++) {
					if (ps[i].steam_id === sid) { p = ps[i]; break; }
				}
				PF.renderPlayerCard(p, c);
			}).catch(function() {
				c.innerHTML = '<div class="pf-loading"><?php echo esc_js( __( 'Failed to load.', 'psyerns-framework' ) ); ?></div>';
			});
		});
		</script>
		<?php
		return ob_get_clean();
	}
}

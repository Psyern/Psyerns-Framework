<?php
/**
 * Admin menu, settings page and whitelist management.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PF_Admin
 *
 * Registers admin menu pages, settings fields and whitelist CRUD actions.
 * Hooks are registered via init(), not in the constructor.
 */
class PF_Admin {

	/**
	 * Register all admin hooks.
	 *
	 * @return void
	 */
	public function init() {
		add_action( 'admin_menu', array( $this, 'register_menu' ) );
		add_action( 'admin_init', array( $this, 'register_settings' ) );
		add_action( 'admin_init', array( $this, 'handle_whitelist_actions' ) );
		add_action( 'admin_init', array( $this, 'handle_leaderboard_reset' ) );
		add_action( 'admin_enqueue_scripts', array( $this, 'enqueue_assets' ) );
	}

	/**
	 * Register admin menu and subpages.
	 *
	 * @return void
	 */
	public function register_menu() {
		add_menu_page(
			__( 'Psyerns Leaderboard', 'psyerns-framework' ),
			__( 'Psyerns Leaderboard', 'psyerns-framework' ),
			'manage_options',
			'pf-settings',
			array( $this, 'render_settings_page' ),
			'dashicons-shield',
			80
		);

		add_submenu_page(
			'pf-settings',
			__( 'Whitelist', 'psyerns-framework' ),
			__( 'Whitelist', 'psyerns-framework' ),
			'manage_options',
			'pf-whitelist',
			array( $this, 'render_whitelist_page' )
		);
	}

	/**
	 * Register plugin settings via Settings API.
	 *
	 * @return void
	 */
	/**
	 * All available leaderboard columns with their default visibility per mode.
	 * Key = option suffix / data-col attribute value.
	 *
	 * @return array<string, string>  key => label
	 */
	public static function get_column_definitions() {
		return array(
			'rank'             => '#',
			'avatar'           => __( 'Avatar', 'psyerns-framework' ),
			'name'             => __( 'Name', 'psyerns-framework' ),
			'kills'            => __( 'Kills', 'psyerns-framework' ),
			'deaths'           => __( 'Deaths', 'psyerns-framework' ),
			'kd'               => __( 'K/D', 'psyerns-framework' ),
			'faction'          => __( 'Faction / War Level', 'psyerns-framework' ),
			'boss'             => __( 'Boss Kills', 'psyerns-framework' ),
			'reputation'       => __( 'Reputation', 'psyerns-framework' ),
			'headshots'        => __( 'Headshots', 'psyerns-framework' ),
			'accuracy'         => __( 'Accuracy %', 'psyerns-framework' ),
			'longest_shot'     => __( 'Longest Shot', 'psyerns-framework' ),
			'distance'         => __( 'Distance Total (km)', 'psyerns-framework' ),
			'distance_foot'    => __( 'Distance on Foot (km)', 'psyerns-framework' ),
			'distance_vehicle' => __( 'Distance in Vehicle (km)', 'psyerns-framework' ),
			'playtime'         => __( 'Playtime', 'psyerns-framework' ),
		);
	}

	/**
	 * Columns that only make sense on the PvE board.
	 * Boss-Kills, Hardline-Reputation and Playtime are progression stats and
	 * not a PvP performance signal, so they are never offered on the PvP board.
	 *
	 * @return string[]
	 */
	public static function get_pve_only_columns() {
		// distance_foot/distance_vehicle/distance are progression stats and
		// share the same PvE-only semantics as boss/reputation —
		// only the "longest_shot" column gets a mode-specific value via
		// pvpLongestShot/pveLongestShot, so it stays available for both.
		// playtime is intentionally PvE-only in the leaderboard table view to
		// avoid duplicating a global progression metric on PvP.
		return array( 'boss', 'reputation', 'distance', 'distance_foot', 'distance_vehicle', 'playtime' );
	}

	/**
	 * Long-form descriptions for each column key. Used to expand abbreviated
	 * table headers (K/D, HS, ACC %, DIST, REP) into a hover-tooltip and to
	 * build the on-page legend.
	 *
	 * Keep keys in lock-step with get_column_definitions().
	 *
	 * @return array<string,string>
	 */
	public static function get_column_descriptions() {
		return array(
			'rank'             => __( 'Rank', 'psyerns-framework' ),
			'avatar'           => __( 'Player Avatar', 'psyerns-framework' ),
			'name'             => __( 'Player Name', 'psyerns-framework' ),
			'kills'            => __( 'Kills', 'psyerns-framework' ),
			'deaths'           => __( 'Deaths', 'psyerns-framework' ),
			'kd'               => __( 'Kill / Death Ratio', 'psyerns-framework' ),
			'faction'          => __( 'War Faction & Level', 'psyerns-framework' ),
			'boss'             => __( 'Boss Kills (DME-WAR)', 'psyerns-framework' ),
			'reputation'       => __( 'Hardline Reputation', 'psyerns-framework' ),
			'headshots'        => __( 'Headshots', 'psyerns-framework' ),
			'accuracy'         => __( 'Hit Accuracy (Hits / Shots Fired)', 'psyerns-framework' ),
			'longest_shot'     => __( 'Longest Kill Range (m)', 'psyerns-framework' ),
			'distance'         => __( 'Total Distance Travelled (km)', 'psyerns-framework' ),
			'distance_foot'    => __( 'Distance Travelled on Foot (km)', 'psyerns-framework' ),
			'distance_vehicle' => __( 'Distance Travelled in Vehicle (km)', 'psyerns-framework' ),
			'playtime'         => __( 'Total Playtime', 'psyerns-framework' ),
		);
	}

	/**
	 * Columns whose header is an abbreviation. Only these need to appear in
	 * the on-page legend — listing trivial ones like "Kills" or "Deaths" only
	 * adds noise.
	 *
	 * @return string[]
	 */
	public static function get_abbreviated_columns() {
		return array( 'kd', 'boss', 'reputation', 'headshots', 'accuracy', 'longest_shot', 'distance', 'distance_foot', 'distance_vehicle', 'playtime' );
	}

	/**
	 * Resolve the list of column keys that are allowed for a given mode.
	 * Applied as a hard filter both server-side (settings UI, template) and
	 * client-side (JS) so that PvE-only columns cannot be rendered on PvP.
	 *
	 * @param string $mode 'pvp' or 'pve'.
	 * @return string[]
	 */
	public static function get_mode_allowed_columns( $mode ) {
		$all = array_keys( self::get_column_definitions() );
		if ( 'pvp' === $mode ) {
			return array_values( array_diff( $all, self::get_pve_only_columns() ) );
		}
		if ( 'pve' === $mode ) {
			return array_values( array_diff( $all, self::get_pvp_only_columns() ) );
		}
		return $all;
	}

	/**
	 * Columns that only make sense on the PvP board.
	 * The server only tracks shots_hit / headshots on hits against players
	 * (PlayerBase.EEHitBy). Treffer gegen AI/Zombies/Animals werden gar nicht
	 * mitgezaehlt, also sind "headshots" und das daraus abgeleitete "accuracy"
	 * konstruktiv PvP-Werte — auf dem PvE-Board wuerden sie nur den
	 * PvP-Wert in einen Kontext duplizieren, in dem er nichts bedeutet.
	 *
	 * @return string[]
	 */
	public static function get_pvp_only_columns() {
		return array( 'headshots', 'accuracy' );
	}

	public function register_settings() {
		// ── Tab: API ──────────────────────────────────────────────────
		register_setting( 'pf_settings_api', 'pf_api_key', array(
			'sanitize_callback' => 'sanitize_text_field',
		) );
		register_setting( 'pf_settings_api', 'pf_steam_api_key', array(
			'sanitize_callback' => 'sanitize_text_field',
		) );

		// ── Tab: Leaderboard ──────────────────────────────────────────
		register_setting( 'pf_settings_leaderboard', 'pf_columns_pvp', array(
			'sanitize_callback' => array( $this, 'sanitize_columns_pvp' ),
			'default'           => wp_json_encode( self::get_mode_allowed_columns( 'pvp' ) ),
		) );
		register_setting( 'pf_settings_leaderboard', 'pf_columns_pve', array(
			'sanitize_callback' => array( $this, 'sanitize_columns_pve' ),
			'default'           => wp_json_encode( self::get_mode_allowed_columns( 'pve' ) ),
		) );
		register_setting( 'pf_settings_leaderboard', 'psyern_enable_pvp', array(
			'sanitize_callback' => 'sanitize_text_field',
			'default'           => '1',
		) );
		register_setting( 'pf_settings_leaderboard', 'psyern_enable_pve', array(
			'sanitize_callback' => 'sanitize_text_field',
			'default'           => '1',
		) );

		// ── Tab: Themes ───────────────────────────────────────────────
		register_setting( 'pf_settings_themes', 'psyern_theme', array(
			'sanitize_callback' => 'sanitize_text_field',
			'default'           => 'military',
		) );

		// ── Tab: Leaderboard / Player Details ─────────────────────────
		register_setting( 'pf_settings_leaderboard', 'pf_player_details_enabled', array(
			'sanitize_callback' => array( $this, 'sanitize_checkbox' ),
			'default'           => '1',
		) );
		register_setting( 'pf_settings_leaderboard', 'pf_player_details_show_avatar', array(
			'sanitize_callback' => array( $this, 'sanitize_checkbox' ),
			'default'           => '1',
		) );
		register_setting( 'pf_settings_leaderboard', 'pf_player_details_max_per_group', array(
			'sanitize_callback' => array( $this, 'sanitize_max_per_group' ),
			'default'           => 20,
		) );
	}

	/**
	 * Sanitize a checkbox option to '1' or ''.
	 *
	 * @param mixed $input Raw value.
	 * @return string '1' when truthy, '' otherwise.
	 */
	public function sanitize_checkbox( $input ) {
		return ( '1' === (string) $input || 1 === $input || true === $input ) ? '1' : '';
	}

	/**
	 * Sanitize the "max categories per group" option (clamped to 1..100).
	 *
	 * @param mixed $input Raw value.
	 * @return int
	 */
	public function sanitize_max_per_group( $input ) {
		$v = absint( $input );
		if ( $v < 1 )   { $v = 1; }
		if ( $v > 100 ) { $v = 100; }
		return $v;
	}

	/**
	 * Sanitize checkbox array → JSON string of enabled column keys.
	 *
	 * @param mixed  $input Array of checked column keys submitted via POST.
	 * @param string $mode  'pvp' or 'pve' — restricts to mode-allowed columns.
	 * @return string JSON-encoded array.
	 */
	public function sanitize_columns( $input, $mode = 'pve' ) {
		$valid       = self::get_mode_allowed_columns( $mode );
		$option_name = 'pf_columns_' . $mode;

		// No checkboxes submitted at all — preserve previous selection
		// instead of silently resetting to all-defaults.
		if ( ! is_array( $input ) ) {
			$existing = get_option( $option_name, '' );
			if ( $existing ) {
				return $existing;
			}
			return wp_json_encode( array( 'rank', 'name' ) );
		}

		$sanitized = array();
		foreach ( $input as $key ) {
			if ( in_array( $key, $valid, true ) && ! in_array( $key, $sanitized, true ) ) {
				$sanitized[] = $key;
			}
		}

		// Always keep fixed columns so they cannot be lost.
		foreach ( array( 'rank', 'name' ) as $fixed_key ) {
			if ( ! in_array( $fixed_key, $sanitized, true ) ) {
				array_unshift( $sanitized, $fixed_key );
			}
		}

		return wp_json_encode( $sanitized );
	}

	/** Sanitize callback bound to pf_columns_pvp. */
	public function sanitize_columns_pvp( $input ) {
		return $this->sanitize_columns( $input, 'pvp' );
	}

	/** Sanitize callback bound to pf_columns_pve. */
	public function sanitize_columns_pve( $input ) {
		return $this->sanitize_columns( $input, 'pve' );
	}

	/**
	 * Render column checkboxes for a given mode.
	 *
	 * @param string $mode 'pvp' or 'pve'.
	 * @return void
	 */
	private function render_columns_field( $mode ) {
		$option_name = 'pf_columns_' . $mode;
		$allowed     = self::get_mode_allowed_columns( $mode );
		$stored      = get_option( $option_name, '' );
		$enabled     = $stored ? json_decode( $stored, true ) : $allowed;
		if ( ! is_array( $enabled ) ) {
			$enabled = $allowed;
		}
		$cols = self::get_column_definitions();

		echo '<fieldset style="display:flex;flex-wrap:wrap;gap:8px 24px;">';
		foreach ( $cols as $key => $label ) {
			$is_allowed = in_array( $key, $allowed, true );
			if ( ! $is_allowed ) {
				continue;
			}
			$checked  = in_array( $key, $enabled, true ) ? 'checked' : '';
			$is_fixed = in_array( $key, array( 'rank', 'name' ), true );
			printf(
				'<label style="display:flex;align-items:center;gap:6px;cursor:%s;">'
				. '<input type="checkbox" name="%s[]" value="%s" %s />'
				. '<span>%s</span>'
				. '</label>',
				$is_fixed ? 'default' : 'pointer',
				esc_attr( $option_name ),
				esc_attr( $key ),
				$is_fixed ? 'checked disabled' : $checked,
				esc_html( $label )
			);
			// Fixed columns are always sent even though HTML disabled checkboxes don't submit.
			if ( $is_fixed ) {
				printf( '<input type="hidden" name="%s[]" value="%s" />', esc_attr( $option_name ), esc_attr( $key ) );
			}
		}
		echo '</fieldset>';
		$hidden_keys = array_diff( array_keys( $cols ), $allowed );
		if ( ! empty( $hidden_keys ) ) {
			echo '<p class="description">' . esc_html(
				sprintf(
					/* translators: %s: comma-separated list of column names. */
					__( '# and Name are always visible. PvE-only columns are hidden here: %s.', 'psyerns-framework' ),
					implode( ', ', array_map( function( $k ) use ( $cols ) { return $cols[ $k ]; }, $hidden_keys ) )
				)
			) . '</p>';
		} else {
			echo '<p class="description">' . esc_html__( '# and Name are always visible.', 'psyerns-framework' ) . '</p>';
		}
	}

	/** Render PvP column checkboxes. */
	public function render_columns_pvp_field() {
		$this->render_columns_field( 'pvp' );
	}

	/** Render PvE column checkboxes. */
	public function render_columns_pve_field() {
		$this->render_columns_field( 'pve' );
	}

	/**
	 * Render API Key settings field.
	 *
	 * @return void
	 */
	public function render_api_key_field() {
		$val = esc_attr( get_option( 'pf_api_key', '' ) );
		echo '<input type="text" id="pf_api_key" name="pf_api_key" value="' . $val . '" class="regular-text" />';
		echo '<p class="description">' . esc_html__( 'Shared secret between DayZ server and this plugin.', 'psyerns-framework' ) . '</p>';
	}

	/**
	 * Render Steam API Key settings field.
	 *
	 * @return void
	 */
	public function render_steam_key_field() {
		$val = esc_attr( get_option( 'pf_steam_api_key', '' ) );
		echo '<input type="text" id="pf_steam_api_key" name="pf_steam_api_key" value="' . $val . '" class="regular-text" />';
		echo '<p class="description">' . wp_kses_post( __( 'Optional. Used for Steam avatar resolution. Get your key at <a href="https://steamcommunity.com/dev/apikey" target="_blank">steamcommunity.com/dev/apikey</a>', 'psyerns-framework' ) ) . '</p>';
	}

	/**
	 * Render the settings page.
	 *
	 * @return void
	 */
	public function render_settings_page() {
		if ( ! current_user_can( 'manage_options' ) ) {
			return;
		}
		include PF_PLUGIN_DIR . 'admin/views/settings-page.php';
	}

	/**
	 * Render the whitelist management page.
	 *
	 * @return void
	 */
	public function render_whitelist_page() {
		if ( ! current_user_can( 'manage_options' ) ) {
			return;
		}
		include PF_PLUGIN_DIR . 'admin/views/whitelist-page.php';
	}

	/**
	 * Process whitelist add/remove form submissions.
	 *
	 * @return void
	 */
	public function handle_whitelist_actions() {
		if ( ! current_user_can( 'manage_options' ) ) {
			return;
		}

		if ( isset( $_POST['pf_whitelist_add'] ) ) {
			check_admin_referer( 'pf_whitelist_add_nonce' );
			global $wpdb;
			$table    = PF_Database::get_table_name( 'whitelist' );
			$steam_id = sanitize_text_field( wp_unslash( $_POST['steam_id'] ?? '' ) );
			$name     = sanitize_text_field( wp_unslash( $_POST['player_name'] ?? '' ) );

			if ( ! empty( $steam_id ) ) {
				$wpdb->query( $wpdb->prepare(
					"INSERT IGNORE INTO {$table} (steam_id, name, added_at) VALUES (%s, %s, %s)",
					$steam_id,
					$name,
					current_time( 'mysql' )
				) );
			}

			wp_safe_redirect( admin_url( 'admin.php?page=pf-whitelist&msg=added' ) );
			exit;
		}

		if ( isset( $_POST['pf_whitelist_remove'] ) ) {
			check_admin_referer( 'pf_whitelist_remove_nonce' );
			global $wpdb;
			$table    = PF_Database::get_table_name( 'whitelist' );
			$steam_id = sanitize_text_field( wp_unslash( $_POST['steam_id'] ?? '' ) );

			if ( ! empty( $steam_id ) ) {
				$wpdb->delete( $table, array( 'steam_id' => $steam_id ) );
			}

			wp_safe_redirect( admin_url( 'admin.php?page=pf-whitelist&msg=removed' ) );
			exit;
		}
	}

	/**
	 * Handle the "Reset Leaderboard" form submission from the Leaderboard tab.
	 *
	 * Truncates the leaderboard table and redirects with a status message.
	 * Player rows will repopulate from the next DayZ-server upload (within 60s).
	 *
	 * @return void
	 */
	public function handle_leaderboard_reset() {
		if ( ! isset( $_POST['pf_reset_leaderboard'] ) ) {
			return;
		}

		if ( ! current_user_can( 'manage_options' ) ) {
			return;
		}

		check_admin_referer( 'pf_reset_leaderboard_nonce' );

		$result = PF_Database::reset_leaderboard();
		$msg    = ( false !== $result ) ? 'reset_ok' : 'reset_fail';

		wp_safe_redirect( admin_url( 'admin.php?page=pf-settings&tab=leaderboard&msg=' . $msg ) );
		exit;
	}

	/**
	 * Enqueue admin CSS + JS on plugin pages only.
	 *
	 * @param string $hook Current admin page hook.
	 * @return void
	 */
	public function enqueue_assets( $hook ) {
		if ( false === strpos( $hook, 'pf-' ) ) {
			return;
		}
		wp_enqueue_style( 'pf-admin', PF_PLUGIN_URL . 'admin/css/pf-admin.css', array( 'dashicons' ), PF_VERSION );
		wp_enqueue_script( 'pf-admin-tabs', PF_PLUGIN_URL . 'admin/js/pf-admin-tabs.js', array(), PF_VERSION, true );
	}
}

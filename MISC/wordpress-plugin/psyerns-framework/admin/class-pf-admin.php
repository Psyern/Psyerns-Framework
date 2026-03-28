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
		add_action( 'admin_enqueue_scripts', array( $this, 'enqueue_assets' ) );
	}

	/**
	 * Register admin menu and subpages.
	 *
	 * @return void
	 */
	public function register_menu() {
		add_menu_page(
			__( 'Psyerns Framework', 'psyerns-framework' ),
			__( 'Psyerns Framework', 'psyerns-framework' ),
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
			'rank'       => '#',
			'avatar'     => __( 'Avatar', 'psyerns-framework' ),
			'name'       => __( 'Name', 'psyerns-framework' ),
			'kills'      => __( 'Kills', 'psyerns-framework' ),
			'deaths'     => __( 'Deaths', 'psyerns-framework' ),
			'kd'         => __( 'K/D', 'psyerns-framework' ),
			'faction'    => __( 'Faction / War Level', 'psyerns-framework' ),
			'boss'       => __( 'Boss Kills', 'psyerns-framework' ),
			'reputation' => __( 'Reputation', 'psyerns-framework' ),
			'playtime'   => __( 'Playtime', 'psyerns-framework' ),
		);
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
			'sanitize_callback' => array( $this, 'sanitize_columns' ),
			'default'           => wp_json_encode( array_keys( self::get_column_definitions() ) ),
		) );
		register_setting( 'pf_settings_leaderboard', 'pf_columns_pve', array(
			'sanitize_callback' => array( $this, 'sanitize_columns' ),
			'default'           => wp_json_encode( array_keys( self::get_column_definitions() ) ),
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
	}

	/**
	 * Sanitize checkbox array → JSON string of enabled column keys.
	 *
	 * @param mixed $input Array of checked column keys submitted via POST.
	 * @return string JSON-encoded array.
	 */
	public function sanitize_columns( $input ) {
		$valid = array_keys( self::get_column_definitions() );
		if ( ! is_array( $input ) ) {
			return wp_json_encode( $valid );
		}
		$sanitized = array();
		foreach ( $input as $key ) {
			if ( in_array( $key, $valid, true ) ) {
				$sanitized[] = $key;
			}
		}
		return wp_json_encode( $sanitized );
	}

	/**
	 * Render column checkboxes for a given mode.
	 *
	 * @param string $mode 'pvp' or 'pve'.
	 * @return void
	 */
	private function render_columns_field( $mode ) {
		$option_name = 'pf_columns_' . $mode;
		$stored      = get_option( $option_name, '' );
		$enabled     = $stored ? json_decode( $stored, true ) : array_keys( self::get_column_definitions() );
		if ( ! is_array( $enabled ) ) {
			$enabled = array_keys( self::get_column_definitions() );
		}
		$cols = self::get_column_definitions();

		echo '<fieldset style="display:flex;flex-wrap:wrap;gap:8px 24px;">';
		foreach ( $cols as $key => $label ) {
			$checked  = in_array( $key, $enabled, true ) ? 'checked' : '';
			$disabled = in_array( $key, array( 'rank', 'name' ), true ) ? 'disabled checked' : $checked;
			$is_fixed = in_array( $key, array( 'rank', 'name' ), true );
			printf(
				'<label style="display:flex;align-items:center;gap:6px;cursor:%s;">'
				. '<input type="checkbox" name="%s[]" value="%s" %s %s />'
				. '<span>%s</span>'
				. '</label>',
				$is_fixed ? 'default' : 'pointer',
				esc_attr( $option_name ),
				esc_attr( $key ),
				$is_fixed ? 'checked disabled' : $checked,
				'',
				esc_html( $label )
			);
			// Fixed columns are always sent even though HTML disabled checkboxes don't submit.
			if ( $is_fixed ) {
				printf( '<input type="hidden" name="%s[]" value="%s" />', esc_attr( $option_name ), esc_attr( $key ) );
			}
		}
		echo '</fieldset>';
		echo '<p class="description">' . esc_html__( '# and Name are always visible.', 'psyerns-framework' ) . '</p>';
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

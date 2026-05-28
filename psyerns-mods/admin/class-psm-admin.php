<?php
/**
 * Admin functionality for Psyerns Mods Showreel.
 *
 * Handles meta boxes, custom columns, settings page,
 * asset enqueuing and AJAX endpoints.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class PSM_Admin
 *
 * Registers all admin-side hooks and renders the backend UI
 * for the psyern_mod custom post type.
 */
class PSM_Admin {

	/**
	 * Settings page hook suffix returned by add_options_page().
	 *
	 * @var string
	 */
	private $settings_hook = '';

	/**
	 * Register all admin hooks.
	 *
	 * @return void
	 */
	public function register() {
		add_action( 'add_meta_boxes', array( $this, 'add_meta_boxes' ) );
		add_action( 'admin_enqueue_scripts', array( $this, 'enqueue_assets' ) );
		add_filter( 'manage_psyern_mod_posts_columns', array( $this, 'add_custom_columns' ) );
		add_action( 'manage_psyern_mod_posts_custom_column', array( $this, 'render_custom_column' ), 10, 2 );
		add_action( 'admin_menu', array( $this, 'add_settings_page' ) );
		add_action( 'admin_init', array( $this, 'register_settings' ) );
		add_action( 'wp_ajax_psm_fetch_steam_data', array( $this, 'ajax_fetch_steam_data' ) );
		add_action( 'admin_post_psm_flush_cache', array( $this, 'handle_flush_cache' ) );
		add_action( 'admin_post_psm_discord_test', array( $this, 'handle_discord_test' ) );
	}

	/**
	 * Register the meta box for the psyern_mod post type.
	 *
	 * @return void
	 */
	public function add_meta_boxes() {
		add_meta_box(
			'psm_mod_details',
			__( 'Mod Details', 'psyerns-mods' ),
			array( $this, 'render_meta_box' ),
			'psyern_mod',
			'normal',
			'high'
		);
	}

	/**
	 * Render the Mod Details meta box.
	 *
	 * @param WP_Post $post Current post object.
	 * @return void
	 */
	public function render_meta_box( $post ) {
		wp_nonce_field( 'psm_meta_nonce', 'psm_meta_nonce_field' );

		$steam_url     = get_post_meta( $post->ID, '_psm_steam_url', true );
		$steam_data    = get_post_meta( $post->ID, '_psm_steam_data', true );
		$is_free       = get_post_meta( $post->ID, '_psm_is_free', true );
		$price         = get_post_meta( $post->ID, '_psm_price', true );
		$has_repack    = get_post_meta( $post->ID, '_psm_has_repack', true );
		$repack_price  = get_post_meta( $post->ID, '_psm_repack_price', true );
		$has_source    = get_post_meta( $post->ID, '_psm_has_source', true );
		$source_price  = get_post_meta( $post->ID, '_psm_source_price', true );
		$discord_url   = get_post_meta( $post->ID, '_psm_discord_url', true );
		$order         = get_post_meta( $post->ID, '_psm_order', true );

		if ( '' === $order ) {
			$order = 0;
		}

		$steam_decoded = array();
		if ( ! empty( $steam_data ) ) {
			$steam_decoded = json_decode( $steam_data, true );
			if ( ! is_array( $steam_decoded ) ) {
				$steam_decoded = array();
			}
		}

		$preview_url   = isset( $steam_decoded['preview_url'] ) ? $steam_decoded['preview_url'] : '';
		$preview_title = isset( $steam_decoded['title'] ) ? $steam_decoded['title'] : '';
		?>

		<!-- Section: Steam Integration -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Steam Integration', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_steam_url"><?php esc_html_e( 'Steam Workshop URL', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="url"
							id="psm_steam_url"
							name="psm_steam_url"
							value="<?php echo esc_url( $steam_url ); ?>"
							class="large-text"
							placeholder="https://steamcommunity.com/sharedfiles/filedetails/?id=..." />
						<button type="button"
							id="psm_fetch_steam"
							class="button button-secondary"
							data-post-id="<?php echo esc_attr( $post->ID ); ?>">
							<?php esc_html_e( 'Daten laden', 'psyerns-mods' ); ?>
						</button>
					</td>
				</tr>
				<tr>
					<th><?php esc_html_e( 'Vorschau', 'psyerns-mods' ); ?></th>
					<td>
						<div id="psm_steam_preview" class="psm-steam-preview">
							<?php if ( ! empty( $preview_url ) ) : ?>
								<img src="<?php echo esc_url( $preview_url ); ?>"
									alt="<?php echo esc_attr( $preview_title ); ?>"
									class="psm-steam-thumbnail" />
								<span class="psm-steam-title"><?php echo esc_html( $preview_title ); ?></span>
							<?php else : ?>
								<span class="psm-steam-placeholder"><?php esc_html_e( 'Keine Steam-Daten geladen.', 'psyerns-mods' ); ?></span>
							<?php endif; ?>
						</div>
					</td>
				</tr>
			</table>
		</div>

		<!-- Section: Verfuegbarkeit & Preise -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Verfuegbarkeit & Preise', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_is_free"><?php esc_html_e( 'Kostenlos', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="checkbox"
							id="psm_is_free"
							name="psm_is_free"
							value="1"
							<?php checked( $is_free, '1' ); ?> />
					</td>
				</tr>
				<tr>
					<th>
						<label for="psm_price"><?php esc_html_e( 'Preis in EUR', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="number"
							id="psm_price"
							name="psm_price"
							value="<?php echo esc_attr( $price ); ?>"
							step="0.01"
							min="0"
							class="small-text"
							<?php echo ( '1' === $is_free ) ? 'disabled="disabled"' : ''; ?> />
						<span class="description">&euro;</span>
					</td>
				</tr>
			</table>
		</div>

		<!-- Section: Repack -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Repack', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_has_repack"><?php esc_html_e( 'Repack verfuegbar', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<label class="psm-toggle">
							<input type="checkbox"
								id="psm_has_repack"
								name="psm_has_repack"
								value="1"
								<?php checked( $has_repack, '1' ); ?> />
							<span class="psm-toggle__track"></span>
						</label>
					</td>
				</tr>
				<tr>
					<th>
						<label for="psm_repack_price"><?php esc_html_e( 'Repack-Preis EUR', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="number"
							id="psm_repack_price"
							name="psm_repack_price"
							value="<?php echo esc_attr( $repack_price ); ?>"
							step="0.01"
							min="0"
							class="small-text"
							<?php echo ( '1' !== $has_repack ) ? 'disabled="disabled"' : ''; ?> />
						<span class="description">&euro;</span>
					</td>
				</tr>
			</table>
		</div>

		<!-- Section: Source File -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Source File', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_has_source"><?php esc_html_e( 'Source File verfuegbar', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<label class="psm-toggle">
							<input type="checkbox"
								id="psm_has_source"
								name="psm_has_source"
								value="1"
								<?php checked( $has_source, '1' ); ?> />
							<span class="psm-toggle__track"></span>
						</label>
					</td>
				</tr>
				<tr>
					<th>
						<label for="psm_source_price"><?php esc_html_e( 'Source-Preis EUR', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="number"
							id="psm_source_price"
							name="psm_source_price"
							value="<?php echo esc_attr( $source_price ); ?>"
							step="0.01"
							min="0"
							class="small-text"
							<?php echo ( '1' !== $has_source ) ? 'disabled="disabled"' : ''; ?> />
						<span class="description">&euro;</span>
					</td>
				</tr>
			</table>
		</div>

		<!-- Section: Links -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Links', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_discord_url"><?php esc_html_e( 'Discord URL', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="url"
							id="psm_discord_url"
							name="psm_discord_url"
							value="<?php echo esc_url( $discord_url ); ?>"
							class="large-text"
							placeholder="https://discord.gg/..." />
						<p class="description">
							<?php esc_html_e( 'Optional. Ueberschreibt die globale Discord-URL fuer diesen Mod.', 'psyerns-mods' ); ?>
						</p>
					</td>
				</tr>
			</table>
		</div>

		<!-- Section: Sortierung -->
		<div class="psm-meta-section">
			<h3><?php esc_html_e( 'Sortierung', 'psyerns-mods' ); ?></h3>
			<table class="form-table">
				<tr>
					<th>
						<label for="psm_order"><?php esc_html_e( 'Reihenfolge', 'psyerns-mods' ); ?></label>
					</th>
					<td>
						<input type="number"
							id="psm_order"
							name="psm_order"
							value="<?php echo esc_attr( $order ); ?>"
							step="1"
							min="0"
							class="small-text" />
						<p class="description">
							<?php esc_html_e( 'Kleinere Werte werden zuerst angezeigt.', 'psyerns-mods' ); ?>
						</p>
					</td>
				</tr>
			</table>
		</div>
		<?php
	}

	/**
	 * Handle AJAX request to fetch Steam Workshop data.
	 *
	 * Expects POST parameters: steam_url, post_id, _ajax_nonce.
	 *
	 * @return void
	 */
	public function ajax_fetch_steam_data() {
		check_ajax_referer( 'psm_admin_nonce' );

		if ( ! current_user_can( 'edit_posts' ) ) {
			wp_send_json_error(
				array( 'message' => __( 'Keine Berechtigung.', 'psyerns-mods' ) ),
				403
			);
		}

		$steam_url = isset( $_POST['steam_url'] ) ? esc_url_raw( wp_unslash( $_POST['steam_url'] ) ) : '';
		$post_id   = isset( $_POST['post_id'] ) ? absint( $_POST['post_id'] ) : 0;

		if ( empty( $steam_url ) ) {
			wp_send_json_error(
				array( 'message' => __( 'Bitte eine Steam Workshop URL eingeben.', 'psyerns-mods' ) )
			);
		}

		if ( false === strpos( $steam_url, 'steamcommunity.com' ) ) {
			wp_send_json_error(
				array( 'message' => __( 'Ungueltige Steam Workshop URL.', 'psyerns-mods' ) )
			);
		}

		if ( ! class_exists( 'PSM_Steam_API' ) ) {
			wp_send_json_error(
				array( 'message' => __( 'Steam API Klasse nicht gefunden.', 'psyerns-mods' ) )
			);
		}

		$data = PSM_Steam_API::get_item_data( $steam_url );

		if ( is_wp_error( $data ) ) {
			wp_send_json_error(
				array( 'message' => $data->get_error_message() )
			);
		}

		if ( empty( $data ) || ! is_array( $data ) ) {
			wp_send_json_error(
				array( 'message' => __( 'Keine Daten vom Steam Workshop erhalten.', 'psyerns-mods' ) )
			);
		}

		$result = array(
			'title'       => isset( $data['title'] ) ? sanitize_text_field( $data['title'] ) : '',
			'description' => isset( $data['description'] ) ? sanitize_textarea_field( $data['description'] ) : '',
			'preview_url' => isset( $data['preview_url'] ) ? esc_url_raw( $data['preview_url'] ) : '',
		);

		if ( 0 < $post_id ) {
			update_post_meta( $post_id, '_psm_steam_data', wp_json_encode( $result ) );
			update_post_meta( $post_id, '_psm_steam_url', esc_url_raw( $steam_url ) );
		}

		wp_send_json_success( $result );
	}

	/**
	 * Add custom columns to the psyern_mod post list table.
	 *
	 * @param array $columns Existing columns.
	 * @return array Modified columns.
	 */
	public function add_custom_columns( $columns ) {
		$new_columns = array();

		foreach ( $columns as $key => $label ) {
			$new_columns[ $key ] = $label;

			if ( 'title' === $key ) {
				$new_columns['psm_preview']   = __( 'Vorschau', 'psyerns-mods' );
				$new_columns['psm_price']     = __( 'Preis', 'psyerns-mods' );
				$new_columns['psm_steam_url'] = __( 'Steam URL', 'psyerns-mods' );
				$new_columns['psm_order']     = __( 'Reihenfolge', 'psyerns-mods' );
			}
		}

		return $new_columns;
	}

	/**
	 * Render the content for each custom column.
	 *
	 * @param string $column  Column slug.
	 * @param int    $post_id Current post ID.
	 * @return void
	 */
	public function render_custom_column( $column, $post_id ) {
		switch ( $column ) {
			case 'psm_preview':
				$steam_data = get_post_meta( $post_id, '_psm_steam_data', true );
				$decoded    = array();
				if ( ! empty( $steam_data ) ) {
					$decoded = json_decode( $steam_data, true );
				}
				if ( is_array( $decoded ) && ! empty( $decoded['preview_url'] ) ) {
					printf(
						'<img src="%s" alt="%s" style="width:60px;height:60px;object-fit:cover;border-radius:4px;" />',
						esc_url( $decoded['preview_url'] ),
						esc_attr( isset( $decoded['title'] ) ? $decoded['title'] : '' )
					);
				} else {
					echo '<span class="dashicons dashicons-format-image" style="color:#ccc;font-size:30px;"></span>';
				}
				break;

			case 'psm_price':
				$is_free = get_post_meta( $post_id, '_psm_is_free', true );
				if ( '1' === $is_free ) {
					echo '<span style="color:#46b450;font-weight:600;">' . esc_html__( 'Kostenlos', 'psyerns-mods' ) . '</span>';
				} else {
					$price = get_post_meta( $post_id, '_psm_price', true );
					if ( '' !== $price && null !== $price ) {
						echo esc_html( number_format_i18n( (float) $price, 2 ) ) . '&nbsp;&euro;';
					} else {
						echo '&mdash;';
					}
				}
				break;

			case 'psm_steam_url':
				$url = get_post_meta( $post_id, '_psm_steam_url', true );
				if ( ! empty( $url ) ) {
					printf(
						'<a href="%s" target="_blank" rel="noopener noreferrer">%s</a>',
						esc_url( $url ),
						esc_html__( 'Workshop', 'psyerns-mods' )
					);
				} else {
					echo '&mdash;';
				}
				break;

			case 'psm_order':
				$order = get_post_meta( $post_id, '_psm_order', true );
				echo esc_html( '' !== $order ? $order : '0' );
				break;
		}
	}

	/**
	 * Add the settings page under Einstellungen.
	 *
	 * @return void
	 */
	public function add_settings_page() {
		$this->settings_hook = add_options_page(
			__( 'Psyerns Mods', 'psyerns-mods' ),
			__( 'Psyerns Mods', 'psyerns-mods' ),
			'manage_options',
			'psm-settings',
			array( $this, 'render_settings_page' )
		);
	}

	/**
	 * Register plugin settings, sections and fields.
	 *
	 * @return void
	 */
	public function register_settings() {
		register_setting(
			'psm_settings_group',
			'psm_global_discord_url',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'esc_url_raw',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_steam_api_key',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discount_enabled',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discount_percent',
			array(
				'type'              => 'integer',
				'sanitize_callback' => 'absint',
				'default'           => 0,
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discount_label',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '',
			)
		);

		/*
		 * Discord Leaderboard settings.
		 */
		register_setting(
			'psm_settings_group',
			'psm_discord_enabled',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_webhook_url',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'esc_url_raw',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_embed_title',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => "\xF0\x9F\x8F\x86 Live Leaderboard",
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_embed_color',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_hex_color',
				'default'           => '#FFD700',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_max_players',
			array(
				'type'              => 'integer',
				'sanitize_callback' => 'absint',
				'default'           => 10,
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_interval',
			array(
				'type'              => 'integer',
				'sanitize_callback' => 'absint',
				'default'           => 30,
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_data_source',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => 'auto',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_fields',
			array(
				'type'              => 'array',
				'sanitize_callback' => array( $this, 'sanitize_discord_fields' ),
				'default'           => array(),
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_discord_custom_fields',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '',
			)
		);

		register_setting(
			'psm_settings_group',
			'psm_leaderboard_public',
			array(
				'type'              => 'string',
				'sanitize_callback' => 'sanitize_text_field',
				'default'           => '1',
			)
		);
	}

	/**
	 * Sanitize the Discord fields checkbox array.
	 *
	 * @param mixed $input Raw input.
	 * @return array Sanitized field map.
	 */
	public function sanitize_discord_fields( $input ) {
		if ( ! is_array( $input ) ) {
			return array();
		}

		$allowed = array(
			'kills', 'deaths', 'kd_ratio', 'zombie_kills',
			'longest_kill', 'playtime', 'distance_traveled',
			'players_online', 'total_players',
		);

		$clean = array();

		foreach ( $allowed as $key ) {
			$clean[ $key ] = isset( $input[ $key ] ) ? '1' : '';
		}

		return $clean;
	}

	/**
	 * Render the settings / overview page.
	 *
	 * @return void
	 */
	public function render_settings_page() {
		if ( ! current_user_can( 'manage_options' ) ) {
			return;
		}

		$flush_url = wp_nonce_url(
			admin_url( 'admin-post.php?action=psm_flush_cache' ),
			'psm_flush_cache_nonce'
		);

		$mod_count       = wp_count_posts( 'psyern_mod' );
		$published_count = isset( $mod_count->publish ) ? (int) $mod_count->publish : 0;
		$draft_count     = isset( $mod_count->draft ) ? (int) $mod_count->draft : 0;

		$discord_url       = get_option( 'psm_global_discord_url', '' );
		$steam_api_key     = get_option( 'psm_steam_api_key', '' );
		$discount_enabled  = get_option( 'psm_discount_enabled', '' );
		$discount_percent  = get_option( 'psm_discount_percent', 0 );
		$discount_label    = get_option( 'psm_discount_label', '' );

		$flushed = isset( $_GET['psm_flushed'] ) && '1' === $_GET['psm_flushed'];
		$saved   = isset( $_GET['settings-updated'] ) && 'true' === $_GET['settings-updated'];
		?>
		<div class="wrap psm-settings-wrap">
			<h1><?php esc_html_e( 'Psyerns Mods Showreel', 'psyerns-mods' ); ?></h1>

			<?php if ( $flushed ) : ?>
				<div class="notice notice-success is-dismissible">
					<p><?php esc_html_e( 'Steam-Cache wurde geleert.', 'psyerns-mods' ); ?></p>
				</div>
			<?php endif; ?>

			<?php if ( $saved ) : ?>
				<div class="notice notice-success is-dismissible">
					<p><?php esc_html_e( 'Einstellungen gespeichert.', 'psyerns-mods' ); ?></p>
				</div>
			<?php endif; ?>

			<!-- Shortcode Info Box -->
			<div class="psm-overview-box psm-overview-box--shortcode">
				<h2><?php esc_html_e( 'Shortcode', 'psyerns-mods' ); ?></h2>
				<p><?php esc_html_e( 'Kopiere diesen Shortcode und fuege ihn auf einer beliebigen Seite oder einem Beitrag ein:', 'psyerns-mods' ); ?></p>
				<div class="psm-shortcode-display">
					<code id="psm_shortcode_code">[psyerns_mods]</code>
					<button type="button" class="button button-small psm-copy-btn" data-copy="[psyerns_mods]">
						<?php esc_html_e( 'Kopieren', 'psyerns-mods' ); ?>
					</button>
				</div>
				<p class="description" style="margin-top: 10px;">
					<?php esc_html_e( 'Optionale Parameter:', 'psyerns-mods' ); ?>
					<code>[psyerns_mods columns="3" discord="https://discord.gg/..." order="ASC" limit="12"]</code>
				</p>
			</div>

			<!-- Stats -->
			<div class="psm-overview-stats">
				<div class="psm-stat-card">
					<span class="psm-stat-card__number"><?php echo esc_html( $published_count ); ?></span>
					<span class="psm-stat-card__label"><?php esc_html_e( 'Veroeffentlicht', 'psyerns-mods' ); ?></span>
				</div>
				<div class="psm-stat-card">
					<span class="psm-stat-card__number"><?php echo esc_html( $draft_count ); ?></span>
					<span class="psm-stat-card__label"><?php esc_html_e( 'Entwuerfe', 'psyerns-mods' ); ?></span>
				</div>
				<div class="psm-stat-card">
					<span class="psm-stat-card__number"><?php echo '1' === $discount_enabled ? esc_html( $discount_percent . '%' ) : '&mdash;'; ?></span>
					<span class="psm-stat-card__label"><?php esc_html_e( 'Aktiver Rabatt', 'psyerns-mods' ); ?></span>
				</div>
			</div>

			<form method="post" action="options.php">
				<?php settings_fields( 'psm_settings_group' ); ?>

				<!-- Rabatt Section -->
				<div class="psm-overview-box">
					<h2><?php esc_html_e( 'Allgemeiner Rabatt', 'psyerns-mods' ); ?></h2>
					<p class="description"><?php esc_html_e( 'Gilt fuer alle kostenpflichtigen Mods, Repacks und Source Files. Der Originalpreis wird durchgestrichen angezeigt.', 'psyerns-mods' ); ?></p>
					<table class="form-table">
						<tr>
							<th>
								<label for="psm_discount_enabled"><?php esc_html_e( 'Rabatt aktivieren', 'psyerns-mods' ); ?></label>
							</th>
							<td>
								<label class="psm-toggle">
									<input type="checkbox"
										id="psm_discount_enabled"
										name="psm_discount_enabled"
										value="1"
										<?php checked( $discount_enabled, '1' ); ?> />
									<span class="psm-toggle__track"></span>
								</label>
							</td>
						</tr>
						<tr>
							<th>
								<label for="psm_discount_percent"><?php esc_html_e( 'Rabatt in %', 'psyerns-mods' ); ?></label>
							</th>
							<td>
								<input type="number"
									id="psm_discount_percent"
									name="psm_discount_percent"
									value="<?php echo esc_attr( $discount_percent ); ?>"
									min="1"
									max="99"
									step="1"
									class="small-text"
									<?php echo ( '1' !== $discount_enabled ) ? 'disabled="disabled"' : ''; ?> />
								<span class="description">%</span>
							</td>
						</tr>
						<tr>
							<th>
								<label for="psm_discount_label"><?php esc_html_e( 'Rabatt-Label', 'psyerns-mods' ); ?></label>
							</th>
							<td>
								<input type="text"
									id="psm_discount_label"
									name="psm_discount_label"
									value="<?php echo esc_attr( $discount_label ); ?>"
									class="regular-text"
									placeholder="z.B. SUMMER SALE, -20%, Aktion"
									<?php echo ( '1' !== $discount_enabled ) ? 'disabled="disabled"' : ''; ?> />
								<p class="description"><?php esc_html_e( 'Optionales Label, das als Badge auf den Karten angezeigt wird.', 'psyerns-mods' ); ?></p>
							</td>
						</tr>
					</table>
				</div>

				<!-- Allgemeine Einstellungen -->
				<div class="psm-overview-box">
					<h2><?php esc_html_e( 'Allgemeine Einstellungen', 'psyerns-mods' ); ?></h2>
					<table class="form-table">
						<tr>
							<th>
								<label for="psm_global_discord_url"><?php esc_html_e( 'Globale Discord URL', 'psyerns-mods' ); ?></label>
							</th>
							<td>
								<input type="url"
									id="psm_global_discord_url"
									name="psm_global_discord_url"
									value="<?php echo esc_url( $discord_url ); ?>"
									class="regular-text"
									placeholder="https://discord.gg/..." />
								<p class="description"><?php esc_html_e( 'Standard-Discord-Link fuer alle Mods. Kann pro Mod ueberschrieben werden.', 'psyerns-mods' ); ?></p>
							</td>
						</tr>
						<tr>
							<th>
								<label for="psm_steam_api_key"><?php esc_html_e( 'Steam API Key', 'psyerns-mods' ); ?></label>
							</th>
							<td>
								<input type="password"
									id="psm_steam_api_key"
									name="psm_steam_api_key"
									value="<?php echo esc_attr( $steam_api_key ); ?>"
									class="regular-text"
									autocomplete="off" />
								<p class="description"><?php esc_html_e( 'Optional. Erhaeltlich unter https://steamcommunity.com/dev/apikey — erhoehte Rate Limits.', 'psyerns-mods' ); ?></p>
							</td>
						</tr>
					</table>
				</div>

				<!-- Discord Leaderboard Section -->
			<div class="psm-overview-box">
				<h2><?php esc_html_e( 'Discord Leaderboard', 'psyerns-mods' ); ?></h2>
				<p class="description"><?php esc_html_e( 'Leaderboard-Daten vom DayZ-Server werden automatisch als Discord Embed angezeigt und live aktualisiert.', 'psyerns-mods' ); ?></p>
				<table class="form-table">
					<tr>
						<th>
							<label for="psm_discord_enabled"><?php esc_html_e( 'Discord Push aktivieren', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<label class="psm-toggle">
								<input type="checkbox"
									id="psm_discord_enabled"
									name="psm_discord_enabled"
									value="1"
									<?php checked( get_option( 'psm_discord_enabled', '' ), '1' ); ?> />
								<span class="psm-toggle__track"></span>
							</label>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_webhook_url"><?php esc_html_e( 'Webhook URL', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="url"
								id="psm_discord_webhook_url"
								name="psm_discord_webhook_url"
								value="<?php echo esc_url( get_option( 'psm_discord_webhook_url', '' ) ); ?>"
								class="regular-text"
								placeholder="https://discord.com/api/webhooks/..." />
							<p class="description"><?php esc_html_e( 'Rechtsklick auf Channel > Integrationen > Webhook erstellen > URL kopieren.', 'psyerns-mods' ); ?></p>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_embed_title"><?php esc_html_e( 'Embed Titel', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="text"
								id="psm_discord_embed_title"
								name="psm_discord_embed_title"
								value="<?php echo esc_attr( get_option( 'psm_discord_embed_title', "\xF0\x9F\x8F\x86 Live Leaderboard" ) ); ?>"
								class="regular-text" />
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_embed_color"><?php esc_html_e( 'Embed Farbe', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="color"
								id="psm_discord_embed_color"
								name="psm_discord_embed_color"
								value="<?php echo esc_attr( get_option( 'psm_discord_embed_color', '#FFD700' ) ); ?>" />
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_max_players"><?php esc_html_e( 'Anzahl Spieler', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="number"
								id="psm_discord_max_players"
								name="psm_discord_max_players"
								value="<?php echo esc_attr( get_option( 'psm_discord_max_players', 10 ) ); ?>"
								min="1"
								max="25"
								step="1"
								class="small-text" />
							<span class="description"><?php esc_html_e( 'Top X Spieler anzeigen (max. 25)', 'psyerns-mods' ); ?></span>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_interval"><?php esc_html_e( 'Update-Intervall', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="number"
								id="psm_discord_interval"
								name="psm_discord_interval"
								value="<?php echo esc_attr( get_option( 'psm_discord_interval', 30 ) ); ?>"
								min="10"
								max="3600"
								step="1"
								class="small-text" />
							<span class="description"><?php esc_html_e( 'Sekunden (min. 10, empfohlen 30+)', 'psyerns-mods' ); ?></span>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_data_source"><?php esc_html_e( 'Datenquelle', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<?php $data_source = get_option( 'psm_discord_data_source', 'auto' ); ?>
							<select id="psm_discord_data_source" name="psm_discord_data_source">
								<option value="auto" <?php selected( $data_source, 'auto' ); ?>><?php esc_html_e( 'Automatisch erkennen', 'psyerns-mods' ); ?></option>
								<option value="topPVPPlayers" <?php selected( $data_source, 'topPVPPlayers' ); ?>>topPVPPlayers</option>
								<option value="topPVEPlayers" <?php selected( $data_source, 'topPVEPlayers' ); ?>>topPVEPlayers</option>
								<option value="players" <?php selected( $data_source, 'players' ); ?>>players</option>
							</select>
							<p class="description"><?php esc_html_e( 'Welches Array aus den empfangenen Daten als Spieler-Liste verwendet wird.', 'psyerns-mods' ); ?></p>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th><?php esc_html_e( 'Angezeigte Felder', 'psyerns-mods' ); ?></th>
						<td>
							<input type="hidden" name="psm_discord_fields[__placeholder]" value="0" />
							<?php
							$fields     = get_option( 'psm_discord_fields', array() );
							$field_defs = array(
								'kills'             => __( 'Kills', 'psyerns-mods' ),
								'deaths'            => __( 'Deaths', 'psyerns-mods' ),
								'kd_ratio'          => __( 'K/D Ratio', 'psyerns-mods' ),
								'zombie_kills'      => __( 'Zombie Kills', 'psyerns-mods' ),
								'longest_kill'      => __( 'Laengste Kill-Distanz', 'psyerns-mods' ),
								'playtime'          => __( 'Spielzeit', 'psyerns-mods' ),
								'distance_traveled' => __( 'Zurueckgelegte Distanz', 'psyerns-mods' ),
								'players_online'    => __( 'Spieler Online (Footer)', 'psyerns-mods' ),
								'total_players'     => __( 'Gesamt Spieler (Footer)', 'psyerns-mods' ),
							);

							foreach ( $field_defs as $key => $label ) :
								$checked = isset( $fields[ $key ] ) && '1' === $fields[ $key ];

								if ( ! is_array( $fields ) || empty( $fields ) ) {
									$checked = in_array( $key, array( 'kills', 'kd_ratio', 'players_online', 'total_players' ), true );
								}
								?>
								<label style="display:block; margin-bottom:4px;">
									<input type="checkbox"
										name="psm_discord_fields[<?php echo esc_attr( $key ); ?>]"
										value="1"
										<?php checked( $checked ); ?> />
									<?php echo esc_html( $label ); ?>
								</label>
							<?php endforeach; ?>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_discord_custom_fields"><?php esc_html_e( 'Eigene Felder', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<input type="text"
								id="psm_discord_custom_fields"
								name="psm_discord_custom_fields"
								value="<?php echo esc_attr( get_option( 'psm_discord_custom_fields', '' ) ); ?>"
								class="regular-text"
								placeholder="war_level, war_faction, hardline_reputation" />
							<p class="description">
								<?php esc_html_e( 'Komma-getrennte Feldnamen aus den Spielerdaten, die zusaetzlich im Discord-Embed angezeigt werden sollen.', 'psyerns-mods' ); ?>
							</p>
							<p class="description" style="margin-top: 6px;">
								<strong><?php esc_html_e( 'Verfuegbare Standard-Felder:', 'psyerns-mods' ); ?></strong><br>
								<code>steam_id</code> &mdash; <?php esc_html_e( 'Steam ID des Spielers', 'psyerns-mods' ); ?><br>
								<code>player_name</code> &mdash; <?php esc_html_e( 'Spielername', 'psyerns-mods' ); ?><br>
								<code>kills</code> &mdash; <?php esc_html_e( 'Gesamt-Kills (PvP)', 'psyerns-mods' ); ?><br>
								<code>deaths</code> &mdash; <?php esc_html_e( 'Gesamt-Tode', 'psyerns-mods' ); ?><br>
								<code>ai_kills</code> &mdash; <?php esc_html_e( 'KI-Kills (Zombies/AI)', 'psyerns-mods' ); ?><br>
								<code>pve_points</code> &mdash; <?php esc_html_e( 'PvE Punkte', 'psyerns-mods' ); ?><br>
								<code>pvp_points</code> &mdash; <?php esc_html_e( 'PvP Punkte', 'psyerns-mods' ); ?><br>
								<code>pve_deaths</code> &mdash; <?php esc_html_e( 'PvE Tode', 'psyerns-mods' ); ?><br>
								<code>pvp_deaths</code> &mdash; <?php esc_html_e( 'PvP Tode', 'psyerns-mods' ); ?><br>
								<code>longest_shot</code> &mdash; <?php esc_html_e( 'Laengster Kill in Metern', 'psyerns-mods' ); ?><br>
								<code>playtime</code> &mdash; <?php esc_html_e( 'Spielzeit in Stunden', 'psyerns-mods' ); ?><br>
								<code>is_online</code> &mdash; <?php esc_html_e( 'Online-Status (0/1)', 'psyerns-mods' ); ?><br>
								<code>last_login</code> &mdash; <?php esc_html_e( 'Letzter Login (Datum)', 'psyerns-mods' ); ?><br>
								<code>war_faction</code> &mdash; <?php esc_html_e( 'Kriegsfraktion (DayZ War Mod)', 'psyerns-mods' ); ?><br>
								<code>war_alignment</code> &mdash; <?php esc_html_e( 'Ausrichtung (Gut/Bose, Integer)', 'psyerns-mods' ); ?><br>
								<code>war_level</code> &mdash; <?php esc_html_e( 'Spieler-Level (War Mod)', 'psyerns-mods' ); ?><br>
								<code>war_boss_kills</code> &mdash; <?php esc_html_e( 'Boss-Kills (War Mod)', 'psyerns-mods' ); ?><br>
								<code>hardline_reputation</code> &mdash; <?php esc_html_e( 'Ruf-Punkte (Hardline Mod)', 'psyerns-mods' ); ?><br>
							</p>
							<p class="description" style="margin-top: 6px; color: #999;">
								<?php esc_html_e( 'Felder aus categoryKills, categoryDeaths und categoryLongestRanges sind JSON-Objekte und werden nicht unterstuetzt. Benutzerdefinierte Felder aus dem DayZ-Mod koennen hier eingetragen werden, wenn sie im Payload vorhanden sind.', 'psyerns-mods' ); ?>
							</p>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th>
							<label for="psm_leaderboard_public"><?php esc_html_e( 'REST API oeffentlich', 'psyerns-mods' ); ?></label>
						</th>
						<td>
							<label class="psm-toggle">
								<input type="checkbox"
									id="psm_leaderboard_public"
									name="psm_leaderboard_public"
									value="1"
									<?php checked( get_option( 'psm_leaderboard_public', '1' ), '1' ); ?> />
								<span class="psm-toggle__track"></span>
							</label>
							<p class="description"><?php esc_html_e( 'Wenn aktiv, ist GET /wp-json/psyern/v1/leaderboard ohne API-Key zugaenglich.', 'psyerns-mods' ); ?></p>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th><?php esc_html_e( 'Message ID', 'psyerns-mods' ); ?></th>
						<td>
							<?php
							$message_id = get_option( 'psm_discord_message_id', '' );
							?>
							<code id="psm_discord_message_id"><?php echo '' !== $message_id ? esc_html( $message_id ) : esc_html__( 'Noch nicht erstellt', 'psyerns-mods' ); ?></code>
							<p class="description"><?php esc_html_e( 'Wird automatisch gesetzt. Die Nachricht mit dieser ID wird bei jedem Update editiert.', 'psyerns-mods' ); ?></p>
						</td>
					</tr>
					<tr class="psm-discord-field">
						<th><?php esc_html_e( 'Verbindung testen', 'psyerns-mods' ); ?></th>
						<td>
							<?php
							$test_url = wp_nonce_url(
								admin_url( 'admin-post.php?action=psm_discord_test' ),
								'psm_discord_test_nonce'
							);
							?>
							<a href="<?php echo esc_url( $test_url ); ?>"
								id="psm_discord_test_btn"
								class="button button-secondary">
								<?php esc_html_e( 'Test-Nachricht senden', 'psyerns-mods' ); ?>
							</a>
							<?php if ( isset( $_GET['psm_discord_test'] ) ) : ?>
								<?php if ( '1' === sanitize_text_field( wp_unslash( $_GET['psm_discord_test'] ) ) ) : ?>
									<span style="color:#16a34a; margin-left:10px; font-weight:600;"><?php esc_html_e( 'Erfolgreich gesendet!', 'psyerns-mods' ); ?></span>
								<?php else : ?>
									<span style="color:#dc2626; margin-left:10px; font-weight:600;"><?php esc_html_e( 'Fehler — Webhook-URL pruefen!', 'psyerns-mods' ); ?></span>
								<?php endif; ?>
							<?php endif; ?>
						</td>
					</tr>
				</table>
			</div>

			<?php submit_button( __( 'Einstellungen speichern', 'psyerns-mods' ) ); ?>
			</form>

			<!-- Cache Section -->
			<div class="psm-overview-box">
				<h2><?php esc_html_e( 'Cache-Verwaltung', 'psyerns-mods' ); ?></h2>
				<p><?php esc_html_e( 'Loescht alle zwischengespeicherten Steam-Workshop-Daten. Die Daten werden beim naechsten Seitenaufruf neu geladen.', 'psyerns-mods' ); ?></p>
				<a href="<?php echo esc_url( $flush_url ); ?>"
					id="psm_flush_cache_btn"
					class="button button-secondary">
					<?php esc_html_e( 'Steam-Cache leeren', 'psyerns-mods' ); ?>
				</a>
			</div>
		</div>
		<?php
	}

	/**
	 * Handle the cache flush admin_post action.
	 *
	 * @return void
	 */
	public function handle_flush_cache() {
		if ( ! current_user_can( 'manage_options' ) ) {
			wp_die( esc_html__( 'Keine Berechtigung.', 'psyerns-mods' ), 403 );
		}

		check_admin_referer( 'psm_flush_cache_nonce' );

		if ( class_exists( 'PSM_Cache' ) ) {
			PSM_Cache::flush_all();
		}

		wp_safe_redirect(
			add_query_arg(
				array(
					'page'        => 'psm-settings',
					'psm_flushed' => '1',
				),
				admin_url( 'options-general.php' )
			)
		);
		exit;
	}

	/**
	 * Handle the Discord test message admin_post action.
	 *
	 * @return void
	 */
	public function handle_discord_test() {
		if ( ! current_user_can( 'manage_options' ) ) {
			wp_die( esc_html__( 'Keine Berechtigung.', 'psyerns-mods' ), 403 );
		}

		check_admin_referer( 'psm_discord_test_nonce' );

		$result = '0';

		if ( class_exists( 'PSM_Discord_Leaderboard' ) ) {
			$test = PSM_Discord_Leaderboard::send_test();
			if ( true === $test ) {
				$result = '1';
			}
		}

		wp_safe_redirect(
			add_query_arg(
				array(
					'page'              => 'psm-settings',
					'psm_discord_test'  => $result,
				),
				admin_url( 'options-general.php' )
			)
		);
		exit;
	}

	/**
	 * Enqueue admin CSS and JavaScript assets.
	 *
	 * Only loads on psyern_mod edit screens and the plugin settings page.
	 *
	 * @param string $hook_suffix The current admin page hook suffix.
	 * @return void
	 */
	public function enqueue_assets( $hook_suffix ) {
		$screen = get_current_screen();

		$is_mod_screen = (
			null !== $screen
			&& 'psyern_mod' === $screen->post_type
			&& in_array( $screen->base, array( 'post', 'edit' ), true )
		);

		$is_settings_screen = ( 'settings_page_psm-settings' === $hook_suffix );

		if ( ! $is_mod_screen && ! $is_settings_screen ) {
			return;
		}

		wp_enqueue_style(
			'psyerns-mods-admin',
			PSM_PLUGIN_URL . 'admin/css/psyerns-mods-admin.css',
			array(),
			PSM_VERSION
		);

		wp_enqueue_script(
			'psyerns-mods-admin',
			PSM_PLUGIN_URL . 'admin/js/psyerns-mods-admin.js',
			array( 'jquery' ),
			PSM_VERSION,
			true
		);

		wp_localize_script(
			'psyerns-mods-admin',
			'psmAdmin',
			array(
				'ajaxurl' => admin_url( 'admin-ajax.php' ),
				'nonce'   => wp_create_nonce( 'psm_admin_nonce' ),
				'i18n'    => array(
					'loading'      => __( 'Laden...', 'psyerns-mods' ),
					'fetchData'    => __( 'Daten laden', 'psyerns-mods' ),
					'fetchError'   => __( 'Fehler beim Laden der Steam-Daten.', 'psyerns-mods' ),
					'confirmFlush' => __( 'Cache wirklich leeren?', 'psyerns-mods' ),
					'noPreview'    => __( 'Keine Steam-Daten geladen.', 'psyerns-mods' ),
				),
			)
		);
	}
}

$psm_admin = new PSM_Admin();
$psm_admin->register();

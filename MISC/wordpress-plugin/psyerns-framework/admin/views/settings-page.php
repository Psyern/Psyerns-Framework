<?php
/**
 * Settings page — tabbed layout.
 * Tabs: API | Leaderboard | Themes | Server Status | Shortcodes
 *
 * @package Psyerns_Framework
 */
if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

$active_tab = isset( $_GET['tab'] ) ? sanitize_key( $_GET['tab'] ) : 'api';
$tabs = array(
	'api'         => array( 'label' => __( 'API',           'psyerns-framework' ), 'icon' => '🔑' ),
	'leaderboard' => array( 'label' => __( 'Leaderboard',   'psyerns-framework' ), 'icon' => '🏆' ),
	'themes'      => array( 'label' => __( 'Themes',        'psyerns-framework' ), 'icon' => '🎨' ),
	'status'      => array( 'label' => __( 'Server Status', 'psyerns-framework' ), 'icon' => '📡' ),
	'shortcodes'  => array( 'label' => __( 'Shortcodes',   'psyerns-framework' ), 'icon' => '📋' ),
);
$page_url = admin_url( 'admin.php?page=pf-settings' );
?>
<div class="wrap pf-admin-wrap">

	<h1 class="pf-admin-title">
		<span class="dashicons dashicons-shield"></span>
		<?php esc_html_e( 'Psyerns Framework', 'psyerns-framework' ); ?>
	</h1>

	<?php if ( isset( $_GET['settings-updated'] ) ) : ?>
		<div class="notice notice-success is-dismissible"><p><?php esc_html_e( 'Settings saved.', 'psyerns-framework' ); ?></p></div>
	<?php endif; ?>

	<!-- ── Tab Navigation (WordPress native) ── -->
	<nav class="nav-tab-wrapper" role="tablist">
		<?php foreach ( $tabs as $slug => $tab ) : ?>
		<a href="<?php echo esc_url( add_query_arg( 'tab', $slug, $page_url ) ); ?>"
		   class="nav-tab<?php echo $active_tab === $slug ? ' nav-tab-active' : ''; ?>"
		   role="tab">
			<?php echo esc_html( $tab['icon'] . ' ' . $tab['label'] ); ?>
		</a>
		<?php endforeach; ?>
	</nav>

	<div class="pf-tab-content">

	<?php /* ════════════════════════════════════════ TAB: API ══ */ ?>
	<?php if ( 'api' === $active_tab ) : ?>

		<div class="pf-card pf-card--info">
			<h3><?php esc_html_e( 'Quick Setup Guide', 'psyerns-framework' ); ?></h3>
			<ol>
				<li><?php echo wp_kses_post( __( 'Set an API Key below — or leave it empty and let the DayZ server auto-generate one on first start (check the server log for the generated key).', 'psyerns-framework' ) ); ?></li>
				<li><?php echo wp_kses_post( __( 'Enter the same key in your DayZ config: <code>PsyernsFrameworkConfig.json</code> → WordPress Endpoint → ApiKey.', 'psyerns-framework' ) ); ?></li>
				<li><?php echo wp_kses_post( sprintf( __( 'Set <code>BaseUrl</code> → <code>%s</code>', 'psyerns-framework' ), esc_url( rest_url( 'psyern/v1' ) ) ) ); ?></li>
				<li><?php esc_html_e( 'Enable the WordPress endpoint in the DayZ config and restart the server.', 'psyerns-framework' ); ?></li>
			</ol>
		</div>

		<?php
		$api_key  = get_option( 'pf_api_key', '' );
		$ping_url = rest_url( 'psyern/v1/ping' );
		if ( ! empty( $api_key ) ) { $ping_url .= '?api_key=' . $api_key; }
		?>
		<div class="pf-card pf-card--success">
			<h3><?php esc_html_e( 'Connection Test', 'psyerns-framework' ); ?></h3>
			<p><?php esc_html_e( 'Expected response:', 'psyerns-framework' ); ?> <code>{"status":"ok"}</code></p>
			<div class="pf-ping-row">
				<code class="pf-ping-url"><?php echo esc_html( $ping_url ); ?></code>
				<a href="<?php echo esc_url( $ping_url ); ?>" target="_blank" class="button button-secondary"><?php esc_html_e( 'Test Now', 'psyerns-framework' ); ?></a>
			</div>
			<?php if ( empty( $api_key ) ) : ?>
				<p class="pf-notice-inline pf-notice-inline--warn"><?php esc_html_e( 'API Key is not set yet.', 'psyerns-framework' ); ?></p>
			<?php endif; ?>
		</div>

		<form method="post" action="options.php" class="pf-settings-form">
			<?php settings_fields( 'pf_settings_api' ); ?>
			<div class="pf-card">
				<h3><?php esc_html_e( 'API Keys', 'psyerns-framework' ); ?></h3>
				<table class="form-table" role="presentation">
					<tr>
						<th scope="row"><label for="pf_api_key"><?php esc_html_e( 'API Key', 'psyerns-framework' ); ?></label></th>
						<td>
							<input type="text" id="pf_api_key" name="pf_api_key" value="<?php echo esc_attr( get_option( 'pf_api_key', '' ) ); ?>" class="regular-text" />
							<p class="description"><?php esc_html_e( 'Shared secret between DayZ server and this plugin.', 'psyerns-framework' ); ?></p>
						</td>
					</tr>
					<tr>
						<th scope="row"><label for="pf_steam_api_key"><?php esc_html_e( 'Steam API Key', 'psyerns-framework' ); ?></label></th>
						<td>
							<input type="text" id="pf_steam_api_key" name="pf_steam_api_key" value="<?php echo esc_attr( get_option( 'pf_steam_api_key', '' ) ); ?>" class="regular-text" />
							<p class="description"><?php echo wp_kses_post( __( 'Optional. For Steam avatars. Get at <a href="https://steamcommunity.com/dev/apikey" target="_blank">steamcommunity.com/dev/apikey</a>', 'psyerns-framework' ) ); ?></p>
						</td>
					</tr>
				</table>
			</div>
			<?php submit_button( __( 'Save API Settings', 'psyerns-framework' ) ); ?>
		</form>

	<?php /* ════════════════════════════════ TAB: LEADERBOARD ══ */ ?>
	<?php elseif ( 'leaderboard' === $active_tab ) : ?>

		<form method="post" action="options.php" class="pf-settings-form">
			<?php settings_fields( 'pf_settings_leaderboard' ); ?>

			<div class="pf-card">
				<h3><?php esc_html_e( 'Game Modes', 'psyerns-framework' ); ?></h3>
				<p class="description"><?php esc_html_e( 'Enable or disable the PvP and PvE tabs on the leaderboard.', 'psyerns-framework' ); ?></p>
				<table class="form-table" role="presentation">
					<tr>
						<th scope="row"><?php esc_html_e( 'PvP Mode', 'psyerns-framework' ); ?></th>
						<td>
							<input type="hidden" name="psyern_enable_pvp" value="0" />
							<label class="pf-toggle">
								<input type="checkbox" name="psyern_enable_pvp" value="1" <?php checked( '1', get_option( 'psyern_enable_pvp', '1' ) ); ?> />
								<span class="pf-toggle__slider"></span>
							</label>
							<span class="pf-toggle__label"><?php esc_html_e( 'Show PvP tab on leaderboard', 'psyerns-framework' ); ?></span>
						</td>
					</tr>
					<tr>
						<th scope="row"><?php esc_html_e( 'PvE Mode', 'psyerns-framework' ); ?></th>
						<td>
							<input type="hidden" name="psyern_enable_pve" value="0" />
							<label class="pf-toggle">
								<input type="checkbox" name="psyern_enable_pve" value="1" <?php checked( '1', get_option( 'psyern_enable_pve', '1' ) ); ?> />
								<span class="pf-toggle__slider"></span>
							</label>
							<span class="pf-toggle__label"><?php esc_html_e( 'Show PvE tab on leaderboard', 'psyerns-framework' ); ?></span>
						</td>
					</tr>
				</table>
			</div>

			<div class="pf-card">
				<h3><?php esc_html_e( 'Column Visibility', 'psyerns-framework' ); ?></h3>
				<p class="description"><?php esc_html_e( 'Choose which columns are displayed for each game mode. # and Name are always visible.', 'psyerns-framework' ); ?></p>

				<div class="pf-col-modes">
				<?php
				$col_defs = PF_Admin::get_column_definitions();
				$fixed    = array( 'rank', 'name' );
				foreach ( array( 'pvp' => 'PvP', 'pve' => 'PvE' ) as $mode_key => $mode_label ) :
					$option_name = 'pf_columns_' . $mode_key;
					$stored      = get_option( $option_name, '' );
					$enabled     = $stored ? json_decode( $stored, true ) : array_keys( $col_defs );
					if ( ! is_array( $enabled ) ) { $enabled = array_keys( $col_defs ); }
				?>
				<div class="pf-col-group">
					<h4 class="pf-col-group__title">
						<span class="pf-mode-badge pf-mode-badge--<?php echo esc_attr( $mode_key ); ?>"><?php echo esc_html( $mode_label ); ?></span>
						<?php esc_html_e( 'Columns', 'psyerns-framework' ); ?>
					</h4>
					<div class="pf-col-checks">
						<?php foreach ( $col_defs as $col_key => $col_label ) :
							$is_fixed = in_array( $col_key, $fixed, true );
							$checked  = $is_fixed || in_array( $col_key, $enabled, true );
						?>
						<label class="pf-col-check<?php echo $is_fixed ? ' pf-col-check--fixed' : ''; ?>">
							<input type="checkbox"
								name="<?php echo esc_attr( $option_name ); ?>[]"
								value="<?php echo esc_attr( $col_key ); ?>"
								<?php checked( $checked ); ?>
								<?php disabled( $is_fixed ); ?>
							/>
							<span class="pf-col-check__label"><?php echo esc_html( $col_label ); ?></span>
							<?php if ( $is_fixed ) : ?>
								<span class="pf-col-check__fixed-badge"><?php esc_html_e( 'fixed', 'psyerns-framework' ); ?></span>
								<input type="hidden" name="<?php echo esc_attr( $option_name ); ?>[]" value="<?php echo esc_attr( $col_key ); ?>" />
							<?php endif; ?>
						</label>
						<?php endforeach; ?>
					</div>
				</div>
				<?php endforeach; ?>
				</div>
			</div>

			<?php submit_button( __( 'Save Leaderboard Settings', 'psyerns-framework' ) ); ?>
		</form>

	<?php /* ════════════════════════════════════ TAB: THEMES ══ */ ?>
	<?php elseif ( 'themes' === $active_tab ) : ?>

		<form method="post" action="options.php" class="pf-settings-form">
			<?php settings_fields( 'pf_settings_themes' ); ?>
			<div class="pf-card">
				<h3><?php esc_html_e( 'Active Theme', 'psyerns-framework' ); ?></h3>
				<p class="description"><?php esc_html_e( 'Default theme for all leaderboard shortcodes. Override per-shortcode with theme="…".', 'psyerns-framework' ); ?></p>

				<?php
				$current_theme = get_option( 'psyern_theme', 'military' );
				$themes = array(
					'military'  => array( 'label' => 'Military — Tactical HUD',     'desc' => 'CRT scanlines, phosphor green, classified briefing aesthetic.',                         'swatch' => '#0a0f0a', 'accent' => '#4ade80' ),
					'ops'       => array( 'label' => 'Ops — CRT Terminal',           'desc' => 'Phosphor green CRT with scanlines, vignette, and flicker effects.',                    'swatch' => '#020d02', 'accent' => '#4ade80' ),
					'stalker'   => array( 'label' => 'Stalker — Radioactive Zone',   'desc' => 'S.T.A.L.K.E.R. inspired. Radiation orange, chromatic aberration, Geiger aesthetic.',  'swatch' => '#0c0a06', 'accent' => '#ff8c00' ),
					'outbreak'  => array( 'label' => 'Outbreak — Quarantine Zone',   'desc' => 'Hazard amber, warning stripes, biohazard aesthetic.',                                  'swatch' => '#0d0d00', 'accent' => '#f59e0b' ),
					'cyberpunk' => array( 'label' => 'Cyberpunk — Neon HUD',         'desc' => 'Matrix green, magenta neon glow. Glitch effects, HUD corners, scanlines.',             'swatch' => '#0a0a0f', 'accent' => '#00ff88' ),
					'inferno'   => array( 'label' => 'Inferno — Fire & Flames',      'desc' => 'Blazing hellfire. Ember particles, lava glow, flame gradient. Scorched earth.',        'swatch' => '#0a0200', 'accent' => '#ff4500' ),
					'ash'       => array( 'label' => 'Ash — Post-Apocalyptic',       'desc' => 'Weathered paper, rust, hand-drawn wanted poster style.',                               'swatch' => '#1a1714', 'accent' => '#c8392b' ),
					'frostbite' => array( 'label' => 'Frostbite — Eternal Winter',   'desc' => 'Ice & snow. Falling snowflakes, frost-rimmed borders, glacial glow.',                  'swatch' => '#060a12', 'accent' => '#5ba8e0' ),
				);
				?>
				<div class="pf-theme-grid">
					<?php foreach ( $themes as $slug => $theme ) : ?>
					<label class="pf-theme-card<?php echo $current_theme === $slug ? ' pf-theme-card--active' : ''; ?>">
						<input type="radio" name="psyern_theme" value="<?php echo esc_attr( $slug ); ?>" <?php checked( $current_theme, $slug ); ?> />
						<span class="pf-theme-card__preview" style="background:<?php echo esc_attr( $theme['swatch'] ); ?>;">
							<span class="pf-theme-card__accent" style="background:<?php echo esc_attr( $theme['accent'] ); ?>;"></span>
						</span>
						<span class="pf-theme-card__name"><?php echo esc_html( $theme['label'] ); ?></span>
						<span class="pf-theme-card__desc"><?php echo esc_html( $theme['desc'] ); ?></span>
					</label>
					<?php endforeach; ?>
				</div>

				<div class="pf-card pf-card--note" style="margin-top:16px;">
					<strong><?php esc_html_e( 'Override examples:', 'psyerns-framework' ); ?></strong><br>
					<code>[pf_leaderboard theme="stalker"]</code> &nbsp;
					<code>[pf_leaderboard theme="cyberpunk"]</code> &nbsp;
					<code>[pf_leaderboard theme="frostbite"]</code> &nbsp;
					<code>[pf_leaderboard theme="inferno"]</code>
				</div>
			</div>
			<?php submit_button( __( 'Save Theme', 'psyerns-framework' ) ); ?>
		</form>

	<?php /* ══════════════════════════════ TAB: SERVER STATUS ══ */ ?>
	<?php elseif ( 'status' === $active_tab ) : ?>

		<?php $status = get_transient( 'pf_server_status' ); ?>
		<?php if ( $status ) : ?>
		<div class="pf-card pf-card--success">
			<h3><?php esc_html_e( 'Live Server Data', 'psyerns-framework' ); ?></h3>
			<div class="pf-status-grid pf-status-grid--large">
				<?php
				$fields = array(
					__( 'Server Name', 'psyerns-framework' )    => esc_html( $status['serverName'] ?? '—' ),
					__( 'Players Online', 'psyerns-framework' ) => '<strong>' . intval( $status['playerCount'] ?? 0 ) . '</strong>',
					__( 'Map', 'psyerns-framework' )            => esc_html( $status['mapName'] ?? '—' ),
					__( 'Day Time', 'psyerns-framework' )       => esc_html( $status['dayTime'] ?? '—' ),
					__( 'Uptime', 'psyerns-framework' )         => intval( ( $status['uptimeSeconds'] ?? 0 ) / 60 ) . ' min',
					__( 'Last Update', 'psyerns-framework' )    => esc_html( $status['received_at'] ?? '—' ),
				);
				foreach ( $fields as $label => $value ) :
				?>
				<div class="pf-status-item">
					<span class="pf-status-item__label"><?php echo esc_html( $label ); ?></span>
					<span class="pf-status-item__value"><?php echo wp_kses_post( $value ); ?></span>
				</div>
				<?php endforeach; ?>
			</div>
		</div>
		<?php else : ?>
		<div class="pf-card pf-card--warn">
			<h3><?php esc_html_e( 'No Status Data', 'psyerns-framework' ); ?></h3>
			<p><?php esc_html_e( 'The server has not sent any status data yet. Make sure the DayZ mod is running and the endpoint is configured correctly.', 'psyerns-framework' ); ?></p>
		</div>
		<?php endif; ?>

	<?php /* ═══════════════════════════════ TAB: SHORTCODES ══ */ ?>
	<?php elseif ( 'shortcodes' === $active_tab ) : ?>

		<div class="pf-card">
			<h3><?php esc_html_e( 'Available Shortcodes', 'psyerns-framework' ); ?></h3>
			<table class="widefat pf-sc-table">
				<thead><tr>
					<th><?php esc_html_e( 'Shortcode', 'psyerns-framework' ); ?></th>
					<th><?php esc_html_e( 'Description', 'psyerns-framework' ); ?></th>
				</tr></thead>
				<tbody>
					<tr><td><code>[pf_leaderboard]</code></td><td><?php esc_html_e( 'Full leaderboard with PvE/PvP tabs, search, pagination.', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>[pf_server_status]</code></td><td><?php esc_html_e( 'Current server status widget.', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>[pf_top3_monthly]</code></td><td><?php esc_html_e( 'Top 3 players of the month.', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>[pf_top3_deadliest]</code></td><td><?php esc_html_e( 'Top 3 deadliest players.', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>[pf_top3_bosskills]</code></td><td><?php esc_html_e( 'Top 3 boss slayers.', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>[pf_player_card steam_id="..."]</code></td><td><?php esc_html_e( 'Single player stats card.', 'psyerns-framework' ); ?></td></tr>
				</tbody>
			</table>
		</div>

		<div class="pf-card">
			<h3><?php esc_html_e( 'Shortcode Attributes', 'psyerns-framework' ); ?></h3>
			<table class="widefat pf-sc-table">
				<thead><tr>
					<th><?php esc_html_e( 'Attribute', 'psyerns-framework' ); ?></th>
					<th><?php esc_html_e( 'Values', 'psyerns-framework' ); ?></th>
					<th><?php esc_html_e( 'Default', 'psyerns-framework' ); ?></th>
					<th><?php esc_html_e( 'Description', 'psyerns-framework' ); ?></th>
				</tr></thead>
				<tbody>
					<tr><td><code>theme</code></td><td><code>military</code> <code>ops</code> <code>stalker</code> <code>outbreak</code> <code>cyberpunk</code> <code>inferno</code> <code>ash</code> <code>frostbite</code></td><td><?php echo esc_html( get_option( 'psyern_theme', 'military' ) ); ?></td><td><?php esc_html_e( 'Visual theme', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>type</code></td><td><code>pvp</code> <code>pve</code></td><td><code>pvp</code></td><td><?php esc_html_e( 'Default board mode', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>limit</code></td><td><code>10</code> <code>20</code> <code>50</code></td><td><code>10</code></td><td><?php esc_html_e( 'Default number of rows', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>show_avatar</code></td><td><code>1</code> <code>0</code></td><td><code>1</code></td><td><?php esc_html_e( 'Show Steam avatars', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>show_playtime</code></td><td><code>1</code> <code>0</code></td><td><code>1</code></td><td><?php esc_html_e( 'Show playtime column', 'psyerns-framework' ); ?></td></tr>
				</tbody>
			</table>

			<div class="pf-sc-examples">
				<h4><?php esc_html_e( 'Examples', 'psyerns-framework' ); ?></h4>
				<code>[pf_leaderboard theme="stalker" type="pvp" limit="20"]</code><br>
				<code>[pf_leaderboard theme="military" type="pve" show_playtime="0"]</code><br>
				<code>[pf_player_card steam_id="76561198000000000"]</code>
			</div>
		</div>

	<?php endif; ?>
	</div><!-- .pf-tab-content -->
</div><!-- .wrap -->

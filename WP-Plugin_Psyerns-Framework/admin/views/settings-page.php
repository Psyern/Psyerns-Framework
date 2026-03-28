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

				/**
				 * Theme definitions.
				 * bg      = card/preview background
				 * accent  = primary highlight colour
				 * text    = main text colour
				 * dim     = dimmed / secondary text
				 * row_alt = alternating row tint
				 * border  = table / card border
				 * header  = column header text colour
				 * glow    = box-shadow glow colour (rgba string)
				 * badge   = rank badge / tag colour
				 */
				$themes = array(
					'military' => array(
						'label'   => 'Military',
						'sub'     => 'Tactical HUD',
						'desc'    => 'CRT scanlines · Phosphor green · Classified briefing aesthetic.',
						'bg'      => '#0a0f0a',
						'accent'  => '#4ade80',
						'text'    => '#a0d080',
						'dim'     => '#5a7a4a',
						'row_alt' => '#0d150d',
						'border'  => '#1a3a1a',
						'header'  => '#4ade80',
						'glow'    => 'rgba(74,222,128,.18)',
						'badge'   => '#4ade80',
					),
					'ops' => array(
						'label'   => 'Ops',
						'sub'     => 'CRT Terminal',
						'desc'    => 'Phosphor green CRT · Scanlines · Vignette · Flicker effects.',
						'bg'      => '#020d02',
						'accent'  => '#39ff14',
						'text'    => '#39ff14',
						'dim'     => '#1a7a0a',
						'row_alt' => '#050f05',
						'border'  => '#0d3a0d',
						'header'  => '#39ff14',
						'glow'    => 'rgba(57,255,20,.22)',
						'badge'   => '#39ff14',
					),
					'stalker' => array(
						'label'   => 'Stalker',
						'sub'     => 'Radioactive Zone',
						'desc'    => 'S.T.A.L.K.E.R. inspired · Radiation amber · Geiger aesthetic.',
						'bg'      => '#0c0a06',
						'accent'  => '#ff8c00',
						'text'    => '#d4c8a0',
						'dim'     => '#7a6a3a',
						'row_alt' => '#120e08',
						'border'  => '#3a3018',
						'header'  => '#ff8c00',
						'glow'    => 'rgba(255,140,0,.20)',
						'badge'   => '#ff8c00',
					),
					'outbreak' => array(
						'label'   => 'Outbreak',
						'sub'     => 'Quarantine Zone',
						'desc'    => 'Hazard amber · Warning stripes · Biohazard aesthetic.',
						'bg'      => '#0d0d00',
						'accent'  => '#f59e0b',
						'text'    => '#d4b860',
						'dim'     => '#6a5a10',
						'row_alt' => '#111100',
						'border'  => '#3a3000',
						'header'  => '#f59e0b',
						'glow'    => 'rgba(245,158,11,.20)',
						'badge'   => '#f59e0b',
					),
					'cyberpunk' => array(
						'label'   => 'Cyberpunk',
						'sub'     => 'Neon HUD',
						'desc'    => 'Matrix green · Magenta neon · Glitch effects · HUD corners.',
						'bg'      => '#0a0a0f',
						'accent'  => '#00ff88',
						'text'    => '#c0f0d0',
						'dim'     => '#306050',
						'row_alt' => '#0d0d14',
						'border'  => '#1a1a3a',
						'header'  => '#00ff88',
						'glow'    => 'rgba(0,255,136,.22)',
						'badge'   => '#ff00aa',
					),
					'inferno' => array(
						'label'   => 'Inferno',
						'sub'     => 'Fire & Flames',
						'desc'    => 'Blazing hellfire · Ember glow · Lava gradient · Scorched earth.',
						'bg'      => '#0a0200',
						'accent'  => '#ff4500',
						'text'    => '#e87040',
						'dim'     => '#6a2010',
						'row_alt' => '#0f0400',
						'border'  => '#3a1000',
						'header'  => '#ff6020',
						'glow'    => 'rgba(255,69,0,.24)',
						'badge'   => '#ff4500',
					),
					'ash' => array(
						'label'   => 'Ash',
						'sub'     => 'Post-Apocalyptic',
						'desc'    => 'Weathered paper · Rust · Hand-drawn wanted poster style.',
						'bg'      => '#1a1714',
						'accent'  => '#c8392b',
						'text'    => '#c8b89a',
						'dim'     => '#6a5a4a',
						'row_alt' => '#1e1b17',
						'border'  => '#3a2a1a',
						'header'  => '#c8392b',
						'glow'    => 'rgba(200,57,43,.16)',
						'badge'   => '#c8392b',
					),
					'frostbite' => array(
						'label'   => 'Frostbite',
						'sub'     => 'Eternal Winter',
						'desc'    => 'Ice & snow · Falling snowflakes · Frost-rimmed borders · Glacial glow.',
						'bg'      => '#060a12',
						'accent'  => '#5ba8e0',
						'text'    => '#a8d4f0',
						'dim'     => '#3a5a7a',
						'row_alt' => '#080c18',
						'border'  => '#1a2a4a',
						'header'  => '#5ba8e0',
						'glow'    => 'rgba(91,168,224,.20)',
						'badge'   => '#5ba8e0',
					),
					'bubblegum' => array(
						'label'   => 'Bubblegum',
						'sub'     => 'Candy Pop',
						'desc'    => 'Sweet & playful · Rising bubbles · Candy glow · Pink shimmer.',
						'bg'      => '#120818',
						'accent'  => '#ff69b4',
						'text'    => '#f0d8e8',
						'dim'     => '#7a3a60',
						'row_alt' => '#1a0e22',
						'border'  => '#2a1240',
						'header'  => '#ff69b4',
						'glow'    => 'rgba(255,105,180,.20)',
						'badge'   => '#ffb6da',
					),
				);

				/* Mock leaderboard rows shown in the preview */
				$mock_rows = array(
					array( '1', 'Gh0stWalker',  '4.7', '312' ),
					array( '2', 'NomadSurvivor','3.1', '187' ),
					array( '3', 'IronSight',    '2.6', '144' ),
				);
				?>
				<div class="pf-theme-grid-wrap">
				<div class="pf-theme-grid">
					<?php foreach ( $themes as $slug => $theme ) : ?>
					<?php
					$bg      = esc_attr( $theme['bg'] );
					$accent  = esc_attr( $theme['accent'] );
					$text    = esc_attr( $theme['text'] );
					$dim     = esc_attr( $theme['dim'] );
					$row_alt = esc_attr( $theme['row_alt'] );
					$border  = esc_attr( $theme['border'] );
					$header  = esc_attr( $theme['header'] );
					$glow    = esc_attr( $theme['glow'] );
					$badge   = esc_attr( $theme['badge'] );
					?>
					<label class="pf-theme-card<?php echo $current_theme === $slug ? ' pf-theme-card--active' : ''; ?>">
						<input type="radio" name="psyern_theme" value="<?php echo esc_attr( $slug ); ?>" <?php checked( $current_theme, $slug ); ?> />

						<!-- ░░ Mini Leaderboard Preview ░░ -->
						<span class="pf-theme-card__preview" style="background:<?php echo $bg; ?>; box-shadow:inset 0 0 20px <?php echo $glow; ?>;">

							<!-- scanline overlay for dark themes -->
							<span class="pf-theme-card__scanlines"></span>

							<!-- mini header bar -->
							<span class="pf-theme-card__mini-header" style="border-bottom:1px solid <?php echo $border; ?>; background:<?php echo $row_alt; ?>;">
								<span style="color:<?php echo $accent; ?>; font-size:7px; font-weight:700; letter-spacing:.12em; text-transform:uppercase; opacity:.9;"><?php echo esc_html( strtoupper( $slug ) ); ?></span>
								<span class="pf-theme-card__mini-dots">
									<span style="background:<?php echo $accent; ?>;"></span>
									<span style="background:<?php echo $accent; ?>;"></span>
									<span style="background:<?php echo $accent; ?>;"></span>
								</span>
							</span>

							<!-- column labels -->
							<span class="pf-theme-card__mini-cols" style="border-bottom:1px solid <?php echo $border; ?>;">
								<span style="color:<?php echo $header; ?>;">#</span>
								<span style="color:<?php echo $header; ?>; flex:1;"><?php esc_html_e( 'Player', 'psyerns-framework' ); ?></span>
								<span style="color:<?php echo $header; ?>;">K/D</span>
								<span style="color:<?php echo $header; ?>;">Kills</span>
							</span>

							<!-- mock data rows -->
							<?php foreach ( $mock_rows as $i => $row ) :
								$row_bg = $i % 2 === 1 ? $row_alt : 'transparent';
							?>
							<span class="pf-theme-card__mini-row" style="background:<?php echo esc_attr( $row_bg ); ?>;">
								<span class="pf-theme-card__mini-rank" style="color:<?php echo $badge; ?>; border:1px solid <?php echo $badge; ?>;"><?php echo esc_html( $row[0] ); ?></span>
								<span style="color:<?php echo $text; ?>; flex:1; overflow:hidden; text-overflow:ellipsis; white-space:nowrap;"><?php echo esc_html( $row[1] ); ?></span>
								<span style="color:<?php echo $accent; ?>;"><?php echo esc_html( $row[2] ); ?></span>
								<span style="color:<?php echo $dim; ?>;"><?php echo esc_html( $row[3] ); ?></span>
							</span>
							<?php endforeach; ?>

							<!-- bottom accent bar -->
							<span class="pf-theme-card__mini-bar" style="background:linear-gradient(90deg, <?php echo $accent; ?> 0%, transparent 100%); box-shadow:0 0 8px <?php echo $glow; ?>;"></span>
						</span>

						<span class="pf-theme-card__name">
							<?php echo esc_html( $theme['label'] ); ?>
							<span class="pf-theme-card__sub" style="color:<?php echo $accent; ?>;"><?php echo esc_html( $theme['sub'] ); ?></span>
						</span>
						<span class="pf-theme-card__desc"><?php echo esc_html( $theme['desc'] ); ?></span>
					</label>
					<?php endforeach; ?>
				</div><!-- .pf-theme-grid -->
				</div><!-- .pf-theme-grid-wrap -->

				<div class="pf-card pf-card--note" style="margin-top:16px;">
					<strong><?php esc_html_e( 'Override examples:', 'psyerns-framework' ); ?></strong>
					<p class="description" style="margin:4px 0 10px;"><?php esc_html_e( 'Use the theme attribute in any shortcode to override the global default per-page.', 'psyerns-framework' ); ?></p>
					<div class="pf-override-grid">
						<?php
						$override_themes = array(
							array( 'slug' => 'military',  'label' => 'Military',  'color' => '#4ade80' ),
							array( 'slug' => 'ops',       'label' => 'Ops',       'color' => '#00ff41' ),
							array( 'slug' => 'stalker',   'label' => 'Stalker',   'color' => '#d4c8a0' ),
							array( 'slug' => 'outbreak',  'label' => 'Outbreak',  'color' => '#fde68a' ),
							array( 'slug' => 'cyberpunk', 'label' => 'Cyberpunk', 'color' => '#00ffff' ),
							array( 'slug' => 'inferno',   'label' => 'Inferno',   'color' => '#f0a040' ),
							array( 'slug' => 'ash',       'label' => 'Ash',       'color' => '#c8b8a0' ),
							array( 'slug' => 'frostbite', 'label' => 'Frostbite', 'color' => '#5ba8e0' ),
							array( 'slug' => 'bubblegum', 'label' => 'Bubblegum', 'color' => '#ff69b4' ),
						);
						foreach ( $override_themes as $ot ) :
							$sc = '[pf_leaderboard theme="' . esc_attr( $ot['slug'] ) . '"]';
						?>
						<div class="pf-copy-row">
							<span style="flex-shrink:0;width:10px;height:10px;border-radius:50%;background:<?php echo esc_attr( $ot['color'] ); ?>;display:inline-block;"></span>
							<code title="<?php echo esc_attr( $sc ); ?>"><?php echo esc_html( $sc ); ?></code>
							<button type="button" class="pf-copy-btn" data-copy="<?php echo esc_attr( $sc ); ?>">
								<span class="dashicons dashicons-clipboard"></span><?php esc_html_e( 'Copy', 'psyerns-framework' ); ?>
							</button>
						</div>
						<?php endforeach; ?>
					</div>
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
					<tr><td><code>theme</code></td><td><code>military</code> <code>ops</code> <code>stalker</code> <code>outbreak</code> <code>cyberpunk</code> <code>inferno</code> <code>ash</code> <code>frostbite</code> <code>bubblegum</code></td><td><?php echo esc_html( get_option( 'psyern_theme', 'military' ) ); ?></td><td><?php esc_html_e( 'Visual theme', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>type</code></td><td><code>pvp</code> <code>pve</code></td><td><code>pvp</code></td><td><?php esc_html_e( 'Default board mode', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>limit</code></td><td><code>10</code> <code>20</code> <code>50</code></td><td><code>10</code></td><td><?php esc_html_e( 'Default number of rows', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>show_avatar</code></td><td><code>1</code> <code>0</code></td><td><code>1</code></td><td><?php esc_html_e( 'Show Steam avatars', 'psyerns-framework' ); ?></td></tr>
					<tr><td><code>show_playtime</code></td><td><code>1</code> <code>0</code></td><td><code>1</code></td><td><?php esc_html_e( 'Show playtime column', 'psyerns-framework' ); ?></td></tr>
				</tbody>
			</table>

			<div class="pf-sc-examples">
				<h4><?php esc_html_e( 'Examples', 'psyerns-framework' ); ?></h4>
				<?php
				$sc_examples = array(
					'[pf_leaderboard theme="stalker" type="pvp" limit="20"]',
					'[pf_leaderboard theme="military" type="pve" show_playtime="0"]',
					'[pf_leaderboard theme="cyberpunk" type="pvp" limit="50"]',
					'[pf_leaderboard theme="ops" type="pve"]',
					'[pf_leaderboard theme="outbreak" type="pvp"]',
					'[pf_leaderboard theme="inferno" type="pve" limit="10"]',
					'[pf_leaderboard theme="ash" type="pve"]',
					'[pf_leaderboard theme="frostbite" type="pvp" limit="20"]',
					'[pf_leaderboard theme="bubblegum" type="pve"]',
					'[pf_player_card steam_id="76561198000000000"]',
				);
				foreach ( $sc_examples as $sc ) :
				?>
				<div class="pf-copy-row">
					<code title="<?php echo esc_attr( $sc ); ?>"><?php echo esc_html( $sc ); ?></code>
					<button type="button" class="pf-copy-btn" data-copy="<?php echo esc_attr( $sc ); ?>">
						<span class="dashicons dashicons-clipboard"></span><?php esc_html_e( 'Copy', 'psyerns-framework' ); ?>
					</button>
				</div>
				<?php endforeach; ?>
			</div>
		</div>

	<?php endif; ?>
	</div><!-- .pf-tab-content -->
</div><!-- .wrap -->

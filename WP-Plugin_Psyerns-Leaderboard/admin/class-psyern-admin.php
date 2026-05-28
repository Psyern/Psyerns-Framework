<?php
/**
 * Leaderboard admin settings — theme selection with live preview, module toggles.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Class Psyern_Admin
 *
 * Adds leaderboard-specific settings to the Psyerns Framework admin page.
 */
class Psyern_Admin {

	/**
	 * Register admin hooks.
	 *
	 * @return void
	 */
	public function init() {
		add_action( 'admin_init', array( $this, 'register_settings' ) );
	}

	/**
	 * Register leaderboard settings fields.
	 *
	 * @return void
	 */
	public function register_settings() {
		register_setting( 'pf_settings', 'psyern_theme', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => 'military' ) );
		register_setting( 'pf_settings', 'psyern_enable_leaderboard', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => '1' ) );
		register_setting( 'pf_settings', 'psyern_enable_pvp', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => '1' ) );
		register_setting( 'pf_settings', 'psyern_enable_pve', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => '1' ) );
		register_setting( 'pf_settings', 'psyern_enable_top3', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => '1' ) );
		register_setting( 'pf_settings', 'psyern_enable_avatars', array( 'sanitize_callback' => 'sanitize_text_field', 'default' => '1' ) );

		add_settings_section(
			'psyern_lb_section',
			__( 'Leaderboard Settings', 'psyerns-framework' ),
			function() {
				echo '<p>' . esc_html__( 'Configure the leaderboard display and theme.', 'psyerns-framework' ) . '</p>';
			},
			'pf-settings'
		);

		add_settings_field( 'psyern_theme', __( 'Design Theme', 'psyerns-framework' ), array( $this, 'render_theme_selector' ), 'pf-settings', 'psyern_lb_section' );

		$toggles = array(
			'psyern_enable_leaderboard' => __( 'Enable Leaderboard', 'psyerns-framework' ),
			'psyern_enable_pvp'         => __( 'Enable PvP Mode', 'psyerns-framework' ),
			'psyern_enable_pve'         => __( 'Enable PvE Mode', 'psyerns-framework' ),
			'psyern_enable_top3'        => __( 'Enable Weekly Top 3', 'psyerns-framework' ),
			'psyern_enable_avatars'     => __( 'Enable Steam Avatars', 'psyerns-framework' ),
		);

		foreach ( $toggles as $key => $label ) {
			add_settings_field( $key, $label, function() use ( $key ) {
				$val = get_option( $key, '1' );
				echo '<label><input type="checkbox" name="' . esc_attr( $key ) . '" value="1"' . checked( '1', $val, false ) . ' /> ';
				echo esc_html__( 'Enabled', 'psyerns-framework' ) . '</label>';
			}, 'pf-settings', 'psyern_lb_section' );
		}
	}

	/**
	 * Theme definitions with preview data.
	 *
	 * @return array
	 */
	private function get_themes() {
		return array(
			'military' => array(
				'label' => __( 'Military — Tactical HUD', 'psyerns-framework' ),
				'desc'  => __( 'CRT scanlines, phosphor green, classified briefing aesthetic.', 'psyerns-framework' ),
				'color' => '#4ade80',
				'bg'    => '#0a0f0a',
				'card'  => '#0d150d',
				'border' => '#1a3a1a',
				'text'  => '#4ade80',
				'muted' => '#3a7a3a',
				'rank'  => '#86efac',
				'font'  => "'Share Tech Mono', monospace",
			),
			'ash' => array(
				'label' => __( 'Ash — Post-Apocalyptic', 'psyerns-framework' ),
				'desc'  => __( 'Weathered paper, rust, hand-drawn wanted poster style.', 'psyerns-framework' ),
				'color' => '#c8392b',
				'bg'    => '#1a1714',
				'card'  => '#2d2825',
				'border' => '#3d3530',
				'text'  => '#f5f0e8',
				'muted' => '#8a7e70',
				'rank'  => '#d4a853',
				'font'  => "'Playfair Display', serif",
			),
			'ops' => array(
				'label' => __( 'Ops — CRT Military Terminal', 'psyerns-framework' ),
				'desc'  => __( 'Phosphor green CRT with scanlines, vignette, and flicker effects.', 'psyerns-framework' ),
				'color' => '#4ade80',
				'bg'    => '#020d02',
				'card'  => '#041a04',
				'border' => '#0d3b0d',
				'text'  => '#4ade80',
				'muted' => '#166534',
				'rank'  => '#86efac',
				'font'  => "'Share Tech Mono', monospace",
			),
			'outbreak' => array(
				'label' => __( 'Outbreak — Quarantine Zone', 'psyerns-framework' ),
				'desc'  => __( 'Hazard amber, warning stripes, biohazard aesthetic.', 'psyerns-framework' ),
				'color' => '#f59e0b',
				'bg'    => '#0d0d00',
				'card'  => '#111100',
				'border' => '#78350f',
				'text'  => '#fde68a',
				'muted' => '#78350f',
				'rank'  => '#f59e0b',
				'font'  => "'Rajdhani', sans-serif",
			),
			'cyberpunk' => array(
				'label' => __( 'Cyberpunk — Neon HUD', 'psyerns-framework' ),
				'desc'  => __( 'Matrix green, magenta, cyan neon glow. Glitch effects, HUD corners, scanlines.', 'psyerns-framework' ),
				'color' => '#00ff88',
				'bg'    => '#0a0a0f',
				'card'  => '#12121a',
				'border' => '#2a2a3a',
				'text'  => '#e0e0e0',
				'muted' => '#4a4a6a',
				'rank'  => '#ff00ff',
				'font'  => "'JetBrains Mono', monospace",
			),
			'stalker' => array(
				'label' => __( 'Stalker — Radioactive Zone', 'psyerns-framework' ),
				'desc'  => __( 'S.T.A.L.K.E.R. inspired. Radiation orange, chromatic aberration, film grain, Geiger counter aesthetic.', 'psyerns-framework' ),
				'color' => '#ff8c00',
				'bg'    => '#0c0a06',
				'card'  => '#141008',
				'border' => '#3a3018',
				'text'  => '#d4c8a0',
				'muted' => '#6b5d3a',
				'rank'  => '#ffcc00',
				'font'  => "'Courier Prime', monospace",
			),
			'inferno' => array(
				'label' => __( 'Inferno — Fire & Flames', 'psyerns-framework' ),
				'desc'  => __( 'Blazing hellfire. Ember particles, lava glow, flame gradient. Scorched earth.', 'psyerns-framework' ),
				'color' => '#ff4500',
				'bg'    => '#0a0200',
				'card'  => '#1a0800',
				'border' => '#3d1800',
				'text'  => '#f0d8b0',
				'muted' => '#7a4a20',
				'rank'  => '#ff6a00',
				'font'  => "'Teko', sans-serif",
			),
			'frostbite' => array(
				'label' => __( 'Frostbite — Eternal Winter', 'psyerns-framework' ),
				'desc'  => __( 'Ice & snow. Falling snowflakes, frost-rimmed borders, glacial glow, frozen tundra.', 'psyerns-framework' ),
				'color' => '#5ba8e0',
				'bg'    => '#060a12',
				'card'  => '#0a1220',
				'border' => '#152540',
				'text'  => '#c8ddf0',
				'muted' => '#3a6080',
				'rank'  => '#a0d4ff',
				'font'  => "'Nunito Sans', sans-serif",
			),
			'bubblegum' => array(
				'label' => __( 'Bubblegum — Candy Pop', 'psyerns-framework' ),
				'desc'  => __( 'Sweet & playful. Rising bubbles, candy glow, pink shimmer, sugar sparkle.', 'psyerns-framework' ),
				'color' => '#ff69b4',
				'bg'    => '#120818',
				'card'  => '#1a0e22',
				'border' => '#2a1240',
				'text'  => '#f0d8e8',
				'muted' => '#7a3a60',
				'rank'  => '#ffb6da',
				'font'  => "'Quicksand', sans-serif",
			),
		);
	}

	/**
	 * Render theme selector with inline mini-preview.
	 *
	 * @return void
	 */
	public function render_theme_selector() {
		$current = get_option( 'psyern_theme', 'military' );
		$themes  = $this->get_themes();

		echo '<div class="psyern-theme-grid" style="display:grid;grid-template-columns:1fr 1fr;gap:12px;max-width:800px;">';

		foreach ( $themes as $slug => $t ) {
			$is_active = ( $slug === $current );
			$border    = $is_active ? esc_attr( $t['color'] ) : '#ccc';
			$shadow    = $is_active ? 'box-shadow:0 0 0 2px ' . esc_attr( $t['color'] ) . '33;' : '';
			?>
			<label style="display:block;border:2px solid <?php echo $border; ?>;border-radius:8px;overflow:hidden;cursor:pointer;transition:border-color .2s,box-shadow .2s;<?php echo $shadow; ?>background:#fff;">
				<input type="radio" name="psyern_theme" value="<?php echo esc_attr( $slug ); ?>" <?php checked( $slug, $current ); ?> style="position:absolute;opacity:0;pointer-events:none;" onchange="document.querySelectorAll('.psyern-theme-grid > label').forEach(function(l){l.style.borderColor='#ccc';l.style.boxShadow='none'});this.closest('label').style.borderColor='<?php echo esc_js( $t['color'] ); ?>';this.closest('label').style.boxShadow='0 0 0 2px <?php echo esc_js( $t['color'] ); ?>33';" />

				<?php // --- Mini Preview --- ?>
				<div style="background:<?php echo esc_attr( $t['bg'] ); ?>;padding:12px 14px;font-family:<?php echo esc_attr( $t['font'] ); ?>;font-size:11px;line-height:1.4;min-height:120px;position:relative;overflow:hidden;">

					<?php // Title bar ?>
					<div style="color:<?php echo esc_attr( $t['color'] ); ?>;font-weight:700;font-size:10px;text-transform:uppercase;letter-spacing:0.1em;margin-bottom:8px;opacity:0.9;">
						<?php echo esc_html( strtoupper( $slug ) ); ?> LEADERBOARD
					</div>

					<?php // Mode tabs ?>
					<div style="display:flex;gap:4px;margin-bottom:8px;">
						<span style="padding:2px 8px;font-size:9px;border:1px solid <?php echo esc_attr( $t['color'] ); ?>;color:<?php echo esc_attr( $t['color'] ); ?>;border-radius:2px;background:<?php echo esc_attr( $t['color'] ); ?>15;">PVP</span>
						<span style="padding:2px 8px;font-size:9px;border:1px solid <?php echo esc_attr( $t['border'] ); ?>;color:<?php echo esc_attr( $t['muted'] ); ?>;border-radius:2px;">PVE</span>
					</div>

					<?php // Mini leaderboard rows ?>
					<div style="border-top:1px solid <?php echo esc_attr( $t['border'] ); ?>;padding-top:6px;">
						<?php
						$demo_players = array(
							array( '#1', 'GhostSniper', '247', '12', '20.6' ),
							array( '#2', 'SurvivorX', '198', '31', '6.4' ),
							array( '#3', 'NomadWolf', '156', '8', '19.5' ),
						);
						foreach ( $demo_players as $i => $dp ) :
							$row_border = ( 0 === $i ) ? esc_attr( $t['color'] ) : 'transparent';
							?>
							<div style="display:flex;align-items:center;gap:6px;padding:3px 0;border-left:2px solid <?php echo $row_border; ?>;padding-left:6px;<?php echo ( 0 === $i ) ? 'background:' . esc_attr( $t['color'] ) . '08;' : ''; ?>">
								<span style="color:<?php echo esc_attr( $t['rank'] ); ?>;font-weight:700;font-size:10px;min-width:18px;"><?php echo esc_html( $dp[0] ); ?></span>
								<span style="color:<?php echo esc_attr( $t['text'] ); ?>;font-weight:600;font-size:10px;flex:1;"><?php echo esc_html( $dp[1] ); ?></span>
								<span style="color:<?php echo esc_attr( $t['muted'] ); ?>;font-size:9px;">K:<?php echo esc_html( $dp[2] ); ?></span>
								<span style="color:<?php echo esc_attr( $t['muted'] ); ?>;font-size:9px;">D:<?php echo esc_html( $dp[3] ); ?></span>
								<span style="color:<?php echo esc_attr( $t['color'] ); ?>;font-size:9px;font-weight:700;"><?php echo esc_html( $dp[4] ); ?></span>
							</div>
						<?php endforeach; ?>
					</div>
				</div>

				<?php // Label bar below preview ?>
				<div style="padding:10px 14px;background:#fff;border-top:1px solid #eee;">
					<strong style="color:<?php echo esc_attr( $t['color'] ); ?>;font-size:13px;"><?php echo esc_html( $t['label'] ); ?></strong><br>
					<span style="color:#666;font-size:11px;"><?php echo esc_html( $t['desc'] ); ?></span>
				</div>
			</label>
			<?php
		}

		echo '</div>';
	}
}

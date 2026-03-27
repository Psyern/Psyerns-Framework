<?php
/**
 * Shortcode rendering for Psyerns Mods Showreel.
 *
 * Provides the [psyerns_mods] shortcode that displays a responsive
 * grid of mod cards with Steam and Discord links.
 *
 * @package Psyerns_Mods
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

/**
 * Handles shortcode registration, rendering and conditional asset loading.
 */
class PSM_Shortcode {

	/**
	 * Whether the shortcode has been rendered on the current page.
	 *
	 * Used to conditionally enqueue assets only when needed.
	 *
	 * @var bool
	 */
	private static $enqueued = false;

	/**
	 * Register the shortcode and hook asset loading.
	 *
	 * @return void
	 */
	public function register() {
		add_shortcode( 'psyerns_mods', array( $this, 'render' ) );
		add_action( 'wp_enqueue_scripts', array( $this, 'register_assets' ) );
		add_action( 'wp_footer', array( $this, 'maybe_enqueue_assets' ) );
	}

	/**
	 * Register (but do not enqueue) CSS and JS assets.
	 *
	 * Assets are only enqueued when the shortcode is actually present
	 * on the page, via the wp_footer hook.
	 *
	 * @return void
	 */
	public function register_assets() {
		wp_register_style(
			'psyerns-mods-public',
			PSM_PLUGIN_URL . 'public/css/psyerns-mods-public.css',
			array(),
			PSM_VERSION
		);

		wp_register_script(
			'psyerns-mods-public',
			PSM_PLUGIN_URL . 'public/js/psyerns-mods-public.js',
			array(),
			PSM_VERSION,
			true
		);
	}

	/**
	 * Enqueue assets in the footer only if the shortcode was rendered.
	 *
	 * @return void
	 */
	public function maybe_enqueue_assets() {
		if ( true === self::$enqueued ) {
			wp_enqueue_style( 'psyerns-mods-public' );
			wp_enqueue_script( 'psyerns-mods-public' );
		}
	}

	/**
	 * Render the [psyerns_mods] shortcode.
	 *
	 * @param array|string $atts Shortcode attributes.
	 * @return string HTML output.
	 */
	public function render( $atts ) {
		$atts = shortcode_atts(
			array(
				'columns' => 3,
				'discord' => '',
				'orderby' => 'menu_order',
				'order'   => 'ASC',
				'limit'   => -1,
			),
			$atts,
			'psyerns_mods'
		);

		$columns = absint( $atts['columns'] );
		$order   = strtoupper( sanitize_text_field( $atts['order'] ) );

		if ( 'DESC' !== $order ) {
			$order = 'ASC';
		}

		$limit           = intval( $atts['limit'] );
		$discord_override = esc_url_raw( $atts['discord'] );

		self::$enqueued = true;

		$args = array(
			'post_type'      => 'psyern_mod',
			'posts_per_page' => $limit,
			'post_status'    => 'publish',
			'meta_key'       => '_psm_order',
			'orderby'        => 'meta_value_num',
			'order'          => $order,
		);

		$query = new WP_Query( $args );

		if ( ! $query->have_posts() ) {
			wp_reset_postdata();
			return $this->render_empty_state();
		}

		ob_start();
		?>
		<div class="psyerns-mods-grid" data-columns="<?php echo esc_attr( $columns ); ?>">
			<div class="psyerns-mods-container">
				<?php
				while ( $query->have_posts() ) {
					$query->the_post();
					$this->render_card( get_the_ID(), $discord_override );
				}
				?>
			</div>
		</div>
		<?php
		wp_reset_postdata();

		return ob_get_clean();
	}

	/**
	 * Render a single mod card.
	 *
	 * @param int    $post_id          The mod post ID.
	 * @param string $discord_override Discord URL from shortcode attribute.
	 * @return void
	 */
	private function render_card( $post_id, $discord_override ) {
		$title       = get_the_title( $post_id );
		$steam_data  = $this->get_steam_data( $post_id );
		$preview_url = $this->get_preview_url( $post_id, $steam_data );
		$description = $this->get_description( $steam_data );
		$steam_url   = PSM_Meta::get( $post_id, PSM_Meta::STEAM_URL );
		$discord_url = $this->get_discord_url( $post_id, $discord_override );

		$is_free      = '1' === PSM_Meta::get( $post_id, PSM_Meta::IS_FREE );
		$price        = floatval( PSM_Meta::get( $post_id, PSM_Meta::PRICE ) );
		$has_repack   = '1' === PSM_Meta::get( $post_id, PSM_Meta::HAS_REPACK );
		$repack_price = floatval( PSM_Meta::get( $post_id, PSM_Meta::REPACK_PRICE ) );
		$has_source   = '1' === PSM_Meta::get( $post_id, PSM_Meta::HAS_SOURCE );
		$source_price = floatval( PSM_Meta::get( $post_id, PSM_Meta::SOURCE_PRICE ) );

		$discount_enabled = '1' === get_option( 'psm_discount_enabled', '' );
		$discount_percent = absint( get_option( 'psm_discount_percent', 0 ) );
		$discount_label   = sanitize_text_field( get_option( 'psm_discount_label', '' ) );
		$has_discount     = $discount_enabled && $discount_percent > 0 && ! $is_free;

		/*
		 * Use Steam title as display name if available, otherwise fall back
		 * to the WordPress post title.
		 */
		if ( is_array( $steam_data ) && ! empty( $steam_data['title'] ) ) {
			$title = $steam_data['title'];
		}
		?>
		<div class="psyerns-mods-item">
			<div class="psm-card">
				<div class="psm-card__image-wrap">
					<img class="psm-card__image" src="<?php echo esc_url( $preview_url ); ?>" alt="<?php echo esc_attr( $title ); ?>" loading="lazy" decoding="async">
				</div>
				<div class="psm-card__body">
					<h3 class="psm-card__title"><?php echo esc_html( $title ); ?></h3>
					<?php if ( '' !== $description ) : ?>
						<p class="psm-card__description"><?php echo esc_html( $description ); ?></p>
					<?php endif; ?>
					<div class="psm-card__badges">
						<?php $this->render_badges( $is_free, $price, $has_repack, $repack_price, $has_source, $source_price, $has_discount, $discount_percent, $discount_label ); ?>
					</div>
					<div class="psm-card__actions">
						<?php if ( '' !== $steam_url ) : ?>
							<a class="psm-btn psm-btn--steam" href="<?php echo esc_url( $steam_url ); ?>" target="_blank" rel="noopener noreferrer">
								<?php echo $this->get_steam_icon(); ?>
								<?php esc_html_e( 'AUF STEAM ANSEHEN', 'psyerns-mods' ); ?>
							</a>
						<?php endif; ?>
						<?php if ( '' !== $discord_url ) : ?>
							<a class="psm-btn psm-btn--discord" href="<?php echo esc_url( $discord_url ); ?>" target="_blank" rel="noopener noreferrer">
								<?php echo $this->get_discord_icon(); ?>
								<?php esc_html_e( 'DISCORD', 'psyerns-mods' ); ?>
							</a>
						<?php endif; ?>
					</div>
				</div>
			</div>
		</div>
		<?php
	}

	/**
	 * Render pricing badges.
	 *
	 * @param bool   $is_free          Whether the mod is free.
	 * @param float  $price            Base price.
	 * @param bool   $has_repack       Whether repack is available.
	 * @param float  $repack_price     Repack price.
	 * @param bool   $has_source       Whether source is available.
	 * @param float  $source_price     Source price.
	 * @param bool   $has_discount     Whether a global discount is active.
	 * @param int    $discount_percent Discount percentage (1–99).
	 * @param string $discount_label   Optional discount label text.
	 * @return void
	 */
	private function render_badges( $is_free, $price, $has_repack, $repack_price, $has_source, $source_price, $has_discount = false, $discount_percent = 0, $discount_label = '' ) {

		if ( $has_discount && '' !== $discount_label ) {
			?>
			<span class="psm-badge psm-badge--discount"><?php echo esc_html( $discount_label ); ?></span>
			<?php
		}

		if ( true === $is_free ) {
			?>
			<span class="psm-badge psm-badge--free"><?php esc_html_e( 'FREE', 'psyerns-mods' ); ?></span>
			<?php
		} elseif ( $price > 0 ) {
			if ( $has_discount ) {
				$discounted = $price * ( 1 - $discount_percent / 100 );
				?>
				<span class="psm-badge psm-badge--price psm-badge--has-discount">
					<span class="psm-badge__original"><?php echo esc_html( $this->format_price( $price ) ); ?></span>
					<?php echo esc_html( $this->format_price( $discounted ) ); ?>
				</span>
				<?php
			} else {
				?>
				<span class="psm-badge psm-badge--price"><?php echo esc_html( $this->format_price( $price ) ); ?></span>
				<?php
			}
		}

		if ( true === $has_repack ) {
			if ( $has_discount && $repack_price > 0 ) {
				$discounted_repack = $repack_price * ( 1 - $discount_percent / 100 );
				?>
				<span class="psm-badge psm-badge--repack psm-badge--has-discount">
					REPACK <span class="psm-badge__original"><?php echo esc_html( $this->format_price( $repack_price ) ); ?></span>
					<?php echo esc_html( $this->format_price( $discounted_repack ) ); ?>
				</span>
				<?php
			} else {
				?>
				<span class="psm-badge psm-badge--repack"><?php echo esc_html( 'REPACK ' . $this->format_price( $repack_price ) ); ?></span>
				<?php
			}
		}

		if ( true === $has_source ) {
			if ( $has_discount && $source_price > 0 ) {
				$discounted_source = $source_price * ( 1 - $discount_percent / 100 );
				?>
				<span class="psm-badge psm-badge--source psm-badge--has-discount">
					SOURCE <span class="psm-badge__original"><?php echo esc_html( $this->format_price( $source_price ) ); ?></span>
					<?php echo esc_html( $this->format_price( $discounted_source ) ); ?>
				</span>
				<?php
			} else {
				?>
				<span class="psm-badge psm-badge--source"><?php echo esc_html( 'SOURCE ' . $this->format_price( $source_price ) ); ?></span>
				<?php
			}
		}
	}

	/**
	 * Format a price value for display.
	 *
	 * @param float $price The price value.
	 * @return string Formatted price string (e.g. "12,99").
	 */
	private function format_price( $price ) {
		return number_format( floatval( $price ), 2, ',', '.' ) . "\xE2\x82\xAC";
	}

	/**
	 * Decode and return the cached Steam data array.
	 *
	 * @param int $post_id The post ID.
	 * @return array|null Decoded Steam data or null.
	 */
	private function get_steam_data( $post_id ) {
		$raw = PSM_Meta::get( $post_id, PSM_Meta::STEAM_DATA );

		if ( empty( $raw ) ) {
			return null;
		}

		if ( is_string( $raw ) ) {
			$decoded = json_decode( $raw, true );
			if ( is_array( $decoded ) ) {
				return $decoded;
			}
		}

		if ( is_array( $raw ) ) {
			return $raw;
		}

		return null;
	}

	/**
	 * Determine the preview image URL for a mod.
	 *
	 * Priority: Steam preview_url > Featured image > Placeholder.
	 *
	 * @param int        $post_id    The post ID.
	 * @param array|null $steam_data Decoded Steam data.
	 * @return string Image URL.
	 */
	private function get_preview_url( $post_id, $steam_data ) {
		if ( is_array( $steam_data ) && ! empty( $steam_data['preview_url'] ) ) {
			return $steam_data['preview_url'];
		}

		$thumbnail_id = get_post_thumbnail_id( $post_id );

		if ( $thumbnail_id ) {
			$thumbnail_url = wp_get_attachment_image_url( $thumbnail_id, 'medium_large' );
			if ( false !== $thumbnail_url ) {
				return $thumbnail_url;
			}
		}

		return PSM_PLUGIN_URL . 'public/images/placeholder.svg';
	}

	/**
	 * Extract and truncate the description from Steam data.
	 *
	 * @param array|null $steam_data Decoded Steam data.
	 * @return string Truncated description or empty string.
	 */
	private function get_description( $steam_data ) {
		if ( ! is_array( $steam_data ) || empty( $steam_data['description'] ) ) {
			return '';
		}

		/*
		 * Strip BBCode first (covers cached data saved before strip_bbcode was introduced),
		 * then remove any residual HTML tags.
		 */
		$description = PSM_Steam_API::strip_bbcode( $steam_data['description'] );
		$description = wp_strip_all_tags( $description );
		$description = trim( $description );

		if ( mb_strlen( $description, 'UTF-8' ) > 150 ) {
			$description = mb_substr( $description, 0, 150, 'UTF-8' ) . '...';
		}

		return $description;
	}

	/**
	 * Resolve the Discord URL for a mod.
	 *
	 * Priority: per-item meta > shortcode attribute > global option.
	 *
	 * @param int    $post_id          The post ID.
	 * @param string $discord_override Discord URL from shortcode attribute.
	 * @return string Discord URL or empty string.
	 */
	private function get_discord_url( $post_id, $discord_override ) {
		$per_item = PSM_Meta::get( $post_id, PSM_Meta::DISCORD_URL );

		if ( ! empty( $per_item ) ) {
			return $per_item;
		}

		if ( '' !== $discord_override ) {
			return $discord_override;
		}

		$global = get_option( 'psm_global_discord_url', '' );

		if ( ! empty( $global ) ) {
			return $global;
		}

		return '';
	}

	/**
	 * Render the empty state HTML.
	 *
	 * @return string HTML markup.
	 */
	private function render_empty_state() {
		ob_start();
		?>
		<div class="psyerns-mods-grid">
			<div class="psm-empty-state">
				<div class="psm-empty-state__icon">&#x1F3AE;</div>
				<p class="psm-empty-state__text"><?php esc_html_e( 'Noch keine Mods vorhanden.', 'psyerns-mods' ); ?></p>
			</div>
		</div>
		<?php
		return ob_get_clean();
	}

	/**
	 * Return the inline Steam SVG icon markup.
	 *
	 * Uses a simple Steam logo path at 16x16.
	 *
	 * @return string SVG markup.
	 */
	private function get_steam_icon() {
		return '<svg class="psm-btn__icon" width="16" height="16" viewBox="0 0 16 16" fill="currentColor" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><path d="M8 0a8 8 0 0 0-7.93 7.07l4.27 1.76a2.26 2.26 0 0 1 1.28-.4h.06l1.91-2.77v-.04a3.02 3.02 0 1 1 3.02 3.02h-.07L7.82 10.6v.05a2.27 2.27 0 0 1-4.5.37L.28 9.7A8 8 0 1 0 8 0z"/></svg>';
	}

	/**
	 * Return the inline Discord SVG icon markup.
	 *
	 * Uses a simple Discord logo path at 16x16.
	 *
	 * @return string SVG markup.
	 */
	private function get_discord_icon() {
		return '<svg class="psm-btn__icon" width="16" height="16" viewBox="0 0 16 16" fill="currentColor" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><path d="M13.55 3.15A13.2 13.2 0 0 0 10.3 2a9.2 9.2 0 0 0-.42.85 12.3 12.3 0 0 0-3.76 0A9.2 9.2 0 0 0 5.7 2a13.2 13.2 0 0 0-3.26 1.15C.36 6.33-.22 9.42.07 12.47A13.4 13.4 0 0 0 4.15 14a9.9 9.9 0 0 0 .87-1.42 8.6 8.6 0 0 1-1.37-.66l.33-.26a9.5 9.5 0 0 0 8.04 0l.33.26c-.44.26-.9.48-1.37.66A9.9 9.9 0 0 0 11.85 14a13.4 13.4 0 0 0 4.08-1.53c.34-3.56-.58-6.63-2.38-9.32zM5.35 10.7c-.8 0-1.45-.73-1.45-1.63s.63-1.64 1.45-1.64 1.47.74 1.45 1.64c0 .9-.64 1.63-1.45 1.63zm5.3 0c-.8 0-1.45-.73-1.45-1.63s.63-1.64 1.45-1.64 1.46.74 1.45 1.64c0 .9-.64 1.63-1.45 1.63z"/></svg>';
	}
}

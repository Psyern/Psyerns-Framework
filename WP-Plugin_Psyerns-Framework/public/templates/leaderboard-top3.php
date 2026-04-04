<?php
/**
 * Standalone Top 3 widget template.
 *
 * Available vars: $theme, $mode, $top3, $week, $year.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

$show_avatar = ( '1' === get_option( 'psyern_enable_avatars', '1' ) );
?>
<div class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>" data-mode="<?php echo esc_attr( $mode ); ?>">
	<div class="psyern-lb__header">
		<h2 class="psyern-lb__title">
			<?php echo esc_html( apply_filters( 'psyerns-framework/top3_title', __( 'Top Operatives', 'psyerns-framework' ) ) ); ?>
		</h2>
		<div class="psyern-lb__meta">
			<span><?php
				/* translators: %1$d: week number, %2$d: year */
				printf( esc_html__( 'Week %1$d / %2$d', 'psyerns-framework' ), $week, $year );
			?></span>
		</div>
	</div>

	<?php if ( ! empty( $top3 ) ) : ?>
	<section class="psyern-lb__top3" aria-label="<?php esc_attr_e( 'Top 3 Players', 'psyerns-framework' ); ?>">
		<?php
		$order = array( 1, 0, 2 );
		foreach ( $order as $idx ) :
			if ( ! isset( $top3[ $idx ] ) ) {
				continue;
			}
			$p    = $top3[ $idx ];
			$rank = $idx + 1;
			?>
			<div class="psyern-lb__top3-card psyern-lb__top3-card--<?php echo esc_attr( $rank ); ?>">
				<span class="psyern-lb__top3-rank">#<?php echo esc_html( $rank ); ?></span>
				<?php if ( $show_avatar && ! empty( $p['avatar_url'] ) ) : ?>
					<img class="psyern-lb__top3-avatar" src="<?php echo esc_url( $p['avatar_url'] ); ?>" alt="" loading="lazy" />
				<?php endif; ?>
				<div class="psyern-lb__top3-name"><?php echo esc_html( $p['player_name'] ); ?></div>
				<div class="psyern-lb__top3-stats">
					<?php echo esc_html( sprintf( 'K:%s  D:%s  KD:%s', number_format_i18n( $p['kills'] ), number_format_i18n( $p['deaths'] ), number_format( $p['kd_ratio'], 1 ) ) ); ?>
				</div>
			</div>
		<?php endforeach; ?>
	</section>
	<?php else : ?>
		<div class="psyern-lb__loading"><?php esc_html_e( 'No data for this week yet.', 'psyerns-framework' ); ?></div>
	<?php endif; ?>
</div>

<?php
/**
 * Single leaderboard table row.
 *
 * Available vars: $p (player array), $rank, $show_avatar, $show_playtime, $pt_h, $pt_m.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

$player_name = esc_html( $p['player_name'] ?? '' );
$faction     = strtoupper( $p['war_faction'] ?? '' );
$faction_cls = '';
if ( 'EAST' === $faction ) {
	$faction_cls = 'psyern-lb__faction--east';
} elseif ( 'WEST' === $faction ) {
	$faction_cls = 'psyern-lb__faction--west';
} elseif ( '' !== $faction ) {
	$faction_cls = 'psyern-lb__faction--neutral';
}
$boss_kills = absint( $p['war_boss_kills'] ?? 0 );
$reputation = absint( $p['hardline_reputation'] ?? 0 );
?>
<tr>
	<td class="psyern-lb__rank" aria-label="<?php printf( esc_attr__( 'Rank %d', 'psyerns-framework' ), $rank ); ?>"><?php echo esc_html( $rank ); ?></td>
	<td>
		<?php if ( $show_avatar && ! empty( $p['avatar_url'] ) ) : ?>
			<img class="psyern-lb__avatar" src="<?php echo esc_url( $p['avatar_url'] ); ?>" alt="<?php printf( esc_attr__( 'Avatar of %s', 'psyerns-framework' ), $player_name ); ?>" loading="lazy" />
		<?php endif; ?>
	</td>
	<td class="psyern-lb__name">
		<?php echo $player_name; ?>
		<?php if ( '' !== $faction && '' !== $faction_cls ) : ?>
			<span class="psyern-lb__faction <?php echo esc_attr( $faction_cls ); ?>"><?php echo esc_html( $faction ); ?></span>
		<?php endif; ?>
	</td>
	<td><?php echo esc_html( number_format_i18n( $p['kills'] ?? 0 ) ); ?></td>
	<td><?php echo esc_html( number_format_i18n( $p['deaths'] ?? 0 ) ); ?></td>
	<td class="psyern-lb__kd"><?php echo esc_html( number_format( $p['kd_ratio'] ?? 0, 2 ) ); ?></td>
	<td>
		<?php if ( '' !== $faction ) : ?>
			<span class="psyern-lb__level" title="<?php esc_attr_e( 'War Level', 'psyerns-framework' ); ?>">Lv<?php echo esc_html( $p['war_level'] ?? 0 ); ?></span>
		<?php endif; ?>
	</td>
	<td class="psyern-lb__boss-kills"><?php echo ( $boss_kills > 0 ) ? esc_html( $boss_kills ) : ''; ?></td>
	<td class="psyern-lb__reputation"><?php echo ( $reputation > 0 ) ? esc_html( number_format_i18n( $reputation ) ) : ''; ?></td>
	<?php if ( $show_playtime ) : ?>
		<td class="psyern-lb__playtime"><?php echo esc_html( $pt_h . 'h ' . $pt_m . 'm' ); ?></td>
	<?php endif; ?>
</tr>

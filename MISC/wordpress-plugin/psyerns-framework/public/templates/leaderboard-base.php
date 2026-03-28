<?php
/**
 * Leaderboard base template.
 *
 * Available vars: $theme, $mode, $limit, $top3, $rest, $week, $year, $atts.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

$enable_pvp    = ( '1' === get_option( 'psyern_enable_pvp', '1' ) );
$enable_pve    = ( '1' === get_option( 'psyern_enable_pve', '1' ) );
$show_avatar   = ( '1' === ( $atts['show_avatar'] ?? '1' ) );
$show_playtime = ( '1' === ( $atts['show_playtime'] ?? '1' ) );

// Resolve enabled columns for the initial mode from the stored option.
$_stored_cols = get_option( 'pf_columns_' . $mode, '' );
$_enabled_cols = $_stored_cols ? json_decode( $_stored_cols, true ) : null;
if ( ! is_array( $_enabled_cols ) || empty( $_enabled_cols ) ) {
	$_enabled_cols = array( 'rank', 'avatar', 'name', 'kills', 'deaths', 'kd', 'faction', 'boss', 'reputation', 'playtime' );
}
// Helper: check if column is enabled.
$col = function( $key ) use ( &$_enabled_cols ) {
	return in_array( $key, $_enabled_cols, true );
};
?>
<div class="psyern-lb psyern-lb--<?php echo esc_attr( $theme ); ?>"
	 data-mode="<?php echo esc_attr( $mode ); ?>"
	 data-limit="<?php echo esc_attr( $limit ); ?>"
	 data-refresh="60000"
	 role="region"
	 aria-label="<?php esc_attr_e( 'Leaderboard', 'psyerns-framework' ); ?>">

	<div class="psyern-lb__header">
		<h2 class="psyern-lb__title">
			<?php echo esc_html( apply_filters( 'psyerns-framework/leaderboard_title', __( 'Survivor Registry', 'psyerns-framework' ) ) ); ?>
		</h2>
		<div class="psyern-lb__meta">
			<span><?php
				/* translators: %1$d: week number, %2$d: year */
				printf( esc_html__( 'Week %1$d / %2$d', 'psyerns-framework' ), $week, $year );
			?></span>
		</div>
	</div>

	<div class="psyern-lb__controls">
		<?php if ( $enable_pvp || $enable_pve ) : ?>
		<div class="psyern-lb__modes" role="group" aria-label="<?php esc_attr_e( 'Game mode', 'psyerns-framework' ); ?>">
			<?php if ( $enable_pvp ) : ?>
				<button class="psyern-lb__mode-btn<?php echo ( 'pvp' === $mode ) ? ' active' : ''; ?>" data-mode="pvp">
					<?php esc_html_e( 'PVP', 'psyerns-framework' ); ?>
				</button>
			<?php endif; ?>
			<?php if ( $enable_pve ) : ?>
				<button class="psyern-lb__mode-btn<?php echo ( 'pve' === $mode ) ? ' active' : ''; ?>" data-mode="pve">
					<?php esc_html_e( 'PVE', 'psyerns-framework' ); ?>
				</button>
			<?php endif; ?>
		</div>
		<?php endif; ?>

		<div class="psyern-lb__limits" role="group" aria-label="<?php esc_attr_e( 'Number of players', 'psyerns-framework' ); ?>">
			<button class="psyern-lb__limit-btn<?php echo ( 10 === $limit ) ? ' active' : ''; ?>" data-limit="10">Top 10</button>
			<button class="psyern-lb__limit-btn<?php echo ( 20 === $limit ) ? ' active' : ''; ?>" data-limit="20">Top 20</button>
			<button class="psyern-lb__limit-btn<?php echo ( 50 === $limit ) ? ' active' : ''; ?>" data-limit="50">Top 50</button>
		</div>

		<div class="psyern-lb__search">
			<input
				class="psyern-lb__search-input"
				type="search"
				placeholder="<?php esc_attr_e( 'Search player…', 'psyerns-framework' ); ?>"
				aria-label="<?php esc_attr_e( 'Search player', 'psyerns-framework' ); ?>"
			/>
			<button class="psyern-lb__search-btn" aria-label="<?php esc_attr_e( 'Search', 'psyerns-framework' ); ?>">&#128269;</button>
		</div>
	</div>

	<?php
	$east_pts = absint( get_transient( 'pf_leaderboard_meta' )['globalEastPoints'] ?? 0 );
	$west_pts = absint( get_transient( 'pf_leaderboard_meta' )['globalWestPoints'] ?? 0 );
	$total_faction = $east_pts + $west_pts;
	if ( $total_faction > 0 ) :
		$east_pct = round( ( $east_pts / $total_faction ) * 100, 1 );
		$west_pct = 100.0 - $east_pct;
	?>
	<div class="psyern-lb__faction-bar" aria-label="<?php esc_attr_e( 'Faction Points', 'psyerns-framework' ); ?>">
		<span class="psyern-lb__faction-label-east"><?php echo esc_html( 'EAST ' . number_format_i18n( $east_pts ) ); ?></span>
		<div class="psyern-lb__faction-bar-track">
			<div class="psyern-lb__faction-bar-east" style="width:<?php echo esc_attr( $east_pct ); ?>%"></div>
			<div class="psyern-lb__faction-bar-west" style="width:<?php echo esc_attr( $west_pct ); ?>%"></div>
		</div>
		<span class="psyern-lb__faction-label-west"><?php echo esc_html( number_format_i18n( $west_pts ) . ' WEST' ); ?></span>
	</div>
	<?php endif; ?>

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
				<span class="psyern-lb__top3-rank" aria-label="<?php
					/* translators: %d: rank number */
					printf( esc_attr__( 'Rank %d', 'psyerns-framework' ), $rank );
				?>">#<?php echo esc_html( $rank ); ?></span>
				<?php if ( $show_avatar && ! empty( $p['avatar_url'] ) ) : ?>
					<img class="psyern-lb__top3-avatar" src="<?php echo esc_url( $p['avatar_url'] ); ?>" alt="<?php printf( esc_attr__( 'Avatar of %s', 'psyerns-framework' ), esc_attr( $p['player_name'] ) ); ?>" loading="lazy" />
				<?php endif; ?>
				<div class="psyern-lb__top3-info">
					<div class="psyern-lb__top3-name"><?php echo esc_html( $p['player_name'] ); ?></div>
					<div class="psyern-lb__top3-stats">
						<?php echo esc_html( sprintf( 'K:%s  D:%s  KD:%s', number_format_i18n( $p['kills'] ), number_format_i18n( $p['deaths'] ), number_format( $p['kd_ratio'], 1 ) ) ); ?>
					</div>
				</div>
			</div>
		<?php endforeach; ?>
	</section>

	<hr class="psyern-lb__separator" />
	<?php endif; ?>

	<div class="psyern-lb__table-wrap">
	<table class="psyern-lb__table" role="table" id="psyern-lb-table" aria-label="<?php esc_attr_e( 'Player rankings', 'psyerns-framework' ); ?>">
		<thead>
			<tr>
				<th scope="col" data-col="rank">#</th>
				<?php if ( $col( 'avatar' ) ) : ?>
					<th scope="col" data-col="avatar"></th>
				<?php endif; ?>
				<th scope="col" data-col="name"><?php esc_html_e( 'Name', 'psyerns-framework' ); ?></th>
				<?php if ( $col( 'kills' ) ) : ?>
					<th scope="col" data-col="kills"><?php esc_html_e( 'Kills', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'deaths' ) ) : ?>
					<th scope="col" data-col="deaths"><?php esc_html_e( 'Deaths', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'kd' ) ) : ?>
					<th scope="col" data-col="kd"><?php esc_html_e( 'K/D', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'faction' ) ) : ?>
					<th scope="col" data-col="faction"><?php esc_html_e( 'Faction', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'boss' ) ) : ?>
					<th scope="col" data-col="boss"><?php esc_html_e( 'Boss', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'reputation' ) ) : ?>
					<th scope="col" data-col="reputation"><?php esc_html_e( 'Rep', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
				<?php if ( $col( 'playtime' ) && $show_playtime ) : ?>
					<th scope="col" data-col="playtime"><?php esc_html_e( 'Playtime', 'psyerns-framework' ); ?></th>
				<?php endif; ?>
			</tr>
		</thead>
		<tbody>
			<?php
			foreach ( $rest as $i => $p ) :
				$rank = $i + 4;
				$pt_h = floor( ( $p['playtime_seconds'] ?? 0 ) / 3600 );
				$pt_m = floor( ( ( $p['playtime_seconds'] ?? 0 ) % 3600 ) / 60 );
				include PF_PLUGIN_DIR . 'public/templates/leaderboard-row.php';
			endforeach;
			?>
			<?php if ( empty( $rest ) && empty( $top3 ) ) : ?>
				<tr>
					<td colspan="<?php echo $show_playtime ? '7' : '6'; ?>" class="psyern-lb__loading">
						<?php esc_html_e( 'No players to display.', 'psyerns-framework' ); ?>
					</td>
				</tr>
			<?php endif; ?>
		</tbody>
	</table>
	</div>

	<nav class="psyern-lb__pagination" aria-label="<?php esc_attr_e( 'Leaderboard pages', 'psyerns-framework' ); ?>"></nav>
</div>

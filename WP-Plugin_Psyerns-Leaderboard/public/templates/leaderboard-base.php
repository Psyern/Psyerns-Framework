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
// Hard filter by mode-allowed columns so PvE-only stats never render on PvP.
$_allowed_cols = class_exists( 'PF_Admin' ) ? PF_Admin::get_mode_allowed_columns( $mode ) : array( 'rank', 'avatar', 'name', 'kills', 'deaths', 'kd', 'faction', 'boss', 'reputation', 'playtime' );
$_stored_cols  = get_option( 'pf_columns_' . $mode, '' );
$_enabled_cols = $_stored_cols ? json_decode( $_stored_cols, true ) : null;
if ( ! is_array( $_enabled_cols ) || empty( $_enabled_cols ) ) {
	$_enabled_cols = $_allowed_cols;
}
$_enabled_cols = array_values( array_intersect( $_enabled_cols, $_allowed_cols ) );
// Helper: check if column is enabled.
$col = function( $key ) use ( &$_enabled_cols ) {
	return in_array( $key, $_enabled_cols, true );
};

// Build legend & header-tooltip data.
$_col_desc       = class_exists( 'PF_Admin' ) ? PF_Admin::get_column_descriptions() : array();
$_abbreviated    = class_exists( 'PF_Admin' ) ? PF_Admin::get_abbreviated_columns() : array( 'kd', 'boss', 'reputation', 'headshots', 'accuracy', 'distance', 'playtime' );
$_th_labels      = array(
	'rank'             => '#',
	'avatar'           => '',
	'name'             => __( 'Name', 'psyerns-framework' ),
	'kills'            => __( 'Kills', 'psyerns-framework' ),
	'deaths'           => __( 'Deaths', 'psyerns-framework' ),
	'kd'               => __( 'K/D', 'psyerns-framework' ),
	'faction'          => __( 'Faction', 'psyerns-framework' ),
	'boss'             => __( 'Boss', 'psyerns-framework' ),
	'reputation'       => __( 'Rep', 'psyerns-framework' ),
	'headshots'        => __( 'HS', 'psyerns-framework' ),
	'accuracy'         => __( 'Acc %', 'psyerns-framework' ),
	'longest_shot'     => __( 'Range', 'psyerns-framework' ),
	'distance'         => __( 'Dist', 'psyerns-framework' ),
	'distance_foot'    => __( 'Foot', 'psyerns-framework' ),
	'distance_vehicle' => __( 'Vehicle', 'psyerns-framework' ),
	'playtime'         => __( 'Playtime', 'psyerns-framework' ),
);
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

	<?php
	// Build legend entries: only abbreviated columns that are actually rendered.
	$_legend_items = array();
	foreach ( $_abbreviated as $_lk ) {
		if ( 'playtime' === $_lk && ! $show_playtime ) {
			continue;
		}
		if ( ! $col( $_lk ) ) {
			continue;
		}
		$_legend_items[] = array(
			'label' => $_th_labels[ $_lk ] ?? $_lk,
			'desc'  => $_col_desc[ $_lk ] ?? $_lk,
		);
	}
	if ( ! empty( $_legend_items ) ) :
	?>
	<details class="psyern-lb__legend" data-mode="<?php echo esc_attr( $mode ); ?>">
		<summary class="psyern-lb__legend-toggle"><?php esc_html_e( 'Legend', 'psyerns-framework' ); ?></summary>
		<dl class="psyern-lb__legend-list">
			<?php foreach ( $_legend_items as $_item ) : ?>
				<div class="psyern-lb__legend-item">
					<dt class="psyern-lb__legend-key"><?php echo esc_html( $_item['label'] ); ?></dt>
					<dd class="psyern-lb__legend-val"><?php echo esc_html( $_item['desc'] ); ?></dd>
				</div>
			<?php endforeach; ?>
		</dl>
	</details>
	<?php endif; ?>

	<div class="psyern-lb__table-wrap">
	<table class="psyern-lb__table" role="table" id="psyern-lb-table" aria-label="<?php esc_attr_e( 'Player rankings', 'psyerns-framework' ); ?>">
		<thead>
			<tr>
			<?php
			foreach ( $_enabled_cols as $_ck ) {
				if ( 'playtime' === $_ck && ! $show_playtime ) {
					continue;
				}
				$_lab  = $_th_labels[ $_ck ] ?? $_ck;
				$_desc = $_col_desc[ $_ck ] ?? '';
				$_title_attr = ( '' !== $_desc && $_desc !== $_lab ) ? ' title="' . esc_attr( $_desc ) . '"' : '';
				echo '<th scope="col" data-col="' . esc_attr( $_ck ) . '"' . $_title_attr . '>' . esc_html( $_lab ) . '</th>';
			}
			?>
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
<?php if ( ! empty( $GLOBALS['pf_player_details_enabled'] ) ) {
	include PF_PLUGIN_DIR . 'public/templates/player-detail-modal.php';
} ?>

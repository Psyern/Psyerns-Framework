<?php
if ( ! defined( 'ABSPATH' ) ) {
	exit;
}

global $wpdb;
$table   = PF_Database::get_table_name( 'whitelist' );
$entries = $wpdb->get_results( "SELECT * FROM {$table} ORDER BY added_at DESC", ARRAY_A );
$count   = count( $entries );
?>
<div class="wrap pf-admin-wrap">
	<h1><?php esc_html_e( 'Whitelist Management', 'psyerns-framework' ); ?></h1>

	<?php if ( isset( $_GET['msg'] ) ) : ?>
		<div class="notice notice-success is-dismissible">
			<p>
				<?php
				if ( 'added' === $_GET['msg'] ) {
					esc_html_e( 'Player added to whitelist.', 'psyerns-framework' );
				} else {
					esc_html_e( 'Player removed from whitelist.', 'psyerns-framework' );
				}
				?>
			</p>
		</div>
	<?php endif; ?>

	<div class="pf-whitelist-form">
		<h3><?php esc_html_e( 'Add Player', 'psyerns-framework' ); ?></h3>
		<form method="post">
			<?php wp_nonce_field( 'pf_whitelist_add_nonce' ); ?>
			<input type="text" name="steam_id" placeholder="<?php esc_attr_e( 'Steam64 ID', 'psyerns-framework' ); ?>" required style="width:200px" />
			<input type="text" name="player_name" placeholder="<?php esc_attr_e( 'Player Name', 'psyerns-framework' ); ?>" style="width:200px" />
			<input type="submit" name="pf_whitelist_add" class="button button-primary" value="<?php esc_attr_e( 'Add to Whitelist', 'psyerns-framework' ); ?>" />
		</form>
	</div>

	<h3>
		<?php
		/* translators: %d: number of whitelisted players */
		printf( esc_html__( '%d player(s) whitelisted', 'psyerns-framework' ), $count );
		?>
	</h3>

	<?php if ( $count > 0 ) : ?>
	<table class="wp-list-table widefat fixed striped pf-whitelist-table">
		<thead>
			<tr>
				<th style="width:40px">#</th>
				<th><?php esc_html_e( 'Steam ID', 'psyerns-framework' ); ?></th>
				<th><?php esc_html_e( 'Name', 'psyerns-framework' ); ?></th>
				<th><?php esc_html_e( 'Added', 'psyerns-framework' ); ?></th>
				<th style="width:120px"><?php esc_html_e( 'Actions', 'psyerns-framework' ); ?></th>
			</tr>
		</thead>
		<tbody>
			<?php foreach ( $entries as $i => $entry ) : ?>
			<tr>
				<td><?php echo intval( $i + 1 ); ?></td>
				<td><code><?php echo esc_html( $entry['steam_id'] ); ?></code></td>
				<td><?php echo esc_html( $entry['name'] ); ?></td>
				<td><?php echo esc_html( $entry['added_at'] ); ?></td>
				<td>
					<form method="post" style="display:inline">
						<?php wp_nonce_field( 'pf_whitelist_remove_nonce' ); ?>
						<input type="hidden" name="steam_id" value="<?php echo esc_attr( $entry['steam_id'] ); ?>" />
						<input type="submit" name="pf_whitelist_remove" class="button button-small"
							value="<?php esc_attr_e( 'Remove', 'psyerns-framework' ); ?>"
							onclick="return confirm('<?php echo esc_js( __( 'Remove this player from whitelist?', 'psyerns-framework' ) ); ?>')" />
					</form>
				</td>
			</tr>
			<?php endforeach; ?>
		</tbody>
	</table>
	<?php else : ?>
		<p><?php esc_html_e( 'No players whitelisted yet.', 'psyerns-framework' ); ?></p>
	<?php endif; ?>
</div>

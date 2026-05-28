<?php
/**
 * Player Detail Modal — server-rendered skeleton.
 *
 * The modal is hidden by default. Agent 5's JS populates the panes by
 * fetching /wp-json/psyern/v1/player/{uid} when a leaderboard row is
 * clicked. All visible text is wrapped for i18n; no dynamic values are
 * echoed here — it's pure markup.
 *
 * @package Psyerns_Framework
 */

if ( ! defined( 'ABSPATH' ) ) {
	exit;
}
?>
<div
	id="psyern-pdm"
	class="psyern-pdm"
	role="dialog"
	aria-modal="true"
	aria-hidden="true"
	aria-labelledby="psyern-pdm-name"
	hidden
>
	<div class="psyern-pdm__backdrop" data-pdm-close="1"></div>

	<div class="psyern-pdm__panel" tabindex="-1">

		<header class="psyern-pdm__header">
			<img
				class="psyern-pdm__avatar"
				src=""
				alt="<?php esc_attr_e( 'Player avatar', 'psyerns-framework' ); ?>"
				loading="lazy"
				data-bind="avatarUrl"
			/>
			<div class="psyern-pdm__identity">
				<h2 id="psyern-pdm-name" class="psyern-pdm__name" data-bind="playerName">
					<?php esc_html_e( 'Player', 'psyerns-framework' ); ?>
				</h2>
				<div class="psyern-pdm__sub">
					<span class="psyern-pdm__chip psyern-pdm__chip--faction" data-bind="war.faction" hidden></span>
					<span class="psyern-pdm__chip psyern-pdm__chip--level"   data-bind="war.level"   hidden></span>
					<span class="psyern-pdm__chip psyern-pdm__chip--rep"     data-bind="hardline.reputation" hidden></span>
				</div>
			</div>
			<button
				type="button"
				class="psyern-pdm__close"
				aria-label="<?php esc_attr_e( 'Close', 'psyerns-framework' ); ?>"
				data-pdm-close="1"
			>&times;</button>
		</header>

		<nav class="psyern-pdm__tabs" role="tablist" aria-label="<?php esc_attr_e( 'Player detail tabs', 'psyerns-framework' ); ?>">
			<button type="button"
				id="psyern-pdm-tab-overview"
				class="psyern-pdm__tab psyern-pdm__tab--active"
				role="tab"
				aria-selected="true"
				aria-controls="psyern-pdm-pane-overview"
				data-tab="overview"
			><?php esc_html_e( 'Übersicht', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-kills"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-kills"
				data-tab="kills"
			><?php esc_html_e( 'Kills', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-deaths"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-deaths"
				data-tab="deaths"
			><?php esc_html_e( 'Deaths', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-war"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-war"
				data-tab="war"
			><?php esc_html_e( 'War', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-skills"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-skills"
				data-tab="skills"
				hidden
			><?php esc_html_e( 'Skills', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-gunplay"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-gunplay"
				data-tab="gunplay"
			><?php esc_html_e( 'Gunplay', 'psyerns-framework' ); ?></button>

			<button type="button"
				id="psyern-pdm-tab-movement"
				class="psyern-pdm__tab"
				role="tab"
				aria-selected="false"
				aria-controls="psyern-pdm-pane-movement"
				data-tab="movement"
			><?php esc_html_e( 'Movement', 'psyerns-framework' ); ?></button>
		</nav>

		<div class="psyern-pdm__body">

			<section
				id="psyern-pdm-pane-overview"
				class="psyern-pdm__pane psyern-pdm__pane--active"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-overview"
				data-pane="overview"
			>
				<div class="psyern-pdm__loading" data-role="loading">
					<?php esc_html_e( 'Lade Spielerdaten…', 'psyerns-framework' ); ?>
				</div>
				<div class="psyern-pdm__error" data-role="error" hidden>
					<span class="psyern-pdm__error-text" data-bind="errorText">
						<?php esc_html_e( 'Daten konnten nicht geladen werden.', 'psyerns-framework' ); ?>
					</span>
				</div>
				<div class="psyern-pdm__kpi-grid" data-role="kpi-grid"></div>
			</section>

			<section
				id="psyern-pdm-pane-kills"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-kills"
				aria-hidden="true"
				data-pane="kills"
				hidden
			>
				<div class="psyern-pdm__groups" data-role="groups"></div>
			</section>

			<section
				id="psyern-pdm-pane-deaths"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-deaths"
				aria-hidden="true"
				data-pane="deaths"
				hidden
			>
				<div class="psyern-pdm__groups" data-role="groups"></div>
			</section>

			<section
				id="psyern-pdm-pane-war"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-war"
				aria-hidden="true"
				data-pane="war"
				hidden
			>
				<div class="psyern-pdm__kpi-grid" data-role="kpi-grid"></div>
			</section>

			<section
				id="psyern-pdm-pane-skills"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-skills"
				aria-hidden="true"
				data-pane="skills"
				hidden
			>
				<div class="psyern-pdm__skills" data-role="skills"></div>
			</section>

			<section
				id="psyern-pdm-pane-gunplay"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-gunplay"
				aria-hidden="true"
				data-pane="gunplay"
				hidden
			>
				<div class="psyern-pdm__kpi-grid" data-role="kpi-grid"></div>
			</section>

			<section
				id="psyern-pdm-pane-movement"
				class="psyern-pdm__pane"
				role="tabpanel"
				aria-labelledby="psyern-pdm-tab-movement"
				aria-hidden="true"
				data-pane="movement"
				hidden
			>
				<div class="psyern-pdm__kpi-grid" data-role="kpi-grid"></div>
			</section>

		</div>

		<footer class="psyern-pdm__footer">
			<span class="psyern-pdm__updated" data-bind="lastUpdated">
				<?php esc_html_e( 'Stand: —', 'psyerns-framework' ); ?>
			</span>
		</footer>

	</div>
</div>

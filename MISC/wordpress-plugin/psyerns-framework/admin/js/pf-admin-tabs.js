/**
 * Psyerns Framework — Admin Tab JS
 *
 * Handles:
 *  - Theme card active state on radio-click (no page reload needed for the visual)
 */
(function () {
	'use strict';

	document.addEventListener('DOMContentLoaded', function () {

		// ── Theme Card Highlight ──────────────────────────────────────
		var themeCards = document.querySelectorAll('.pf-theme-card');
		themeCards.forEach(function (card) {
			card.addEventListener('click', function () {
				themeCards.forEach(function (c) { c.classList.remove('pf-theme-card--active'); });
				card.classList.add('pf-theme-card--active');
			});
		});

		// ── Column Checkbox Row Highlight ────────────────────────────
		var colChecks = document.querySelectorAll('.pf-col-check:not(.pf-col-check--fixed) input[type="checkbox"]');
		colChecks.forEach(function (cb) {
			var label = cb.closest('.pf-col-check');
			function update() {
				if (cb.checked) {
					label.style.borderColor = '#2271b1';
					label.style.background  = '#f0f6fc';
				} else {
					label.style.borderColor = '';
					label.style.background  = '';
				}
			}
			update();
			cb.addEventListener('change', update);
		});

	});
})();

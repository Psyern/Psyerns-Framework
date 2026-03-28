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

		// ── Copy-to-Clipboard Buttons ────────────────────────────────
		document.querySelectorAll('.pf-copy-btn').forEach(function (btn) {
			btn.addEventListener('click', function () {
				var text = btn.getAttribute('data-copy');
				if (!text) return;

				var icon = btn.querySelector('.dashicons');
				var originalHtml = btn.innerHTML;

				navigator.clipboard.writeText(text).then(function () {
					btn.classList.add('copied');
					if (icon) {
						icon.classList.remove('dashicons-clipboard');
						icon.classList.add('dashicons-yes');
					}
					btn.childNodes[btn.childNodes.length - 1].textContent = ' Copied!';
					setTimeout(function () {
						btn.classList.remove('copied');
						btn.innerHTML = originalHtml;
					}, 2000);
				}).catch(function () {
					// Fallback für ältere Browser
					var ta = document.createElement('textarea');
					ta.value = text;
					ta.style.position = 'fixed';
					ta.style.opacity = '0';
					document.body.appendChild(ta);
					ta.select();
					document.execCommand('copy');
					document.body.removeChild(ta);
					btn.classList.add('copied');
					if (icon) {
						icon.classList.remove('dashicons-clipboard');
						icon.classList.add('dashicons-yes');
					}
					setTimeout(function () {
						btn.classList.remove('copied');
						btn.innerHTML = originalHtml;
					}, 2000);
				});
			});
		});

	});
})();

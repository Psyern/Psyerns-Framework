/**
 * Psyern Leaderboard v2 — Frontend JavaScript
 *
 * Handles mode switching (PvP/PvE), limit toggle,
 * AJAX data loading, staggered row animations, auto-refresh.
 * Includes: Faction Badges, Boss Kills, Reputation, Faction Bar.
 *
 * @package Psyerns_Framework
 */
(function() {
	'use strict';

	var config = window.psyernConfig || {};

	function init() {
		var els = document.querySelectorAll('.psyern-lb[data-mode]');
		for (var i = 0; i < els.length; i++) {
			initInstance(els[i]);
		}
	}

	function initInstance(el) {
		var state = {
			el: el,
			mode: el.getAttribute('data-mode') || 'pvp',
			limit: parseInt(el.getAttribute('data-limit'), 10) || 10,
			refreshInterval: parseInt(el.getAttribute('data-refresh'), 10) || 60000,
			page: 1,
			totalPages: 1,
			searchQuery: '',
		};

		bindModeButtons(state);
		bindLimitButtons(state);
		initSearch(state);
		rebuildHeader(state);
		loadData(state);

		if (state.refreshInterval > 0) {
			setInterval(function() { loadData(state); }, state.refreshInterval);
		}
	}

	function bindModeButtons(state) {
		var btns = state.el.querySelectorAll('.psyern-lb__mode-btn');
		for (var i = 0; i < btns.length; i++) {
			btns[i].addEventListener('click', function(e) {
				e.preventDefault();
				var newMode = this.getAttribute('data-mode');
				if (newMode === state.mode) return;
				state.mode = newMode;
				state.page = 1;
				var all = state.el.querySelectorAll('.psyern-lb__mode-btn');
				for (var j = 0; j < all.length; j++) {
					all[j].classList.toggle('active', all[j].getAttribute('data-mode') === newMode);
				}
				rebuildHeader(state);
				loadData(state);
			});
		}
	}

	function bindLimitButtons(state) {
		var btns = state.el.querySelectorAll('.psyern-lb__limit-btn');
		for (var i = 0; i < btns.length; i++) {
			btns[i].addEventListener('click', function(e) {
				e.preventDefault();
				var newLimit = parseInt(this.getAttribute('data-limit'), 10);
				if (newLimit === state.limit) return;
				state.limit = newLimit;
				state.page = 1;
				var all = state.el.querySelectorAll('.psyern-lb__limit-btn');
				for (var j = 0; j < all.length; j++) {
					all[j].classList.toggle('active', parseInt(all[j].getAttribute('data-limit'), 10) === newLimit);
				}
				loadData(state);
			});
		}
	}

	function loadData(state) {
		var url = config.ajaxUrl +
			'?action=psyern_get_leaderboard' +
			'&mode=' + encodeURIComponent(state.mode) +
			'&per_page=' + state.limit +
			'&page=' + state.page +
			'&search=' + encodeURIComponent(state.searchQuery) +
			'&nonce=' + encodeURIComponent(config.nonce);

		var tbody = state.el.querySelector('.psyern-lb__table tbody');
		if (!tbody) return;

		fetch(url)
			.then(function(res) { return res.json(); })
			.then(function(json) {
				if (!json.success || !json.data || !json.data.players) return;
				state.totalPages = json.data.total_pages || 1;
				renderRows(tbody, json.data.players, state);
				renderFactionBar(state.el, json.data);
				renderPagination(state);
			})
			.catch(function() {
				tbody.innerHTML = '<tr><td colspan="10" class="psyern-lb__loading">' +
					escHtml(config.i18n ? config.i18n.error : 'Error') + '</td></tr>';
			});
	}

	/**
	 * Return the enabled column array for the current mode.
	 * Falls back to all columns if nothing is configured.
	 *
	 * @param {string} mode 'pvp' or 'pve'
	 * @returns {string[]}
	 */
	function getCols(mode) {
		var allCols = ['rank','avatar','name','kills','deaths','kd','faction','boss','reputation','playtime'];
		if (config.columns && config.columns[mode] && config.columns[mode].length) {
			return config.columns[mode];
		}
		return allCols;
	}

	/**
	 * Re-render the <thead> to match the enabled columns for the current mode.
	 * Called once on mode-switch so the header stays in sync with rows.
	 *
	 * @param {object} state
	 */
	function rebuildHeader(state) {
		var thead = state.el.querySelector('.psyern-lb__table thead tr');
		if (!thead) return;
		var cols = getCols(state.mode);
		var i18n = config.i18n || {};

		var labels = {
			rank:       '#',
			avatar:     '',
			name:       'Name',
			kills:      i18n.kills      || 'Kills',
			deaths:     i18n.deaths     || 'Deaths',
			kd:         i18n.kd         || 'K/D',
			faction:    'Faction',
			boss:       'Boss',
			reputation: 'Rep',
			playtime:   i18n.playtime   || 'Playtime',
		};

		var html = '';
		for (var i = 0; i < cols.length; i++) {
			var key = cols[i];
			html += '<th scope="col" data-col="' + key + '">' + (labels[key] || key) + '</th>';
		}
		thead.innerHTML = html;
	}

	function renderRows(tbody, players, state) {
		var cols = getCols(state.mode);

		// Check helpers.
		var has = function(key) { return cols.indexOf(key) !== -1; };

		var html = '';
		for (var i = 0; i < players.length; i++) {
			var p = players[i];
			var rank = (state.page - 1) * state.limit + i + 1;
			var pt = formatPlaytime(parseInt(p.playtime_seconds, 10) || 0);
			var bk = p.war_boss_kills || 0;
			var rep = p.hardline_reputation || 0;
			var faction = (p.war_faction || '').toUpperCase();
			var warLevel = parseInt(p.war_level, 10) || 0;

			html += '<tr style="animation-delay:' + (i * 0.05) + 's">';

			// rank — always shown
			html += '<td class="psyern-lb__rank">' + rank + '</td>';

			if (has('avatar')) {
				html += '<td>';
				if (p.avatar_url) {
					html += '<img class="psyern-lb__avatar" src="' + escHtml(p.avatar_url) + '" alt="" loading="lazy" />';
				}
				html += '</td>';
			}

			// name — always shown
			html += '<td class="psyern-lb__name">' + escHtml(p.player_name) + factionBadge(p.war_faction) + '</td>';

			if (has('kills'))      { html += '<td>' + fmtN(p.kills) + '</td>'; }
			if (has('deaths'))     { html += '<td>' + fmtN(p.deaths) + '</td>'; }
			if (has('kd'))         { html += '<td class="psyern-lb__kd">' + (parseFloat(p.kd_ratio) || 0).toFixed(2) + '</td>'; }

			if (has('faction')) {
				html += '<td>';
				if (faction) {
					html += '<span class="psyern-lb__level" title="War Level">Lv' + warLevel + '</span>';
				}
				html += '</td>';
			}

			if (has('boss'))       { html += '<td class="psyern-lb__boss-kills">' + (bk > 0 ? bk : '') + '</td>'; }
			if (has('reputation')) { html += '<td class="psyern-lb__reputation">' + (rep > 0 ? fmtN(rep) : '') + '</td>'; }
			if (has('playtime'))   { html += '<td class="psyern-lb__playtime">' + pt + '</td>'; }

			html += '</tr>';
		}

		if (0 === players.length) {
			html = '<tr><td colspan="' + cols.length + '" class="psyern-lb__loading">' +
				escHtml(config.i18n ? config.i18n.loading : 'No data') + '</td></tr>';
		}

		tbody.innerHTML = html;
	}

	function renderFactionBar(el, data) {
		var existing = el.querySelector('.psyern-lb__faction-bar');
		if (existing) existing.remove();

		var east = parseInt(data.globalEastPoints, 10) || 0;
		var west = parseInt(data.globalWestPoints, 10) || 0;
		var total = east + west;
		if (total === 0) return;

		var ePct = Math.round((east / total) * 100);
		var wPct = 100 - ePct;

		var bar = document.createElement('div');
		bar.className = 'psyern-lb__faction-bar';
		bar.innerHTML = '<span class="psyern-lb__faction-label-east">EAST ' + fmtN(east) + '</span>' +
			'<div class="psyern-lb__faction-bar-track">' +
			'<div class="psyern-lb__faction-bar-east" style="width:' + ePct + '%"></div>' +
			'<div class="psyern-lb__faction-bar-west" style="width:' + wPct + '%"></div>' +
			'</div>' +
			'<span class="psyern-lb__faction-label-west">' + fmtN(west) + ' WEST</span>';

		var header = el.querySelector('.psyern-lb__header');
		if (header && header.nextSibling) {
			header.parentNode.insertBefore(bar, header.nextSibling);
		}
	}

	function factionBadge(faction) {
		if (!faction || faction === '') return '';
		var f = faction.toUpperCase();
		var cls = 'psyern-lb__faction--neutral';
		if (f === 'EAST') cls = 'psyern-lb__faction--east';
		if (f === 'WEST') cls = 'psyern-lb__faction--west';
		return ' <span class="psyern-lb__faction ' + cls + '">' + escHtml(f) + '</span>';
	}

	function formatPlaytime(seconds) {
		var h = Math.floor(seconds / 3600);
		var m = Math.floor((seconds % 3600) / 60);
		return h + 'h ' + m + 'm';
	}

	function fmtN(n) {
		return Number(n || 0).toLocaleString();
	}

	function initSearch(state) {
		var input = state.el.querySelector('.psyern-lb__search-input');
		if (!input) return;

		// Debounced input for desktop typing
		var timer;
		input.addEventListener('input', function() {
			clearTimeout(timer);
			timer = setTimeout(function() {
				state.searchQuery = input.value.trim();
				state.page = 1;
				loadData(state);
			}, 350);
		});

		// Immediate search on Enter key
		input.addEventListener('keydown', function(e) {
			if (e.key === 'Enter') {
				clearTimeout(timer);
				state.searchQuery = input.value.trim();
				state.page = 1;
				loadData(state);
			}
		});

		// Mobile search button
		var btn = state.el.querySelector('.psyern-lb__search-btn');
		if (btn) {
			btn.addEventListener('click', function() {
				clearTimeout(timer);
				state.searchQuery = input.value.trim();
				state.page = 1;
				loadData(state);
				input.blur(); // Tastatur auf Mobile schließen
			});
		}
	}

	function buildPageRange(current, total) {
		var delta = 2;
		var range = [];
		var prev = 0;
		for (var p = 1; p <= total; p++) {
			if (p === 1 || p === total || (p >= current - delta && p <= current + delta)) {
				if (prev && p - prev > 1) range.push('…');
				range.push(p);
				prev = p;
			}
		}
		return range;
	}

	function renderPagination(state) {
		var nav = state.el.querySelector('.psyern-lb__pagination');
		if (!nav) return;
		if (state.totalPages <= 1) { nav.innerHTML = ''; return; }

		var html = '';
		html += '<button class="psyern-lb__page-btn psyern-lb__page-prev"' +
			(state.page <= 1 ? ' disabled' : '') + ' aria-label="Previous">&#8249;</button>';

		var pages = buildPageRange(state.page, state.totalPages);
		for (var i = 0; i < pages.length; i++) {
			var pg = pages[i];
			if (pg === '…') {
				html += '<span class="psyern-lb__page-ellipsis">…</span>';
			} else {
				html += '<button class="psyern-lb__page-btn' + (pg === state.page ? ' active' : '') + '"' +
					' data-page="' + pg + '">' + pg + '</button>';
			}
		}

		html += '<button class="psyern-lb__page-btn psyern-lb__page-next"' +
			(state.page >= state.totalPages ? ' disabled' : '') + ' aria-label="Next">&#8250;</button>';

		nav.innerHTML = html;

		nav.querySelectorAll('.psyern-lb__page-btn[data-page]').forEach(function(btn) {
			btn.addEventListener('click', function() {
				var pg = parseInt(this.getAttribute('data-page'), 10);
				if (pg === state.page) return;
				state.page = pg;
				loadData(state);
			});
		});
		var prev = nav.querySelector('.psyern-lb__page-prev');
		if (prev) prev.addEventListener('click', function() {
			if (state.page > 1) { state.page--; loadData(state); }
		});
		var next = nav.querySelector('.psyern-lb__page-next');
		if (next) next.addEventListener('click', function() {
			if (state.page < state.totalPages) { state.page++; loadData(state); }
		});
	}

	function escHtml(s) {
		var div = document.createElement('div');
		div.appendChild(document.createTextNode(s || ''));
		return div.innerHTML;
	}

	if ('loading' === document.readyState) {
		document.addEventListener('DOMContentLoaded', init);
	} else {
		init();
	}
})();

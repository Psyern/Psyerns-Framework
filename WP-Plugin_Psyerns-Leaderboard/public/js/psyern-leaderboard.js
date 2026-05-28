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
			// Server-side sort. Empty sortBy = default (points DESC).
			sortBy: '',
			sortDir: 'desc',
		};

		bindModeButtons(state);
		bindLimitButtons(state);
		bindHeaderSort(state);
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

	// Columns whose header is clickable to sort. Whitelist must mirror the
	// server-side $sort_map in PHP — anything not in BOTH lists is ignored.
	var SORTABLE_COLS = [
		'name', 'faction', 'kills', 'deaths', 'kd',
		'boss', 'reputation', 'headshots', 'accuracy',
		'longest_shot', 'distance', 'distance_foot', 'distance_vehicle', 'playtime'
	];

	function loadData(state) {
		var url = config.ajaxUrl +
			'?action=psyern_get_leaderboard' +
			'&mode=' + encodeURIComponent(state.mode) +
			'&per_page=' + state.limit +
			'&page=' + state.page +
			'&search=' + encodeURIComponent(state.searchQuery) +
			'&sort_by=' + encodeURIComponent(state.sortBy || '') +
			'&sort_dir=' + encodeURIComponent(state.sortDir || 'desc') +
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
	// Columns that are only meaningful on one board. Mirrors
	// PF_Admin::get_pve_only_columns() / get_pvp_only_columns() server-side.
	var PVE_ONLY_COLS = ['boss', 'reputation', 'distance', 'distance_foot', 'distance_vehicle'];
	var PVP_ONLY_COLS = ['headshots', 'accuracy'];

	function getAllowedCols(mode) {
		var all = ['rank','avatar','name','kills','deaths','kd','faction','boss','reputation','headshots','accuracy','longest_shot','distance','distance_foot','distance_vehicle','playtime'];
		if ('pvp' === mode) {
			return all.filter(function(c) { return PVE_ONLY_COLS.indexOf(c) === -1; });
		}
		if ('pve' === mode) {
			return all.filter(function(c) { return PVP_ONLY_COLS.indexOf(c) === -1; });
		}
		return all;
	}

	function getCols(mode) {
		var allowed = getAllowedCols(mode);
		var configured = (config.columns && config.columns[mode] && config.columns[mode].length) ? config.columns[mode] : allowed;
		return configured.filter(function(c) { return allowed.indexOf(c) !== -1; });
	}

	/**
	 * Re-render the <thead> to match the enabled columns for the current mode.
	 * Called once on mode-switch so the header stays in sync with rows.
	 *
	 * @param {object} state
	 */
	// Columns whose header text is an abbreviation and therefore needs a
	// tooltip + legend entry. Mirrors PF_Admin::get_abbreviated_columns().
	var ABBREV_COLS = ['kd','boss','reputation','headshots','accuracy','longest_shot','distance','distance_foot','distance_vehicle','playtime'];

	function getLabels() {
		var i18n = config.i18n || {};
		return {
			rank:             '#',
			avatar:           '',
			name:             i18n.name             || 'Name',
			kills:            i18n.kills            || 'Kills',
			deaths:           i18n.deaths           || 'Deaths',
			kd:               i18n.kd               || 'K/D',
			faction:          i18n.faction          || 'Faction',
			boss:             i18n.boss             || 'Boss',
			reputation:       i18n.reputation       || 'Rep',
			headshots:        i18n.headshots        || 'HS',
			accuracy:         i18n.accuracy         || 'Acc %',
			longest_shot:     i18n.longest_shot     || 'Range',
			distance:         i18n.distance         || 'Dist',
			distance_foot:    i18n.distance_foot    || 'Foot',
			distance_vehicle: i18n.distance_vehicle || 'Vehicle',
			playtime:         i18n.playtime         || 'Playtime',
		};
	}

	function getDescriptions() {
		var descs = (config.columnDescriptions && typeof config.columnDescriptions === 'object') ? config.columnDescriptions : {};
		// Sensible fallbacks if the server didn't localize them.
		return {
			rank:             descs.rank             || 'Rank',
			avatar:           descs.avatar           || 'Player Avatar',
			name:             descs.name             || 'Player Name',
			kills:            descs.kills            || 'Kills',
			deaths:           descs.deaths           || 'Deaths',
			kd:               descs.kd               || 'Kill / Death Ratio',
			faction:          descs.faction          || 'War Faction & Level',
			boss:             descs.boss             || 'Boss Kills (DME-WAR)',
			reputation:       descs.reputation       || 'Hardline Reputation',
			headshots:        descs.headshots        || 'Headshots',
			accuracy:         descs.accuracy         || 'Hit Accuracy (Hits / Shots Fired)',
			longest_shot:     descs.longest_shot     || 'Longest Kill Range (m)',
			distance:         descs.distance         || 'Total Distance Travelled (km)',
			distance_foot:    descs.distance_foot    || 'Distance Travelled on Foot (km)',
			distance_vehicle: descs.distance_vehicle || 'Distance Travelled in Vehicle (km)',
			playtime:         descs.playtime         || 'Total Playtime',
		};
	}

	function rebuildHeader(state) {
		var thead = state.el.querySelector('.psyern-lb__table thead tr');
		if (!thead) return;
		var cols   = getCols(state.mode);
		var labels = getLabels();
		var descs  = getDescriptions();

		var html = '';
		for (var i = 0; i < cols.length; i++) {
			var key   = cols[i];
			var lab   = labels[key] || key;
			var desc  = descs[key] || '';
			var isSortable = SORTABLE_COLS.indexOf(key) !== -1;
			var isActive = isSortable && state.sortBy === key;
			var arrow = isActive ? ' <span class="psyern-lb__sort-arrow">' + (state.sortDir === 'asc' ? '▲' : '▼') + '</span>' : '';
			// Tooltip merges the descriptive label with a hint about click-to-sort.
			var tip = desc;
			if (isSortable) {
				tip = (tip ? tip + ' — ' : '') + 'Click to sort';
			}
			var titleAttr = tip ? ' title="' + escAttr(tip) + '"' : '';
			var sortableAttr = isSortable ? ' data-sortable="1"' : '';
			var classAttr    = isActive ? ' class="psyern-lb__th--active"' : '';
			html += '<th scope="col" data-col="' + key + '"' + sortableAttr + classAttr + titleAttr + '>' + escHtml(lab) + arrow + '</th>';
		}
		thead.innerHTML = html;

		rebuildLegend(state, cols, labels, descs);
	}

	/**
	 * Delegated click listener for sortable column headers. Bound once on
	 * init at the container level so it survives rebuildHeader() replacing
	 * the <thead> innerHTML on every mode-switch and AJAX response.
	 *
	 * Click cycle for a column:
	 *   1st click on a fresh column  -> sort by it DESC
	 *   2nd click on same column     -> flip to ASC
	 *   3rd click on same column     -> clear sort, back to default (points)
	 */
	function bindHeaderSort(state) {
		state.el.addEventListener('click', function(e) {
			var th = e.target;
			// Walk up if the user clicked the arrow span or other inline child.
			while (th && th !== state.el && (th.nodeName !== 'TH' || !th.hasAttribute('data-col'))) {
				th = th.parentNode;
			}
			if (!th || th === state.el || !th.hasAttribute('data-sortable')) return;
			var col = th.getAttribute('data-col');
			if (state.sortBy === col) {
				if (state.sortDir === 'desc') {
					state.sortDir = 'asc';
				} else {
					state.sortBy = '';
					state.sortDir = 'desc';
				}
			} else {
				state.sortBy = col;
				state.sortDir = 'desc';
			}
			state.page = 1;
			rebuildHeader(state);
			loadData(state);
		});
	}

	function rebuildLegend(state, cols, labels, descs) {
		var legend = state.el.querySelector('.psyern-lb__legend');
		if (!legend) return;
		var list   = legend.querySelector('.psyern-lb__legend-list');
		if (!list) return;

		var visible = {};
		for (var i = 0; i < cols.length; i++) {
			visible[cols[i]] = true;
		}

		var html = '';
		for (var j = 0; j < ABBREV_COLS.length; j++) {
			var key = ABBREV_COLS[j];
			if (!visible[key]) continue;
			html += '<div class="psyern-lb__legend-item">' +
				'<dt class="psyern-lb__legend-key">' + escHtml(labels[key] || key) + '</dt>' +
				'<dd class="psyern-lb__legend-val">' + escHtml(descs[key] || '') + '</dd>' +
				'</div>';
		}

		if ('' === html) {
			legend.style.display = 'none';
		} else {
			legend.style.display = '';
			list.innerHTML = html;
			legend.setAttribute('data-mode', state.mode);
		}
	}

	function escAttr(s) {
		return String(s == null ? '' : s).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
	}

	function renderRows(tbody, players, state) {
		var cols = getCols(state.mode);

		// Check helpers.
		var has = function(key) { return cols.indexOf(key) !== -1; };

		var html = '';
		for (var i = 0; i < players.length; i++) {
			var p = players[i];
			// Prefer the server-supplied rank (matches the current ORDER BY
			// exactly, including any active sort_by). Fall back to positional
			// counting only if the response predates this change.
			var rank = (typeof p.rank === 'number' && p.rank > 0)
				? p.rank
				: ((state.page - 1) * state.limit + i + 1);
			var pt = formatPlaytime(parseInt(p.playtime_seconds, 10) || 0);
			var bk = p.war_boss_kills || 0;
			var rep = p.hardline_reputation || 0;
			var faction = (p.war_faction || '').toUpperCase();
			var warLevel = parseInt(p.war_level, 10) || 0;

			html += '<tr class="psyern-lb__row psyern-lb__row--clickable" data-uid="' + escAttr(p.steam_id) + '" data-name="' + escAttr(p.player_name) + '" tabindex="0" role="button" style="animation-delay:' + (i * 0.05) + 's">';

			// rank — always shown
			html += '<td class="psyern-lb__rank">' + rank + '</td>';

			if (has('avatar')) {
				html += '<td>';
				if (p.avatar_url) {
					html += '<img class="psyern-lb__avatar" src="' + escHtml(p.avatar_url) + '" alt="" loading="lazy" />';
				}
				html += '</td>';
			}

			// name — always shown. title attribute exposes the full name when
			// the visible label gets ellipsis-truncated by the sticky column.
			html += '<td class="psyern-lb__name" title="' + escAttr(p.player_name) + '">' +
				'<span class="psyern-lb__name-text">' + escHtml(p.player_name) + '</span>' +
				factionBadge(p.war_faction) + '</td>';

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
			if (has('headshots'))  {
				var hs = parseInt(p.headshots, 10) || 0;
				html += '<td class="psyern-lb__headshots">' + (hs > 0 ? fmtN(hs) : '') + '</td>';
			}
			if (has('accuracy')) {
				var acc = parseFloat(p.accuracy) || 0;
				html += '<td class="psyern-lb__accuracy">' + (acc > 0 ? acc.toFixed(1) + '%' : '') + '</td>';
			}
			if (has('longest_shot')) {
				var lsM = parseInt(p.longest_shot, 10) || 0;
				html += '<td class="psyern-lb__longest-shot">' + (lsM > 0 ? fmtN(lsM) + ' m' : '') + '</td>';
			}
			if (has('distance')) {
				var distKm = (parseFloat(p.distance_travelled) || 0) / 1000;
				html += '<td class="psyern-lb__distance">' + (distKm > 0 ? distKm.toFixed(1) + ' km' : '') + '</td>';
			}
			if (has('distance_foot')) {
				var footKm = (parseFloat(p.distance_on_foot) || 0) / 1000;
				html += '<td class="psyern-lb__distance-foot">' + (footKm > 0 ? footKm.toFixed(1) + ' km' : '') + '</td>';
			}
			if (has('distance_vehicle')) {
				var vehKm = (parseFloat(p.distance_in_vehicle) || 0) / 1000;
				html += '<td class="psyern-lb__distance-vehicle">' + (vehKm > 0 ? vehKm.toFixed(1) + ' km' : '') + '</td>';
			}
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
		else if (f === 'WEST') cls = 'psyern-lb__faction--west';
		else if (f === 'BANDIT') cls = 'psyern-lb__faction--bandit';
		else if (f === 'HERO') cls = 'psyern-lb__faction--hero';
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

	// ============================================================
	// Player Detail Modal Controller
	// ============================================================

	var GROUP_LABELS = {
		zombies: 'Zombies',
		players: 'Spieler',
		bosses:  'Bosse',
		ai:      'KI / NPCs',
		animals: 'Tiere',
		other:   'Sonstige'
	};

	var pdmCache = new Map();
	var pdmLastFocus = null;
	var pdmCurrentUid = null;
	var pdmRefs = null;

	function getPdmRefs() {
		if (pdmRefs) return pdmRefs;
		var modal = document.getElementById('psyern-pdm');
		if (!modal) return null;
		pdmRefs = {
			modal:    modal,
			panel:    modal.querySelector('.psyern-pdm__panel'),
			backdrop: modal.querySelector('.psyern-pdm__backdrop'),
			closeBtn: modal.querySelector('.psyern-pdm__close'),
			tabs:     modal.querySelectorAll('.psyern-pdm__tab'),
			panes:    modal.querySelectorAll('.psyern-pdm__pane'),
			loading:  modal.querySelector('.psyern-pdm__loading'),
			error:    modal.querySelector('.psyern-pdm__error'),
			name:     modal.querySelector('.psyern-pdm__name'),
			avatar:   modal.querySelector('.psyern-pdm__avatar'),
			faction:  modal.querySelector('.psyern-pdm__faction-chip'),
			level:    modal.querySelector('.psyern-pdm__level-chip'),
			rep:      modal.querySelector('.psyern-pdm__rep-chip')
		};
		return pdmRefs;
	}

	function fmtKm(meters) {
		return ((parseFloat(meters) || 0) / 1000).toFixed(1) + ' km';
	}

	function fmtPlaytime(seconds) {
		var s = parseInt(seconds, 10) || 0;
		return Math.floor(s / 3600) + 'h ' + Math.floor((s % 3600) / 60) + 'm';
	}

	function setPaneHtml(modal, pane, html) {
		var el = modal.querySelector('.psyern-pdm__pane[data-pane="' + pane + '"] .psyern-pdm__pane-body');
		if (!el) {
			el = modal.querySelector('.psyern-pdm__pane[data-pane="' + pane + '"]');
		}
		if (el) el.innerHTML = html;
	}

	function kpiCard(label, value) {
		return '<div class="psyern-pdm__kpi">' +
			'<div class="psyern-pdm__kpi-label">' + escHtml(label) + '</div>' +
			'<div class="psyern-pdm__kpi-value">' + escHtml(value) + '</div>' +
			'</div>';
	}

	function buildGroupedList(groupObj) {
		if (!groupObj || typeof groupObj !== 'object') return '<div class="psyern-pdm__empty">—</div>';
		var html = '';
		var groupKeys = Object.keys(GROUP_LABELS);
		for (var gi = 0; gi < groupKeys.length; gi++) {
			var gk = groupKeys[gi];
			var entries = groupObj[gk];
			if (!entries) continue;

			// Normalize: entries may be array of {label,count} or object map.
			var arr = [];
			if (Array.isArray(entries)) {
				for (var i = 0; i < entries.length; i++) {
					arr.push({ label: entries[i].label || entries[i].name || entries[i].key || '', count: parseInt(entries[i].count, 10) || 0 });
				}
			} else if (typeof entries === 'object') {
				for (var k in entries) {
					if (!Object.prototype.hasOwnProperty.call(entries, k)) continue;
					arr.push({ label: k, count: parseInt(entries[k], 10) || 0 });
				}
			}
			if (arr.length === 0) continue;

			arr.sort(function(a, b) { return b.count - a.count; });

			var total = 0;
			for (var t = 0; t < arr.length; t++) total += arr[t].count;
			if (total === 0) continue;

			var cap = parseInt(config.playerDetailsMaxPerGroup, 10);
			if (!cap || cap < 1) cap = 20;
			if (cap > 100) cap = 100;
			var capped = arr.length > cap;

			html += '<details open class="psyern-pdm__group" data-group="' + escAttr(gk) + '">';
			html += '<summary class="psyern-pdm__group-header">' +
				escHtml(GROUP_LABELS[gk]) + ' (' + fmtN(total) + ')' +
				'</summary>';
			html += '<div class="psyern-pdm__group-list">';
			for (var j = 0; j < arr.length; j++) {
				var hiddenAttr = (j >= cap) ? ' hidden' : '';
				html += '<div class="psyern-pdm__group-row"' + hiddenAttr + '>' +
					'<span class="psyern-pdm__group-label">' + escHtml(arr[j].label) + '</span>' +
					'<span class="psyern-pdm__group-count">' + fmtN(arr[j].count) + '</span>' +
					'</div>';
			}
			html += '</div>';
			if (capped) {
				html += '<button type="button" class="psyern-pdm__show-all">Alle anzeigen (' + fmtN(arr.length) + ')</button>';
			}
			html += '</details>';
		}
		if (html === '') html = '<div class="psyern-pdm__empty">—</div>';
		return html;
	}

	function setActiveTab(refs, tabName) {
		for (var i = 0; i < refs.tabs.length; i++) {
			var t = refs.tabs[i];
			var match = t.getAttribute('data-tab') === tabName;
			t.setAttribute('aria-selected', match ? 'true' : 'false');
			t.classList.toggle('psyern-pdm__tab--active', match);
		}
		for (var j = 0; j < refs.panes.length; j++) {
			var p = refs.panes[j];
			var pmatch = p.getAttribute('data-pane') === tabName;
			p.classList.toggle('psyern-pdm__pane--active', pmatch);
			if (pmatch) {
				p.removeAttribute('hidden');
				p.setAttribute('aria-hidden', 'false');
			} else {
				p.setAttribute('hidden', '');
				p.setAttribute('aria-hidden', 'true');
			}
		}
	}

	function getFocusable(panel) {
		if (!panel) return [];
		return Array.prototype.slice.call(
			panel.querySelectorAll('button, [href], [tabindex]:not([tabindex="-1"]), input, select, textarea')
		).filter(function(n) { return !n.hasAttribute('disabled') && n.offsetParent !== null; });
	}

	function openPlayerModal(uid, fallbackName) {
		var refs = getPdmRefs();
		if (!refs || !refs.modal) return;
		pdmLastFocus = document.activeElement;
		pdmCurrentUid = uid;

		refs.modal.classList.add('psyern-pdm--open');
		refs.modal.removeAttribute('hidden');
		refs.modal.setAttribute('aria-hidden', 'false');
		document.body.classList.add('psyern-pdm-open');

		if (refs.name) refs.name.textContent = fallbackName || '';
		if (refs.loading) {
			refs.loading.removeAttribute('hidden');
			refs.loading.style.display = '';
		}
		if (refs.error) {
			refs.error.setAttribute('hidden', '');
			refs.error.style.display = 'none';
			refs.error.textContent = '';
		}

		setActiveTab(refs, 'overview');

		if (refs.panel) {
			refs.panel.setAttribute('tabindex', '-1');
			refs.panel.focus();
		}

		fetchPlayer(uid);
	}

	function closePlayerModal() {
		var refs = getPdmRefs();
		if (!refs || !refs.modal) return;
		refs.modal.classList.remove('psyern-pdm--open');
		refs.modal.setAttribute('aria-hidden', 'true');
		refs.modal.setAttribute('hidden', '');
		document.body.classList.remove('psyern-pdm-open');
		pdmCurrentUid = null;
		if (pdmLastFocus && typeof pdmLastFocus.focus === 'function') {
			try { pdmLastFocus.focus(); } catch (e) { /* noop */ }
		}
		pdmLastFocus = null;
	}

	function buildPlayerUrl(uid) {
		var encUid = encodeURIComponent(uid);
		if (config.restUrl) {
			var base = config.restUrl;
			if (base.charAt(base.length - 1) !== '/') base += '/';
			return base + 'public/player/' + encUid;
		}
		if (window.wpApiSettings && window.wpApiSettings.root) {
			var root = window.wpApiSettings.root;
			if (root.charAt(root.length - 1) !== '/') root += '/';
			return root + 'psyern/v1/public/player/' + encUid;
		}
		return '/wp-json/psyern/v1/public/player/' + encUid;
	}

	function showPdmError(msg) {
		var refs = getPdmRefs();
		if (!refs) return;
		if (refs.loading) {
			refs.loading.setAttribute('hidden', '');
			refs.loading.style.display = 'none';
		}
		if (refs.error) {
			refs.error.removeAttribute('hidden');
			refs.error.style.display = '';
			refs.error.textContent = msg;
		}
	}

	function fetchPlayer(uid) {
		var cached = pdmCache.get(uid);
		var now = Date.now();
		if (cached && (now - cached.at) < 60000) {
			renderPlayer(cached.data);
			return;
		}

		var url = buildPlayerUrl(uid);
		fetch(url, { credentials: 'same-origin' })
			.then(function(res) {
				if (res.status === 404) {
					showPdmError('Keine Daten gefunden');
					return null;
				}
				if (!res.ok) {
					showPdmError('Bitte später erneut');
					return null;
				}
				return res.json();
			})
			.then(function(data) {
				if (!data) return;
				// Guard: server may wrap response, accept either shape.
				if (data.data && !data.summary && !data.kills) data = data.data;
				if (uid !== pdmCurrentUid) return; // user closed/switched
				pdmCache.set(uid, { at: Date.now(), data: data });
				renderPlayer(data);
			})
			.catch(function() {
				showPdmError('Bitte später erneut');
			});
	}

	function renderPlayer(data) {
		var refs = getPdmRefs();
		if (!refs) return;

		if (refs.loading) {
			refs.loading.setAttribute('hidden', '');
			refs.loading.style.display = 'none';
		}
		if (refs.error) {
			refs.error.setAttribute('hidden', '');
			refs.error.style.display = 'none';
		}

		var s = data.summary || {};
		var war = data.war || {};
		var gun = data.gunplay || {};
		var mov = data.movement || {};

		// Header chips.
		if (refs.name) refs.name.textContent = data.player_name || s.player_name || refs.name.textContent;

		if (refs.avatar) {
			var avatarUrl = data.avatar_url || '';
			if (config.playerDetailsShowAvatar === false) {
				refs.avatar.src = '';
				refs.avatar.setAttribute('hidden', '');
				refs.avatar.style.display = 'none';
			} else if (avatarUrl) {
				refs.avatar.src = avatarUrl;
				refs.avatar.removeAttribute('hidden');
				refs.avatar.style.display = '';
			} else {
				refs.avatar.src = '';
			}
		}

		if (refs.faction) {
			var f = (war.faction || data.war_faction || '').toUpperCase();
			if (f) {
				refs.faction.textContent = f;
				refs.faction.removeAttribute('hidden');
				refs.faction.style.display = '';
			} else {
				refs.faction.setAttribute('hidden', '');
				refs.faction.style.display = 'none';
			}
		}
		if (refs.level) {
			var lvl = parseInt(war.level || data.war_level, 10) || 0;
			refs.level.textContent = 'Lv ' + lvl;
		}
		if (refs.rep) {
			var rep = parseInt(s.reputation || data.hardline_reputation, 10) || 0;
			refs.rep.textContent = fmtN(rep);
		}

		// Overview pane.
		var kdRatio = (parseFloat(s.kd_ratio) || 0).toFixed(2);
		var overviewHtml =
			kpiCard('Total Kills',     fmtN(s.kills || 0)) +
			kpiCard('K/D',             kdRatio) +
			kpiCard('Boss Kills',      fmtN(s.boss_kills || war.boss_kills || 0)) +
			kpiCard('Playtime',        fmtPlaytime(s.playtime_seconds || 0)) +
			kpiCard('Reputation',      fmtN(s.reputation || 0)) +
			kpiCard('Survivors Down',  fmtN(s.pvp_kills || 0));
		setPaneHtml(refs.modal, 'overview', overviewHtml);

		// War pane.
		var warHtml =
			kpiCard('Faction',     (war.faction || '—').toString().toUpperCase()) +
			kpiCard('Level',       'Lv ' + (parseInt(war.level, 10) || 0)) +
			kpiCard('Alignment',   war.alignment || '—') +
			kpiCard('Boss Kills',  fmtN(war.boss_kills || 0));
		setPaneHtml(refs.modal, 'war', warHtml);

		// Gunplay pane.
		var ranges = (data.longestRanges || data.longest_ranges || []).slice().sort(function(a, b) {
			return (parseFloat(b.range || b.distance || 0)) - (parseFloat(a.range || a.distance || 0));
		}).slice(0, 3);
		var rangesHtml = '';
		for (var ri = 0; ri < ranges.length; ri++) {
			var r = ranges[ri];
			var rDist = parseFloat(r.range || r.distance || 0);
			var rLabel = r.weapon || r.label || ('#' + (ri + 1));
			rangesHtml += '<div class="psyern-pdm__range-row">' +
				'<span class="psyern-pdm__range-label">' + escHtml(rLabel) + '</span>' +
				'<span class="psyern-pdm__range-val">' + fmtN(Math.round(rDist)) + ' m</span>' +
				'</div>';
		}
		if (rangesHtml === '') rangesHtml = '<div class="psyern-pdm__empty">—</div>';
		var gunHtml =
			kpiCard('Shots Fired',  fmtN(gun.shots_fired || 0)) +
			kpiCard('Shots Hit',    fmtN(gun.shots_hit || 0)) +
			kpiCard('Accuracy',     (parseFloat(gun.accuracy) || 0) + '%') +
			kpiCard('Headshots',    fmtN(gun.headshots || 0)) +
			kpiCard('HS %',         (parseFloat(gun.headshot_percent || gun.hs_percent) || 0) + '%') +
			'<div class="psyern-pdm__longest-shots">' +
			'<div class="psyern-pdm__section-title">Longest Shots</div>' +
			rangesHtml +
			'</div>';
		setPaneHtml(refs.modal, 'gunplay', gunHtml);

		// Movement pane.
		var movHtml =
			kpiCard('Total Distance', fmtKm(mov.distance || mov.total || 0)) +
			kpiCard('On Foot',        fmtKm(mov.on_foot || 0)) +
			kpiCard('In Vehicle',     fmtKm(mov.in_vehicle || 0)) +
			kpiCard('Suicides',       fmtN(mov.suicides || s.suicides || 0)) +
			kpiCard('Playtime',       fmtPlaytime(s.playtime_seconds || 0));
		setPaneHtml(refs.modal, 'movement', movHtml);

		// Kills / Deaths panes.
		setPaneHtml(refs.modal, 'kills',  buildGroupedList(data.kills));
		setPaneHtml(refs.modal, 'deaths', buildGroupedList(data.deaths));

		// Skills pane — only surface the tab when the backend actually has
		// Terje data for this player. Hidden by default in the template.
		var skillsArr = (data.skills && data.skills.length) ? data.skills : null;
		var skillsTab = refs.modal.querySelector('[data-tab="skills"]');
		if (skillsArr) {
			setPaneHtml(refs.modal, 'skills', buildSkillsHtml(skillsArr));
			if (skillsTab) skillsTab.removeAttribute('hidden');
		} else {
			setPaneHtml(refs.modal, 'skills', '<div class="psyern-pdm__empty">—</div>');
			if (skillsTab) skillsTab.setAttribute('hidden', '');
		}

		// Show panes container (in case Agent 4 hides until loaded).
		var body = refs.modal.querySelector('.psyern-pdm__body');
		if (body) body.removeAttribute('hidden');
	}

	function buildSkillsHtml(skills) {
		// Sort by level desc for nicer ordering.
		skills.sort(function(a, b) {
			return (parseInt(b.level, 10) || 0) - (parseInt(a.level, 10) || 0);
		});

		var SKILL_LABELS = {
			athletic:   'Athletic',
			ath:        'Athletic',
			hunting:    'Hunting',
			hunt:       'Hunting',
			fishing:    'Fishing',
			fish:       'Fishing',
			survival:   'Survival',
			surv:       'Survival',
			stealth:    'Stealth',
			ste:        'Stealth',
			strength:   'Strength',
			str:        'Strength',
			metabolism: 'Metabolism',
			meta:       'Metabolism'
		};

		var html = '<div class="psyern-pdm__skills-grid">';
		for (var i = 0; i < skills.length; i++) {
			var sk     = skills[i];
			var id     = String(sk.id || '');
			var label  = SKILL_LABELS[id] || (id.charAt(0).toUpperCase() + id.slice(1));
			var level  = parseInt(sk.level, 10) || 0;
			var xp     = parseInt(sk.experience, 10) || 0;
			var pp     = parseInt(sk.perkPoints, 10) || 0;
			var perks  = sk.perks || {};
			var books  = sk.knownBooks || [];

			html += '<article class="psyern-pdm__skill" data-skill="' + escHtml(id) + '">';
			html += '<header class="psyern-pdm__skill-head">';
			html += '<span class="psyern-pdm__skill-name">' + escHtml(label) + '</span>';
			html += '<span class="psyern-pdm__skill-level">Lv ' + level + '</span>';
			html += '</header>';

			html += '<div class="psyern-pdm__skill-meta">';
			html += '<span>XP: ' + fmtN(xp) + '</span>';
			html += '<span>Perk Pts: ' + fmtN(pp) + '</span>';
			html += '</div>';

			// Perks (only ones with value > 0).
			var perkKeys = Object.keys(perks).filter(function(k) {
				return (parseInt(perks[k], 10) || 0) > 0;
			});
			if (perkKeys.length > 0) {
				html += '<div class="psyern-pdm__skill-section-title">Perks</div>';
				html += '<ul class="psyern-pdm__skill-perks">';
				for (var pk = 0; pk < perkKeys.length; pk++) {
					var pid = perkKeys[pk];
					html += '<li><span class="psyern-pdm__perk-id">' + escHtml(pid) + '</span>';
					html += '<span class="psyern-pdm__perk-val">' + (parseInt(perks[pid], 10) || 0) + '</span></li>';
				}
				html += '</ul>';
			}

			// Known books (compact list).
			if (books.length > 0) {
				html += '<div class="psyern-pdm__skill-section-title">Books Read (' + books.length + ')</div>';
				html += '<ul class="psyern-pdm__skill-books">';
				for (var bk = 0; bk < books.length; bk++) {
					html += '<li>' + escHtml(books[bk]) + '</li>';
				}
				html += '</ul>';
			}

			html += '</article>';
		}
		html += '</div>';
		return html;
	}

	function bindPdmHandlers() {
		// Player Details disabled by setting / shortcode? Skip wiring entirely.
		if (config.playerDetailsEnabled === false) return;

		// Delegated row click.
		document.addEventListener('click', function(e) {
			var row = e.target;
			while (row && row !== document && !(row.classList && row.classList.contains('psyern-lb__row') && row.hasAttribute('data-uid'))) {
				row = row.parentNode;
			}
			if (!row || row === document) return;
			var uid = row.getAttribute('data-uid');
			if (!uid) return;
			openPlayerModal(uid, row.getAttribute('data-name') || '');
		});

		// Keyboard activate row.
		document.addEventListener('keydown', function(e) {
			var t = e.target;
			if (!t || !t.classList || !t.classList.contains('psyern-lb__row') || !t.hasAttribute('data-uid')) return;
			if (e.key === 'Enter' || e.key === ' ' || e.key === 'Spacebar') {
				e.preventDefault();
				openPlayerModal(t.getAttribute('data-uid'), t.getAttribute('data-name') || '');
			}
		});

		// Tab buttons + close + backdrop.
		var refs = getPdmRefs();
		if (!refs) return;

		if (refs.closeBtn) {
			refs.closeBtn.addEventListener('click', function(e) {
				e.preventDefault();
				closePlayerModal();
			});
		}
		if (refs.backdrop) {
			refs.backdrop.addEventListener('click', function() { closePlayerModal(); });
		}

		for (var i = 0; i < refs.tabs.length; i++) {
			(function(tab) {
				tab.addEventListener('click', function(e) {
					e.preventDefault();
					var name = tab.getAttribute('data-tab');
					if (name) setActiveTab(refs, name);
				});
				tab.addEventListener('keydown', function(e) {
					if (e.key !== 'ArrowLeft' && e.key !== 'ArrowRight') return;
					e.preventDefault();
					var list = Array.prototype.slice.call(refs.tabs);
					var idx = list.indexOf(tab);
					if (idx === -1) return;
					var nextIdx = (e.key === 'ArrowRight') ? (idx + 1) % list.length : (idx - 1 + list.length) % list.length;
					var next = list[nextIdx];
					var name = next.getAttribute('data-tab');
					if (name) setActiveTab(refs, name);
					next.focus();
				});
			})(refs.tabs[i]);
		}

		// Show-all button (delegated on modal).
		refs.modal.addEventListener('click', function(e) {
			var btn = e.target;
			if (!btn || !btn.classList || !btn.classList.contains('psyern-pdm__show-all')) return;
			var details = btn.closest('details') || btn.parentNode;
			if (!details) return;
			details.classList.add('psyern-pdm__group--expanded');
			var hidden = details.querySelectorAll('.psyern-pdm__group-row[hidden]');
			for (var i = 0; i < hidden.length; i++) hidden[i].removeAttribute('hidden');
			btn.setAttribute('hidden', '');
		});

		// ESC + focus trap.
		document.addEventListener('keydown', function(e) {
			if (!refs.modal.classList.contains('psyern-pdm--open')) return;
			if (e.key === 'Escape' || e.key === 'Esc') {
				e.preventDefault();
				closePlayerModal();
				return;
			}
			if (e.key === 'Tab') {
				var focusable = getFocusable(refs.panel);
				if (focusable.length === 0) return;
				var first = focusable[0];
				var last = focusable[focusable.length - 1];
				if (e.shiftKey && document.activeElement === first) {
					e.preventDefault();
					last.focus();
				} else if (!e.shiftKey && document.activeElement === last) {
					e.preventDefault();
					first.focus();
				}
			}
		});
	}

	// Wire modal handlers once the DOM is parsed.
	if ('loading' === document.readyState) {
		document.addEventListener('DOMContentLoaded', bindPdmHandlers);
	} else {
		bindPdmHandlers();
	}

	if ('loading' === document.readyState) {
		document.addEventListener('DOMContentLoaded', init);
	} else {
		init();
	}
})();

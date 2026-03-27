You are an orchestrating AI agent. Coordinate 4 parallel sub-agents to implement full server-side pagination and a live player name search feature in the WordPress plugin.

BASE PATH: C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\wordpress-plugin\psyerns-framework\

═══════════════════════════════════════════════════════════════
PROBLEM DIAGNOSIS (read-only context — do not change)
═══════════════════════════════════════════════════════════════

The leaderboard is broken in two ways:

1. PAGINATION: Psyern_Database::get_leaderboard() has no OFFSET clause and no page
   parameter. ajax_get_leaderboard() only reads ?limit= (max 100). The JS has no
   page state. Result: only 10 or 20 rows ever shown, no way to navigate further.

2. SEARCH: No search input in the template. No search parameter in the AJAX handler.
   No LIKE query in the database layer. No client-side filtering in JS.

The CSS file (public/css/psyern-leaderboard.css) already contains these classes:
  .psyern-lb__pagination, .psyern-lb__page-btn, .psyern-lb__page-btn.active,
  .psyern-lb__page-btn[disabled]
So pagination button styles are ALREADY DONE — only the HTML and JS are missing.

All 7 themes (military, ash, ops, outbreak, cyberpunk, stalker, inferno) must keep
working. Changes must be design-neutral: use only base class names, never touch
any psyern-theme-*.css file.

═══════════════════════════════════════════════════════════════
AGENT 1 — PHP: DATABASE LAYER
File: includes/class-psyern-database.php
═══════════════════════════════════════════════════════════════

TASK A — Extend get_leaderboard() signature and body.

Find this exact method signature and body:

	public static function get_leaderboard( $mode = 'pvp', $limit = 20 ) {
		global $wpdb;
		$table = self::table( 'leaderboard' );

		return $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE mode = %s ORDER BY score DESC, kills DESC LIMIT %d",
			$mode,
			$limit
		), ARRAY_A );
	}

Replace it entirely with:

	public static function get_leaderboard( $mode = 'pvp', $per_page = 20, $page = 1, $search = '' ) {
		global $wpdb;
		$table    = self::table( 'leaderboard' );
		$per_page = max( 1, absint( $per_page ) );
		$offset   = max( 0, ( absint( $page ) - 1 ) * $per_page );

		if ( ! empty( $search ) ) {
			return $wpdb->get_results( $wpdb->prepare(
				"SELECT * FROM {$table} WHERE mode = %s AND player_name LIKE %s ORDER BY score DESC, kills DESC LIMIT %d OFFSET %d",
				$mode,
				'%' . $wpdb->esc_like( $search ) . '%',
				$per_page,
				$offset
			), ARRAY_A );
		}

		return $wpdb->get_results( $wpdb->prepare(
			"SELECT * FROM {$table} WHERE mode = %s ORDER BY score DESC, kills DESC LIMIT %d OFFSET %d",
			$mode,
			$per_page,
			$offset
		), ARRAY_A );
	}

TASK B — Add count_leaderboard() directly after the closing brace of get_leaderboard().
Insert this new method:

	public static function count_leaderboard( $mode = 'pvp', $search = '' ) {
		global $wpdb;
		$table = self::table( 'leaderboard' );

		if ( ! empty( $search ) ) {
			return (int) $wpdb->get_var( $wpdb->prepare(
				"SELECT COUNT(*) FROM {$table} WHERE mode = %s AND player_name LIKE %s",
				$mode,
				'%' . $wpdb->esc_like( $search ) . '%'
			) );
		}

		return (int) $wpdb->get_var( $wpdb->prepare(
			"SELECT COUNT(*) FROM {$table} WHERE mode = %s",
			$mode
		) );
	}

═══════════════════════════════════════════════════════════════
AGENT 2 — PHP: AJAX HANDLER
File: includes/class-psyern-main.php
═══════════════════════════════════════════════════════════════

Find the entire body of ajax_get_leaderboard():

		check_ajax_referer( 'psyern_leaderboard_nonce', 'nonce' );

		$mode  = sanitize_text_field( wp_unslash( $_GET['mode'] ?? 'pvp' ) );
		$limit = min( absint( $_GET['limit'] ?? 10 ), 100 );

		$players = Psyern_Database::get_leaderboard( $mode, $limit );

		wp_send_json_success( array(
			'mode'    => $mode,
			'players' => $players,
		) );

Replace it with:

		check_ajax_referer( 'psyern_leaderboard_nonce', 'nonce' );

		$mode     = sanitize_text_field( wp_unslash( $_GET['mode'] ?? 'pvp' ) );
		$per_page = min( max( absint( $_GET['per_page'] ?? 20 ), 1 ), 100 );
		$page     = max( absint( $_GET['page'] ?? 1 ), 1 );
		$search   = sanitize_text_field( wp_unslash( $_GET['search'] ?? '' ) );

		$players     = Psyern_Database::get_leaderboard( $mode, $per_page, $page, $search );
		$total       = Psyern_Database::count_leaderboard( $mode, $search );
		$total_pages = ( $per_page > 0 ) ? (int) ceil( $total / $per_page ) : 1;

		wp_send_json_success( array(
			'mode'        => $mode,
			'players'     => $players,
			'total'       => $total,
			'total_pages' => $total_pages,
			'page'        => $page,
			'per_page'    => $per_page,
		) );

═══════════════════════════════════════════════════════════════
AGENT 3 — JAVASCRIPT: FRONTEND STATE, SEARCH, PAGINATION
File: public/js/psyern-leaderboard.js
═══════════════════════════════════════════════════════════════

TASK A — Extend the state object in initInstance().
Find:
		var state = {
			el: el,
			mode: el.getAttribute('data-mode') || 'pvp',
			limit: parseInt(el.getAttribute('data-limit'), 10) || 10,
			refreshInterval: parseInt(el.getAttribute('data-refresh'), 10) || 60000,
		};

Replace with:
		var state = {
			el: el,
			mode: el.getAttribute('data-mode') || 'pvp',
			limit: parseInt(el.getAttribute('data-limit'), 10) || 10,
			refreshInterval: parseInt(el.getAttribute('data-refresh'), 10) || 60000,
			page: 1,
			totalPages: 1,
			searchQuery: '',
		};

TASK B — Reset page on mode switch.
Inside bindModeButtons(), find:
				state.mode = newMode;

Add directly after it:
				state.page = 1;

TASK C — Reset page on limit switch.
Inside bindLimitButtons(), find:
				state.limit = newLimit;

Add directly after it:
				state.page = 1;

TASK D — Add initSearch(state) call inside initInstance().
Find:
		bindLimitButtons(state);
		loadData(state);

Replace with:
		bindLimitButtons(state);
		initSearch(state);
		loadData(state);

TASK E — Update the AJAX URL in loadData() to include page, per_page, search.
Find:
		var url = config.ajaxUrl +
			'?action=psyern_get_leaderboard' +
			'&mode=' + encodeURIComponent(state.mode) +
			'&limit=' + state.limit +
			'&nonce=' + encodeURIComponent(config.nonce);

Replace with:
		var url = config.ajaxUrl +
			'?action=psyern_get_leaderboard' +
			'&mode='     + encodeURIComponent(state.mode) +
			'&per_page=' + state.limit +
			'&page='     + state.page +
			'&search='   + encodeURIComponent(state.searchQuery || '') +
			'&nonce='    + encodeURIComponent(config.nonce);

TASK F — Read total_pages from the response and render pagination.
Inside the fetch .then() callback, find:
				if (!json.success || !json.data || !json.data.players) return;
				renderRows(tbody, json.data.players, state);
				renderFactionBar(state.el, json.data);

Replace with:
				if (!json.success || !json.data || !json.data.players) return;
				state.totalPages = json.data.total_pages || 1;
				renderRows(tbody, json.data.players, state);
				renderFactionBar(state.el, json.data);
				renderPagination(state);

TASK G — Fix rank numbering to be correct across pages.
Inside renderRows(), find:
			var rank = i + 1;

Replace with:
			var rank = (state.page - 1) * state.limit + i + 1;

TASK H — Add 3 new functions.
Insert these 3 functions directly before the existing escHtml function:

	function initSearch(state) {
		var input = state.el.querySelector('.psyern-lb__search-input');
		if (!input) return;
		var timer = null;
		input.addEventListener('input', function() {
			clearTimeout(timer);
			var val = input.value;
			timer = setTimeout(function() {
				state.searchQuery = val.trim();
				state.page = 1;
				loadData(state);
			}, 300);
		});
		input.addEventListener('search', function() {
			if ('' === input.value) {
				state.searchQuery = '';
				state.page = 1;
				loadData(state);
			}
		});
	}

	function buildPageRange(current, total) {
		if (total <= 7) {
			var arr = [];
			for (var i = 1; i <= total; i++) { arr.push(i); }
			return arr;
		}
		var pages = [1];
		if (current > 3) { pages.push('...'); }
		var start = Math.max(2, current - 1);
		var end   = Math.min(total - 1, current + 1);
		for (var i = start; i <= end; i++) { pages.push(i); }
		if (current < total - 2) { pages.push('...'); }
		pages.push(total);
		return pages;
	}

	function renderPagination(state) {
		var container = state.el.querySelector('.psyern-lb__pagination');
		if (!container) return;
		if (state.totalPages <= 1) {
			container.innerHTML = '';
			return;
		}

		var html = '';
		html += '<button class="psyern-lb__page-btn psyern-lb__page-prev"' +
			(state.page <= 1 ? ' disabled aria-disabled="true"' : '') +
			' aria-label="Previous page">\u2039</button>';

		var range = buildPageRange(state.page, state.totalPages);
		for (var i = 0; i < range.length; i++) {
			if (range[i] === '...') {
				html += '<span class="psyern-lb__page-ellipsis" aria-hidden="true">\u2026</span>';
			} else {
				html += '<button class="psyern-lb__page-btn' +
					(range[i] === state.page ? ' active' : '') + '"' +
					' data-page="' + range[i] + '"' +
					' aria-label="Page ' + range[i] + '"' +
					(range[i] === state.page ? ' aria-current="page"' : '') +
					'>' + range[i] + '</button>';
			}
		}

		html += '<button class="psyern-lb__page-btn psyern-lb__page-next"' +
			(state.page >= state.totalPages ? ' disabled aria-disabled="true"' : '') +
			' aria-label="Next page">\u203a</button>';

		container.innerHTML = html;

		var prevBtn = container.querySelector('.psyern-lb__page-prev');
		var nextBtn = container.querySelector('.psyern-lb__page-next');
		var numBtns = container.querySelectorAll('[data-page]');

		if (prevBtn) {
			prevBtn.addEventListener('click', function() {
				if (state.page > 1) { state.page--; loadData(state); }
			});
		}
		if (nextBtn) {
			nextBtn.addEventListener('click', function() {
				if (state.page < state.totalPages) { state.page++; loadData(state); }
			});
		}
		for (var j = 0; j < numBtns.length; j++) {
			numBtns[j].addEventListener('click', function() {
				state.page = parseInt(this.getAttribute('data-page'), 10);
				loadData(state);
			});
		}
	}

═══════════════════════════════════════════════════════════════
AGENT 4 — PHP TEMPLATE + BASE CSS
Files:
  A) public/templates/leaderboard-base.php
  B) public/css/psyern-leaderboard.css
═══════════════════════════════════════════════════════════════

FILE A: public/templates/leaderboard-base.php

TASK 1 — Insert the search input inside .psyern-lb__controls.
Find the closing tag of the limits group (the last line before </div> that closes controls):

		<div class="psyern-lb__limits" role="group" aria-label="<?php esc_attr_e( 'Number of players', 'psyerns-framework' ); ?>">
			<button class="psyern-lb__limit-btn<?php echo ( 10 === $limit ) ? ' active' : ''; ?>" data-limit="10">Top 10</button>
			<button class="psyern-lb__limit-btn<?php echo ( 20 === $limit ) ? ' active' : ''; ?>" data-limit="20">Top 20</button>
		</div>
	</div>

Replace with:

		<div class="psyern-lb__limits" role="group" aria-label="<?php esc_attr_e( 'Number of players', 'psyerns-framework' ); ?>">
			<button class="psyern-lb__limit-btn<?php echo ( 10 === $limit ) ? ' active' : ''; ?>" data-limit="10">Top 10</button>
			<button class="psyern-lb__limit-btn<?php echo ( 20 === $limit ) ? ' active' : ''; ?>" data-limit="20">Top 20</button>
		</div>

		<div class="psyern-lb__search">
			<input
				class="psyern-lb__search-input"
				type="search"
				placeholder="<?php esc_attr_e( 'Search player\u2026', 'psyerns-framework' ); ?>"
				autocomplete="off"
				spellcheck="false"
				aria-label="<?php esc_attr_e( 'Search players by name', 'psyerns-framework' ); ?>"
			/>
		</div>
	</div>

TASK 2 — Insert the pagination nav container.
Find the very last two lines of the file:

	</div>
</div>

Replace with:

	</div>

	<nav class="psyern-lb__pagination" aria-label="<?php esc_attr_e( 'Leaderboard pages', 'psyerns-framework' ); ?>"></nav>
</div>

---

FILE B: public/css/psyern-leaderboard.css

TASK 1 — Add search styles.
Find the existing comment line:
/* ─── Pagination ─── */

Insert these rules DIRECTLY BEFORE that comment (no blank lines required between):

/* ─── Search ─── */
.psyern-lb__search {
	flex: 1;
	min-width: 160px;
	max-width: 260px;
}

.psyern-lb__search-input {
	width: 100%;
	padding: 7px 12px;
	border: 1px solid currentColor;
	border-radius: 4px;
	background: transparent;
	color: inherit;
	font-family: inherit;
	font-size: 0.82rem;
	opacity: 0.7;
	transition: opacity 0.2s;
	outline: none;
	box-sizing: border-box;
	-webkit-appearance: none;
}

.psyern-lb__search-input:focus,
.psyern-lb__search-input:focus-visible {
	opacity: 1;
	outline: 2px solid currentColor;
	outline-offset: 1px;
}

.psyern-lb__search-input::placeholder {
	opacity: 0.5;
}

.psyern-lb__search-input::-webkit-search-cancel-button {
	cursor: pointer;
	opacity: 0.5;
}

@media (max-width: 480px) {
	.psyern-lb__search {
		max-width: 100%;
		width: 100%;
		order: 10;
	}
}

TASK 2 — Add ellipsis style for pagination dots.
Find the existing rule:

.psyern-lb__page-btn[disabled] {
	opacity: 0.2;
	cursor: default;
}

Append DIRECTLY AFTER the closing brace of that rule:

.psyern-lb__page-ellipsis {
	padding: 6px 4px;
	opacity: 0.4;
	font-size: 0.8rem;
	user-select: none;
	pointer-events: none;
}

═══════════════════════════════════════════════════════════════
EXECUTION PLAN
═══════════════════════════════════════════════════════════════

Step 1 — Run Agent 1 and Agent 2 in PARALLEL (both are independent PHP changes).
Step 2 — Run Agent 3 and Agent 4 in PARALLEL (JS and template/CSS are independent).
Step 3 — After all 4 agents finish, verify:
  - No PHP syntax errors in class-psyern-database.php and class-psyern-main.php
  - No JS syntax errors in psyern-leaderboard.js
  - leaderboard-base.php contains .psyern-lb__search and .psyern-lb__pagination elements
  - psyern-leaderboard.css contains .psyern-lb__search-input and .psyern-lb__page-ellipsis
Step 4 — Report: list each modified file, confirm all 5 changes per file are complete.

═══════════════════════════════════════════════════════════════
CONSTRAINTS
═══════════════════════════════════════════════════════════════

- DO NOT touch any psyern-theme-*.css file (military, ash, ops, outbreak, cyberpunk, stalker, inferno).
- DO NOT create new files.
- DO NOT modify class-pf-leaderboard.php (that is a separate, older API endpoint).
- DO NOT modify class-pf-api.php or any other file not listed above.
- Preserve all existing PHP docblocks and WordPress coding style.
- Preserve all existing JS comments and IIFE structure.
- All changes must be backward-compatible: shortcode attributes (mode, limit, theme) still work.

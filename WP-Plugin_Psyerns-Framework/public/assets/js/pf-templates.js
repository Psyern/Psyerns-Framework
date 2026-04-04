const PF = {
	config: {
		apiUrl: '/wp-json/psyern/v1',
		refreshInterval: 300000,
	},

	async fetchLeaderboard(type, limit) {
		type = type || 'pve'; limit = limit || 20;
		var res = await fetch(PF.config.apiUrl + '/public/leaderboard?type=' + type + '&limit=' + limit);
		if (!res.ok) throw new Error('HTTP ' + res.status);
		return res.json();
	},

	async fetchServerStatus() {
		var res = await fetch(PF.config.apiUrl + '/public/status');
		if (!res.ok) throw new Error('HTTP ' + res.status);
		return res.json();
	},

	async fetchTop3(type) {
		type = type || 'monthly';
		var res = await fetch(PF.config.apiUrl + '/public/top3?type=' + type);
		if (!res.ok) throw new Error('HTTP ' + res.status);
		return res.json();
	},

	formatNumber: function(n) { return Number(n || 0).toLocaleString(); },

	formatDate: function(iso) {
		if (!iso) return '\u2014';
		var d = new Date(iso);
		if (isNaN(d.getTime())) return iso;
		var dd = String(d.getDate()).padStart(2, '0');
		var mm = String(d.getMonth() + 1).padStart(2, '0');
		var hh = String(d.getHours()).padStart(2, '0');
		var mi = String(d.getMinutes()).padStart(2, '0');
		return dd + '.' + mm + '.' + d.getFullYear() + ' ' + hh + ':' + mi;
	},

	getKDRatio: function(k, d) {
		k = Number(k || 0); d = Number(d || 0);
		if (d === 0) return k > 0 ? k.toFixed(1) : '0.0';
		return (k / d).toFixed(2);
	},

	escHtml: function(s) {
		var div = document.createElement('div');
		div.appendChild(document.createTextNode(s || ''));
		return div.innerHTML;
	},

	startAutoRefresh: function(cb, interval) {
		return setInterval(cb, interval || PF.config.refreshInterval);
	},

	switchTheme: function(container) {
		var cl = container.classList;
		var themes = ['psyern-lb--military', 'psyern-lb--ash', 'psyern-lb--ops', 'psyern-lb--outbreak', 'psyern-lb--cyberpunk', 'psyern-lb--stalker', 'psyern-lb--inferno'];
		var current = -1;
		for (var i = 0; i < themes.length; i++) {
			if (cl.contains(themes[i])) { current = i; break; }
		}
		if (current >= 0) cl.remove(themes[current]);
		var next = (current + 1) % themes.length;
		cl.add(themes[next]);
	},

	factionBadge: function(faction) {
		if (!faction) return '';
		var f = faction.toUpperCase();
		var cls = 'psyern-lb__faction--neutral';
		if (f === 'EAST') cls = 'psyern-lb__faction--east';
		if (f === 'WEST') cls = 'psyern-lb__faction--west';
		return ' <span class="psyern-lb__faction ' + cls + '">' + PF.escHtml(f) + '</span>';
	},

	renderFactionBar: function(east, west) {
		if (!east && !west) return '';
		var total = east + west;
		if (total === 0) return '';
		var ePct = Math.round((east / total) * 100);
		var wPct = 100 - ePct;
		var h = '<div class="psyern-lb__faction-bar">';
		h += '<span class="psyern-lb__faction-label-east">EAST ' + PF.formatNumber(east) + '</span>';
		h += '<div class="psyern-lb__faction-bar-track">';
		h += '<div class="psyern-lb__faction-bar-east" style="width:' + ePct + '%"></div>';
		h += '<div class="psyern-lb__faction-bar-west" style="width:' + wPct + '%"></div>';
		h += '</div>';
		h += '<span class="psyern-lb__faction-label-west">' + PF.formatNumber(west) + ' WEST</span>';
		h += '</div>';
		return h;
	},

	_lbPage: 0,
	_lbPerPage: 10,

	renderLeaderboardTable: function(data, type, container) {
		var players = data.players || data;
		var meta = data.generatedAt ? data : {};
		var cats = {};
		var i, j, p, keys;
		for (i = 0; i < players.length; i++) {
			var ck = players[i].category_kills || {};
			keys = Object.keys(ck);
			for (j = 0; j < keys.length; j++) cats[keys[j]] = true;
		}
		var catNames = Object.keys(cats);
		var perPage = PF._lbPerPage;
		var totalPages = Math.max(1, Math.ceil(players.length / perPage));
		if (PF._lbPage >= totalPages) PF._lbPage = totalPages - 1;
		if (PF._lbPage < 0) PF._lbPage = 0;
		var page = PF._lbPage;
		var start = page * perPage;
		var end = Math.min(start + perPage, players.length);

		// Header
		var h = '<div class="psyern-lb__header"><h1 class="psyern-lb__title">Leaderboard</h1><div class="psyern-lb__meta">';
		h += '<span>Players Online: ' + (meta.playerOnlineCounter || 0) + '</span>';
		h += '<span>Total: ' + (meta.totalPlayers || 0) + '</span>';
		h += '<span>Updated: ' + PF.formatDate(meta.generatedAt) + '</span>';
		h += '</div></div>';

		// Faction bar
		h += PF.renderFactionBar(meta.globalEastPoints || 0, meta.globalWestPoints || 0);

		// Top 3 cards (only on page 1)
		if (page === 0 && players.length >= 3) {
			var cardOrder = [1, 0, 2];
			h += '<div class="psyern-lb__top3">';
			for (var oi = 0; oi < 3; oi++) {
				var ci = cardOrder[oi];
				var cp = players[ci];
				var cRank = ci + 1;
				h += '<div class="psyern-lb__top3-card psyern-lb__top3-card--' + cRank + '">';
				h += '<span class="psyern-lb__top3-rank">#' + cRank + '</span>';
				if (cp.avatar_url) h += '<img class="psyern-lb__top3-avatar" src="' + PF.escHtml(cp.avatar_url) + '" alt="" loading="lazy" />';
				h += '<div class="psyern-lb__top3-name">' + PF.escHtml(cp.player_name) + PF.factionBadge(cp.war_faction) + '</div>';
				h += '<div class="psyern-lb__top3-stats">K:' + PF.formatNumber(cp.kills) + ' D:' + PF.formatNumber(cp.deaths) + ' KD:' + PF.getKDRatio(cp.kills, cp.deaths) + '</div>';
				h += '</div>';
			}
			h += '</div>';
			h += '<hr class="psyern-lb__separator" />';
		}

		// Table
		h += '<div class="psyern-lb__table-wrap"><table class="psyern-lb__table"><thead><tr>';
		h += '<th>#</th><th></th><th>Name</th><th>Points</th>';
		for (i = 0; i < catNames.length; i++) h += '<th>' + PF.escHtml(catNames[i]) + '</th>';
		h += '<th>Deaths</th><th>Range</th><th>Boss</th><th>Rep</th><th>Status</th><th>Login</th>';
		h += '</tr></thead><tbody>';

		for (i = start; i < end; i++) {
			p = players[i];
			var rank = i + 1;
			var rc = rank <= 3 ? ' psyern-lb__rank-' + rank : '';
			var pts = type === 'pvp' ? (p.pvp_points || 0) : (p.pve_points || 0);
			var deaths = type === 'pvp' ? (p.pvp_deaths || 0) : (p.pve_deaths || 0);
			var online = p.is_online == 1;
			var ck2 = p.category_kills || {};
			var ranges = p.category_longest_ranges || {};
			var bestRange = '\u2014';
			var rKeys = Object.keys(ranges);
			var maxR = 0;
			for (j = 0; j < rKeys.length; j++) { if (ranges[rKeys[j]] > maxR) maxR = ranges[rKeys[j]]; }
			if (maxR > 0) bestRange = maxR + 'm';
			var bk = p.war_boss_kills || 0;
			var rep = p.hardline_reputation || 0;
			var delay = (i - start) * 0.05;

			h += '<tr class="' + rc.trim() + '" style="animation-delay:' + delay + 's">';
			h += '<td><span class="psyern-lb__rank">' + rank + '</span></td>';
			h += '<td>' + (p.avatar_url ? '<img class="psyern-lb__avatar" src="' + PF.escHtml(p.avatar_url) + '" alt="" loading="lazy" />' : '') + '</td>';
			h += '<td class="psyern-lb__name">' + PF.escHtml(p.player_name) + PF.factionBadge(p.war_faction) + '</td>';
			h += '<td class="psyern-lb__kd">' + PF.formatNumber(pts) + '</td>';
			for (j = 0; j < catNames.length; j++) h += '<td>' + PF.formatNumber(ck2[catNames[j]] || 0) + '</td>';
			h += '<td>' + PF.formatNumber(deaths) + '</td>';
			h += '<td>' + bestRange + '</td>';
			h += '<td class="psyern-lb__boss-kills">' + (bk > 0 ? bk : '') + '</td>';
			h += '<td class="psyern-lb__reputation">' + (rep > 0 ? PF.formatNumber(rep) : '') + '</td>';
			h += '<td><span class="' + (online ? 'psyern-lb__badge-online' : 'psyern-lb__badge-offline') + '"></span></td>';
			h += '<td class="psyern-lb__playtime">' + PF.formatDate(p.last_login) + '</td>';
			h += '</tr>';
		}
		if (players.length === 0) {
			h += '<tr><td colspan="' + (10 + catNames.length) + '" class="psyern-lb__loading">No players.</td></tr>';
		}
		h += '</tbody></table></div>';

		// Pagination
		if (totalPages > 1) {
			h += '<div class="psyern-lb__pagination">';
			h += '<button class="psyern-lb__page-btn" data-page="prev"' + (page === 0 ? ' disabled' : '') + '>&laquo; Prev</button>';
			for (i = 0; i < totalPages; i++) {
				h += '<button class="psyern-lb__page-btn' + (i === page ? ' active' : '') + '" data-page="' + i + '">' + (i + 1) + '</button>';
			}
			h += '<button class="psyern-lb__page-btn" data-page="next"' + (page === totalPages - 1 ? ' disabled' : '') + '>Next &raquo;</button>';
			h += '</div>';
		}

		container.innerHTML = h;
		if (window.PsyernEffects) PsyernEffects.init(container);

		// Bind pagination clicks
		var btns = container.querySelectorAll('.psyern-lb__page-btn');
		for (i = 0; i < btns.length; i++) {
			btns[i].addEventListener('click', function(e) {
				var val = this.getAttribute('data-page');
				if (val === 'prev') PF._lbPage--;
				else if (val === 'next') PF._lbPage++;
				else PF._lbPage = parseInt(val, 10);
				PF.renderLeaderboardTable(data, type, container);
			});
		}
	},

	renderTop3Cards: function(players, container, deadliest) {
		var order = [1, 0, 2];
		var medals = ['#1', '#2', '#3'];
		var title = deadliest ? 'Deadliest Players' : 'Top Players of the Month';
		var h = '<h2 class="psyern-lb__title" style="text-align:center;margin-bottom:16px">' + title + '</h2>';
		h += '<div class="psyern-lb__top3" style="display:grid;grid-template-columns:1fr 1.2fr 1fr;gap:12px;align-items:end;width:100%">';
		for (var oi = 0; oi < 3; oi++) {
			var idx = order[oi];
			var p = players[idx];
			if (!p) { h += '<div></div>'; continue; }
			var rank = idx + 1;
			var combined = (p.pve_points || 0) + (p.pvp_points || 0);
			var scale = rank === 1 ? 'transform:scale(1.05)' : '';
			h += '<div class="psyern-lb__top3-card psyern-lb__top3-card--' + rank + '" style="text-align:center;padding:20px 12px;' + scale + '">';
			h += '<span class="psyern-lb__top3-rank">' + medals[idx] + '</span>';
			h += '<img class="psyern-lb__top3-avatar" src="' + PF.escHtml(p.avatar_url || '') + '" alt="" loading="lazy" style="width:64px;height:64px;border-radius:50%;object-fit:cover;display:block;margin:0 auto 8px" />';
			h += '<div class="psyern-lb__top3-name">' + PF.escHtml(p.player_name) + PF.factionBadge(p.war_faction) + '</div>';
			if (deadliest) {
				h += '<div class="psyern-lb__top3-stats">' + PF.formatNumber(p.kills) + ' Kills | K/D ' + PF.getKDRatio(p.kills, p.deaths) + '</div>';
			} else {
				h += '<div class="psyern-lb__top3-stats">' + PF.formatNumber(combined) + ' Pts | ' + PF.formatNumber(p.kills) + 'K ' + PF.formatNumber(p.deaths) + 'D</div>';
			}
			if (p.hardline_reputation > 0) {
				h += '<div style="margin-top:4px;font-size:0.75rem;opacity:0.7">' + PF.formatNumber(p.hardline_reputation) + ' Rep</div>';
			}
			h += '</div>';
		}
		h += '</div>';
		container.innerHTML = h;
		if (window.PsyernEffects) PsyernEffects.init(container);
	},

	renderTop3BossKills: function(players, container) {
		var sorted = players.filter(function(p) { return (p.war_boss_kills || 0) > 0; });
		sorted.sort(function(a, b) { return (b.war_boss_kills || 0) - (a.war_boss_kills || 0); });
		var top = sorted.slice(0, 3);
		if (top.length === 0) {
			container.innerHTML = '<div class="psyern-lb__loading">No boss kills recorded.</div>';
			return;
		}
		var order = top.length >= 3 ? [1, 0, 2] : [];
		if (top.length === 1) order = [0];
		if (top.length === 2) order = [1, 0];

		var h = '<h2 class="psyern-lb__title" style="text-align:center;margin-bottom:16px">Top Boss Slayers</h2>';
		var cols = top.length >= 3 ? '1fr 1.2fr 1fr' : top.length === 2 ? '1fr 1fr' : '1fr';
		h += '<div class="psyern-lb__top3" style="display:grid;grid-template-columns:' + cols + ';gap:12px;align-items:end;width:100%;max-width:700px;margin:0 auto">';
		for (var oi = 0; oi < order.length; oi++) {
			var idx = order[oi];
			var p = top[idx];
			if (!p) continue;
			var rank = idx + 1;
			var scale = rank === 1 ? 'transform:scale(1.05)' : '';
			h += '<div class="psyern-lb__top3-card psyern-lb__top3-card--' + rank + '" style="text-align:center;padding:20px 12px;' + scale + '">';
			h += '<span class="psyern-lb__top3-rank">#' + rank + '</span>';
			if (p.avatar_url) h += '<img class="psyern-lb__top3-avatar" src="' + PF.escHtml(p.avatar_url) + '" alt="" loading="lazy" style="border-radius:50%;object-fit:cover;display:block;margin:0 auto 8px" />';
			h += '<div class="psyern-lb__top3-name">' + PF.escHtml(p.player_name) + PF.factionBadge(p.war_faction) + '</div>';
			h += '<div class="psyern-lb__top3-stats" style="font-size:1.1rem;font-weight:700">' + (p.war_boss_kills || 0) + ' Boss Kills</div>';
			h += '<div style="margin-top:4px;font-size:0.75rem;opacity:0.7">K:' + PF.formatNumber(p.kills) + ' D:' + PF.formatNumber(p.deaths) + '</div>';
			h += '</div>';
		}
		h += '</div>';
		container.innerHTML = h;
		if (window.PsyernEffects) PsyernEffects.init(container);
	},

	renderPlayerCard: function(player, container) {
		var p = player;
		if (!p) { container.innerHTML = '<div class="psyern-lb__loading">Player not found.</div>'; return; }
		var online = p.is_online == 1;
		var h = '<div class="psyern-lb__player-card">';
		h += '<img class="psyern-lb__player-avatar" src="' + PF.escHtml(p.avatar_url || '') + '" alt="" loading="lazy" />';
		h += '<div class="psyern-lb__player-name">' + PF.escHtml(p.player_name) + PF.factionBadge(p.war_faction) + '</div>';
		h += '<div class="psyern-lb__player-status">';
		h += '<span class="' + (online ? 'psyern-lb__badge-online' : 'psyern-lb__badge-offline') + '"></span>';
		h += '<span class="psyern-lb__player-status-label">' + (online ? 'Online' : 'Offline') + '</span>';
		h += '</div>';
		h += '<div class="psyern-lb__player-stats-grid">';
		var stats = [
			['PvE Points', PF.formatNumber(p.pve_points)], ['PvP Points', PF.formatNumber(p.pvp_points)],
			['Kills', PF.formatNumber(p.kills)], ['Deaths', PF.formatNumber(p.deaths)],
			['K/D Ratio', PF.getKDRatio(p.kills, p.deaths)], ['Longest Shot', (p.longest_shot || 0).toFixed(0) + 'm'],
			['AI Kills', PF.formatNumber(p.ai_kills)], ['Playtime', (p.playtime || 0).toFixed(1) + 'h'],
		];
		if (p.war_boss_kills > 0) stats.push(['Boss Kills', p.war_boss_kills]);
		if (p.hardline_reputation > 0) stats.push(['Reputation', PF.formatNumber(p.hardline_reputation)]);
		for (var i = 0; i < stats.length; i++) {
			h += '<div class="psyern-lb__stat-item"><div class="psyern-lb__stat-label">' + stats[i][0] + '</div><div class="psyern-lb__stat-value">' + stats[i][1] + '</div></div>';
		}
		h += '</div>';
		h += '<div class="psyern-lb__player-last-login">Last Login: ' + PF.formatDate(p.last_login) + '</div>';
		if (p.category_kills) {
			var ck = p.category_kills; var cks = Object.keys(ck);
			if (cks.length > 0) {
				h += '<div class="psyern-lb__player-kills">';
				for (var ci = 0; ci < cks.length; ci++) h += '<span class="psyern-lb__faction psyern-lb__faction--neutral psyern-lb__faction--sm">' + PF.escHtml(cks[ci]) + ': ' + ck[cks[ci]] + '</span>';
				h += '</div>';
			}
		}
		h += '</div>';
		container.innerHTML = h;
		if (window.PsyernEffects) PsyernEffects.init(container);
	},

	renderServerStatus: function(status, container) {
		if (!status || status.error) {
			container.innerHTML = '<div class="psyern-lb__loading">No server status available.</div>';
			return;
		}
		var upMin = Math.floor((status.uptimeSeconds || 0) / 60);
		var upH = Math.floor(upMin / 60);
		var upM = upMin % 60;
		var h = '<div class="psyern-lb__server-card">';
		h += '<div class="psyern-lb__server-name">' + PF.escHtml(status.serverName || 'Server') + '</div>';
		h += '<div class="psyern-lb__player-stats-grid">';
		h += '<div class="psyern-lb__stat-item"><div class="psyern-lb__stat-label">Players</div><div class="psyern-lb__stat-value">' + (status.playerCount || 0) + '</div></div>';
		h += '<div class="psyern-lb__stat-item"><div class="psyern-lb__stat-label">Map</div><div class="psyern-lb__stat-value">' + PF.escHtml(status.mapName || '\u2014') + '</div></div>';
		h += '<div class="psyern-lb__stat-item"><div class="psyern-lb__stat-label">Day Time</div><div class="psyern-lb__stat-value">' + PF.escHtml(status.dayTime || '\u2014') + '</div></div>';
		h += '<div class="psyern-lb__stat-item"><div class="psyern-lb__stat-label">Uptime</div><div class="psyern-lb__stat-value">' + upH + 'h ' + upM + 'm</div></div>';
		h += '</div>';
		h += '<div class="psyern-lb__player-last-login">Last update: ' + PF.formatDate(status.timestamp || status.received_at) + '</div>';
		h += '</div>';
		container.innerHTML = h;
		if (window.PsyernEffects) PsyernEffects.init(container);
	},
};

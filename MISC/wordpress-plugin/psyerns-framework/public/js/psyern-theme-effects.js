/**
 * Psyern Theme Effects — Canvas particle systems and JS-driven animations
 * Loaded alongside psyern-leaderboard.js when a theme with effects is active.
 */
(function() {
	'use strict';

	function init() {
		var containers = document.querySelectorAll('.psyern-lb');
		for (var i = 0; i < containers.length; i++) {
			initContainer(containers[i]);
		}
	}

	function initContainer(el) {
		// Remove any existing effect canvases/elements before re-creating
		var old = el.querySelectorAll('[data-pf-effect]');
		for (var k = 0; k < old.length; k++) old[k].remove();
		var cl = el.classList;

		// Inferno — ember particles
		if (cl.contains('psyern-lb--inferno')) {
			createEmberSystem(el);
		}

		// Stalker — grain canvas + Geiger counter + floating biohazard symbols
		if (cl.contains('psyern-lb--stalker')) {
			createGrainCanvas(el);
			createGeigerDisplay(el);
			createBiohazardBg(el);
		}

		// Ops — matrix rain + typewriter title + live status line + moving scanline
		if (cl.contains('psyern-lb--ops')) {
			createMatrixRain(el);
			typewriterTitle(el);
			createStatusLine(el);
			createMovingScanline(el);
		}

		// Military — moving scanline
		if (cl.contains('psyern-lb--military')) {
			createMovingScanline(el);
		}
	}

	/* ═══════════════════════════════════════
	   INFERNO — Ember Canvas (single element, no DOM spam)
	   ═══════════════════════════════════════ */
	function createEmberSystem(container) {
		var canvas = document.createElement('canvas');
		canvas.setAttribute('data-pf-effect', 'ember');
		canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;z-index:2;';
		container.appendChild(canvas);
		var ctx = canvas.getContext('2d');

		function resize() {
			canvas.width = container.offsetWidth;
			canvas.height = container.offsetHeight;
		}
		resize();

		var colors = [[255,69,0],[255,106,0],[255,140,0],[255,200,0],[255,51,0]];
		var embers = [];
		for (var i = 0; i < 25; i++) {
			embers.push({
				x: Math.random() * canvas.width,
				y: canvas.height + Math.random() * 40,
				r: 0.5 + Math.random() * 1.5,
				vx: (Math.random() - 0.5) * 0.3,
				vy: -(0.3 + Math.random() * 0.8),
				life: Math.random(),
				c: colors[Math.floor(Math.random() * colors.length)]
			});
		}

		function draw() {
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			for (var i = 0; i < embers.length; i++) {
				var e = embers[i];
				e.x += e.vx;
				e.y += e.vy;
				e.life -= 0.004;
				if (e.life <= 0 || e.y < -10) {
					e.x = Math.random() * canvas.width;
					e.y = canvas.height + Math.random() * 20;
					e.life = 0.7 + Math.random() * 0.3;
				}
				var a = Math.max(0, e.life * 0.8);
				ctx.beginPath();
				ctx.arc(e.x, e.y, e.r, 0, 6.28);
				ctx.fillStyle = 'rgba(' + e.c[0] + ',' + e.c[1] + ',' + e.c[2] + ',' + a + ')';
				ctx.fill();
			}
			requestAnimationFrame(draw);
		}
		draw();
	}

	/* ═══════════════════════════════════════
	   STALKER — Grain Canvas
	   ═══════════════════════════════════════ */
	function createGrainCanvas(container) {
		var canvas = document.createElement('canvas');
		canvas.setAttribute('data-pf-effect', 'grain');
		canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;z-index:3;opacity:0.06;mix-blend-mode:overlay;';
		container.appendChild(canvas);
		var ctx = canvas.getContext('2d');

		function resize() {
			canvas.width = container.offsetWidth;
			canvas.height = container.offsetHeight;
		}
		resize();

		setInterval(function() {
			var w = canvas.width;
			var h = canvas.height;
			if (w === 0 || h === 0) return;
			var imgData = ctx.createImageData(w, h);
			var d = imgData.data;
			for (var i = 0; i < d.length; i += 4) {
				var v = Math.random() * 255;
				d[i] = v * 0.6; d[i+1] = v * 0.4; d[i+2] = 0; d[i+3] = v * 0.3;
			}
			ctx.putImageData(imgData, 0, 0);
		}, 80);
	}

	/* ═══════════════════════════════════════
	   STALKER — Geiger Counter Display
	   ═══════════════════════════════════════ */
	function createGeigerDisplay(container) {
		var geiger = document.createElement('div');
		geiger.style.cssText = 'position:absolute;top:8px;right:12px;font-family:"Courier Prime",monospace;font-size:10px;color:#ff8c00;opacity:0.6;z-index:5;letter-spacing:0.1em;text-shadow:0 0 4px rgba(255,140,0,0.4);pointer-events:none;';
		geiger.setAttribute('data-pf-effect', 'geiger');
		geiger.textContent = '\u2622 RAD: --- \u03BCSv/h';
		container.appendChild(geiger);

		function tick() {
			var val = 140 + Math.floor(Math.random() * 750);
			geiger.textContent = '\u2622 RAD: ' + val + ' \u03BCSv/h';
			geiger.style.opacity = 0.4 + Math.random() * 0.4;
			setTimeout(tick, 200 + Math.random() * 1800);
		}
		tick();
	}

	/* ═══════════════════════════════════════
	   STALKER — Floating Biohazard Symbols
	   ═══════════════════════════════════════ */
	function createBiohazardBg(container) {
		for (var i = 0; i < 5; i++) {
			var sym = document.createElement('div');
			sym.setAttribute('data-pf-effect', 'biohazard');
			var size = 30 + Math.random() * 50;
			var x = 5 + Math.random() * 85;
			var y = 30 + Math.random() * 60;
			var dur = 15 + Math.random() * 20;
			var rot = Math.random() * 360;
			sym.textContent = '\u2622';
			sym.style.cssText = 'position:absolute;left:' + x + '%;top:' + y + '%;font-size:' + size + 'px;color:rgba(255,140,0,0.025);pointer-events:none;z-index:0;transform:rotate(' + rot + 'deg);will-change:transform;animation:psyern-js-biohazard-float ' + dur + 's ease-in-out infinite alternate;filter:blur(2px);';
			container.appendChild(sym);
		}
	}

	/* ═══════════════════════════════════════
	   OPS — Matrix Rain Canvas
	   ═══════════════════════════════════════ */
	function createMatrixRain(container) {
		var canvas = document.createElement('canvas');
		canvas.setAttribute('data-pf-effect', 'matrix');
		canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;z-index:0;opacity:0.08;';
		container.insertBefore(canvas, container.firstChild);
		var ctx = canvas.getContext('2d');

		function resize() {
			canvas.width = container.offsetWidth;
			canvas.height = container.offsetHeight;
		}
		resize();

		var chars = '\u30A2\u30A4\u30A6\u30A8\u30AA\u30AB\u30AD\u30AF0123456789ABCDEF>_|/\\';
		var fontSize = 12;
		var cols = Math.floor(canvas.width / fontSize) || 1;
		var drops = [];
		for (var i = 0; i < cols; i++) drops[i] = Math.random() * -50;

		setInterval(function() {
			ctx.fillStyle = 'rgba(0, 10, 0, 0.06)';
			ctx.fillRect(0, 0, canvas.width, canvas.height);
			ctx.fillStyle = '#00ff41';
			ctx.font = fontSize + 'px Share Tech Mono, monospace';
			for (var i = 0; i < drops.length; i++) {
				var ch = chars[Math.floor(Math.random() * chars.length)];
				ctx.fillText(ch, i * fontSize, drops[i] * fontSize);
				if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) drops[i] = 0;
				drops[i]++;
			}
		}, 50);
	}

	/* ═══════════════════════════════════════
	   OPS — Typewriter Title Effect
	   ═══════════════════════════════════════ */
	function typewriterTitle(container) {
		var title = container.querySelector('.psyern-lb__title');
		if (!title) return;
		var full = title.textContent;
		title.textContent = '';
		title.style.borderRight = '2px solid #00ff41';
		var idx = 0;
		var interval = setInterval(function() {
			if (idx < full.length) {
				title.textContent += full.charAt(idx);
				idx++;
			} else {
				clearInterval(interval);
				setTimeout(function() { title.style.borderRight = 'none'; }, 1500);
			}
		}, 60);
	}

	/* ═══════════════════════════════════════
	   OPS + MILITARY — Moving Scanline
	   ═══════════════════════════════════════ */
	function createMovingScanline(container) {
		var line = document.createElement('div');
		line.setAttribute('data-pf-effect', 'scanline');
		line.style.cssText = 'position:absolute;left:0;right:0;height:4px;background:linear-gradient(180deg,transparent,rgba(0,255,65,0.06),transparent);pointer-events:none;z-index:4;will-change:top;animation:psyern-js-scanmove 6s linear infinite;';
		container.appendChild(line);
	}

	/* ═══════════════════════════════════════
	   OPS — Live Status Line
	   ═══════════════════════════════════════ */
	function createStatusLine(container) {
		var line = document.createElement('div');
		line.setAttribute('data-pf-effect', 'statusline');
		line.style.cssText = 'font-family:"Share Tech Mono",monospace;font-size:10px;color:#2a8a4a;letter-spacing:0.08em;padding:8px 12px 0;text-transform:uppercase;opacity:0.7;z-index:2;position:relative;';
		container.appendChild(line);

		function update() {
			var now = new Date();
			var t = String(now.getHours()).padStart(2, '0') + ':' + String(now.getMinutes()).padStart(2, '0') + ':' + String(now.getSeconds()).padStart(2, '0');
			var rows = container.querySelectorAll('.psyern-lb__table tbody tr');
			line.textContent = '// LAST UPDATED: ' + t + ' \u2014 SIGNAL: STRONG \u2014 OPERATIVES: ' + rows.length;
		}
		update();
		setInterval(update, 1000);
	}

	/* ═══════════════════════════════════════
	   KEYFRAME INJECTION (for JS-spawned elements)
	   ═══════════════════════════════════════ */
	var style = document.createElement('style');
	style.textContent = '@keyframes psyern-js-ember-rise{0%{opacity:0;transform:translateY(0) translateX(0)}15%{opacity:0.9}50%{transform:translateY(-60px) translateX(' + (Math.random()>0.5?'':'-') + '15px)}100%{opacity:0;transform:translateY(-120px) translateX(' + (Math.random()>0.5?'':'-') + '30px)}}@keyframes psyern-js-biohazard-float{0%{transform:translateY(0) rotate(0deg);opacity:0.03}50%{transform:translateY(-10px) rotate(5deg);opacity:0.05}100%{transform:translateY(5px) rotate(-3deg);opacity:0.03}}@keyframes psyern-js-scanmove{0%{top:-4px}100%{top:100%}}';
	document.head.appendChild(style);

	// Expose re-init for use after dynamic content updates (innerHTML)
	window.PsyernEffects = { init: initContainer };

	// Init on DOM ready
	if (document.readyState === 'loading') {
		document.addEventListener('DOMContentLoaded', init);
	} else {
		init();
	}
})();

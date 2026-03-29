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

		// Frostbite — snowfall canvas + frost breath
		if (cl.contains('psyern-lb--frostbite')) {
			createSnowfallSystem(el);
		}

		// Bubblegum — floating bubble canvas
		if (cl.contains('psyern-lb--bubblegum')) {
			createBubbleSystem(el);
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
	   Uses <img> because WordPress converts ☢ unicode to
	   <img class="emoji"> SVGs — CSS `color` has no effect on <img>.
	   We use opacity + filter (blur, sepia, hue-rotate) instead.
	   ═══════════════════════════════════════ */
	function createBiohazardBg(container) {
		// Animation class names cycle through 5 variants for varied motion
		var animClasses = [
			'psyern-bh--a',
			'psyern-bh--b',
			'psyern-bh--c',
			'psyern-bh--d',
			'psyern-bh--e',
		];
		for (var i = 0; i < 5; i++) {
			var wrap = document.createElement('div');
			wrap.setAttribute('data-pf-effect', 'biohazard');

			var size   = 36 + Math.floor(Math.random() * 48);   // 36–84 px
			var x      = 4  + Math.random() * 88;               // 4–92 %
			var y      = 8  + Math.random() * 78;               // 8–86 %
			var dur    = 14 + Math.random() * 22;               // 14–36 s
			var delay  = -(Math.random() * dur);                // staggered start
			var rot    = Math.floor(Math.random() * 360);       // initial rotation
			var blurPx = (1 + Math.random() * 3).toFixed(1);   // 1–4 px blur
			var opBase = (0.18 + Math.random() * 0.07).toFixed(3); // 0.18–0.25

			// Wrapper positions the icon and handles the float animation
			wrap.style.cssText = [
				'position:absolute',
				'left:'  + x    + '%',
				'top:'   + y    + '%',
				'width:' + size + 'px',
				'height:'+ size + 'px',
				'pointer-events:none',
				'z-index:-1',
				'will-change:transform,opacity',
				'animation:' + animClasses[i] + ' ' + dur.toFixed(1) + 's ' + delay.toFixed(1) + 's ease-in-out infinite alternate',
				'transform:rotate(' + rot + 'deg)',
			].join(';');

			// <img> — immune to WP emoji conversion because it's created via JS DOM,
			// not parsed from HTML. Opacity + filter give us the tint effect.
			var img = document.createElement('img');
			img.src = 'https://s.w.org/images/core/emoji/17.0.2/svg/2622.svg';
			img.alt = '';
			img.setAttribute('aria-hidden', 'true');
			img.style.cssText = [
				'width:100%',
				'height:100%',
				'display:block',
				'opacity:' + opBase,
				// sepia(1) + hue-rotate pushes the SVG toward orange; brightness boosts it
				'filter:sepia(1) saturate(4) hue-rotate(10deg) brightness(0.9) blur(' + blurPx + 'px)',
				'user-select:none',
				'-webkit-user-drag:none',
			].join(';');

			wrap.appendChild(img);
			container.appendChild(wrap);
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
	   FROSTBITE — Snowfall Canvas
	   ═══════════════════════════════════════ */
	function createSnowfallSystem(container) {
		var canvas = document.createElement('canvas');
		canvas.setAttribute('data-pf-effect', 'snowfall');
		canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;z-index:2;';
		container.appendChild(canvas);
		var ctx = canvas.getContext('2d');

		function resize() {
			canvas.width = container.offsetWidth;
			canvas.height = container.offsetHeight;
		}
		resize();

		var flakes = [];
		for (var i = 0; i < 35; i++) {
			flakes.push({
				x: Math.random() * canvas.width,
				y: Math.random() * canvas.height - canvas.height,
				r: 0.5 + Math.random() * 2.5,
				vx: (Math.random() - 0.5) * 0.3,
				vy: 0.2 + Math.random() * 0.8,
				opacity: 0.3 + Math.random() * 0.5,
				wobble: Math.random() * Math.PI * 2
			});
		}

		function draw() {
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			for (var i = 0; i < flakes.length; i++) {
				var f = flakes[i];
				f.wobble += 0.01;
				f.x += f.vx + Math.sin(f.wobble) * 0.3;
				f.y += f.vy;
				if (f.y > canvas.height + 10) {
					f.y = -10;
					f.x = Math.random() * canvas.width;
				}
				if (f.x > canvas.width + 10) f.x = -10;
				if (f.x < -10) f.x = canvas.width + 10;
				ctx.beginPath();
				ctx.arc(f.x, f.y, f.r, 0, 6.28);
				ctx.fillStyle = 'rgba(200, 230, 255, ' + f.opacity + ')';
				ctx.fill();
			}
			requestAnimationFrame(draw);
		}
		draw();
	}

	/* ═══════════════════════════════════════
	   BUBBLEGUM — Floating Bubble Canvas
	   ═══════════════════════════════════════ */
	function createBubbleSystem(container) {
		var canvas = document.createElement('canvas');
		canvas.setAttribute('data-pf-effect', 'bubbles');
		canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;z-index:2;';
		container.appendChild(canvas);
		var ctx = canvas.getContext('2d');

		function resize() {
			canvas.width = container.offsetWidth;
			canvas.height = container.offsetHeight;
		}
		resize();

		var colors = [[255,105,180],[255,158,207],[255,182,218],[255,20,147],[255,130,195]];
		var bubbles = [];
		for (var i = 0; i < 18; i++) {
			bubbles.push({
				x: Math.random() * canvas.width,
				y: canvas.height + Math.random() * 40,
				r: 2 + Math.random() * 6,
				vx: (Math.random() - 0.5) * 0.2,
				vy: -(0.2 + Math.random() * 0.5),
				life: Math.random(),
				c: colors[Math.floor(Math.random() * colors.length)]
			});
		}

		function draw() {
			ctx.clearRect(0, 0, canvas.width, canvas.height);
			for (var i = 0; i < bubbles.length; i++) {
				var b = bubbles[i];
				b.x += b.vx + Math.sin(b.y * 0.02) * 0.3;
				b.y += b.vy;
				b.life -= 0.003;
				if (b.life <= 0 || b.y < -10) {
					b.x = Math.random() * canvas.width;
					b.y = canvas.height + Math.random() * 20;
					b.r = 2 + Math.random() * 6;
					b.life = 0.7 + Math.random() * 0.3;
				}
				var a = Math.max(0, b.life * 0.5);
				ctx.beginPath();
				ctx.arc(b.x, b.y, b.r, 0, 6.28);
				ctx.strokeStyle = 'rgba(' + b.c[0] + ',' + b.c[1] + ',' + b.c[2] + ',' + a + ')';
				ctx.lineWidth = 1;
				ctx.stroke();
				// Inner highlight
				ctx.beginPath();
				ctx.arc(b.x - b.r * 0.25, b.y - b.r * 0.25, b.r * 0.3, 0, 6.28);
				ctx.fillStyle = 'rgba(255,255,255,' + (a * 0.4) + ')';
				ctx.fill();
			}
			requestAnimationFrame(draw);
		}
		draw();
	}

	/* ═══════════════════════════════════════
	   KEYFRAME INJECTION (for JS-spawned elements)
	   ═══════════════════════════════════════ */
	var style = document.createElement('style');
	// 5 biohazard float variants — different Y-travel, rotation arc, and scale pulse.
	// All use transform only (no top/left) for GPU compositing. opacity is on the <img>
	// child so the wrapper animation can stay transform-only for best performance.
	var bhKf = [
		'@keyframes psyern-bh--a{' +
			'0%  {transform:translateY(  0px) translateX(  0px) rotate(  0deg) scale(1.00)}' +
			'33% {transform:translateY(-14px) translateX(  4px) rotate(  6deg) scale(1.02)}' +
			'66% {transform:translateY( -6px) translateX( -5px) rotate( -3deg) scale(0.98)}' +
			'100%{transform:translateY(-18px) translateX(  2px) rotate(  8deg) scale(1.01)}' +
		'}',
		'@keyframes psyern-bh--b{' +
			'0%  {transform:translateY(  0px) translateX(  0px) rotate(  0deg) scale(1.00)}' +
			'40% {transform:translateY(-22px) translateX( -7px) rotate(-10deg) scale(1.03)}' +
			'70% {transform:translateY(-10px) translateX(  6px) rotate(  4deg) scale(0.97)}' +
			'100%{transform:translateY(-28px) translateX( -4px) rotate(-12deg) scale(1.02)}' +
		'}',
		'@keyframes psyern-bh--c{' +
			'0%  {transform:translateY(  0px) translateX(  0px) rotate(  0deg) scale(1.00)}' +
			'25% {transform:translateY( -8px) translateX(  8px) rotate(  5deg) scale(1.01)}' +
			'75% {transform:translateY(-16px) translateX( -3px) rotate( -7deg) scale(1.03)}' +
			'100%{transform:translateY(-10px) translateX(  5px) rotate(  3deg) scale(0.99)}' +
		'}',
		'@keyframes psyern-bh--d{' +
			'0%  {transform:translateY(  0px) translateX(  0px) rotate(  0deg) scale(1.00)}' +
			'50% {transform:translateY(-30px) translateX( 10px) rotate( 14deg) scale(1.04)}' +
			'100%{transform:translateY(-12px) translateX( -8px) rotate( -6deg) scale(0.98)}' +
		'}',
		'@keyframes psyern-bh--e{' +
			'0%  {transform:translateY(  0px) translateX(  0px) rotate(  0deg) scale(1.00)}' +
			'35% {transform:translateY(-20px) translateX( -6px) rotate(-11deg) scale(1.02)}' +
			'80% {transform:translateY( -5px) translateX(  9px) rotate(  7deg) scale(0.96)}' +
			'100%{transform:translateY(-24px) translateX( -2px) rotate( -9deg) scale(1.01)}' +
		'}',
	].join('');

	style.textContent = '@keyframes psyern-js-ember-rise{0%{opacity:0;transform:translateY(0) translateX(0)}15%{opacity:0.9}50%{transform:translateY(-60px) translateX(' + (Math.random()>0.5?'':'-') + '15px)}100%{opacity:0;transform:translateY(-120px) translateX(' + (Math.random()>0.5?'':'-') + '30px)}}' + bhKf + '@keyframes psyern-js-scanmove{0%{top:-4px}100%{top:100%}}';
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

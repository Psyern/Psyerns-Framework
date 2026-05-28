# 🖥️ CRT Military Terminal — CSS Style Guide
**Ops Theme: Phosphor Green · Scanlines · Flicker · Matrix Data Feed**

---

## 🎨 1. CSS Custom Properties (Variablen)

```css
:root {
  --crt-green:        #00ff41;   /* Phosphor-Grün (Matrix/Terminal) */
  --crt-green-dim:    #00b32c;   /* Gedämpftes Grün für Sekundärtext */
  --crt-green-glow:   #00ff4188; /* Grün mit Transparenz für Glows */
  --crt-amber:        #ffb000;   /* Amber-Alternative für Warnungen */
  --crt-bg:           #000a00;   /* Fast-Schwarz mit grünem Hauch */
  --crt-bg-dark:      #000500;   /* Tiefstes Schwarz für Hintergrund */
  --crt-border:       #00ff41;
  --crt-border-glow:  0 0 8px #00ff41, 0 0 20px #00ff41, 0 0 40px #00b32c;
  --scanline-opacity: 0.08;
  --flicker-speed:    0.15s;
}
```

---

## 📺 2. Globales Body-Setup

```css
body {
  background-color: var(--crt-bg-dark);
  color: var(--crt-green);
  font-family: 'Share Tech Mono', 'Courier New', monospace;
  font-size: 14px;
  line-height: 1.6;
  overflow-x: hidden;

  /* Leichte Textkurve für CRT-Wölbung */
  text-shadow: 0 0 5px var(--crt-green), 0 0 10px var(--crt-green-glow);
}
```

> 💡 **Google Font empfohlen:** `Share Tech Mono` oder `VT323` für authentischen Terminal-Look
> ```html
> <link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&display=swap" rel="stylesheet">
> ```

---

## 🟢 3. Äußerer Rahmen — Glow Effekt

```css
.terminal-container {
  border: 1px solid var(--crt-border);
  border-radius: 4px;
  padding: 20px;
  position: relative;

  /* Mehrstufiger Glow-Effekt für Militär-Terminal */
  box-shadow:
    0 0 5px  #00ff41,
    0 0 15px #00ff41,
    0 0 30px #00b32c,
    0 0 60px #004410,
    inset 0 0 30px rgba(0, 255, 65, 0.03);

  /* Animierter Puls */
  animation: borderPulse 4s ease-in-out infinite;
}

@keyframes borderPulse {
  0%, 100% {
    box-shadow:
      0 0 5px  #00ff41,
      0 0 15px #00ff41,
      0 0 30px #00b32c,
      0 0 60px #004410,
      inset 0 0 30px rgba(0, 255, 65, 0.03);
  }
  50% {
    box-shadow:
      0 0 8px  #00ff41,
      0 0 25px #00ff41,
      0 0 50px #00b32c,
      0 0 80px #006620,
      inset 0 0 40px rgba(0, 255, 65, 0.06);
  }
}
```

---

## 📡 4. Scanlines Effekt

```css
/* Methode A: Pseudo-Element (empfohlen, kein Extra-HTML) */
.terminal-container::before {
  content: '';
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  z-index: 9999;

  background: repeating-linear-gradient(
    to bottom,
    transparent,
    transparent 2px,
    rgba(0, 0, 0, var(--scanline-opacity)) 2px,
    rgba(0, 0, 0, var(--scanline-opacity)) 4px
  );
}

/* Methode B: Bewegende Scanline (einzelne Linie scrollt durch) */
.crt-scanline-move {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 6px;
  background: rgba(0, 255, 65, 0.06);
  z-index: 9998;
  pointer-events: none;
  animation: scanMove 6s linear infinite;
}

@keyframes scanMove {
  0%   { top: -6px; }
  100% { top: 100vh; }
}
```

---

## 💡 5. Bildschirm-Flicker Effekt

```css
/* Ganzseitiges Flicker via ::after auf body */
body::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 10000;
  animation: crtFlicker 0.15s infinite;
  background: transparent;
}

@keyframes crtFlicker {
  0%   { opacity: 1; }
  92%  { opacity: 1; }
  93%  { opacity: 0.92; }
  94%  { opacity: 1; }
  96%  { opacity: 0.97; }
  100% { opacity: 1; }
}

/* Stärkerer, seltener Flicker-Effekt (z.B. für Wrapper-Div) */
.screen-flicker {
  animation: hardFlicker 8s ease-in-out infinite;
}

@keyframes hardFlicker {
  0%, 90%, 100% { opacity: 1; filter: brightness(1); }
  91%            { opacity: 0.8; filter: brightness(0.7); }
  92%            { opacity: 1;   filter: brightness(1.1); }
  93%            { opacity: 0.9; filter: brightness(0.9); }
}
```

---

## 🌑 6. Vignette (CRT-Randabdunkelung)

```css
/* Klassische CRT-Vignette */
.crt-vignette::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9997;

  background: radial-gradient(
    ellipse at center,
    transparent 60%,
    rgba(0, 0, 0, 0.7) 100%
  );
}

/* Stärkere Vignette für mehr Militär-Atmosphäre */
.crt-vignette-strong::after {
  background: radial-gradient(
    ellipse at center,
    transparent 40%,
    rgba(0, 5, 0, 0.5) 70%,
    rgba(0, 0, 0, 0.9) 100%
  );
}
```

---

## 💻 7. Text-Glow & Typografie

```css
/* Phosphor-Grün Glow auf Text */
.glow-text {
  color: var(--crt-green);
  text-shadow:
    0 0 4px  var(--crt-green),
    0 0 10px var(--crt-green),
    0 0 20px var(--crt-green-dim);
}

/* Starkes Heading-Glow */
h1, h2, .terminal-title {
  color: var(--crt-green);
  letter-spacing: 0.2em;
  text-transform: uppercase;
  text-shadow:
    0 0 5px  #00ff41,
    0 0 15px #00ff41,
    0 0 30px #00b32c,
    0 0 60px #004410;
}

/* Blinkendes Terminal-Cursor */
.cursor::after {
  content: '_';
  animation: blink 1s step-end infinite;
}

@keyframes blink {
  0%, 100% { opacity: 1; }
  50%       { opacity: 0; }
}

/* Monospace Tabellen-/Datenstil */
.data-value {
  font-family: 'Share Tech Mono', monospace;
  color: var(--crt-green);
  letter-spacing: 0.05em;
}
```

---

## 📊 8. Leaderboard Tabelle — Militär-Style

```css
table.terminal-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Share Tech Mono', monospace;
  font-size: 13px;
}

table.terminal-table th {
  color: var(--crt-green);
  text-transform: uppercase;
  letter-spacing: 0.15em;
  border-bottom: 1px solid var(--crt-green);
  padding: 8px 12px;
  text-shadow: 0 0 8px var(--crt-green);
}

table.terminal-table td {
  padding: 6px 12px;
  border-bottom: 1px solid rgba(0, 255, 65, 0.1);
  color: var(--crt-green-dim);
  transition: all 0.2s;
}

/* Hover: Zeile leuchtet auf */
table.terminal-table tr:hover td {
  background: rgba(0, 255, 65, 0.05);
  color: var(--crt-green);
  text-shadow: 0 0 6px var(--crt-green);
}

/* Top 3 Einträge hervorheben */
table.terminal-table tr:nth-child(1) td { color: #ffd700; text-shadow: 0 0 8px #ffd700; }
table.terminal-table tr:nth-child(2) td { color: #c0c0c0; text-shadow: 0 0 8px #c0c0c0; }
table.terminal-table tr:nth-child(3) td { color: #cd7f32; text-shadow: 0 0 8px #cd7f32; }
```

---

## ⚡ 9. Matrix-Datenverkehr Animation (Hintergrund)

```css
/* Hintergrund-Canvas für fallende Zeichen via JS */
#matrix-canvas {
  position: fixed;
  top: 0;
  left: 0;
  z-index: -1;
  opacity: 0.12;  /* Subtil im Hintergrund halten */
}
```

```javascript
// Minimales Matrix-Rain Script
const canvas = document.createElement('canvas');
canvas.id = 'matrix-canvas';
document.body.prepend(canvas);
const ctx = canvas.getContext('2d');

canvas.width  = window.innerWidth;
canvas.height = window.innerHeight;

const chars = 'アイウエオカキクケコ01アABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()';
const cols   = Math.floor(canvas.width / 16);
const drops  = Array(cols).fill(1);

function draw() {
  ctx.fillStyle = 'rgba(0, 5, 0, 0.05)';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  ctx.fillStyle = '#00ff41';
  ctx.font = '14px Share Tech Mono';

  drops.forEach((y, i) => {
    const char = chars[Math.floor(Math.random() * chars.length)];
    ctx.fillText(char, i * 16, y * 16);
    if (y * 16 > canvas.height && Math.random() > 0.975) drops[i] = 0;
    drops[i]++;
  });
}

setInterval(draw, 50);
```

---

## 🖥️ 10. CRT-Bildschirmwölbung (CSS-only)

```css
/* Leichte Kurve/Wölbung simulieren */
.crt-screen {
  border-radius: 8px;
  overflow: hidden;

  /* Perspektive für 3D-Wölbung */
  transform: perspective(1000px) rotateX(0.3deg);

  /* Curved Glass Reflexion */
  box-shadow:
    inset 0 0 60px rgba(0, 0, 0, 0.5),
    inset 0 0 100px rgba(0, 0, 0, 0.2),
    0 0 5px  #00ff41,
    0 0 20px #00ff41,
    0 0 60px #004410;
}
```

---

## 📦 11. Vollständige Klassen-Übersicht — CRT Military

| Klasse | Effekt |
|---|---|
| `.terminal-container` | Grüner Glow-Rahmen mit Puls-Animation |
| `.crt-scanline-move` | Einzelne scrollende Scanline |
| `.screen-flicker` | Gelegentlicher Bildschirm-Flicker |
| `.crt-vignette` | Rand-Abdunkelung (Vignette) |
| `.crt-screen` | Leichte Bildschirm-Wölbung |
| `.glow-text` | Phosphor-Grün Textglow |
| `.cursor` | Blinkender Terminal-Cursor `_` |
| `.terminal-table` | Militär-Style Datentabelle |
| `#matrix-canvas` | Matrix-Regen Hintergrund (via JS) |

---

## 🔗 12. Empfohlene Google Fonts

```html
<!-- Im <head> einbinden -->
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=VT323&display=swap" rel="stylesheet">
```

| Font | Style |
|---|---|
| `Share Tech Mono` | Technisch, gut lesbar — für Body-Text |
| `VT323` | Echter Pixel-Terminal-Look — für Überschriften |

---

---

# ☢️ STALKER — Radioactive Zone CSS Style Guide
**Theme: Radiation Orange · Chromatic Aberration · Film Grain · Geiger Counter**

---

## 🎨 13. CSS Custom Properties — STALKER Zone

```css
:root {
  /* Strahlung: Gelb-Orange Farbpalette */
  --rad-yellow:       #ffe600;   /* Strahlendes Warngelb */
  --rad-orange:       #ff7b00;   /* Radioaktiv-Orange */
  --rad-orange-dim:   #b85500;   /* Gedämpftes Orange für Sekundärtext */
  --rad-green:        #7fff00;   /* Kontaminiertes Giftgrün */
  --rad-bg:           #0a0800;   /* Fast-Schwarz mit gelblichem Hauch */
  --rad-bg-dark:      #050400;   /* Tiefstes Zone-Schwarz */

  /* Glow-Werte */
  --rad-glow-soft:    0 0 8px  #ffe600, 0 0 20px #ff7b00, 0 0 50px #7a3800;
  --rad-glow-hard:    0 0 10px #ffe600, 0 0 30px #ff7b00, 0 0 80px #ff4400, 0 0 120px #3a1500;

  /* Film Grain */
  --grain-opacity:    0.06;

  /* Chromatic Aberration Stärke */
  --chroma-offset:    2px;
}
```

---

## 🟡 14. Äußerer Rahmen — Radioaktiver Glow + Pulsieren

```css
.stalker-container {
  border: 1px solid var(--rad-yellow);
  border-radius: 3px;
  padding: 20px;
  position: relative;
  background: rgba(10, 8, 0, 0.95);

  /* Mehrstufiger gelber Außenglow */
  box-shadow:
    0 0 6px   #ffe600,
    0 0 20px  #ff7b00,
    0 0 50px  #ff4400,
    0 0 100px #7a3800,
    0 0 160px #2a1000,
    inset 0 0 40px rgba(255, 123, 0, 0.04);

  animation: radPulse 3s ease-in-out infinite;
}

@keyframes radPulse {
  0%, 100% {
    box-shadow:
      0 0 6px   #ffe600,
      0 0 20px  #ff7b00,
      0 0 50px  #ff4400,
      0 0 100px #7a3800,
      0 0 160px #2a1000,
      inset 0 0 40px rgba(255, 123, 0, 0.04);
    border-color: #ffe600;
  }
  33% {
    box-shadow:
      0 0 4px   #ffcc00,
      0 0 14px  #ff6600,
      0 0 35px  #cc3300,
      0 0 70px  #551500,
      inset 0 0 20px rgba(255, 100, 0, 0.02);
    border-color: #ffaa00;
  }
  66% {
    box-shadow:
      0 0 12px  #ffff00,
      0 0 35px  #ff9900,
      0 0 80px  #ff5500,
      0 0 140px #aa4400,
      0 0 200px #331100,
      inset 0 0 60px rgba(255, 150, 0, 0.08);
    border-color: #ffff66;
  }
}
```

---

## ☢️ 15. Radioaktive Zeichen — Drehend im Hintergrund

```css
/* SVG-Radioaktiv-Symbol als CSS-Hintergrund */
.stalker-bg-radiation {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 0;
  overflow: hidden;
}

/* Einzelnes drehendes Symbol — via Pseudo-Element */
.stalker-bg-radiation::before,
.stalker-bg-radiation::after {
  content: '☢';
  position: absolute;
  font-size: 320px;
  color: rgba(255, 180, 0, 0.04);   /* Sehr schwach sichtbar */
  line-height: 1;
  animation: radRotate 30s linear infinite;
  filter: blur(1px);
}

.stalker-bg-radiation::before {
  top: 10%;
  left: 15%;
  animation-duration: 40s;
  font-size: 420px;
  color: rgba(255, 150, 0, 0.03);
}

.stalker-bg-radiation::after {
  bottom: 5%;
  right: 10%;
  animation-duration: 55s;
  animation-direction: reverse;
  font-size: 280px;
  color: rgba(255, 200, 0, 0.035);
}

@keyframes radRotate {
  from { transform: rotate(0deg);   }
  to   { transform: rotate(360deg); }
}

/* Mehrere Symbole via JS platzieren */
```

```javascript
// Mehrere rotierende ☢ Symbole im Hintergrund verteilen
const radContainer = document.createElement('div');
radContainer.className = 'stalker-bg-radiation';
document.body.prepend(radContainer);

const symbols = [
  { size: 380, top:  8, left: 20, dur: 42, opacity: 0.035 },
  { size: 260, top: 55, left: 65, dur: 60, opacity: 0.025 },
  { size: 180, top: 75, left:  5, dur: 35, opacity: 0.04  },
  { size: 440, top: 20, left: 75, dur: 70, opacity: 0.02  },
  { size: 200, top: 45, left: 40, dur: 48, opacity: 0.03  },
];

symbols.forEach(({ size, top, left, dur, opacity }, i) => {
  const el = document.createElement('span');
  el.textContent = '☢';
  Object.assign(el.style, {
    position:        'fixed',
    fontSize:        `${size}px`,
    top:             `${top}%`,
    left:            `${left}%`,
    color:           `rgba(255, 170, 0, ${opacity})`,
    pointerEvents:   'none',
    zIndex:          '0',
    lineHeight:      '1',
    filter:          'blur(0.8px)',
    animation:       `radRotate ${dur}s linear infinite ${i % 2 === 0 ? '' : 'reverse'}`,
    userSelect:      'none',
  });
  radContainer.appendChild(el);
});
```

---

## ✨ 16. Überschriften — Gelbes Glimmern & Pulsieren

```css
/* STALKER Heading-Glow: gelb-orange glimmernd */
.stalker-title,
.stalker-container h1,
.stalker-container h2 {
  color: var(--rad-yellow);
  font-family: 'Share Tech Mono', 'VT323', monospace;
  text-transform: uppercase;
  letter-spacing: 0.25em;
  animation: titleGlimmer 4s ease-in-out infinite;
}

@keyframes titleGlimmer {
  0%, 100% {
    text-shadow:
      0 0 4px  #ffe600,
      0 0 12px #ff9900,
      0 0 25px #ff6600,
      0 0 50px #cc3300;
    color: #ffe600;
  }
  25% {
    text-shadow:
      0 0 2px  #ffcc00,
      0 0 8px  #ff7700,
      0 0 16px #ff4400;
    color: #ffcc00;
  }
  50% {
    text-shadow:
      0 0 6px  #ffff00,
      0 0 18px #ffaa00,
      0 0 40px #ff7700,
      0 0 70px #ff3300,
      0 0 100px #551100;
    color: #ffff88;
  }
  75% {
    text-shadow:
      0 0 3px  #ffd000,
      0 0 10px #ff8800,
      0 0 20px #ff5500;
    color: #ffdd00;
  }
}

/* Subheadings: ruhigeres Orange-Glimmen */
.stalker-container h3,
.stalker-subtitle {
  color: var(--rad-orange);
  letter-spacing: 0.12em;
  text-shadow:
    0 0 4px  #ff7b00,
    0 0 12px #cc4400,
    0 0 25px #882200;
  animation: subtitlePulse 6s ease-in-out infinite;
}

@keyframes subtitlePulse {
  0%, 100% { opacity: 1;    text-shadow: 0 0 4px #ff7b00, 0 0 12px #cc4400; }
  50%       { opacity: 0.85; text-shadow: 0 0 8px #ff9900, 0 0 20px #ff5500, 0 0 40px #aa2200; }
}
```

---

## 📷 17. Chromatic Aberration (Farbkanal-Versatz)

```css
/* Subtiler RGB-Split auf Text und Container */
.chroma-aberration {
  position: relative;
}

.chroma-aberration::before,
.chroma-aberration::after {
  content: attr(data-text);   /* <div data-text="LEADERBOARD"> erforderlich */
  position: absolute;
  top: 0; left: 0;
  width: 100%; height: 100%;
  pointer-events: none;
}

.chroma-aberration::before {
  color: rgba(255, 0, 0, 0.4);
  transform: translateX(calc(-1 * var(--chroma-offset)));
  mix-blend-mode: screen;
}

.chroma-aberration::after {
  color: rgba(0, 200, 255, 0.4);
  transform: translateX(var(--chroma-offset));
  mix-blend-mode: screen;
}

/* Animierter Chroma-Glitch (gelegentlich) */
.chroma-glitch {
  animation: chromaGlitch 7s ease-in-out infinite;
}

@keyframes chromaGlitch {
  0%, 88%, 100% {
    filter: none;
  }
  89% {
    filter: blur(0.3px)
            drop-shadow(-2px 0 rgba(255, 0, 0, 0.7))
            drop-shadow(2px  0 rgba(0, 200, 255, 0.7));
  }
  90% {
    filter: none;
  }
  92% {
    filter: blur(0.5px)
            drop-shadow(-3px 0 rgba(255, 80, 0, 0.5))
            drop-shadow(3px  0 rgba(0, 150, 255, 0.5));
    transform: translateX(1px);
  }
  93% {
    filter: none;
    transform: translateX(0);
  }
}
```

---

## 🎞️ 18. Film Grain / Rauschen Effekt

```css
/* CSS-only Film Grain via SVG-Filter + Pseudo-Element */
.film-grain::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9996;
  opacity: var(--grain-opacity);

  /* SVG-basiertes Rauschen */
  background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 256 256' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noise'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noise)'/%3E%3C/svg%3E");
  background-size: 200px 200px;
  animation: grainShift 0.08s steps(1) infinite;
}

@keyframes grainShift {
  /* Zufällige Grain-Verschiebung für lebendiges Rauschen */
  0%  { background-position:   0px   0px; }
  10% { background-position: -50px -30px; }
  20% { background-position:  80px  20px; }
  30% { background-position: -20px  70px; }
  40% { background-position:  60px -60px; }
  50% { background-position: -80px  40px; }
  60% { background-position:  30px -80px; }
  70% { background-position: -60px  10px; }
  80% { background-position:  90px  50px; }
  90% { background-position: -10px -50px; }
}

/* Stärkeres Grain via Canvas (JS) */
```

```javascript
// Dynamisches Film-Grain via Canvas
function createGrainCanvas() {
  const canvas = document.createElement('canvas');
  canvas.style.cssText = `
    position: fixed; inset: 0;
    width: 100%; height: 100%;
    pointer-events: none;
    z-index: 9996;
    opacity: 0.055;
    mix-blend-mode: overlay;
  `;
  document.body.appendChild(canvas);

  const ctx = canvas.getContext('2d');

  function generateGrain() {
    canvas.width  = window.innerWidth;
    canvas.height = window.innerHeight;
    const imageData = ctx.createImageData(canvas.width, canvas.height);
    const data = imageData.data;

    for (let i = 0; i < data.length; i += 4) {
      const val = Math.random() * 255;
      data[i]     = val;   // R
      data[i + 1] = val;   // G
      data[i + 2] = val;   // B
      data[i + 3] = 30;    // Alpha — niedrig halten!
    }
    ctx.putImageData(imageData, 0, 0);
  }

  generateGrain();
  setInterval(generateGrain, 80); // Alle 80ms neues Grain
}

createGrainCanvas();
```

---

## 📻 19. Geiger Counter — Tick-Effekt & Warnanzeige

```css
/* Geiger Counter Warnanzeige Widget */
.geiger-display {
  font-family: 'Share Tech Mono', monospace;
  font-size: 11px;
  color: var(--rad-orange);
  letter-spacing: 0.08em;
  border: 1px solid rgba(255, 123, 0, 0.4);
  padding: 4px 10px;
  display: inline-block;
  position: relative;
  text-shadow: 0 0 6px #ff7b00;
  animation: geigerTick 0.8s steps(1) infinite;
}

/* Blinkende RAD-Warnanzeige */
.rad-warning {
  color: var(--rad-yellow);
  font-weight: bold;
  letter-spacing: 0.3em;
  animation: radWarn 1.4s ease-in-out infinite;
}

@keyframes radWarn {
  0%, 100% {
    opacity: 1;
    text-shadow: 0 0 6px #ffe600, 0 0 15px #ff7700;
    color: #ffe600;
  }
  50% {
    opacity: 0.6;
    text-shadow: 0 0 3px #ff9900;
    color: #ffaa00;
  }
}

@keyframes geigerTick {
  0%, 85%, 100% { opacity: 1; }
  86%            { opacity: 0.7; color: #ffff00; }
  87%            { opacity: 1;   color: var(--rad-orange); }
  90%            { opacity: 0.9; }
}
```

```javascript
// Geiger Counter Sound-Simulation via Visual Tick
// Zufällige Tick-Intensität erzeugt echten Geiger-Rhythmus
function geigerEffect(element) {
  const baseInterval = 600; // ms zwischen Ticks

  function tick() {
    const intensity = Math.random();
    element.style.textShadow = `0 0 ${4 + intensity * 12}px #ff7b00, 0 0 ${intensity * 25}px #ff3300`;
    element.style.color = intensity > 0.7 ? '#ffff00' : '#ff7b00';

    setTimeout(() => {
      element.style.textShadow = '0 0 6px #ff7b00';
      element.style.color = '#ff7b00';
    }, 60);

    // Nächster Tick: zufälliger Abstand (Geiger-typisch unregelmäßig)
    const nextTick = baseInterval * (0.3 + Math.random() * 1.4);
    setTimeout(tick, nextTick);
  }

  tick();
}

// Aufruf: geigerEffect(document.querySelector('.geiger-display'));
```

---

## 🌡️ 20. Radioaktive Vignette — Orange-Zone Atmosphäre

```css
/* Zone-Vignette: warmes Orange am Rand */
.rad-vignette::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9994;

  background: radial-gradient(
    ellipse at center,
    transparent 45%,
    rgba(80, 30, 0, 0.4) 75%,
    rgba(30, 10, 0, 0.85) 100%
  );
}

/* Kontaminierter Boden-Effekt: stärkere Abdunkelung unten */
.rad-vignette-ground::after {
  background:
    radial-gradient(ellipse at center, transparent 40%, rgba(20, 8, 0, 0.7) 100%),
    linear-gradient(to bottom, transparent 60%, rgba(255, 80, 0, 0.06) 100%);
}
```

---

## 📦 21. Vollständige Klassen-Übersicht — STALKER Zone

| Klasse | Effekt |
|---|---|
| `.stalker-container` | Gelb-orange Glow-Rahmen mit 3-Phasen-Puls |
| `.stalker-bg-radiation` | Rotierende ☢ Symbole im Hintergrund |
| `.stalker-title` | Glimmerndes gelbes Heading mit Atem-Animation |
| `.stalker-subtitle` | Pulsierendes Orange für Subheadings |
| `.chroma-aberration` | RGB-Split Chromatic Aberration auf Elementen |
| `.chroma-glitch` | Gelegentlicher Glitch-Filter-Effekt |
| `.film-grain` | CSS Film Grain via SVG-Filter |
| `createGrainCanvas()` | Dynamisches Canvas Film Grain (JS) |
| `.geiger-display` | Geiger-Counter Tick-Anzeige |
| `.rad-warning` | Blinkende RAD-Warnanzeige |
| `geigerEffect(el)` | Unregelmäßiger Geiger-Tick Rhythmus (JS) |
| `.rad-vignette` | Orange Zone-Vignette am Rand |

---

## 🔗 22. Empfohlene Google Fonts — STALKER

```html
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=VT323&family=Orbitron:wght@400;700&display=swap" rel="stylesheet">
```

| Font | Verwendung |
|---|---|
| `Share Tech Mono` | Body-Text, Datenwerte |
| `VT323` | Retro Pixel-Terminal-Look |
| `Orbitron` | Sci-Fi Headings, Zone-Titel |

---

*// END OF TRANSMISSION — DEADMANS-ECHO OPS TERMINAL //*
*// ☢ ZONE SIGNAL LOST — STATIC — RAD LEVEL: CRITICAL //*

---

---

# 🔥 INFERNO — Fire & Flames CSS Style Guide
**Theme: Blazing Hellfire · Ember Particles · Lava Glow · Scorched Earth · Burning Borders**

---

## 🎨 23. CSS Custom Properties — Inferno

```css
:root {
  /* Feuer-Farbpalette */
  --fire-core:       #fff0a0;   /* Heißester Kern — fast weiß-gelb */
  --fire-yellow:     #ffcc00;   /* Helles Flammengelb */
  --fire-orange:     #ff6600;   /* Klassisches Orange */
  --fire-red:        #cc1100;   /* Tiefes Glutrot */
  --fire-ember:      #ff3300;   /* Glühende Glut */
  --fire-dark:       #3a0a00;   /* Verkohltes Dunkelbraun */
  --lava-glow:       #ff4400;   /* Lava-Unterglow */

  /* Hintergrundschwärze mit rotem Hauch */
  --inferno-bg:      #0a0200;
  --inferno-bg-dark: #050000;

  /* Glow-Presets */
  --fire-glow-soft:  0 0 8px #ff6600, 0 0 20px #cc2200, 0 0 40px #660800;
  --fire-glow-hard:  0 0 6px #ffcc00, 0 0 15px #ff6600, 0 0 35px #cc1100, 0 0 70px #550500, 0 0 120px #1a0000;

  /* Ember Partikel */
  --ember-count:     40;
}
```

---

## 🟠 24. Äußerer Rahmen — Glühender Feuerrand + Pulsieren

```css
.inferno-container {
  border: 1px solid var(--fire-orange);
  border-radius: 3px;
  padding: 20px;
  position: relative;
  background: rgba(10, 2, 0, 0.96);
  overflow: visible;

  /* Mehrstufiger Lava-Außenglow */
  box-shadow:
    0 0 4px   #ffcc00,
    0 0 12px  #ff6600,
    0 0 30px  #cc2200,
    0 0 70px  #880800,
    0 0 130px #330200,
    inset 0 0 50px rgba(200, 30, 0, 0.05);

  animation: infernoPulse 2.5s ease-in-out infinite;
}

@keyframes infernoPulse {
  0%, 100% {
    box-shadow:
      0 0 4px   #ffcc00,
      0 0 12px  #ff6600,
      0 0 30px  #cc2200,
      0 0 70px  #880800,
      0 0 130px #330200,
      inset 0 0 50px rgba(200, 30, 0, 0.05);
    border-color: #ff6600;
  }
  30% {
    box-shadow:
      0 0 2px   #ffaa00,
      0 0 8px   #ff4400,
      0 0 18px  #aa1500,
      0 0 45px  #550500,
      inset 0 0 25px rgba(150, 20, 0, 0.03);
    border-color: #cc3300;
  }
  65% {
    box-shadow:
      0 0 8px   #fff0a0,
      0 0 20px  #ff8800,
      0 0 50px  #ff2200,
      0 0 100px #aa0800,
      0 0 180px #440000,
      inset 0 0 70px rgba(255, 50, 0, 0.08);
    border-color: #ffaa00;
  }
}
```

---

## 🔥 25. Brennende Ränder — Flame Edge Effekt

```css
/* Flackernde Feuerkante oben und unten */
.burning-border {
  position: relative;
}

.burning-border::before,
.burning-border::after {
  content: '';
  position: absolute;
  left: 0;
  width: 100%;
  height: 6px;
  pointer-events: none;
  z-index: 10;
}

/* Oberer brennender Rand */
.burning-border::before {
  top: -3px;
  background: linear-gradient(
    to right,
    transparent 0%,
    #cc2200 15%,
    #ff6600 30%,
    #ffcc00 50%,
    #ff6600 70%,
    #cc2200 85%,
    transparent 100%
  );
  filter: blur(3px);
  animation: burnEdge 1.8s ease-in-out infinite alternate;
}

/* Unterer brennender Rand */
.burning-border::after {
  bottom: -3px;
  background: linear-gradient(
    to right,
    transparent 0%,
    #880500 15%,
    #cc3300 35%,
    #ff5500 50%,
    #cc3300 65%,
    #880500 85%,
    transparent 100%
  );
  filter: blur(4px);
  animation: burnEdge 2.2s ease-in-out infinite alternate-reverse;
}

@keyframes burnEdge {
  0%   { opacity: 0.7; transform: scaleY(1)   scaleX(0.98); filter: blur(2px); }
  33%  { opacity: 1;   transform: scaleY(1.5) scaleX(1.01); filter: blur(3px); }
  66%  { opacity: 0.85; transform: scaleY(0.8) scaleX(0.99); filter: blur(2.5px); }
  100% { opacity: 1;   transform: scaleY(1.3) scaleX(1);    filter: blur(3.5px); }
}

/* Seitenränder brennend */
.burning-border-full {
  box-shadow:
    0 0 15px 2px rgba(255, 80, 0, 0.4),
    0 0 40px 4px rgba(200, 30, 0, 0.25),
    inset 0 0 20px rgba(255, 60, 0, 0.04);
  border-image: linear-gradient(
    135deg,
    #cc2200, #ff6600, #ffcc00, #ff6600, #cc2200
  ) 1;
}
```

---

## ✨ 26. Überschriften — Flackerndes Flammen-Glimmen

```css
/* Inferno Heading — Feuer-Text-Animation */
.inferno-title,
.inferno-container h1,
.inferno-container h2 {
  color: var(--fire-yellow);
  font-family: 'Cinzel', 'Orbitron', serif;
  text-transform: uppercase;
  letter-spacing: 0.2em;
  animation: flameTitle 3s ease-in-out infinite;
}

@keyframes flameTitle {
  0%, 100% {
    color: #ffcc00;
    text-shadow:
      0 0 4px  #fff0a0,
      0 0 10px #ffcc00,
      0 0 22px #ff6600,
      0 0 45px #cc2200,
      0 0 80px #660800;
  }
  20% {
    color: #ff8800;
    text-shadow:
      0 0 2px  #ffaa00,
      0 0 7px  #ff6600,
      0 0 15px #cc2200,
      0 0 30px #880500;
  }
  50% {
    color: #fff0a0;
    text-shadow:
      0 0 6px  #ffffff,
      0 0 14px #fff0a0,
      0 0 28px #ffcc00,
      0 0 55px #ff6600,
      0 0 90px #cc1100,
      0 0 130px #440000;
  }
  75% {
    color: #ff9900;
    text-shadow:
      0 0 3px  #ffcc00,
      0 0 9px  #ff7700,
      0 0 20px #cc3300,
      0 0 40px #770500;
  }
}

/* Subheadings: ruhigeres Glut-Rot */
.inferno-container h3,
.inferno-subtitle {
  color: var(--fire-orange);
  letter-spacing: 0.1em;
  animation: emberGlow 5s ease-in-out infinite;
}

@keyframes emberGlow {
  0%, 100% {
    text-shadow: 0 0 5px #ff6600, 0 0 14px #cc2200, 0 0 28px #660500;
    color: #ff6600;
  }
  50% {
    text-shadow: 0 0 8px #ff8800, 0 0 20px #ff3300, 0 0 45px #aa1100;
    color: #ff8800;
  }
}
```

---

## 🌟 27. Ember Partikel — Fliegende Glut (CSS + JS)

```css
/* Wrapper für alle Ember */
#ember-canvas-wrapper {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9995;
  overflow: hidden;
}

/* Einzelne Ember-Partikel */
.ember {
  position: absolute;
  border-radius: 50%;
  pointer-events: none;
  animation: emberFly linear infinite;
  will-change: transform, opacity;
}

@keyframes emberFly {
  0% {
    transform: translateY(0) translateX(0) scale(1);
    opacity: 0;
  }
  10% {
    opacity: 1;
  }
  80% {
    opacity: 0.6;
  }
  100% {
    transform: translateY(-80vh) translateX(var(--drift, 30px)) scale(0.1);
    opacity: 0;
  }
}
```

```javascript
// Ember Partikel System — fliegende Glut
function createEmberSystem() {
  const wrapper = document.createElement('div');
  wrapper.id = 'ember-canvas-wrapper';
  document.body.appendChild(wrapper);

  const colors = ['#fff0a0', '#ffcc00', '#ff8800', '#ff4400', '#cc2200'];
  const EMBER_COUNT = 55;

  function spawnEmber() {
    const ember = document.createElement('div');
    ember.className = 'ember';

    // Zufällige Eigenschaften
    const size     = 2 + Math.random() * 5;   // 2–7px
    const startX   = Math.random() * 100;      // % von links
    const drift    = (Math.random() - 0.5) * 120; // horizontale Drift
    const duration = 3 + Math.random() * 6;   // 3–9s Flugdauer
    const delay    = Math.random() * 5;        // Start-Verzögerung
    const color    = colors[Math.floor(Math.random() * colors.length)];

    Object.assign(ember.style, {
      width:      `${size}px`,
      height:     `${size}px`,
      bottom:     `-${size}px`,
      left:       `${startX}%`,
      background: color,
      boxShadow:  `0 0 ${size * 2}px ${color}, 0 0 ${size * 4}px ${color}80`,
      '--drift':  `${drift}px`,
      animationDuration:  `${duration}s`,
      animationDelay:     `${delay}s`,
    });

    wrapper.appendChild(ember);

    // Partikel nach Animation entfernen & neu spawnen
    setTimeout(() => {
      ember.remove();
      spawnEmber();
    }, (duration + delay) * 1000);
  }

  // Initial alle Ember spawnen
  for (let i = 0; i < EMBER_COUNT; i++) spawnEmber();
}

createEmberSystem();
```

---

## 🌋 28. Lava Glow — Hintergrundboden-Effekt

```css
/* Lava-Leuchten von unten */
.lava-ground {
  position: fixed;
  bottom: 0;
  left: 0;
  width: 100%;
  height: 30vh;
  pointer-events: none;
  z-index: 9993;

  background: linear-gradient(
    to top,
    rgba(180, 20, 0, 0.25) 0%,
    rgba(100, 10, 0, 0.12) 40%,
    transparent 100%
  );

  animation: lavaFlow 4s ease-in-out infinite;
}

@keyframes lavaFlow {
  0%, 100% {
    background: linear-gradient(
      to top,
      rgba(180, 20, 0, 0.25) 0%,
      rgba(100, 10, 0, 0.12) 40%,
      transparent 100%
    );
    filter: blur(0px);
  }
  50% {
    background: linear-gradient(
      to top,
      rgba(255, 50, 0, 0.35) 0%,
      rgba(160, 20, 0, 0.18) 35%,
      rgba(50, 5, 0, 0.06) 70%,
      transparent 100%
    );
    filter: blur(1px);
  }
}

/* Scorched Earth — verkohlter Boden-Overlay */
.scorched-overlay {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9992;

  background:
    radial-gradient(ellipse at 20% 100%, rgba(255, 60, 0, 0.08) 0%, transparent 50%),
    radial-gradient(ellipse at 80% 100%, rgba(200, 30, 0, 0.07) 0%, transparent 45%),
    radial-gradient(ellipse at 50% 100%, rgba(255, 100, 0, 0.1) 0%, transparent 40%);

  animation: scorchedPulse 6s ease-in-out infinite alternate;
}

@keyframes scorchedPulse {
  from { opacity: 0.7; }
  to   { opacity: 1.0; }
}
```

---

## 🌑 29. Inferno Vignette — Verkohlte Ränder

```css
/* Feuer-Vignette: heiße Mitte, verkohlte Ränder */
.inferno-vignette::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9991;

  background: radial-gradient(
    ellipse at center,
    transparent 40%,
    rgba(60, 5, 0, 0.45) 70%,
    rgba(15, 0, 0, 0.88) 100%
  );
}

/* Zusatz: Flackernde Rand-Vignette */
.inferno-vignette-flicker::after {
  animation: vignetteFlicker 2s ease-in-out infinite;
}

@keyframes vignetteFlicker {
  0%, 100% {
    background: radial-gradient(
      ellipse at center,
      transparent 40%,
      rgba(60, 5, 0, 0.45) 70%,
      rgba(15, 0, 0, 0.88) 100%
    );
  }
  40% {
    background: radial-gradient(
      ellipse at center,
      transparent 35%,
      rgba(90, 8, 0, 0.55) 68%,
      rgba(20, 0, 0, 0.92) 100%
    );
  }
  70% {
    background: radial-gradient(
      ellipse at center,
      transparent 45%,
      rgba(40, 3, 0, 0.38) 72%,
      rgba(10, 0, 0, 0.82) 100%
    );
  }
}
```

---

## 📊 30. Leaderboard Tabelle — Inferno Style

```css
table.inferno-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Share Tech Mono', monospace;
  font-size: 13px;
}

table.inferno-table th {
  color: var(--fire-yellow);
  text-transform: uppercase;
  letter-spacing: 0.15em;
  border-bottom: 1px solid #ff6600;
  padding: 8px 12px;
  text-shadow: 0 0 6px #ffcc00, 0 0 14px #ff6600;
  background: rgba(80, 10, 0, 0.3);
}

table.inferno-table td {
  padding: 6px 12px;
  border-bottom: 1px solid rgba(180, 40, 0, 0.2);
  color: #cc5500;
  transition: all 0.25s;
}

/* Hover: Zeile lodert auf */
table.inferno-table tr:hover td {
  background: rgba(255, 60, 0, 0.06);
  color: #ff8800;
  text-shadow: 0 0 6px #ff6600, 0 0 12px #cc2200;
}

/* Top 3 — Feuer-Intensität steigt */
table.inferno-table tr:nth-child(1) td {
  color: #fff0a0;
  text-shadow: 0 0 6px #fff0a0, 0 0 14px #ffcc00, 0 0 28px #ff6600;
}
table.inferno-table tr:nth-child(2) td {
  color: #ffaa00;
  text-shadow: 0 0 5px #ffaa00, 0 0 12px #ff6600;
}
table.inferno-table tr:nth-child(3) td {
  color: #ff6600;
  text-shadow: 0 0 5px #ff6600, 0 0 10px #cc2200;
}
```

---

## 📦 31. Vollständige Klassen-Übersicht — Inferno

| Klasse / Funktion | Effekt |
|---|---|
| `.inferno-container` | Lava-Glow Rahmen mit 3-Phasen Feuer-Puls |
| `.burning-border` | Brennende Kanten oben/unten (Pseudo-Elemente) |
| `.burning-border-full` | Vollständiger Feuer-Border inkl. Seiten |
| `.inferno-title` | Flackerndes Flammen-Heading (weiß-gelb–orange) |
| `.inferno-subtitle` | Glut-Rot Subheading mit Atem-Glow |
| `createEmberSystem()` | 55 fliegende Ember-Partikel (JS) |
| `.lava-ground` | Lava-Leuchten vom Boden (animiert) |
| `.scorched-overlay` | Verkohlter Boden-Glow (3 Radial-Gradients) |
| `.inferno-vignette` | Verkohlte Rand-Abdunkelung |
| `.inferno-vignette-flicker` | Flackernde Vignette-Variante |
| `table.inferno-table` | Tabelle mit Feuer-Hover + Top-3 Glow |

---

## 🔗 32. Empfohlene Google Fonts — Inferno

```html
<link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@400;700&family=Share+Tech+Mono&family=Orbitron:wght@400;700&display=swap" rel="stylesheet">
```

| Font | Verwendung |
|---|---|
| `Cinzel` | Epische Feuer-Headings, Hell-Titel |
| `Share Tech Mono` | Daten, Tabellenwerte |
| `Orbitron` | Sci-Fi / Militär Subheadings |

---

*// END OF TRANSMISSION — DEADMANS-ECHO OPS TERMINAL //*
*// ☢ ZONE SIGNAL LOST — STATIC — RAD LEVEL: CRITICAL //*
*// 🔥 INFERNO SECTOR OFFLINE — HEAT SIGNATURE: MAXIMUM //*

---

---

# ☣️ OUTBREAK — Quarantine Zone CSS Style Guide
**Theme: Militär-Sperrzone · Apokalyptischer Verfall · Biohazard · Toxisch-Gelb/Orange/Schwarz**

---

## 🎨 33. CSS Custom Properties — Outbreak

```css
:root {
  /* Toxische Farbpalette */
  --bio-yellow:      #d4ff00;   /* Giftgelb — Hauptfarbe */
  --bio-yellow-dim:  #8faa00;   /* Gedämpftes Giftgelb */
  --bio-orange:      #ff8c00;   /* Warnorange */
  --bio-green:       #39ff14;   /* Neon-Schimmelgrün (Akzent) */
  --bio-black:       #080a00;   /* Fast-Schwarz mit gelblichem Stich */
  --bio-dark:        #0d0f00;   /* Hintergrund-Dunkel */
  --bio-stripe:      #1a1a00;   /* Warnstreifen-Dunkel */

  /* Glow-Presets */
  --bio-glow-soft:   0 0 6px #d4ff00, 0 0 18px #8faa00, 0 0 40px #3a4400;
  --bio-glow-hard:   0 0 8px #d4ff00, 0 0 20px #d4ff00, 0 0 50px #aacc00, 0 0 100px #445500;

  /* Hazard Stripes */
  --stripe-width:    40px;
  --stripe-angle:    -45deg;
  --stripe-speed:    8s;

  /* Partikel */
  --bubble-count:    30;
}
```

---

## ⚠️ 34. Äußerer Rahmen — Hazard Glow + Warnstreifen-Border

```css
.outbreak-container {
  border: 2px solid var(--bio-yellow);
  border-radius: 2px;
  padding: 20px;
  position: relative;
  background: rgba(8, 10, 0, 0.96);
  overflow: hidden;

  /* Giftgelber Außenglow */
  box-shadow:
    0 0 5px   #d4ff00,
    0 0 18px  #aacc00,
    0 0 45px  #557700,
    0 0 90px  #223300,
    inset 0 0 40px rgba(180, 220, 0, 0.03);

  animation: bioPulse 3.5s ease-in-out infinite;
}

@keyframes bioPulse {
  0%, 100% {
    box-shadow:
      0 0 5px   #d4ff00,
      0 0 18px  #aacc00,
      0 0 45px  #557700,
      0 0 90px  #223300,
      inset 0 0 40px rgba(180, 220, 0, 0.03);
    border-color: #d4ff00;
  }
  35% {
    box-shadow:
      0 0 3px   #aacc00,
      0 0 10px  #889900,
      0 0 25px  #334400,
      0 0 55px  #111a00,
      inset 0 0 20px rgba(120, 160, 0, 0.02);
    border-color: #99aa00;
  }
  70% {
    box-shadow:
      0 0 10px  #eeff44,
      0 0 28px  #ccee00,
      0 0 65px  #88aa00,
      0 0 130px #334400,
      inset 0 0 60px rgba(220, 255, 0, 0.06);
    border-color: #eeff44;
  }
}

/* Warnstreifen entlang des Rahmens */
.outbreak-container::before {
  content: '';
  position: absolute;
  inset: 0;
  pointer-events: none;
  z-index: 0;
  border-radius: 2px;

  /* Diagonale Warnstreifen als Rand-Overlay */
  background: repeating-linear-gradient(
    var(--stripe-angle),
    transparent,
    transparent calc(var(--stripe-width) - 6px),
    rgba(212, 255, 0, 0.04) calc(var(--stripe-width) - 6px),
    rgba(212, 255, 0, 0.04) var(--stripe-width)
  );
}
```

---

## 🚧 35. Animierte Warnstreifen — Laufendes Hazard-Band

```css
/* Bewegende Diagonalstreifen — klassisches Absperrband */
.hazard-stripes {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9990;
  opacity: 0.045;

  background: repeating-linear-gradient(
    var(--stripe-angle),
    #d4ff00,
    #d4ff00 10px,
    #000000 10px,
    #000000 var(--stripe-width)
  );

  background-size: calc(var(--stripe-width) * 2) calc(var(--stripe-width) * 2);
  animation: stripeScroll var(--stripe-speed) linear infinite;
}

@keyframes stripeScroll {
  from { background-position: 0 0; }
  to   { background-position: calc(var(--stripe-width) * 2) calc(var(--stripe-width) * 2); }
}

/* Härtere Variante: Rand-Warnband oben und unten */
.hazard-band-top,
.hazard-band-bottom {
  position: fixed;
  left: 0;
  width: 100%;
  height: 8px;
  pointer-events: none;
  z-index: 9989;
  opacity: 0.55;

  background: repeating-linear-gradient(
    90deg,
    #d4ff00 0px,
    #d4ff00 16px,
    #111100 16px,
    #111100 32px
  );
  animation: bandFlicker 4s ease-in-out infinite;
}

.hazard-band-top    { top: 0; }
.hazard-band-bottom { bottom: 0; }

@keyframes bandFlicker {
  0%, 100% { opacity: 0.55; }
  45%       { opacity: 0.35; }
  50%       { opacity: 0.6; }
  80%       { opacity: 0.45; }
}
```

---

## ☣️ 36. Biohazard-Symbole — Rotierend im Hintergrund

```css
/* Container für alle Biohazard-Symbole */
#biohazard-bg {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 1;
  overflow: hidden;
}
```

```javascript
// Rotierende ☣ Symbole zufällig verteilt
function createBiohazardBg() {
  const container = document.createElement('div');
  container.id = 'biohazard-bg';
  document.body.prepend(container);

  const symbols = [
    { size: 360, top:  5, left: 10, dur: 50, opacity: 0.032, dir: 1  },
    { size: 240, top: 55, left: 70, dur: 38, opacity: 0.025, dir: -1 },
    { size: 160, top: 72, left: 25, dur: 65, opacity: 0.04,  dir: 1  },
    { size: 480, top: 15, left: 60, dur: 80, opacity: 0.018, dir: -1 },
    { size: 200, top: 40, left: 45, dur: 44, opacity: 0.028, dir: 1  },
    { size: 130, top: 80, left: 82, dur: 30, opacity: 0.038, dir: -1 },
  ];

  symbols.forEach(({ size, top, left, dur, opacity, dir }, i) => {
    const el = document.createElement('span');
    el.textContent = '☣';
    Object.assign(el.style, {
      position:   'fixed',
      fontSize:   `${size}px`,
      top:        `${top}%`,
      left:       `${left}%`,
      color:      `rgba(180, 220, 0, ${opacity})`,
      lineHeight: '1',
      userSelect: 'none',
      filter:     'blur(0.6px)',
      animation:  `bioRotate ${dur}s linear infinite ${dir === -1 ? 'reverse' : ''}`,
    });
    container.appendChild(el);
  });
}

// CSS keyframe via JS injizieren
const bioStyle = document.createElement('style');
bioStyle.textContent = `
  @keyframes bioRotate {
    from { transform: rotate(0deg);   }
    to   { transform: rotate(360deg); }
  }
`;
document.head.appendChild(bioStyle);

createBiohazardBg();
```

---

## 🧫 37. Toxische Blasen & Tröpfchen — Partikel-System

```css
/* Wrapper */
#bubble-wrapper {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9988;
  overflow: hidden;
}

/* Einzelne Blase */
.bio-bubble {
  position: absolute;
  border-radius: 50%;
  pointer-events: none;
  animation: bubbleRise linear infinite;
  will-change: transform, opacity;
}

@keyframes bubbleRise {
  0% {
    transform: translateY(0) translateX(0) scale(1);
    opacity: 0;
  }
  8%  { opacity: 0.8; }
  85% { opacity: 0.5; }
  100% {
    transform: translateY(-75vh) translateX(var(--b-drift, 20px)) scale(0.3);
    opacity: 0;
  }
}
```

```javascript
// Toxisches Blasen-Partikel-System
function createBubbleSystem() {
  const wrapper = document.createElement('div');
  wrapper.id = 'bubble-wrapper';
  document.body.appendChild(wrapper);

  // Farben: giftgelb, orange, schimmelgrün
  const colors = [
    'rgba(180, 220, 0,  0.55)',
    'rgba(212, 255, 0,  0.45)',
    'rgba(255, 140, 0,  0.40)',
    'rgba( 57, 255, 20, 0.30)',
    'rgba(160, 200, 0,  0.50)',
  ];

  const BUBBLE_COUNT = 35;

  function spawnBubble() {
    const el      = document.createElement('div');
    el.className  = 'bio-bubble';

    const size    = 4 + Math.random() * 14;        // 4–18px
    const startX  = Math.random() * 100;            // % horizontal
    const drift   = (Math.random() - 0.5) * 80;    // Links/Rechts-Drift
    const dur     = 6 + Math.random() * 10;         // 6–16s Aufstieg
    const delay   = Math.random() * 8;
    const color   = colors[Math.floor(Math.random() * colors.length)];
    const blur    = Math.random() * 2;              // leichte Unschärfe

    Object.assign(el.style, {
      width:             `${size}px`,
      height:            `${size}px`,
      bottom:            `-${size}px`,
      left:              `${startX}%`,
      background:        color,
      '--b-drift':       `${drift}px`,
      boxShadow:         `0 0 ${size}px ${color}, inset 0 0 ${size * 0.4}px rgba(255,255,255,0.15)`,
      filter:            `blur(${blur}px)`,
      animationDuration: `${dur}s`,
      animationDelay:    `${delay}s`,
    });

    wrapper.appendChild(el);

    setTimeout(() => {
      el.remove();
      spawnBubble();
    }, (dur + delay) * 1000);
  }

  for (let i = 0; i < BUBBLE_COUNT; i++) spawnBubble();
}

createBubbleSystem();
```

---

## 📡 38. Glitch-Text — Störsignal-Effekt

```css
/* Basis Glitch auf beliebigem Element */
.bio-glitch {
  position: relative;
  animation: glitchBase 6s infinite;
}

/* Farbkanal-Versatz Klone via data-text Attribut */
/* Nutzung: <h1 class="bio-glitch" data-text="QUARANTINE">QUARANTINE</h1> */
.bio-glitch::before,
.bio-glitch::after {
  content: attr(data-text);
  position: absolute;
  top: 0; left: 0;
  width: 100%;
  pointer-events: none;
}

.bio-glitch::before {
  color: rgba(212, 255, 0, 0.75);
  animation: glitchTop 6s infinite;
  clip-path: polygon(0 0, 100% 0, 100% 35%, 0 35%);
}

.bio-glitch::after {
  color: rgba(255, 140, 0, 0.65);
  animation: glitchBottom 6s infinite;
  clip-path: polygon(0 60%, 100% 60%, 100% 100%, 0 100%);
}

@keyframes glitchBase {
  0%, 90%, 100% { transform: none; filter: none; }
  91%            { transform: skewX(-2deg); }
  92%            { transform: skewX(2deg) translateX(2px); filter: brightness(1.3); }
  93%            { transform: none; }
  96%            { transform: translateX(-2px) skewX(1deg); }
  97%            { transform: none; }
}

@keyframes glitchTop {
  0%, 88%, 100%  { transform: none; opacity: 0; }
  89%             { transform: translateX(-4px); opacity: 1; }
  90%             { transform: translateX(3px);  opacity: 1; }
  91%             { transform: none; opacity: 0; }
  95%             { transform: translateX(-2px); opacity: 0.8; }
  96%             { opacity: 0; }
}

@keyframes glitchBottom {
  0%, 88%, 100%  { transform: none; opacity: 0; }
  89%             { transform: translateX(4px);  opacity: 1; }
  90%             { transform: translateX(-3px); opacity: 1; }
  91%             { transform: none; opacity: 0; }
  94%             { transform: translateX(2px);  opacity: 0.7; }
  95%             { opacity: 0; }
}

/* Rauschen-Overlay beim Glitch */
.bio-glitch-noise {
  animation: glitchNoise 7s steps(1) infinite;
}

@keyframes glitchNoise {
  0%, 85%, 100% { clip-path: none; transform: none; }
  86% { clip-path: polygon(0 15%, 100% 15%, 100% 18%, 0 18%); transform: translateX(3px); }
  87% { clip-path: polygon(0 62%, 100% 62%, 100% 68%, 0 68%); transform: translateX(-2px); }
  88% { clip-path: polygon(0 44%, 100% 44%, 100% 46%, 0 46%); transform: translateX(4px); }
  89% { clip-path: none; transform: none; }
}
```

---

## 💀 39. Verfall & Schimmel — Organischer Hintergrund-Effekt

```css
/* Schimmel-Flecken als radiale Verläufe im Hintergrund */
.decay-overlay {
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9987;
  mix-blend-mode: multiply;

  background:
    radial-gradient(ellipse 200px 150px at 12%  88%, rgba(30, 60, 0, 0.55) 0%, transparent 70%),
    radial-gradient(ellipse 120px 200px at 90%  75%, rgba(20, 50, 0, 0.45) 0%, transparent 65%),
    radial-gradient(ellipse 300px 100px at 50% 100%, rgba(40, 80, 0, 0.35) 0%, transparent 60%),
    radial-gradient(ellipse 180px 140px at  5%  30%, rgba(15, 40, 0, 0.40) 0%, transparent 70%),
    radial-gradient(ellipse 100px 180px at 85%  10%, rgba(25, 55, 0, 0.30) 0%, transparent 60%);

  animation: decayShift 12s ease-in-out infinite alternate;
}

@keyframes decayShift {
  from { opacity: 0.6; }
  to   { opacity: 1.0; }
}

/* Organische Textur via SVG-Filter */
.decay-texture::before {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9986;
  opacity: 0.03;

  background-image: url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='300' height='300'%3E%3Cfilter id='org'%3E%3CfeTurbulence type='turbulence' baseFrequency='0.65' numOctaves='3' stitchTiles='stitch'/%3E%3CfeColorMatrix type='saturate' values='0'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23org)'/%3E%3C/svg%3E");
  background-size: 300px 300px;
  animation: textureShift 20s steps(1) infinite;
}

@keyframes textureShift {
  0%   { background-position:   0px   0px; }
  25%  { background-position: -80px  40px; }
  50%  { background-position:  60px -60px; }
  75%  { background-position: -40px  80px; }
  100% { background-position:   0px   0px; }
}
```

---

## ✍️ 40. Überschriften — Giftgelb Toxisch-Glow

```css
/* OUTBREAK Heading — toxisch glimmernd */
.outbreak-title,
.outbreak-container h1,
.outbreak-container h2 {
  color: var(--bio-yellow);
  font-family: 'Share Tech Mono', monospace;
  text-transform: uppercase;
  letter-spacing: 0.25em;
  animation: toxicGlow 4s ease-in-out infinite;
}

@keyframes toxicGlow {
  0%, 100% {
    color: #d4ff00;
    text-shadow:
      0 0 4px  #d4ff00,
      0 0 12px #aacc00,
      0 0 28px #778800,
      0 0 55px #334400;
  }
  30% {
    color: #bbdd00;
    text-shadow:
      0 0 2px  #aacc00,
      0 0 8px  #889900,
      0 0 16px #445500;
  }
  60% {
    color: #eeff44;
    text-shadow:
      0 0 6px  #eeff44,
      0 0 16px #ccee00,
      0 0 36px #aacc00,
      0 0 70px #557700,
      0 0 110px #223300;
  }
}

/* Warnorange Subheadings */
.outbreak-container h3,
.outbreak-subtitle {
  color: var(--bio-orange);
  letter-spacing: 0.12em;
  text-shadow: 0 0 5px #ff8c00, 0 0 14px #cc5500, 0 0 28px #663300;
  animation: orangeBreath 5s ease-in-out infinite;
}

@keyframes orangeBreath {
  0%, 100% { opacity: 1;   text-shadow: 0 0 5px #ff8c00, 0 0 14px #cc5500; }
  50%       { opacity: 0.8; text-shadow: 0 0 8px #ffaa00, 0 0 22px #ff6600, 0 0 45px #aa3300; }
}

/* Quarantine-Alarm: abwechselnd Gelb / Orange (Blinken) */
.quarantine-alarm {
  animation: quarantineBlink 1.2s step-end infinite;
}

@keyframes quarantineBlink {
  0%, 49% {
    color: #d4ff00;
    text-shadow: 0 0 8px #d4ff00, 0 0 20px #aacc00;
  }
  50%, 100% {
    color: #ff8c00;
    text-shadow: 0 0 8px #ff8c00, 0 0 20px #cc5500;
  }
}
```

---

## 📊 41. Leaderboard Tabelle — Quarantine Style

```css
table.outbreak-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Share Tech Mono', monospace;
  font-size: 13px;
}

table.outbreak-table th {
  color: var(--bio-yellow);
  text-transform: uppercase;
  letter-spacing: 0.15em;
  border-bottom: 1px solid #d4ff00;
  padding: 8px 12px;
  text-shadow: 0 0 6px #d4ff00, 0 0 14px #aacc00;
  background: rgba(20, 30, 0, 0.5);
}

/* Hazard-Streifen auf jedem zweiten Header-Abschnitt */
table.outbreak-table th:nth-child(even) {
  background: repeating-linear-gradient(
    -45deg,
    rgba(212, 255, 0, 0.04),
    rgba(212, 255, 0, 0.04) 4px,
    transparent 4px,
    transparent 10px
  );
}

table.outbreak-table td {
  padding: 6px 12px;
  border-bottom: 1px solid rgba(180, 220, 0, 0.12);
  color: var(--bio-yellow-dim);
  transition: all 0.2s;
}

/* Hover: Kontamination aktiv */
table.outbreak-table tr:hover td {
  background: rgba(180, 220, 0, 0.05);
  color: var(--bio-yellow);
  text-shadow: 0 0 6px #d4ff00, 0 0 12px #88aa00;
}

/* Top 3 — Kontaminierungsstufen */
table.outbreak-table tr:nth-child(1) td {
  color: #eeff44;
  text-shadow: 0 0 6px #eeff44, 0 0 16px #d4ff00, 0 0 30px #aacc00;
}
table.outbreak-table tr:nth-child(2) td {
  color: #ffaa00;
  text-shadow: 0 0 5px #ffaa00, 0 0 12px #ff7700;
}
table.outbreak-table tr:nth-child(3) td {
  color: #aacc00;
  text-shadow: 0 0 4px #aacc00, 0 0 10px #778800;
}
```

---

## 🌑 42. Quarantine Vignette

```css
/* Vergiftete Ränder — Giftgelb-schwarz */
.outbreak-vignette::after {
  content: '';
  position: fixed;
  inset: 0;
  pointer-events: none;
  z-index: 9985;

  background: radial-gradient(
    ellipse at center,
    transparent 42%,
    rgba(10, 20, 0, 0.5) 72%,
    rgba(2, 5, 0, 0.92) 100%
  );

  animation: outbreakVig 5s ease-in-out infinite alternate;
}

@keyframes outbreakVig {
  from {
    background: radial-gradient(ellipse at center,
      transparent 42%, rgba(10, 20, 0, 0.5) 72%, rgba(2, 5, 0, 0.92) 100%);
  }
  to {
    background: radial-gradient(ellipse at center,
      transparent 38%, rgba(15, 30, 0, 0.6) 68%, rgba(0, 4, 0, 0.95) 100%);
  }
}
```

---

## 📦 43. Vollständige Klassen-Übersicht — Outbreak

| Klasse / Funktion | Effekt |
|---|---|
| `.outbreak-container` | Giftgelber Glow-Rahmen mit 3-Phasen Puls |
| `.hazard-stripes` | Animiertes Warnstreifen-Overlay (ganzseitig) |
| `.hazard-band-top/bottom` | Hazard-Band oben und unten |
| `createBiohazardBg()` | 6 rotierende ☣ Symbole im Hintergrund (JS) |
| `createBubbleSystem()` | 35 toxische Blasen-Partikel aufsteigend (JS) |
| `.bio-glitch` | RGB-Split Glitch mit Clip-Path Segmenten |
| `.bio-glitch-noise` | Zeilenweiser Glitch-Noise Effekt |
| `.decay-overlay` | Schimmel-Flecken als Radial-Gradients |
| `.decay-texture` | Organische SVG-Rauschtextur |
| `.outbreak-title` | Toxisch-gelbes Heading Glow |
| `.quarantine-alarm` | Gelb/Orange Warnblink alternierend |
| `table.outbreak-table` | Tabelle mit Hazard-Streifen + Top-3 Kontamination |
| `.outbreak-vignette` | Vergiftete Randabdunkelung (atmend) |

---

## 🔗 44. Empfohlene Google Fonts — Outbreak

```html
<link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Orbitron:wght@400;700&family=Rajdhani:wght@600;700&display=swap" rel="stylesheet">
```

| Font | Verwendung |
|---|---|
| `Share Tech Mono` | Daten, Terminal-Text, Tabellen |
| `Orbitron` | Militär-Headings, Zone-Titel |
| `Rajdhani` | Warnschilder, Kurztext, Labels |

---

*// END OF TRANSMISSION — DEADMANS-ECHO OPS TERMINAL //*
*// ☢ ZONE SIGNAL LOST — STATIC — RAD LEVEL: CRITICAL //*
*// 🔥 INFERNO SECTOR OFFLINE — HEAT SIGNATURE: MAXIMUM //*
*// ☣ OUTBREAK CONTAINMENT FAILED — SECTOR: COMPROMISED //*

---

---

# 🏆 TOP 3 PODIUM — Nebeneinander mit gestaffelter Avatar-Größe
**Layout: 2. Platz links · 1. Platz Mitte (größter Avatar) · 3. Platz rechts**

---

## 45. Podium HTML-Struktur

```html
<!--
  Reihenfolge im HTML: 2 — 1 — 3
  Damit 1. Platz optisch in der Mitte und höher steht.
  order: via CSS Flexbox gesteuert.
-->
<div class="podium-wrapper">

  <!-- 2. Platz — LINKS -->
  <div class="podium-card podium-silver" data-rank="2">
    <div class="podium-avatar-wrap">
      <img class="podium-avatar" src="[AVATAR_URL]" alt="[NAME]">
      <span class="podium-rank-badge">2</span>
    </div>
    <div class="podium-name">[PLAYERNAME]</div>
    <div class="podium-stats">
      <span class="podium-stat">K: <strong>198</strong></span>
      <span class="podium-stat">D: <strong>31</strong></span>
      <span class="podium-stat">KD: <strong>6.3</strong></span>
    </div>
    <div class="podium-stand podium-stand--2"></div>
  </div>

  <!-- 1. Platz — MITTE (größter Avatar) -->
  <div class="podium-card podium-gold" data-rank="1">
    <div class="podium-crown">👑</div>
    <div class="podium-avatar-wrap">
      <img class="podium-avatar" src="[AVATAR_URL]" alt="[NAME]">
      <span class="podium-rank-badge">1</span>
    </div>
    <div class="podium-name">[PLAYERNAME]</div>
    <div class="podium-stats">
      <span class="podium-stat">K: <strong>247</strong></span>
      <span class="podium-stat">D: <strong>12</strong></span>
      <span class="podium-stat">KD: <strong>20.5</strong></span>
    </div>
    <div class="podium-stand podium-stand--1"></div>
  </div>

  <!-- 3. Platz — RECHTS (kleinster Avatar) -->
  <div class="podium-card podium-bronze" data-rank="3">
    <div class="podium-avatar-wrap">
      <img class="podium-avatar" src="[AVATAR_URL]" alt="[NAME]">
      <span class="podium-rank-badge">3</span>
    </div>
    <div class="podium-name">[PLAYERNAME]</div>
    <div class="podium-stats">
      <span class="podium-stat">K: <strong>156</strong></span>
      <span class="podium-stat">D: <strong>8</strong></span>
      <span class="podium-stat">KD: <strong>19.5</strong></span>
    </div>
    <div class="podium-stand podium-stand--3"></div>
  </div>

</div>
```

---

## 46. Podium CSS — Basis-Layout (theme-unabhängig)

```css
/* =============================================
   PODIUM WRAPPER — Flex-Container
   Reihenfolge im DOM: 2 · 1 · 3
   Visuelle Reihenfolge via order: links · mitte · rechts
   ============================================= */

.podium-wrapper {
  display: flex;
  align-items: flex-end;       /* Alle Cards unten ausrichten → Podium-Effekt */
  justify-content: center;
  gap: 16px;
  padding: 32px 16px 0;
  width: 100%;
  position: relative;
}

/* Reihenfolge: DOM ist 2–1–3, visuell 2–1–3 → passt bereits */
/* Falls DOM-Reihenfolge 1–2–3 ist, diese order-Regeln nutzen: */
.podium-card[data-rank="2"] { order: 1; }
.podium-card[data-rank="1"] { order: 2; }
.podium-card[data-rank="3"] { order: 3; }

/* =============================================
   PODIUM CARD — Basis
   ============================================= */

.podium-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  position: relative;
  flex-shrink: 0;
  transition: transform 0.3s ease;
}

.podium-card:hover {
  transform: translateY(-4px);
}

/* =============================================
   AVATAR GRÖSSEN — gestaffelt
   1. Platz: 120px  (groß, dominant)
   2. Platz:  90px  (mittelgroß)
   3. Platz:  70px  (klein)
   ============================================= */

.podium-avatar-wrap {
  position: relative;
  display: inline-block;
}

.podium-avatar {
  border-radius: 50%;
  object-fit: cover;
  display: block;
  border: 2px solid currentColor;
}

/* Platz 1 — größter Avatar */
.podium-gold .podium-avatar {
  width:  120px;
  height: 120px;
}

/* Platz 2 — mittelgroß */
.podium-silver .podium-avatar {
  width:  90px;
  height: 90px;
}

/* Platz 3 — kleinster Avatar */
.podium-bronze .podium-avatar {
  width:  70px;
  height: 70px;
}

/* =============================================
   RANG-BADGE — Zahl auf dem Avatar
   ============================================= */

.podium-rank-badge {
  position: absolute;
  bottom: -4px;
  right: -4px;
  width: 22px;
  height: 22px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 11px;
  font-weight: 700;
  font-family: 'Share Tech Mono', monospace;
  border: 2px solid var(--podium-bg, #000);
}

.podium-gold   .podium-rank-badge { background: #ffd700; color: #000; }
.podium-silver .podium-rank-badge { background: #c0c0c0; color: #000; }
.podium-bronze .podium-rank-badge { background: #cd7f32; color: #000; }

/* =============================================
   KRONE — nur Platz 1
   ============================================= */

.podium-crown {
  font-size: 22px;
  margin-bottom: 4px;
  animation: crownFloat 2.5s ease-in-out infinite;
}

@keyframes crownFloat {
  0%, 100% { transform: translateY(0);    }
  50%       { transform: translateY(-5px); }
}

/* =============================================
   NAME & STATS
   ============================================= */

.podium-name {
  font-family: 'Share Tech Mono', monospace;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  margin-top: 8px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  max-width: 130px;
  text-align: center;
}

.podium-gold   .podium-name { font-size: 15px; }
.podium-silver .podium-name { font-size: 13px; }
.podium-bronze .podium-name { font-size: 12px; }

.podium-stats {
  display: flex;
  gap: 8px;
  margin-top: 4px;
  flex-wrap: wrap;
  justify-content: center;
}

.podium-stat {
  font-family: 'Share Tech Mono', monospace;
  font-size: 10px;
  opacity: 0.75;
  letter-spacing: 0.05em;
}

.podium-gold   .podium-stat { font-size: 11px; }
.podium-silver .podium-stat { font-size: 10px; }
.podium-bronze .podium-stat { font-size: 9px;  }

/* =============================================
   PODIUM STUFEN — Treppchen-Effekt
   ============================================= */

.podium-stand {
  width: 100%;
  margin-top: 10px;
  border-radius: 3px 3px 0 0;
}

.podium-stand--1 { height: 60px; }   /* Höchste Stufe */
.podium-stand--2 { height: 40px; }   /* Mittlere Stufe */
.podium-stand--3 { height: 24px; }   /* Niedrigste Stufe */
```

---

## 47. Theme-spezifische Podium-Farben

### 🔥 Inferno-Variante

```css
/* Podium an Inferno-Theme anpassen */
.inferno-container .podium-wrapper {
  --podium-bg: var(--inferno-bg-dark);
}

/* Avatar-Ringe */
.inferno-container .podium-gold   .podium-avatar {
  border-color: #fff0a0;
  box-shadow: 0 0 10px #ffcc00, 0 0 25px #ff6600, 0 0 50px #cc2200;
  animation: infernoPodiumPulse 2s ease-in-out infinite;
}
.inferno-container .podium-silver .podium-avatar {
  border-color: #ff8800;
  box-shadow: 0 0 6px #ff8800, 0 0 15px #cc4400;
}
.inferno-container .podium-bronze .podium-avatar {
  border-color: #cc3300;
  box-shadow: 0 0 5px #cc3300, 0 0 12px #882200;
}

@keyframes infernoPodiumPulse {
  0%, 100% { box-shadow: 0 0 10px #ffcc00, 0 0 25px #ff6600, 0 0 50px #cc2200; }
  50%       { box-shadow: 0 0 18px #fff0a0, 0 0 40px #ff8800, 0 0 80px #ff3300; }
}

/* Namen */
.inferno-container .podium-gold   .podium-name {
  color: #ffcc00;
  text-shadow: 0 0 8px #ff6600, 0 0 20px #cc2200;
}
.inferno-container .podium-silver .podium-name { color: #ff8800; }
.inferno-container .podium-bronze .podium-name { color: #cc5500; }

/* Stufen */
.inferno-container .podium-stand--1 {
  background: linear-gradient(to bottom, #cc2200, #3a0a00);
  box-shadow: 0 -4px 12px rgba(255,80,0,0.4);
}
.inferno-container .podium-stand--2 {
  background: linear-gradient(to bottom, #882200, #1a0500);
}
.inferno-container .podium-stand--3 {
  background: linear-gradient(to bottom, #551100, #0f0200);
}
```

### ☢️ Stalker-Variante

```css
.stalker-container .podium-wrapper {
  --podium-bg: var(--rad-bg-dark);
}

.stalker-container .podium-gold .podium-avatar {
  border-color: #ffe600;
  box-shadow: 0 0 10px #ffe600, 0 0 25px #ff7b00, 0 0 50px #7a3800;
  animation: stalkerPodiumPulse 3s ease-in-out infinite;
}
.stalker-container .podium-silver .podium-avatar {
  border-color: #ff7b00;
  box-shadow: 0 0 6px #ff7b00, 0 0 14px #b85500;
}
.stalker-container .podium-bronze .podium-avatar {
  border-color: #b85500;
  box-shadow: 0 0 5px #b85500;
}

@keyframes stalkerPodiumPulse {
  0%, 100% { box-shadow: 0 0 10px #ffe600, 0 0 25px #ff7b00, 0 0 50px #7a3800; }
  50%       { box-shadow: 0 0 18px #ffff44, 0 0 40px #ffaa00, 0 0 80px #cc4400; }
}

.stalker-container .podium-gold   .podium-name { color: #ffe600; text-shadow: 0 0 8px #ff7b00; }
.stalker-container .podium-silver .podium-name { color: #ff7b00; }
.stalker-container .podium-bronze .podium-name { color: #b85500; }

.stalker-container .podium-stand--1 {
  background: linear-gradient(to bottom, #7a3800, #1a0a00);
  box-shadow: 0 -4px 12px rgba(255,123,0,0.3);
}
.stalker-container .podium-stand--2 { background: linear-gradient(to bottom, #4a2000, #0f0600); }
.stalker-container .podium-stand--3 { background: linear-gradient(to bottom, #2a1000, #080300); }
```

### 🖥️ OPS-Variante

```css
.terminal-container .podium-wrapper {
  --podium-bg: var(--crt-bg-dark);
}

.terminal-container .podium-gold .podium-avatar {
  border-color: #00ff41;
  box-shadow: 0 0 8px #00ff41, 0 0 20px #00b32c, 0 0 40px #004410;
  animation: opsPodiumPulse 4s ease-in-out infinite;
}
.terminal-container .podium-silver .podium-avatar {
  border-color: #00b32c;
  box-shadow: 0 0 6px #00b32c, 0 0 14px #006618;
}
.terminal-container .podium-bronze .podium-avatar {
  border-color: #007718;
  box-shadow: 0 0 4px #007718;
}

@keyframes opsPodiumPulse {
  0%, 100% { box-shadow: 0 0 8px #00ff41, 0 0 20px #00b32c, 0 0 40px #004410; }
  50%       { box-shadow: 0 0 14px #00ff41, 0 0 35px #00ff41, 0 0 70px #00b32c; }
}

.terminal-container .podium-gold   .podium-name {
  color: #00ff41;
  text-shadow: 0 0 6px #00ff41, 0 0 14px #00b32c;
}
.terminal-container .podium-silver .podium-name { color: #00b32c; }
.terminal-container .podium-bronze .podium-name { color: #007718; }

/* Stufen: Phosphor-Grün Verlauf */
.terminal-container .podium-stand--1 {
  background: linear-gradient(to bottom, #004410, #000a00);
  box-shadow: 0 -3px 10px rgba(0,255,65,0.2);
}
.terminal-container .podium-stand--2 { background: linear-gradient(to bottom, #002808, #000500); }
.terminal-container .podium-stand--3 { background: linear-gradient(to bottom, #001505, #000300); }
```

---

## 48. Responsive — Mobile Podium

```css
/* Tablet: etwas enger */
@media (max-width: 768px) {
  .podium-gold   .podium-avatar { width: 90px;  height: 90px;  }
  .podium-silver .podium-avatar { width: 70px;  height: 70px;  }
  .podium-bronze .podium-avatar { width: 55px;  height: 55px;  }

  .podium-wrapper { gap: 10px; padding-top: 20px; }
  .podium-name    { max-width: 100px; font-size: 11px !important; }
  .podium-stat    { font-size: 9px !important; }
}

/* Mobile: sehr kompakt, Stats ausblenden */
@media (max-width: 480px) {
  .podium-gold   .podium-avatar { width: 72px;  height: 72px;  }
  .podium-silver .podium-avatar { width: 56px;  height: 56px;  }
  .podium-bronze .podium-avatar { width: 44px;  height: 44px;  }

  .podium-stats   { display: none; }   /* Stats erst in der Tabelle unten */
  .podium-stand--1 { height: 40px; }
  .podium-stand--2 { height: 26px; }
  .podium-stand--3 { height: 16px; }
  .podium-wrapper  { gap: 8px; }
}
```

---

## 49. Einbindungs-Anleitung

```
SCHRITT 1 — PHP-Template anpassen:
Ersetze die bestehende Top-3-Schleife durch das Podium HTML aus Section 45.
Reihenfolge im DOM: Platz 2 → Platz 1 → Platz 3
(flexbox order-Regeln in Section 46 regeln die visuelle Anordnung)

SCHRITT 2 — CSS einbinden:
Die Basis-CSS (Section 46) in jede Theme-CSS-Datei integrieren.
Die theme-spezifischen Farben (Section 47) danach, damit sie die Basis überschreiben.

SCHRITT 3 — Avatar-URL:
Falls kein Steam-Avatar verfügbar: Fallback-Bild setzen:
  <img class="podium-avatar" src="[AVATAR]" onerror="this.src='/assets/default-avatar.png'" alt="[NAME]">

SCHRITT 4 — Klassen-Checkliste:
  ✓ .podium-wrapper    → auf dem äußeren Container
  ✓ .podium-card       → auf jeder der 3 Cards
  ✓ .podium-gold       → Platz 1
  ✓ .podium-silver     → Platz 2
  ✓ .podium-bronze     → Platz 3
  ✓ data-rank="1/2/3"  → für CSS order-Steuerung
  ✓ .podium-avatar     → auf dem <img>
  ✓ .podium-stand--1/2/3 → auf dem Sockel-Div
```

---

*// END OF TRANSMISSION — DEADMANS-ECHO OPS TERMINAL //*
*// ☢ ZONE SIGNAL LOST — STATIC — RAD LEVEL: CRITICAL //*
*// 🔥 INFERNO SECTOR OFFLINE — HEAT SIGNATURE: MAXIMUM //*
*// ☣ OUTBREAK CONTAINMENT FAILED — SECTOR: COMPROMISED //*
*// 🏆 PODIUM ONLINE — TOP 3 STANDING BY //*

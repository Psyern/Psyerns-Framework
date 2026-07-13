# DME_Api Web Service (Psyerns_Framework)

Node.js backend for the DME_Api (Psyerns_Framework) DayZ integration.

> Modernized fork of **DayZ‑UniversalApi 1.3.2** by **daemonforge** (AGPL‑3.0).
> Original work Copyright (c) daemonforge — https://github.com/daemonforge/DayZ-UniveralApi
> Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo.
> Licensed under the GNU Affero General Public License v3.0 (see repo‑root `LICENSE`).
> SPDX-License-Identifier: AGPL-3.0-only

The HTTP wire contract (routes, `auth-key` header transport, Mongo collections,
config keys, JSON field names, version string `1.3.2`) is **unchanged** from 1.3.2.

## Requirements

- **Node.js >= 22** (`engines` in `package.json`). Uses the native global `fetch`.
- MongoDB reachable at `DBServer`.
- **You must run `npm install`** to (re)generate `node_modules` and a fresh
  `package-lock.json` — the dependency set changed substantially (discord.js 14,
  mongodb 6, jsonwebtoken 9, express‑rate‑limit 7, gamedig 5, native fetch, and
  removal of greenlock / body‑parser / node‑fetch / websocket / the crypto & https
  npm stubs). No lockfile is committed by this change.

```bash
npm install
npm start        # node app.js
```

## TLS / deployment — terminate TLS at a reverse proxy

Built‑in TLS has been removed from Node (no more `greenlock`/ACME, no embedded
self‑signed key). Run the service as **plain HTTP behind a reverse proxy**
(nginx / Caddy / Cloudflare) that terminates TLS:

- Leave `Certificate` / `CertificateKey` empty ⇒ the service listens on plain HTTP.
- Set `Certificate` + `CertificateKey` to real cert files on disk ⇒ direct HTTPS
  (for setups without a proxy).
- The `LetsEncypt` config key is kept for compatibility but is **ignored**.

**Critical:** the game client sends its auth token as the request **Content‑Type**
header. Configure your proxy so it does **not** rewrite/normalize `Content-Type`,
or all authentication breaks. Example nginx location:

```nginx
location / {
    proxy_pass http://127.0.0.1:8443;
    proxy_set_header Host $host;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_pass_request_headers on;   # keep the Content-Type auth token intact
}
```

Set `TrustProxy` in `config.json` to match your proxy topology so `req.ip` (used
for rate limiting and the IP whitelist) reflects the real client IP:
`false` (default, no proxy) · `true` (trust all — only on a private network) ·
a hop count (e.g. `1`) · or a subnet/CIDR string.

## New / changed config keys (additive — wire keys unchanged)

| Key | Default | Purpose |
|---|---|---|
| `TrustProxy` | `false` | Express `trust proxy` value. Prevents rate‑limit / whitelist bypass via spoofed `X-Forwarded-For` / `CF-Connecting-IP`. |
| `ForwardAllowList` | `[]` | SSRF guard for `/Forward`. If non‑empty, only these hosts (exact or suffix) may be fetched. If empty, internal/loopback/link‑local/metadata targets are blocked. |
| `Toxicity.Enabled` | `false` | Feature flag for `/Toxicity`. When off, the route returns `501 {Status:"Disabled"}` and TensorFlow is never loaded. TF deps are `optionalDependencies`; install with `npm install --include=optional` if enabled. |
| `JwtSecret` | `""` | Optional: dedicated HMAC signing secret for player JWTs, decoupled from the `ServerAuth` bearer. Empty ⇒ falls back to `ServerAuth` (wire/rolling‑update compatible). |
| `CheckForNewVersion` | `false` | Now defaults off (the upstream GitHub version poll targets daemonforge's repo). |

`ServerAuth` is auto‑generated with a CSPRNG (`crypto.randomBytes`) on first run.
Never commit a real `config.json`. The old embedded TLS key in `defaultkeys.json`
was removed (that file is now an empty placeholder).

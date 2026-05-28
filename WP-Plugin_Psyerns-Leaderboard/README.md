<p align="center">
  <img src="https://img.shields.io/badge/WordPress-5.8+-21759B?style=for-the-badge&logo=wordpress&logoColor=white" alt="WordPress 5.8+">
  <img src="https://img.shields.io/badge/PHP-7.4+-777BB4?style=for-the-badge&logo=php&logoColor=white" alt="PHP 7.4+">
  <img src="https://img.shields.io/badge/DayZ-Integration-0074D9?style=for-the-badge&logo=steam&logoColor=white" alt="DayZ Integration">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" alt="License MIT"></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Leaderboards-PvP_%7C_PvE-E74C3C?style=flat-square" alt="Leaderboards">
  <img src="https://img.shields.io/badge/Themes-9_Styles-9B59B6?style=flat-square" alt="9 Themes">
  <img src="https://img.shields.io/badge/Whitelist-Management-2ECC40?style=flat-square" alt="Whitelist">
  <img src="https://img.shields.io/badge/Server_Status-Live-F39C12?style=flat-square" alt="Server Status">
  <img src="https://img.shields.io/badge/Kill_Feed-Tracking-1ABC9C?style=flat-square" alt="Kill Feed">
  <img src="https://img.shields.io/badge/Steam-Avatar_API-1B2838?style=flat-square&logo=steam&logoColor=white" alt="Steam Avatars">
</p>

<p align="center">
  <b>A WordPress plugin that brings your DayZ server to life on your website</b><br>
  Leaderboards, server status, whitelist management, and kill feeds — all powered by a REST API<br>
  that connects directly to your DayZ server via the Psyerns Framework mod.
</p>

<p align="center">
  <a href="https://deadmans-echo.de">
    <img src="https://img.shields.io/badge/Community-Deadmans_Echo-F0C040?style=for-the-badge" alt="Deadmans Echo">
  </a>
</p>

---

## Features

<table>
<tr>
<td width="33%" valign="top">

### Leaderboards
- **PvP & PvE** modes with separate tracking
- **9 visual themes** (Military, Cyberpunk, Stalker, etc.)
- **Weekly top 3** showcase with avatars
- Real-time search & filtering
- Configurable columns per mode
- Pagination (top 10, 20, 50)
- Steam avatar display (cached 24h)
- Faction tracking (East/West)

</td>
<td width="33%" valign="top">

### Server Integration
- **Live server status** (players, map, uptime)
- **Whitelist management** via admin panel
- **Kill feed** tracking with weapons & distance
- Player lookup by Steam ID
- Online player list
- Auto-generated API key
- 2-minute status caching

</td>
<td width="33%" valign="top">

### Admin Panel
- **Settings dashboard** with tabbed interface
- API key configuration
- Column visibility per game mode
- Interactive theme selector with previews
- Whitelist CRUD management
- Shortcode reference docs
- Connection test endpoint

</td>
</tr>
</table>

---

## Themes

9 unique visual styles, each with custom colors, fonts, and effects:

| Theme | Style | Font |
|-------|-------|------|
| **Military** | CRT scanlines, phosphor green, classified briefing | Share Tech Mono, Oswald |
| **Ash** | Weathered paper, rust, wanted poster | Playfair Display, DM Mono |
| **Ops** | Phosphor green CRT, vignette, flicker | Orbitron, Share Tech Mono |
| **Outbreak** | Hazard amber, warning stripes, biohazard | Rajdhani, Exo 2 |
| **Cyberpunk** | Matrix green, neon magenta, glitch effects | Orbitron, JetBrains Mono |
| **Stalker** | Radiation orange, chromatic aberration | Courier Prime |
| **Inferno** | Hellfire, ember glow, lava gradients | Teko |
| **Frostbite** | Ice & snow, falling snowflakes, frost borders | Nunito Sans |
| **Bubblegum** | Sweet & playful, candy glow, pink shimmer | Quicksand |

Set the global theme in **Settings → Themes**, or override per shortcode with the `theme` attribute.

---

## Shortcodes

### Leaderboard

```
[psyern_leaderboard mode="pvp" limit="20" theme="military" show_avatar="1" show_playtime="1"]
```

| Attribute | Default | Description |
|-----------|---------|-------------|
| `mode` | `pvp` | Game mode: `pvp` or `pve` |
| `limit` | `10` | Number of players to display |
| `theme` | (global) | Override global theme for this instance |
| `show_avatar` | `1` | Show Steam profile avatars |
| `show_playtime` | `1` | Show playtime column |

### Top 3 Widgets

```
[psyern_top3 mode="pvp" theme="cyberpunk"]
[pf_top3_monthly theme="military"]
[pf_top3_deadliest theme="ops"]
[pf_top3_bosskills theme="stalker"]
```

### Server Status

```
[pf_server_status theme="military"]
```

### Player Card

```
[pf_player_card steam_id="76561198000000000" theme="outbreak"]
```

### Faction War Banner

Compact EAST vs WEST banner with live points, percentages, split bar and an optional link button.

```
[pf_faction_war theme="military" url="https://your-site.com/leaderboard"]
```

| Attribute | Default | Description |
|-----------|---------|-------------|
| `theme` | (global) | Override global theme for this instance |
| `url` | (empty) | Link target for the "Full Leaderboard" button; button is hidden when empty |

### Stat Cards

Three headline counters — Online Now, Tracked Players and Total Kills (sum of all PvP + PvE/AI kills) — loaded live from `/wp-json/psyern/v1/public/stats`.

```
[pf_stats theme="military"]
```

| Attribute | Default | Description |
|-----------|---------|-------------|
| `theme` | (global) | Override global theme for this instance |

---

## Installation

1. Upload the `WP-Plugin_Psyerns-Leaderboard` folder to `/wp-content/plugins/`
2. Activate the plugin in **Plugins → Installed Plugins**
3. Navigate to **Psyerns Framework → Settings**
4. Configure your **API Key** (shared with your DayZ server mod)
5. *(Optional)* Add your **Steam API Key** for avatar display
6. Choose your **theme** and configure **column visibility**
7. Add shortcodes to any page or post

---

## REST API

### Private Endpoints (Require API Key)

All private endpoints require `?api_key=YOUR_KEY` as query parameter.

| Method | Endpoint | Description |
|--------|----------|-------------|
| `POST` | `/psyern/v1/upload` | Upload leaderboard data from DayZ server |
| `POST` | `/psyern/v1/server/status` | Push server status update |
| `GET` | `/psyern/v1/whitelist/check` | Check if Steam ID is whitelisted |
| `POST` | `/psyern/v1/whitelist/add` | Add player to whitelist |
| `POST` | `/psyern/v1/whitelist/remove` | Remove player from whitelist |
| `GET` | `/psyern/v1/players/lookup` | Look up player by Steam ID |
| `GET` | `/psyern/v1/players/online` | Get all online players |
| `GET` | `/psyern/v1/ping` | Connection test |

### Public Endpoints (No Auth)

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/psyern/v1/public/leaderboard` | Get leaderboard data |
| `GET` | `/psyern/v1/public/status` | Get server status |
| `GET` | `/psyern/v1/public/top3` | Get top 3 players |
| `GET` | `/psyern/v1/public/avatar` | Get Steam avatar URL |
| `GET` | `/psyern/v1/leaderboard` | v2 leaderboard with pagination |
| `GET` | `/psyern/v1/leaderboard/top3` | v2 weekly top 3 |

---

## Data Flow

```
DayZ Server (Psyerns Framework Mod)
  │
  ├── POST /psyern/v1/upload          ──→ Leaderboard data saved to DB
  ├── POST /psyern/v1/server/status   ──→ Server status cached (2 min)
  └── GET  /psyern/v1/whitelist/check ──→ Whitelist lookup response
  
WordPress Website
  │
  ├── Shortcodes on pages  ──→ Fetch from DB / REST API
  ├── Admin panel           ──→ Settings, whitelist CRUD
  └── Public REST endpoints ──→ JSON API for external tools
```

---

## Leaderboard Columns

Configurable per game mode (PvP/PvE) in **Settings → Leaderboard**:

| Column | Key | Description |
|--------|-----|-------------|
| Rank | `rank` | Position number (always visible) |
| Avatar | `avatar` | Steam profile image |
| Name | `name` | Player display name (always visible) |
| Kills | `kills` | Total kill count |
| Deaths | `deaths` | Total death count |
| K/D | `kd` | Kill-to-death ratio |
| Faction | `faction` | War faction (East/West) with level bar |
| Boss Kills | `boss` | Boss kill count |
| Reputation | `reputation` | Hardline reputation score |
| Playtime | `playtime` | Total time played (hours:minutes) |

---

## Database Tables

The plugin creates the following tables on activation:

| Table | Purpose |
|-------|---------|
| `wp_pf_leaderboard` | Core player stats (kills, deaths, K/D, playtime, faction, reputation) |
| `wp_pf_whitelist` | Whitelisted Steam IDs with timestamps |
| `wp_pf_killfeed` | Kill events with weapon, distance, and coordinates |
| `wp_psyern_leaderboard` | v2 weekly leaderboard with mode separation |
| `wp_psyern_weekly_winners` | Archived weekly top players |

All tables and options are fully cleaned up on plugin uninstallation.

---

## Settings

### API Tab
- **API Key** — Shared secret between DayZ server and WordPress
- **Steam API Key** — Optional, enables Steam avatar resolution

### Leaderboard Tab
- **PvP/PvE toggles** — Enable or disable game modes
- **Column visibility** — Per-mode column checkboxes
- **Top 3 display** — Show/hide weekly winners showcase
- **Avatar display** — Toggle Steam avatars globally

### Themes Tab
- **Theme selector** — Interactive previews of all 9 themes
- **Per-shortcode override** — Use `theme="name"` attribute

---

## Caching

| Cache Key | TTL | Content |
|-----------|-----|---------|
| `pf_server_status` | 2 minutes | Server status data |
| `pf_leaderboard_meta` | 10 minutes | Leaderboard metadata |
| `pf_avatar_{steam_id}` | 24 hours | Individual Steam avatars |

---

## Requirements

| Requirement | Version |
|-------------|---------|
| WordPress | 5.8+ |
| PHP | 7.4+ |
| DayZ Server | Psyerns Framework mod installed |
| Steam API Key | Optional (for avatars) |

---

## Hooks & Filters

### Filters

| Filter | Description |
|--------|-------------|
| `psyerns-framework/server_status_data` | Modify server status before save |
| `psyerns-framework/leaderboard_players` | Filter players before API response |
| `psyerns-framework/leaderboard_title` | Change leaderboard page title |
| `psyerns-framework/top3_title` | Change top 3 widget title |
| `psyerns-framework/top3_monthly_title` | Change monthly top 3 title |

---

## Credits

| | |
|---|---|
| **Author** | Psyern |
| **Community** | [Deadmans Echo](https://deadmans-echo.de) |
| **License** | MIT |
| **Version** | 1.0.0 |

---

<p align="center">
  <sub>Built for the DayZ community</sub><br>
  <sub>Made by <b>Psyern</b> for <b>Deadmans Echo</b></sub>
</p>

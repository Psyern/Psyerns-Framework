# Discord Leaderboard mit Echtzeit-Updates – Developer Guide

> Ziel: Eine REST API (z. B. Spieleserver, eigene Backend-API) pollt regelmäßig Daten
> und stellt diese als Live-Leaderboard in einem Discord-Channel dar.

---

## 1. Welchen Ansatz brauchst du?

| Ansatz | Wann sinnvoll | Komplexität |
|---|---|---|
| **Incoming Webhook** | Nur Nachrichten posten, kein Bot nötig | ⭐ Einfach |
| **Bot (discord.py / discord.js)** | Reagiert auf Events, edit messages, Slash Commands | ⭐⭐ Mittel |
| **Bot + Gateway WebSocket** | Echte Real-Time Events vom Discord-Server empfangen | ⭐⭐⭐ Komplex |

**Für ein Leaderboard mit Echtzeit-Update empfehle ich:**
→ **Bot-Token** + periodisches **EDIT einer bestehenden Nachricht** via REST API.

Ein Webhook allein reicht für einfaches Posting. Aber um dieselbe Nachricht zu editieren
(statt immer neue zu posten), brauchst du einen Bot-Token.

---

## 2. Setup im Discord Developer Portal

### Schritt 1: App erstellen
1. Gehe zu https://discord.com/developers/applications
2. Klicke **"New Application"** → Name eingeben
3. Im Menü links: **"Bot"** → **"Add Bot"**
4. **Token kopieren** (einmalig sichtbar!) → sicher aufbewahren

### Schritt 2: Bot-Berechtigungen setzen
Unter **"Bot"** → "Privileged Gateway Intents":
- `MESSAGE CONTENT INTENT` aktivieren (falls Nachrichteninhalte gelesen werden sollen)

Unter **"OAuth2" → "URL Generator"**:
- Scope: `bot`
- Bot Permissions: `Send Messages`, `Read Messages/View Channels`, `Manage Messages` (optional)
- Generierte URL öffnen → Bot zum Server einladen

### Schritt 3: Channel-ID ermitteln
In Discord: Einstellungen → Erweitert → **Entwicklermodus aktivieren**
Dann Rechtsklick auf den Channel → **"ID kopieren"**

---

## 3. Wie das Leaderboard-System funktioniert

```
Deine REST API
     │
     │  (GET /leaderboard alle X Sekunden)
     ▼
Dein Python/Node-Script (poller)
     │
     │  PATCH /webhooks/{id}/{token}/messages/{message_id}
     │  oder
     │  PATCH /channels/{id}/messages/{message_id}  (Bot-Token)
     ▼
Discord Channel – dieselbe Nachricht wird editiert (kein Spam)
```

---

## 4. Methode A: Incoming Webhook (einfachste Variante)

### Webhook erstellen (einmalig, im Discord Client)
Rechtsklick auf Channel → **"Integrationen"** → **"Webhook erstellen"** → URL kopieren

Format der URL:
```
https://discord.com/api/webhooks/{webhook.id}/{webhook.token}
```

### Erste Nachricht senden + Message-ID speichern

```bash
# Erste Nachricht senden, message_id merken (wait=true wichtig!)
curl -X POST "https://discord.com/api/webhooks/WEBHOOK_ID/WEBHOOK_TOKEN?wait=true" \
  -H "Content-Type: application/json" \
  -d '{"content": "Leaderboard wird geladen..."}'
# → Antwort enthält "id" (= message_id)
```

### Nachricht editieren (Echtzeit-Update)

```bash
curl -X PATCH \
  "https://discord.com/api/webhooks/WEBHOOK_ID/WEBHOOK_TOKEN/messages/MESSAGE_ID" \
  -H "Content-Type: application/json" \
  -d '{
    "embeds": [{
      "title": "🏆 Leaderboard",
      "description": "1. PlayerA — 1500 Punkte\n2. PlayerB — 1200 Punkte\n3. PlayerC — 900 Punkte",
      "color": 16766720,
      "footer": {"text": "Zuletzt aktualisiert: 2026-03-26 14:00 UTC"}
    }]
  }'
```

---

## 5. Methode B: Bot-Token (empfohlen für mehr Kontrolle)

### Python-Beispiel mit `discord.py`

```bash
pip install discord.py aiohttp
```

```python
import discord
import asyncio
import aiohttp
import json
from datetime import datetime

DISCORD_TOKEN = "DEIN_BOT_TOKEN"
CHANNEL_ID = 123456789012345678      # Deine Channel-ID
LEADERBOARD_API = "https://deine-api.example.com/leaderboard"
UPDATE_INTERVAL = 30  # Sekunden

intents = discord.Intents.default()
client = discord.Client(intents=intents)

leaderboard_message = None  # Wird beim Start gesetzt

async def fetch_leaderboard():
    """Holt Daten von deiner REST API"""
    async with aiohttp.ClientSession() as session:
        async with session.get(LEADERBOARD_API) as resp:
            return await resp.json()

def format_leaderboard(data: list) -> discord.Embed:
    """Erstellt ein Discord Embed aus den API-Daten"""
    medals = ["🥇", "🥈", "🥉"]
    lines = []
    for i, entry in enumerate(data[:10]):
        medal = medals[i] if i < 3 else f"`#{i+1}`"
        lines.append(f"{medal} **{entry['name']}** — {entry['score']:,} Punkte")

    embed = discord.Embed(
        title="🏆 Live Leaderboard",
        description="\n".join(lines),
        color=0xFFD700
    )
    embed.set_footer(text=f"Aktualisiert: {datetime.utcnow().strftime('%Y-%m-%d %H:%M')} UTC")
    return embed

async def leaderboard_loop():
    """Endlosschleife: API pollen → Nachricht editieren"""
    global leaderboard_message

    await client.wait_until_ready()
    channel = client.get_channel(CHANNEL_ID)

    # Erste Nachricht erstellen
    data = await fetch_leaderboard()
    leaderboard_message = await channel.send(embed=format_leaderboard(data))

    while not client.is_closed():
        await asyncio.sleep(UPDATE_INTERVAL)
        try:
            data = await fetch_leaderboard()
            await leaderboard_message.edit(embed=format_leaderboard(data))
            print(f"[{datetime.utcnow()}] Leaderboard aktualisiert.")
        except Exception as e:
            print(f"Fehler beim Update: {e}")

@client.event
async def on_ready():
    print(f"Bot eingeloggt als {client.user}")
    client.loop.create_task(leaderboard_loop())

client.run(DISCORD_TOKEN)
```

---

## 6. Node.js / JavaScript Variante (ohne Framework)

```bash
npm install node-fetch
```

```js
const fetch = (...args) => import('node-fetch').then(({default: fetch}) => fetch(...args));

const WEBHOOK_ID    = "DEIN_WEBHOOK_ID";
const WEBHOOK_TOKEN = "DEIN_WEBHOOK_TOKEN";
const API_URL       = "https://deine-api.example.com/leaderboard";
const INTERVAL_MS   = 30_000; // 30 Sekunden

let messageId = null;

async function fetchLeaderboard() {
  const res = await fetch(API_URL);
  return res.json();
}

function buildEmbed(data) {
  const lines = data.slice(0, 10).map((e, i) =>
    `**#${i + 1}** ${e.name} — ${e.score.toLocaleString()} Punkte`
  );
  return {
    title: "🏆 Live Leaderboard",
    description: lines.join("\n"),
    color: 0xFFD700,
    footer: { text: `Aktualisiert: ${new Date().toISOString()}` }
  };
}

async function postOrEdit(embed) {
  if (!messageId) {
    // Erste Nachricht senden
    const res = await fetch(
      `https://discord.com/api/webhooks/${WEBHOOK_ID}/${WEBHOOK_TOKEN}?wait=true`,
      {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ embeds: [embed] })
      }
    );
    const msg = await res.json();
    messageId = msg.id;
    console.log("Erste Nachricht erstellt, ID:", messageId);
  } else {
    // Nachricht editieren
    await fetch(
      `https://discord.com/api/webhooks/${WEBHOOK_ID}/${WEBHOOK_TOKEN}/messages/${messageId}`,
      {
        method: "PATCH",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ embeds: [embed] })
      }
    );
    console.log("Leaderboard aktualisiert:", new Date().toISOString());
  }
}

async function run() {
  const data = await fetchLeaderboard();
  await postOrEdit(buildEmbed(data));
  setInterval(async () => {
    const data = await fetchLeaderboard();
    await postOrEdit(buildEmbed(data));
  }, INTERVAL_MS);
}

run().catch(console.error);
```

---

## 7. Rate Limits – das Wichtigste

Discord begrenzt API-Anfragen. Für das Editieren einer Nachricht gilt:

| Limit | Wert |
|---|---|
| Edit Message (global) | 5 Anfragen / 5 Sekunden pro Bot |
| Webhook Execute | 30 Anfragen / 60 Sekunden |
| **Nachrichten editieren** | max. **5× pro 5s** pro Route |

**Faustregel für Leaderboards:**
- Update-Interval **≥ 10 Sekunden** → sicher
- Update-Interval **≥ 30 Sekunden** → empfohlen für Webhooks
- Bei Rate-Limit-Fehler (HTTP 429): `retry_after` aus der Antwort auslesen

```json
// HTTP 429 Antwort von Discord
{
  "message": "You are being rate limited.",
  "retry_after": 2.5,
  "global": false
}
```

---

## 8. Embed-Struktur Referenz

```json
{
  "embeds": [{
    "title": "🏆 Leaderboard",
    "description": "Text hier (max. 4096 Zeichen)",
    "color": 16766720,
    "fields": [
      { "name": "Spalte 1", "value": "Inhalt", "inline": true }
    ],
    "thumbnail": { "url": "https://..." },
    "footer": { "text": "Footer Text", "icon_url": "https://..." },
    "timestamp": "2026-03-26T14:00:00Z"
  }]
}
```

**Farbe als Dezimal:** `0xFFD700` (Gold) = `16766720`

---

## 9. Umgebungsvariablen – Empfehlung

Niemals Tokens hardcoden! Stattdessen `.env` nutzen:

```bash
# .env
DISCORD_BOT_TOKEN=dein_token_hier
DISCORD_CHANNEL_ID=123456789012345678
DISCORD_WEBHOOK_URL=https://discord.com/api/webhooks/ID/TOKEN
LEADERBOARD_API_URL=https://deine-api.example.com/leaderboard
UPDATE_INTERVAL_SECONDS=30
```

```python
# Python: python-dotenv
pip install python-dotenv

from dotenv import load_dotenv
import os
load_dotenv()
TOKEN = os.getenv("DISCORD_BOT_TOKEN")
```

```js
// Node.js
npm install dotenv
require('dotenv').config();
const token = process.env.DISCORD_BOT_TOKEN;
```

---

## 10. Schnellstart-Checkliste

- [ ] App im Developer Portal erstellt: https://discord.com/developers/applications
- [ ] Bot erstellt + Token gespeichert
- [ ] Bot in Server eingeladen (OAuth2 URL Generator → `bot` scope)
- [ ] Channel-ID aus Entwicklermodus kopiert
- [ ] Webhook erstellt (optional, falls ohne Bot)
- [ ] `.env` Datei angelegt mit allen Tokens
- [ ] Script startet → erste Nachricht wird gepostet, `message_id` gespeichert
- [ ] Loop läuft → PATCH auf dieselbe Message-ID alle X Sekunden

---

## 11. Nützliche Links

| Ressource | URL |
|---|---|
| Developer Portal | https://discord.com/developers/applications |
| Discord Docs (Bots) | https://docs.discord.com/developers/platform/bots |
| Webhook Resource API | https://docs.discord.com/developers/resources/webhook |
| Rate Limits | https://docs.discord.com/developers/topics/rate-limits |
| Embed Builder (Tool) | https://discohook.org |
| discord.py Docs | https://discordpy.readthedocs.io |
| discord.js Docs | https://discord.js.org |

# Prompt: Neue Webhooks für Psyerns Framework

## Kontext

Das Psyerns Framework ist ein DayZ HTTP/Webhook-Framework. Es hat bereits:
- Server Start Notification (Discord)
- Player Connect/Disconnect (Discord)
- Player Kill (KillFeed + Discord)

**Fehlend und jetzt zu implementieren:**
1. Server Stop Notification
2. Server Heartbeat (für Crash-Erkennung)
3. Server gesperrt/entsperrt
4. Mod Update Erkennung
5. Spieler Quest-Erfolg (via Expansion Quests `#ifdef`)

**Später (nicht in diesem Auftrag):**
- Spieler gekickt / gebannt (braucht RCON-Integration)

**Arbeitsverzeichnis:** `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\`

**Bestehender Code lesen:**
- `C:\Users\Administrator\Desktop\Psyerns_Framework\MISC\pf_API.md` — API Dokumentation
- `C:\Users\Administrator\Desktop\Psyerns_Framework\.claude\rules\coding-rules.md` — Enforce Script Regeln (PFLICHTLEKTÜRE)
- `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\scripts\` — Bestehender Framework-Code

---

## Enforce Script Regeln (Kritisch)

Diese Regeln MÜSSEN eingehalten werden, sonst kompiliert der Code nicht:

- **Kein Zeilenumbruch in String-Concatenation** — alles auf einer Zeile
- **Keine mehrzeiligen Method-Chains** — `PF_JsonBuilder` Aufrufe einzeln: `b.Add("k","v");`
- **`ref` nicht als Methodenparameter** — nur als Member-Deklaration
- **`PlayerBase`, `EntityAI`, `ItemBase`** nur in `4_World`, nicht in `3_Game`
- **String-Indexer `str[i]` nicht mit `.ToString()`** — nutze `str.Substring(idx, 1)`
- **Void-Methoden nicht in `if()`-Bedingungen**
- **`JsonSerializer.ReadFromString`** hat 3 Parameter, nicht 4

---

## Agent-Orchestrierung (6 Agents parallel)

### Agent 1: Expansion Quests Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Analysiere das Expansion Quests System
**Verzeichnis:** `C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion\Quests`
**Suche nach:**
- Wie werden Quests abgeschlossen? Welche Events/Callbacks gibt es?
- Gibt es ein `OnQuestCompleted`, `OnObjectiveCompleted` oder ähnliches Event?
- Wie ist die Quest-Completion Klasse strukturiert?
- Welche Daten sind bei Quest-Completion verfügbar? (Spieler, Quest-Name, Reward etc.)
- Gibt es einen Hook-Punkt wo man sich reinhängen kann? (modded class, Event-System, CF Module)
- Prüfe `ExpansionQuestModule`, `ExpansionQuest`, `ExpansionQuestObjective` Klassen
**Output:** Vollständige Analyse mit Dateinamen, Klassennamen, Methodensignaturen und empfohlenem Hook-Punkt

### Agent 2: DME Teleport Overhaul Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Analysiere das DME Teleport Overhaul Mod
**Verzeichnis:** `C:\Users\Administrator\Desktop\DME_Teleport_Overhaul`
**Suche nach:**
- Wird Psyerns Framework bereits genutzt (`#ifdef PSYERNS_FRAMEWORK`)?
- Gibt es Events die als Webhook gesendet werden könnten? (Teleport-Events, Zone-Events)
- Wie ist die Mod-Struktur (config.cpp, Scripts)?
- Potenzielle Integrationspunkte mit dem Webhook-System
**Output:** Strukturanalyse und Integrationsmöglichkeiten

### Agent 3: Community Framework Analyse
**Typ:** Explore (read-only)
**Aufgabe:** Analysiere relevante CF-Systeme für die neuen Webhooks
**Verzeichnis:** `C:\Users\Administrator\Desktop\Mod Repositories\DayZ-CommunityFramework-production`
**Suche nach:**
- Wie funktioniert das CF Module-System? (`CF_ModuleGame`, `CF_ModuleWorld`)
- Gibt es Server-Lifecycle Events? (OnServerShutdown, OnMissionFinish etc.)
- Wie implementiert CF RPC/Networking?
- Gibt es ein Event-Bus System das für Webhooks genutzt werden kann?
- Prüfe ob CF `GetGame().IsServerShuttingDown()` oder ähnliches bietet
**Output:** Relevante APIs und Patterns für Server-Lifecycle und Event-Hooks

### Agent 4: Webhook Design
**Typ:** Plan (read-only)
**Aufgabe:** Designe die neuen Webhooks basierend auf den Ergebnissen von Agent 1-3
**Kontext:** Lies zuerst `pf_API.md` und die bestehenden Webhook-Implementierungen:
- `scripts/5_Mission/Psyerns_Framework/PF_MissionInit.c` (Server Start Webhook)
- `scripts/3_Game/Psyerns_Framework/REST/Discord/PF_DiscordIntegration.c` (Event Webhooks)
- `scripts/3_Game/Psyerns_Framework/Web/Payload/PF_DiscordPayload.c` (Embed Builder)
- `scripts/3_Game/Psyerns_Framework/Web/Config/PF_WebConfig.c` (Config)

**Designe folgende Webhooks:**

#### 1. Server Stop Notification
- Hook: `MissionServer.OnMissionFinish()` — vor `super.OnMissionFinish()`
- Discord Embed: Rot, "Server Offline", Servername, Uptime seit Start
- Config-Feld: `EnableServerStopNotification` (bool)

#### 2. Server Heartbeat
- Periodischer POST an WordPress/Webhook mit Timestamp
- WordPress-Seite erkennt Ausfall wenn kein Heartbeat kommt (Timeout)
- Config-Felder: `EnableHeartbeat` (bool), `HeartbeatIntervalSeconds` (int, default 60)
- Endpoint: `POST /server/heartbeat` mit `{"serverName":"...","timestamp":"...","playerCount":N}`
- WordPress Plugin: Speichert als Transient mit TTL = 2x Intervall

#### 3. Server Lock/Unlock
- Prüfe ob DayZ ein Script-Event für Lock/Unlock hat
- Falls nicht: Polling-basiert über `GetGame()` API (falls verfügbar)
- Falls gar nicht möglich: Überspringen und dokumentieren warum
- Discord Embed: Gelb für Lock, Grün für Unlock

#### 4. Mod Update Erkennung
- Beim Server-Start: Mod-Liste mit Versionen erfassen
- Vergleich mit gespeicherter Liste aus letztem Start
- Bei Änderung: Discord Webhook mit geänderten Mods
- Config-Feld: `EnableModUpdateNotification` (bool)
- Nutze `CfgMods` oder `GetGame().GetModsList()` falls verfügbar

#### 5. Quest Completion (Expansion)
- `#ifdef EXPANSIONMODQUESTS` Guard
- Hook in das Expansion Quest-Completion Event (basierend auf Agent 1 Analyse)
- Discord Embed: Lila/Gold, "Quest Completed", Spielername, Quest-Name
- Config-Feld: `EnableQuestNotifications` (bool)

**Output:** Für jeden Webhook: Embed-Design (Farbe, Titel, Felder), Config-Felder, Hook-Punkt, Datei die erstellt/modifiziert wird

### Agent 5: Webhook Implementierung
**Typ:** General (schreibend)
**Aufgabe:** Implementiere alle Webhooks basierend auf dem Design von Agent 4
**WICHTIG:** Warte auf die Ergebnisse von Agent 1-4 bevor du Code schreibst!

**Zu erstellen/modifizieren:**

Neue Dateien:
- `scripts/3_Game/Psyerns_Framework/Web/Notifications/PF_ServerNotifications.c` — Zentrale Klasse für alle Server-Webhooks (Stop, Heartbeat, Lock/Unlock, Mod Updates)
- `scripts/4_World/Psyerns_Framework/REST/Quests/PF_QuestWebhook.c` — Quest-Completion Webhook (in 4_World wegen Expansion-Abhängigkeiten, in `#ifdef EXPANSIONMODQUESTS`)

Zu modifizierende Dateien:
- `scripts/3_Game/Psyerns_Framework/Web/Config/PF_WebConfig.c` — Neue Config-Felder
- `scripts/5_Mission/Psyerns_Framework/PF_MissionInit.c` — Server Stop in OnMissionFinish, Heartbeat Timer in OnUpdate
- `data/PsyernsFrameworkConfig.json` — Neue Default-Werte

**Regeln:**
- Alle Enforce Script Pitfalls beachten (siehe oben)
- Bestehende `PF_DiscordWebhook` und `PF_DiscordPayload` Klassen wiederverwenden
- Discord Endpoint Config aus `PF_WebConfig.GetInstance().GetEndpoint("Discord")` lesen
- Webhook ID/Token mit `ParseWebhookApiKey()` aus `PF_MissionInit.c` parsen (bereits vorhanden)
- Alle neuen Features hinter Config-Toggles (default: false)
- Debug-Logging mit `PF_Logger.Debug()` für alle neuen Funktionen

### Agent 6: Dokumentation
**Typ:** General (schreibend)
**Aufgabe:** Aktualisiere die Dokumentation nach Abschluss aller anderen Agents

**Zu aktualisieren:**
- `Psyerns_Framework/README.md` — Neue Webhooks in Features, Config-Tabelle, Setup-Anleitungen
- `MISC/pf_API.md` — Neue Endpoints (Heartbeat)
- `data/PsyernsFrameworkConfig.example.md` — Neue Config-Felder dokumentieren

**Neue Feature-Tabelle für README:**
| Webhook | Event | Embed Color |
|---------|-------|-------------|
| Server Start | Server fully booted | Green |
| Server Stop | Server shutting down | Red |
| Server Heartbeat | Periodic pulse | — (REST only) |
| Server Lock/Unlock | BattlEye lock state | Yellow/Green |
| Mod Update | Mod version changed | Blue |
| Quest Completed | Expansion Quest done | Purple |
| Player Connect | Player joined | Green |
| Player Disconnect | Player left | Red |
| Player Kill | PvP/PvE kill | Orange |

---

## Ausführungsreihenfolge

```
Phase 1 — Parallel (Read-Only):
┌──────────────┬─────────────────┬───────────────┐
│   Agent 1    │    Agent 2      │   Agent 3     │
│ Exp. Quests  │ DME Teleport    │ CF Framework  │
│  (Explore)   │   (Explore)     │  (Explore)    │
└──────┬───────┴────────┬────────┴───────┬───────┘
       │                │                │
       ▼                ▼                ▼
Phase 2 — Design (Read-Only):
┌────────────────────────────────────────────────┐
│                  Agent 4                       │
│          Webhook Design (Plan)                 │
│   Nutzt Ergebnisse von Agent 1-3               │
└────────────────────┬───────────────────────────┘
                     │
                     ▼
Phase 3 — Implementierung:
┌────────────────────────────────────────────────┐
│                  Agent 5                       │
│       Webhook Programmierung                   │
│   Nutzt Design von Agent 4                     │
└────────────────────┬───────────────────────────┘
                     │
                     ▼
Phase 4 — Dokumentation:
┌────────────────────────────────────────────────┐
│                  Agent 6                       │
│    README + API Docs aktualisieren             │
└────────────────────────────────────────────────┘
```

**WICHTIG:** Phase 1 (Agent 1-3) parallel starten. Agent 4 startet erst wenn 1-3 fertig sind. Agent 5 startet erst wenn 4 fertig ist. Agent 6 startet erst wenn 5 fertig ist.

## Hinweise für den Orchestrator

1. **Phase 1:** Starte Agent 1, 2, 3 gleichzeitig (3 parallele Agent-Tool-Aufrufe)
2. **Phase 2:** Wenn alle 3 fertig sind, starte Agent 4 mit den zusammengefassten Ergebnissen
3. **Phase 3:** Wenn Agent 4 fertig ist, starte Agent 5 mit dem Design-Dokument
4. **Phase 4:** Wenn Agent 5 fertig ist, starte Agent 6
5. **Nach Abschluss:** Enforce Script Error-Scan (wie gehabt) über alle neuen/geänderten Dateien
6. **Jeder Agent liest die Enforce Script Regeln** aus `.claude/rules/coding-rules.md`

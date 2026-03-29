# Psyerns_Framework - DayZ 1.29 Kompatibilitaets-Audit (Multi-Agent Orchestration)

## Auftrag

Fuehre ein vollstaendiges Code-Audit des Mods **Psyerns_Framework** durch, um alle Fehlerquellen und Inkompatibilitaeten mit **DayZ 1.29+** zu finden. Nutze dafuer parallele Sub-Agenten. Aendere in Phase 1 und 2 NICHTS - nur analysieren und reporten.

## Referenz-Wissensbasis (autoritativ)

Alle Regeln und Patterns stammen aus:
`C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main\`

Lies bei Bedarf die Dateien in diesen Unterordnern:
- `Tips/` - Alle EnScript Best Practices und Common Pitfalls
- `How-To/` - RPC, Actions, Menus, ModStorage, Profile Settings, etc.
- `Frameworks/` - CF, Expansion, Dabs Framework Referenz
- `DayZGame/DayZ-1.29.161219.md` - Breaking Changes 1.29

## Mod-Verzeichnis (Ziel des Audits)

```
C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\scripts\
```

### Vollstaendige Dateiliste (34 Dateien)

**3_Game Layer:**
```
3_Game\Psyerns_Framework\Logging\PF_Logger.c
3_Game\Psyerns_Framework\RPC\PF_RPCConstants.c
3_Game\Psyerns_Framework\Utils\PF_HttpArguments.c
3_Game\Psyerns_Framework\Utils\PF_JsonBuilder.c
3_Game\Psyerns_Framework\REST\Base\PF_RestBase.c
3_Game\Psyerns_Framework\REST\Config\PF_RestConfig.c
3_Game\Psyerns_Framework\REST\Discord\PF_DiscordIntegration.c
3_Game\Psyerns_Framework\REST\Leaderboard\PF_LeaderboardExport.c
3_Game\Psyerns_Framework\REST\Leaderboard\PF_LeaderboardReader.c
3_Game\Psyerns_Framework\REST\PlayerLookup\PF_PlayerLookup.c
3_Game\Psyerns_Framework\REST\ServerStatus\PF_ServerStatus.c
3_Game\Psyerns_Framework\REST\Whitelist\PF_WhitelistManager.c
3_Game\Psyerns_Framework\Web\PF_WebClient.c
3_Game\Psyerns_Framework\Web\PF_WebRequest.c
3_Game\Psyerns_Framework\Web\PF_WebResponse.c
3_Game\Psyerns_Framework\Web\Config\PF_WebConfig.c
3_Game\Psyerns_Framework\Web\Config\PF_WebEndpoint.c
3_Game\Psyerns_Framework\Web\Notifications\PF_ServerNotifications.c
3_Game\Psyerns_Framework\Web\Payload\PF_DiscordPayload.c
3_Game\Psyerns_Framework\Web\Payload\PF_JsonPayload.c
3_Game\Psyerns_Framework\Web\Payload\PF_WordPressPayload.c
3_Game\Psyerns_Framework\Web\Queue\PF_WebQueue.c
3_Game\Psyerns_Framework\Web\Queue\PF_WebQueueItem.c
3_Game\Psyerns_Framework\Web\RestCallback\PF_RestCallback.c
3_Game\Psyerns_Framework\Web\WebApi\PF_DiscordWebhook.c
3_Game\Psyerns_Framework\Web\WebApi\PF_WebApiBase.c
3_Game\Psyerns_Framework\Web\WebApi\PF_WordPressApi.c
```

**4_World Layer:**
```
4_World\Psyerns_Framework\PF_WebQueueProcessor.c
4_World\Psyerns_Framework\REST\PF_KillFeedHook.c
4_World\Psyerns_Framework\REST\Alerts\PF_AlertSystem.c
4_World\Psyerns_Framework\REST\KillFeed\PF_KillFeedManager.c
4_World\Psyerns_Framework\REST\Quests\PF_QuestWebhook.c
```

**5_Mission Layer:**
```
5_Mission\Psyerns_Framework\PF_MissionClient.c
5_Mission\Psyerns_Framework\PF_MissionInit.c
5_Mission\Psyerns_Framework\PF_RestInit.c
```

---

## Bereits bekannte Problemstellen (vorab identifiziert)

Die folgenden Probleme wurden bei einer ersten Inspektion festgestellt. Die Agenten MUESSEN diese verifizieren und vollstaendig dokumentieren:

### Kritisch: GetGame() Verwendung
- **PF_MissionClient.c** - Mehrfach `GetGame().IsClient()`, `GetGame().IsServer()`, `GetGame().IsMultiplayer()`, `GetGame().GetMission()` verwendet
- **PF_ServerNotifications.c** - `GetGame().GetTickTime()`, `GetGame().GetPlayers()`, `GetGame().ConfigGetChildrenCount()`, `GetGame().ConfigGetChildName()` verwendet
- **PF_QuestWebhook.c** - `GetGame().IsDedicatedServer()` verwendet
- **PF_KillFeedHook.c** - `GetGame().IsDedicatedServer()` verwendet

### Kritisch: IsClient() / IsServer() Checks
- **PF_MissionClient.c** - `GetGame().IsClient()` und `!GetGame().IsClient()` als Client-Checks
- **PF_MissionClient.c** - `GetGame().IsMultiplayer()` kombiniert mit IsClient-Checks (unzuverlaessig)

### RPC System
- **PF_MissionInit.c** und **PF_MissionClient.c** - Verwendung von `GetRPCManager()` (CF-Framework RPC)
- Pruefen ob CF-Framework korrekt als Dependency deklariert ist
- `PF_RPC_CHANNEL` (String-basiert) statt Integer-Enum (Vanilla-RPC wuerde `>= 10000` erfordern - CF-RPC ist anders)

---

## Phase 1: Parallele Analyse - Starte diese 5 Agenten GLEICHZEITIG

---

### Agent 1: GetGame() -> g_Game Migration

**Aufgabe:** Finde ALLE `GetGame()` Aufrufe und alle unzuverlaessigen Client/Server-Checks im **Psyerns_Framework** Mod.

**Regeln (aus Tips/Tips-g_Game-GetGame.md):**
- `GetGame()` ist DEPRECATED - darf NICHT mehr verwendet werden ab DayZ 1.29
- Ersetze JEDEN `GetGame()` Aufruf mit `g_Game`
- Vor JEDER `g_Game` Nutzung MUSS ein Null-Check stehen: `if (!g_Game) return;`
- Bei inline-Nutzung: `if (g_Game && g_Game.IsDedicatedServer())`
- Bei mehrfacher Nutzung in einer Funktion: einmal am Anfang pruefen, dann sicher verwenden
- `IsClient()` und `IsServer()` sind UNZUVERLAESSIG - NUR `IsDedicatedServer()` verwenden

**Suche nach diesen Patterns in allen 34 Dateien:**
```
GetGame()
GetGame().
GetGame().IsClient()
GetGame().IsServer()
GetGame().IsMultiplayer()
GetGame().IsDedicatedServer()
GetGame().GetTickTime()
GetGame().GetMission()
GetGame().GetPlayers()
GetGame().GetCallQueue()
GetGame().ConfigGetChildrenCount()
GetGame().ConfigGetChildName()
g_Game.IsClient()
g_Game.IsServer()
.IsClient()
.IsServer()
.IsMultiplayer()
```

**Prioritaetsdateien (bekannte Treffer):**
- `5_Mission\Psyerns_Framework\PF_MissionClient.c`
- `3_Game\Psyerns_Framework\Web\Notifications\PF_ServerNotifications.c`
- `4_World\Psyerns_Framework\REST\Quests\PF_QuestWebhook.c`
- `4_World\Psyerns_Framework\REST\PF_KillFeedHook.c`

**Korrektur-Beispiele fuer PF-spezifische Patterns:**

```c
// VORHER (PF_MissionClient.c - IsClient Check):
if (GetGame().IsClient() || !GetGame().IsMultiplayer())

// NACHHER:
if (!g_Game) return;
if (!g_Game.IsDedicatedServer())  // "nicht dedizierter Server" = Client

// VORHER (PF_ServerNotifications.c):
s_ServerStartTime = GetGame().GetTickTime();

// NACHHER:
if (!g_Game) return;
s_ServerStartTime = g_Game.GetTickTime();

// VORHER (GetPlayers):
GetGame().GetPlayers(players);

// NACHHER:
if (!g_Game) return;
g_Game.GetPlayers(players);

// VORHER (ConfigGet):
int count = GetGame().ConfigGetChildrenCount("CfgMods");
GetGame().ConfigGetChildName("CfgMods", i, name);

// NACHHER:
if (!g_Game) return;
int count = g_Game.ConfigGetChildrenCount("CfgMods");
g_Game.ConfigGetChildName("CfgMods", i, name);

// VORHER (GetMission Chat):
GetGame().GetMission().OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(...));

// NACHHER:
if (!g_Game) return;
Mission mission = g_Game.GetMission();
if (!mission) return;
mission.OnEvent(ChatMessageEventTypeID, new ChatMessageEventParams(...));
```

**Fuer jede Fundstelle zeige:**
1. Datei + Zeilennummer + aktuellen Code
2. Korrigierten Code mit `g_Game` + passendem Null-Check
3. Falls `IsClient()` -> `!IsDedicatedServer()` ersetzen
4. Falls `IsServer()` / `IsDedicatedServer()` -> `IsDedicatedServer()` behalten aber `GetGame()` ersetzen

---

### Agent 2: EnScript Syntax & Common Pitfalls

**Aufgabe:** Pruefe alle 34 Dateien auf verbotene Syntax und bekannte EnScript-Fallen.

**Pruefe auf diese Fehler (aus Tips/Tips-Common-Pitfalls.md, Tips/Tips-Code-Structure.md):**

1. **Ternary-Operator** (`? :`) - EnScript unterstuetzt KEINEN Ternary. Suche nach Pattern `= expr ? val1 : val2`. Jede Fundstelle -> if-else Ersatz zeigen.

2. **Multi-Variable-Deklarationen** - `int a, b, c;` verursacht Compile-Fehler. Jede Variable muss einzeln deklariert werden.
   - Beispiel in PF_Logger.c: `int year, month, day, hour, minute, second;` - **PRUEFEN ob dies in EnScript erlaubt ist oder Fehler verursacht**

3. **Mehrzeilige Funktionsaufrufe** - Funktionsaufrufe duerfen NICHT ueber mehrere Zeilen gebrochen werden.

4. **`delete` Keyword** - NIEMALS verwenden. Stattdessen `= null` setzen.

5. **`auto` Keyword** - Nicht unterstuetzt in vanilla EnScript.

6. **Optional Chaining `?.`** - Nicht unterstuetzt.

7. **Null Coalescing `??`** - Nicht unterstuetzt.

8. **Lambdas** - Nicht unterstuetzt.

9. **Leere `#ifdef/#endif` Bloecke** - Verursachen Segfaults.
   - `PF_QuestWebhook.c` komplett in `#ifdef EXPANSIONMODQUESTS` - pruefe ob der Block nie leer sein kann.

10. **Variablen-Redekleration** - Gleicher Variablenname in verschachteltem Scope.
    - Besondere Aufmerksamkeit: `PF_ServerNotifications.c` verwendet Schleifenvariable `ci` in `CheckModUpdates()` - pruefe ob korrekt.

11. **`int uptimeMin = (int)(uptimeSeconds / 60.0)` in `PF_ServerNotifications.c`** - Cast-Syntax `(int)` pruefen ob in EnScript erlaubt.

12. **`ref` auf Return-Type** - z.B. `protected static ref array<string> SplitModString(...)` in PF_ServerNotifications.c - pruefen ob `ref` auf Return-Type erlaubt ist.

13. **Default-Parameter** - `void Send(..., int embedColor = 0)` in PF_DiscordIntegration.c - Default-Parameter in EnScript pruefen.

**Fuer jede Fundstelle zeige:** Datei + Zeile + aktueller Code + korrigierter Code.

---

### Agent 3: Memory Management, ref & Crash-Risiken

**Aufgabe:** Pruefe `ref` Verwendung, Memory-Patterns und bekannte Crash-Ursachen im gesamten Mod.

**Regeln (aus Tips/Tips-Memory-Management.md, Tips/Tips-Common-Pitfalls.md, Tips/Tips-Best-Practices.md):**

**ref-Regeln:**
- `ref` auf Member-Variablen: PFLICHT fuer Objekt-Referenzen (verhindert vorzeitige GC)
- `ref` auf Parameter: VERBOTEN (Compile-Fehler)
- `ref` auf Return-Types: VERBOTEN (Compile-Fehler) - **`SplitModString()` zurueckgibt `ref array<string>` - kritisch pruefen**
- `ref` auf lokale Variablen: VERBOTEN (Compile-Fehler) - **`PF_ServerNotifications.c` deklariert `ref array<string> prevList = SplitModString(...)` und `ref array<string> currList = ...` als lokale Variablen**
- `delete obj;` VERBOTEN

**Crash-Risiken speziell im PF-Code:**

1. **PF_WebApiBase.c** - `m_RestContext.POST(...)` ohne Null-Check auf `m_RestContext`. Ist `m_RestContext` initialisiert wenn `Post()` aufgerufen wird?

2. **PF_AlertRule** - `m_RestCtx.POST(cb, "", json)` in `Fire()` - `m_RestCtx` wird nur gesetzt wenn `webhookUrl != ""`, aber was wenn `webhookUrl` leer ist? (Null-Check existiert aber pruefen)

3. **PF_KillFeedManager** - `m_Contexts[i].POST(cb, "", json)` - Index `i` wird in `m_WebhookUrls` iteriert, aber `m_Contexts` hat denselben Index - sicher solange beide synchron befuellt werden.

4. **PF_ServerNotifications.SendHeartbeat()** - `new PF_RestBase(...)` wird erstellt aber NICHT in einer `ref`-Variable gespeichert - moegliche sofortige GC?
   ```c
   PF_RestBase restBase = new PF_RestBase(wpEp.BaseUrl, wpEp.ApiKey);
   restBase.PostJson("/server/heartbeat", json);
   ```
   Ist `PF_RestBase` hier sofort nach der Zeile fuer GC kandidat?

5. **`new PF_DiscordWebhook(...)` ohne ref** - In `SendServerStopNotification()`, `CheckModUpdates()`, `PF_QuestWebhook` wird `PF_DiscordWebhook webhook = new PF_DiscordWebhook(...)` ohne `ref` erstellt und sofort `webhook.Send(payload)` aufgerufen - pruefen ob GC hier ein Problem ist bevor `.Send()` abgeschlossen ist.

6. **PF_RestCallback in tight loops** - `new PF_RestCallback()` wird in `SendToWebhooks()` und `AlertRule.Fire()` in jeder Iteration neu erstellt - GC-Druck pruefen.

7. **`GetHumanInventory()` ohne Null-Check** in `PF_KillFeedManager.OnPlayerKilled()`:
   ```c
   EntityAI itemInHands = killerPlayer.GetHumanInventory().GetEntityInHands();
   ```
   `GetHumanInventory()` kann null zurueckgeben - Segfault-Risiko!

8. **`GetIdentity()` ohne Null-Check** - In `PF_KillFeedManager`, `PF_KillFeedHook`, `PF_QuestWebhook` wird `GetIdentity()` an mehreren Stellen gecheckt, aber nicht immer. Alle Fundstellen verifizieren.

9. **PF_RestConfig** - `m_Config.WebhookUrls` und `m_Config.AlertRules` koennen theoretisch null sein wenn JSON-Deserialisierung fehlschlaegt - Null-Checks in `GetInstance()` fehlen?

**Fuer jede Fundstelle zeige:** Datei + Zeile + Problem + Fix-Vorschlag.

---

### Agent 4: RPC System & Netzwerk-Korrektheit

**Aufgabe:** Pruefe alle RPC-Implementierungen auf Korrektheit und 1.29-Kompatibilitaet.

**Kontext:** Psyerns_Framework nutzt das **CF (Community Framework) RPC System** ueber `GetRPCManager()`. Dies ist ein anderes System als Native DayZ RPC.

**Regeln (aus Frameworks/Community-Framework/CF/How-To-CF-RPC.md):**

**CF RPC System:**
- MUSS `GetRPCManager().AddRPC(channel, name, handler, executionType)` verwenden
- MUSS `GetRPCManager().SendRPC(channel, name, params, broadcast, target)` verwenden
- Handler-Signatur: `void HandlerName(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)`
- `CallType` MUSS im Handler geprueft werden
- RPC-Channel: String-basiert bei CF - kein Integer-Enum noetig

**Pruefe in PF_MissionInit.c:**
```
GetRPCManager().AddRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_REQUEST, this, SingleplayerExecutionType.Server);
```
- Ist `SingleplayerExecutionType.Server` korrekt fuer CF-RPC? Oder muss es `SingleplayerExecutionType.Both` sein?
- Wird CF Framework als Dependency deklariert? Pruefen in `config.cpp`:
  ```
  requiredAddons[] = {"DZ_Data"};
  ```
  **KRITISCH: Wenn CF verwendet wird, muss `"CommunityFramework"` in requiredAddons sein!**

**Pruefe in PF_MissionClient.c:**
```
GetRPCManager().AddRPC(PF_RPC_CHANNEL, PF_RPC_RELOAD_RESPONSE, this, SingleplayerExecutionType.Client);
```
- `AddRPC` wird in `MissionBase.OnInit()` aufgerufen - ist das der richtige Zeitpunkt fuer CF-RPC?
- `SendRPC` wird in `OnUpdate()` aufgerufen ohne `sender`-Parameter (`null, true`) - ist das valide?

**Pruefe PF_ReloadRequest Handler (PF_MissionInit.c):**
```c
void PF_ReloadRequest(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
{
    if (type != CallType.Server || !sender)
        return;
```
- Handler-Signatur korrekt ✓
- `CallType.Server` Check korrekt ✓
- Aber: Handler ist in `modded class MissionServer` definiert - ist `this` als Handler korrekt wenn die Klasse modded ist?

**Pruefe PF_ReloadResponse Handler (PF_MissionClient.c):**
```c
void PF_ReloadResponse(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
{
    if (type != CallType.Client)
        return;
    Param2<bool, string> data;
    if (!ctx.Read(data))
        return;
```
- Handler-Signatur korrekt ✓
- `CallType.Client` Check korrekt ✓
- `ctx.Read(data)` - liest ein `Param2` als Ganzes - ist das CF-RPC korrekt?

**Pruefe config.cpp auf CF-Dependency:**
```
C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\config.cpp
```
Aktuell: `requiredAddons[] = {"DZ_Data"};`
Falls CF genutzt wird aber nicht deklariert: Silent load order failure!

**Fuer jede Fundstelle zeige:** Datei + Problem + Fix.

---

### Agent 5: Modded Classes, Script Layer & config.cpp

**Aufgabe:** Pruefe Mod-Struktur, Klassen-Definitionen und Konfiguration.

**Modded Class Regeln (aus Tips/Tips-Modded-Classes.md, Tips/Tips-Override-Keyword.md):**
- `modded class X` darf KEINE Vererbungssyntax haben (`: SomeClass`)
- `override` ist PFLICHT auf allen ueberschriebenen Methoden
- `super.Method()` muss ZUERST aufgerufen werden
- Member-Variablen in modded Classes MUESSEN Mod-Prefix haben: `m_PF_VarName`
- Fehlende `override` erzeugt Warning

**Pruefe folgende modded Classes:**

**1. `modded class MissionServer` (PF_MissionInit.c):**
- `override void OnInit()` ✓ (pruefe super.OnInit() zuerst ✓)
- `override void OnUpdate(float timeslice)` ✓
- `override void OnMissionFinish()` - pruefe `super.OnMissionFinish()` Aufruf-Reihenfolge ✓
- Neue Member-Variablen: `m_PF_QueueProcessor`, `m_PF_StartNotifyTimer`, `m_PF_StartNotifySent` - Prefix `m_PF_` korrekt ✓
- Aber: Hilfsmethoden `SendServerStartNotification`, `GetEndpointStatusDot`, `ParseWebhookApiKey` - fehlen `protected` Modifier? Pruefe ob korrekt.

**2. `modded class MissionServer` (PF_RestInit.c):**
- Zweite `modded class MissionServer` - ist das erlaubt? CF/DayZ erlaubt mehrere modded class Definitionen per Stacking - BESTAETIGEN.
- `override void OnInit()` - ruft `super.OnInit()` zuerst ✓
- `override void OnUpdate()` - ruft `super.OnUpdate()` zuerst ✓
- `override void OnMissionFinish()` - ruft `super.OnMissionFinish()` ganz am Ende - **IST DAS KORREKT? Normalerweise sollte super ZUERST aufgerufen werden fuer MissionFinish?**
- KEIN Prefix auf Singleton-Globals: `g_PF_WhitelistManager = null;` etc. - Globals sind kein modded-class-Member, OK.

**3. `modded class MissionBase` (PF_MissionClient.c):**
- `override void OnInit()` - `super.OnInit()` zuerst ✓
- `override void OnUpdate()` - `super.OnUpdate()` zuerst ✓
- Keine neuen Member-Variablen - OK
- `PF_ReloadResponse` ist eine neue Methode (kein override) - kein `override` noetig ✓

**4. `modded class PlayerBase` (PF_KillFeedHook.c):**
- `override void EEKilled(Object killer)` - `super.EEKilled(killer)` zuerst ✓
- 4_World Layer korrekt fuer PlayerBase ✓

**5. `modded class MissionBaseWorld` (PF_QuestWebhook.c):**
- `void Expansion_OnQuestCompletion(ExpansionQuest quest)` - IST DAS EIN OVERRIDE? Fehlt `override` Keyword?
- Expansion-spezifische Methode - pruefe ob `override` noetig ist.

**Naming-Regeln:**
- Alle neuen Klassen haben Prefix `PF_` ✓
- Globale Singletons haben Prefix `g_PF_` ✓
- Member-Variablen in neuen Klassen: `m_` Prefix ✓
- Member-Variablen in modded Classes: `m_PF_` Prefix ✓

**Script Layer Regeln:**
- `PF_WebQueueProcessor` in `4_World` - ABER die Klasse erbt nicht von einer World-spezifischen Klasse. Ist `4_World` der richtige Layer fuer diese Utility-Klasse? Sie wird aus `5_Mission\PF_MissionInit.c` instanziiert - 5_Mission darf 4_World referenzieren ✓
- `PF_KillFeedHook` (modded PlayerBase) korrekt in `4_World` ✓
- `PF_AlertSystem`, `PF_KillFeedManager` in `4_World` - nutzen `PlayerBase` Typ (4_World) - Layer korrekt ✓
- `PF_QuestWebhook` (modded MissionBaseWorld) in `4_World` - **FALSCH? MissionBaseWorld gehoert zu `5_Mission` Layer. Pruefe ob modded MissionBaseWorld in 4_World oder 5_Mission sein muss.**

**config.cpp Pruefung:**
```
C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\config.cpp
```
- `requiredVersion = 0.1;` ✓
- `requiredAddons[] = {"DZ_Data"};` - **KRITISCH: Fehlt `"CommunityFramework"` wenn CF-RPC verwendet wird?**
- `files[]` Pfade: pruefe ob alle Unterverzeichnisse durch die Modul-Definitionen abgedeckt sind
  - `files[] = {"Psyerns_Framework/scripts/3_Game"}` - deckt ALLE Dateien in allen Unterordnern ab ✓ (rekursiv)
- Pfad-Format: einfache Forward-Slashes werden verwendet - pruefe ob Backslashes noetig sind

**Fuer jede Fundstelle zeige:** Datei + Problem + Fix.

---

## Phase 2: Zusammenfuehrung & Priorisierte Fix-Liste

Nachdem alle 5 Agenten fertig sind, erstelle:

### 1. Zusammenfassung
- Anzahl Probleme pro Agent/Kategorie
- Betroffene Dateien auflisten

### 2. Priorisierte Fix-Liste

Sortiert nach Schwere:

| Prio | Kategorie | Beschreibung | Datei(en) |
|------|-----------|-------------|-----------|
| P0 | Compile-Fehler | Blockiert Build komplett | |
| P1 | Segfault/Crash | Runtime-Crashes (Null-Pointer, GC-Probleme) | |
| P2 | 1.29 Breaking | GetGame() Migration, IsClient/IsServer Ersatz | PF_MissionClient.c, PF_ServerNotifications.c, PF_QuestWebhook.c, PF_KillFeedHook.c |
| P3 | Silent Failures | Fehlende Dependencies, falsche Layer, RPC-Fehler | config.cpp, PF_QuestWebhook.c |
| P4 | Best Practices | ref-Korrekturen, Naming, Override, Super-Reihenfolge | |

### 3. Frage den User
- Sollen alle Fixes automatisch angewendet werden?
- Oder Kategorie fuer Kategorie mit Review dazwischen?
- Oder nur bestimmte Prioritaeten (z.B. nur P0-P2)?

---

## Phase 3: Fixes anwenden (NUR nach User-Bestaetigung)

Wende Fixes in dieser Reihenfolge an:
1. **P0:** Compile-Fehler beheben
2. **P1:** Crash-Risiken eliminieren (besonders `GetHumanInventory()` Null-Check)
3. **P2:** `GetGame()` -> `g_Game` Migration (alle 4 betroffenen Dateien)
4. **P3:** Silent Failures: CF-Dependency in config.cpp, Layer-Fixes
5. **P4:** Best Practices anwenden

**Migrations-Reihenfolge fuer GetGame():**
1. Zuerst `PF_KillFeedHook.c` (einfachste Datei, ein Aufruf)
2. Dann `PF_QuestWebhook.c` (ein Aufruf)
3. Dann `PF_MissionClient.c` (mehrere Aufrufe, Client-Logik kritisch)
4. Zuletzt `PF_ServerNotifications.c` (komplexeste, viele Aufrufe)

Nutze parallele Agents (max 5) um mehrere Dateien gleichzeitig zu fixen.
Nach allen Fixes: `git diff` zeigen zur finalen Review.

---

## Absolute Regeln fuer ALLE Agents

1. **Phase 1+2: NUR LESEN** - Keine Dateien aendern
2. **Vollstaendige Fundstellen**: Datei + Zeile + aktueller Code + vorgeschlagener Fix
3. **Keine Logik-Aenderungen**: Nur syntaktische/API-Korrekturen
4. **Referenz-Dateien nutzen**: Bei Unsicherheit die Dateien in `DAYZ_Enforce-Script-main/` lesen
5. **Keine falschen Positiven**: Nur echte, verifizierte Probleme melden
6. **Kontext beachten**: Code im Kontext der umgebenden Funktion lesen, nicht isoliert
7. **Bestehende Funktionalitaet erhalten**: Keine neuen Features, kein Refactoring
8. **PF-spezifisch**: Der Mod nutzt CF-Framework RPC und REST-API - keine Native-RPC-Regeln anwenden wo CF verwendet wird
9. **`#ifdef EXPANSIONMODQUESTS`**: Code innerhalb dieser Bloecke nur auditieren wenn Expansion installiert - pruefe trotzdem Syntax und Layer

---

## PF-spezifische Kontext-Hinweise fuer alle Agents

- **Kein Action-System**: Der Mod nutzt keine Custom Actions - Agent 5 muss keine Action-Registrierung pruefen
- **CF-RPC statt Native**: `GetRPCManager()` ist CF-API - andere Regeln als Native `g_Game.RPC()`
- **REST-API**: `RestApi`, `RestContext`, `RestCallback` sind DayZ-native Klassen - kein Drittanbieter
- **Singleton-Pattern**: Globals `g_PF_*` mit Accessor-Funktionen `GetPF_*()` sind das primaeere Muster
- **`#ifdef DME_War`**: Bedingte Kompilierung fuer Integration mit DME_War Mod - kein Audit noetig, nur Syntax pruefen
- **Config-Serialisierung**: `JsonFileLoader<PF_WebConfig>` - Standard DayZ JSON-Serialisierung, Member ohne Prefix wegen JSON-Key-Matching in `PF_AlertRuleConfig` (korrekt laut Regeln)
- **Log-Pfade**: `$profile:...` Pfade sind DayZ-native Profile-Verzeichnis-Syntax - korrekt

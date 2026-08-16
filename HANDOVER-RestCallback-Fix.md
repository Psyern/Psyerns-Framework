# Handover: RestCallback Use-after-free beheben

> Diese Datei wurde aus einer anderen Claude-Session erzeugt (Diagnose lag im Projekt `DME-VoteSystemV2`).
> Sie ist **selbsterklärend** — du brauchst keinen Kontext aus jener Session.
> Nach erledigter Arbeit darf die Datei gelöscht werden (sie ist untracked).

---

## 1. Was kaputt ist

Zwei DayZ-Server (`DME-Test`, Port 2602 und `DeadmansEcho-Origin`, Port 2302) crashen dauerhaft. Belege aus
`d:\Agent\deployments\*\log_storage\*\crash_*.log`:

- `SEH exception thrown. Exception code: 0xc0000374` — das ist `STATUS_HEAP_CORRUPTION`
- `Access violation. Illegal write by 0x7ff700000001 at 0x7ff700000001`
- `Access violation. Illegal read at 0x7ff600000039`
- `Access violation. Illegal read at 0xffffffffffffffff`

Der Crash-**Ort** wandert (mal in Expansions `ExpansionStateType.LoadXML`, mal in nativen Frames ohne
Skript-Stack). Das ist die typische Signatur von Heap-Korruption: Der Absturz passiert dort, wo als Nächstes
stark allokiert wird — nicht dort, wo der Fehler sitzt. Auffällig: Origin faultet bei `0x7ff63658fa9f`,
Test bei `0x7ff7eaeafa9f` — identische untere 16 Bit, also **dieselbe Instruktion** bei anderer ASLR-Basis.
Gemeinsamer Nenner beider Server ist `Psyerns_Framework`.

### Die Ursache

In DayZ (Vanilla `scripts/3_Game/DayZ/Http/RestApi.c:50`) gilt:

```c
class RestCallback : Managed
```

`RestContext.GET()` / `.POST()` sind **asynchron**. Die Engine merkt sich einen rohen Pointer auf den
Callback und ruft `OnSuccess` / `OnError` / `OnTimeout` erst Millisekunden bis Sekunden später auf.

`RestCallback` ist `Managed`, also skriptseitig refcounted. Wird der Callback nur in einer **lokalen
Variable** oder als **Temporary** übergeben, sinkt der Refcount beim Verlassen der Funktion auf 0 und das
Objekt wird freigegeben — **während der HTTP-Request noch läuft**. Die Engine schreibt die Antwort dann in
bereits freigegebenen Heap. Ergebnis: Heap-Korruption, Crash irgendwo später.

Verschärfend: Die Framework-Config läuft mit `RetryCount: 3` — jeder Fehlversuch feuert erneut in den
freigegebenen Speicher.

---

## 2. Aufgabe 1 — Retention-Mechanismus in `PF_RestCallback`

**Datei:** `Psyerns_Framework/scripts/3_Game/Psyerns_Framework/Web/RestCallback/PF_RestCallback.c`

Ein einzelnes `ref`-Member reicht **nicht**: Bei parallelen Requests würde der zweite den ersten
überschreiben und wir hätten den Bug zurück. Nötig ist eine statische Liste der in-flight Callbacks.

### Warum zeitbasiert und nicht "im Callback abmelden"

Zwei Fallstricke, die du unbedingt vermeiden musst:

1. **Selbst-Freigabe:** Entfernt sich der Callback in `OnSuccess()` selbst aus der Liste, fällt der letzte
   Refcount **während der laufenden Methode** — das ist wieder ein Use-after-free.
2. **Retries:** Vanilla dokumentiert zu `OnError`: *"May be called multiple times in case of
   (RetryCount > 1)"*. Ein Abmelden beim ersten Fehler gibt das Objekt vor dem Retry frei.

Deshalb: **Zeitfenster**. Vanilla begrenzt REST-Timeouts auf max. 120 s (`ERestOption`), 180 s Haltezeit ist
also sicher.

### Umsetzung

Ergänze die Klasse um:

```c
	// Haelt alle laufenden Callbacks am Leben, bis die Engine geantwortet hat.
	// Ohne diese Referenz wird der Callback beim Verlassen der Sende-Funktion
	// freigegeben, waehrend der HTTP-Request noch laeuft -> Heap-Korruption.
	protected static ref array<ref PF_RestCallback> s_PF_InFlight = new array<ref PF_RestCallback>();

	// Haltezeit in Sekunden. Vanilla begrenzt REST-Timeouts auf 120s.
	protected static const float PF_RETAIN_SECONDS = 180.0;

	protected float m_PF_RetainUntil;

	// Muss VOR jedem GET/POST mit dem Callback aufgerufen werden.
	static void PF_Retain(PF_RestCallback cb)
	{
		PF_SweepExpired();

		if (!cb)
			return;

		if (g_Game)
			cb.m_PF_RetainUntil = g_Game.GetTickTime() + PF_RETAIN_SECONDS;

		s_PF_InFlight.Insert(cb);
	}

	// Laeuft nur aus PF_Retain heraus, nie aus einem Callback — so gibt sich
	// kein Objekt waehrend seiner eigenen Methode selbst frei.
	protected static void PF_SweepExpired()
	{
		if (!g_Game)
			return;

		float now = g_Game.GetTickTime();
		int i = s_PF_InFlight.Count() - 1;
		while (i >= 0)
		{
			bool isExpired = s_PF_InFlight[i].m_PF_RetainUntil < now;
			if (isExpired)
				s_PF_InFlight.Remove(i);

			i = i - 1;
		}
	}
```

**Beachte:** `bool isExpired = ...;` als Zwischenvariable ist Absicht — komplexe Ausdrücke direkt in
Array-Index-Operationen sind in EnScript eine bekannte Segfault-Quelle.

**Bekannte, akzeptierte Einschränkung:** Der Sweep läuft nur beim nächsten `PF_Retain`. Versiegen die
Requests, bleiben die letzten Callbacks bis zum nächsten Request im Speicher. Das ist beschränkt und
harmlos — bitte **nicht** durch einen Timer "verbessern".

---

## 3. Aufgabe 2 — Alle Call-Sites umstellen

Muster überall identisch: Callback in eine lokale Variable, `PF_RestCallback.PF_Retain(cb);`, **dann**
senden.

```c
// VORHER (defekt)
m_RestContext.GET(new PF_RestCallback(), endpoint);

// NACHHER
PF_RestCallback cb = new PF_RestCallback();
PF_RestCallback.PF_Retain(cb);
m_RestContext.GET(cb, endpoint);
```

### Zu ändernde Stellen

| Datei | Zeile(n) | Aktuell |
|---|---|---|
| `scripts/3_Game/Psyerns_Framework/Web/PF_WebClient.c` | 53–58 | lokales `callback`, kein Retain |
| `scripts/3_Game/Psyerns_Framework/Web/WebApi/PF_WebApiBase.c` | 27 | `POST(new PF_RestCallback(), …)` |
| `scripts/3_Game/Psyerns_Framework/Web/WebApi/PF_WebApiBase.c` | 38 | `GET(new PF_RestCallback(), …)` |
| `scripts/4_World/Psyerns_Framework/REST/Alerts/PF_AlertSystem.c` | 89–90 | lokales `cb`, kein Retain |
| `scripts/4_World/Psyerns_Framework/REST/KillFeed/PF_KillFeedManager.c` | 172–173 | lokales `cb` **in einer Schleife** |
| `scripts/3_Game/DME_Api/DME_Api_Core.c` | 68 | `POST(new DME_Api_SilentCallBack, …)` |
| `scripts/3_Game/DME_Api/DME_Api_Core.c` | 91 | `POST(new DME_Api_DBNestedCallBack(cb,cid), …)` |
| `scripts/3_Game/DME_Api/DME_Api_Core.c` | 101 | `GET(new DME_Api_SilentCallBack, …)` |
| `scripts/3_Game/DME_Api/DME_Api_Core.c` | 120 | `GET(new DME_Api_DBNestedCallBack(cb,cid), …)` |

**Zu `DME_Api_Core.c`:** Die dortigen Callback-Klassen (`DME_Api_SilentCallBack`,
`DME_Api_DBNestedCallBack`, …) erben **nicht** von `PF_RestCallback`. Prüfe ihre Basisklasse. Erben sie von
`RestCallback`, haben sie exakt dasselbe Problem. Zwei saubere Optionen — **entscheide dich für eine und
wende sie konsequent an**:

- **A (bevorzugt):** Retention auf `RestCallback`-Ebene generisch machen, z. B. eine eigene kleine
  Registry-Klasse, die `RestCallback` statt `PF_RestCallback` hält. Dann funktioniert derselbe Aufruf für
  alle Callback-Typen.
- **B:** Die `DME_Api_*`-Callbacks von `PF_RestCallback` ableiten lassen — nur wenn das ihre bestehende
  Logik nicht bricht.

Falls A gewählt wird, verwende in Aufgabe 1 durchgehend `RestCallback` statt `PF_RestCallback` als
Element-Typ. Das ist die robustere Variante.

Zeilennummern stammen vom Stand 2026-07-25 — **verifiziere den Inhalt vor jedem Edit**, statt blind auf die
Zeile zu editieren.

---

## 4. Aufgabe 3 — `CreateRestApi()` absichern

**Datei:** `scripts/3_Game/Psyerns_Framework/Web/PF_WebClient.c`, Zeile 10

```c
// VORHER — erzeugt bedingungslos eine RestApi
void PF_WebClient()
{
	m_Contexts = new map<string, RestContext>();
	m_RestApi = CreateRestApi();
}

// NACHHER — vorhandene wiederverwenden
void PF_WebClient()
{
	m_Contexts = new map<string, RestContext>();
	m_RestApi = GetRestApi();
	if (!m_RestApi)
		m_RestApi = CreateRestApi();
}
```

**Warum:** `RestApi` und `RestContext` haben in Vanilla **private Destruktoren** — das Skript besitzt sie
nie, es hält nur Engine-Pointer. Ersetzt ein zweiter `CreateRestApi()`-Aufruf die globale RestApi, werden
alle bereits gecachten `RestContext`-Pointer (auch in anderen Mods) zu Dangling Pointers.

Genau dieses Guard-Muster wird im selben Projekt an vier anderen Stellen bereits korrekt verwendet —
`DME_Api_Core.c:229`, `DME_Api_Rest.c:17`, `PF_WebApiBase.c:9`, `DME_Api_EndpointBase.c:28`.
`PF_WebClient.c:10` ist der einzige Ausreißer.

---

## 5. EnScript-Regeln (nicht verletzen)

- **Kein Ternary** (`? :`) — `if/else` benutzen
- **Keine Mehrfach-Deklaration** — `int a, b;` ist ein Compile-Fehler, jede Variable einzeln
- **Kein `delete`** — stattdessen `= null`
- **Kein `ref` auf Locals, Parameter oder Rückgabetypen** — nur auf Member- und statische Variablen
- **Kein `auto`**, kein `?.`, kein `??`, keine Lambdas
- **Keine mehrzeiligen Funktionsaufrufe** — Argumente auf eine Zeile oder Zwischenvariablen
- **`override`** auf jeder überschreibenden Methode, **`super.Method()` zuerst**
- **Kein `GetGame()`** — `g_Game` benutzen, vorher auf `null` prüfen
- **Kein `IsClient()` / `IsServer()`** für Authority-Checks — nur `IsDedicatedServer()`
- **Tabs** einrücken, öffnende Klammer auf derselben Zeile
- Komplexe Ausdrücke vor Array-Index-Zuweisungen in Zwischenvariablen auflösen (Segfault-Vermeidung)

---

## 6. Was ausdrücklich NICHT zu tun ist

- Keine Umbenennung bestehender Klassen, Methoden oder Member
- Kein Refactoring der REST-Architektur — nur die Lebensdauer der Callbacks reparieren
- Keine neuen Features, kein Logging-Umbau
- Nicht versuchen, den Crash in Expansions `LoadXML` zu "fixen" — Expansion ist dort nur das Opfer,
  der Code ist in Ordnung (die generierte `$profile:ExpansionMod\AI\FSM\Master.c` ist vollständig und korrekt)

---

## 7. Verifikation

1. Alle geänderten Dateien auf die EnScript-Regeln aus Abschnitt 5 prüfen.
2. Sicherstellen: **kein** `.GET(` / `.POST(` / `.FILE(` im Projekt übergibt noch ein Temporary oder eine
   nicht-retainte lokale Variable. Suchmuster:
   ```
   grep -rnE "\.(GET|POST|FILE)\s*\(\s*new " --include=*.c .
   ```
   Treffer = noch defekt.
3. Mod bauen, deployen, Server starten. Erfolgskriterium: **keine** `0xc0000374`-Crashes und keine
   Access Violations auf Adressen wie `0x7ff7........` mit genullter unterer Hälfte mehr in
   `d:\Agent\deployments\DME-Test\profiles\crash_*.log`.
4. Die Server liefen bisher zwischen 77 Sekunden und ~30 Minuten bis zum Crash — plane einen Testlauf von
   **mindestens einer Stunde** ein, bevor du "behoben" meldest.

Ein sauberer Kompilierlauf beweist hier **nichts** — der Fehler ist ein Laufzeit-Lebensdauerproblem.

---

## 8. Zwei weitere Projekte mit demselben Defekt

Nicht Teil dieser Aufgabe, aber betroffen — jeweils separat übergeben:

**`C:\Users\Administrator\Desktop\DME-VoteSystemV2`** — `DME-VoteSystemV2/Scripts/4_World/module/DMEVoteModule.c`,
Zeilen 772, 840, 928, 1260, 1306 (inline `new obfc_DMEVoteClaimCallback(...)` bzw. `new obfc_DMEVoteAPICallback(...)`).
Dort ist es in Zeilen 34–35 bereits korrekt gelöst (`protected ref RestCallback obfv_m_ApiCallback;`) — das
ist die Vorlage.

**`C:\Users\Administrator\Desktop\DME-WAR`** — `DME_War/scripts/4_World/WarWebhookService.c`, Zeilen 452–453
(lokales `cb` an async `POST`).

> **Niedrige Priorität.** Derselbe Defekt, aber **nicht** ursächlich für die beobachteten Crashes:
> `PostJson()` loggt bei jedem Aufruf (`[WarWebhook] POST sent to: …` bzw. eine Fehlermeldung davor), und
> über sämtliche Logs beider Server gibt es **null** solche Zeilen — der Pfad wurde nie ausgeführt. Der Mod
> selbst läuft (`[DMEW]`-Zeilen), nur der Webhook feuert nicht. Latenter Bug, der erst beim ersten
> ausgelösten War-Event zuschlägt.
>
> Ursächlich sind ausschliesslich **Psyerns_Framework** und **DME-VoteSystemV2** — deren REST-Aufrufe feuern
> bei jedem Serverstart (7 von 7 Runs: `Starting initial API poll` / `Creating new RestContext`).

# PF_Sort (Timsort) für PF_LeaderboardExport — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Den O(n·k)-Selection-Sort in `PF_LeaderboardExport` durch einen stabilen Timsort ersetzen, portiert als framework-eigene Utility-Klasse `PF_Sort`.

**Architecture:** Die fertige, review-verifizierte Timsort-Implementierung aus dem Ninjin-Leaderboard-Mod (`TrackingModSort`) wird als `PF_Sort` in das Framework kopiert (Copy, NICHT referenzieren — das Framework darf keine Klassen des Ninjin-Mods benutzen, sonst entsteht genau die Cross-PBO-Abhängigkeit, die kürzlich den Compile-Fehler `Unknown type 'PF_WebApiBase'` auf der Mod-Seite verursacht hat, nur in Gegenrichtung). Der Sortierkern arbeitet auf einem Index-Permutations-Array über vorab extrahierten Keys; Objekt-Arrays werden erst am Ende einmalig umsortiert. `PF_LeaderboardExport.SortByPvE/SortByPvP` werden zu dünnen Wrappern über eine gemeinsame `SortByPoints`.

**Tech Stack:** DayZ Enforce Script (EnScript), Psyerns Framework V2, Script-Layer `3_Game`.

**Spec:** Es gibt kein separates Spec-Dokument. Referenz-Implementierung (Quelle des Ports, algorithmisch bereits gegen java.util.TimSort/CPython verifiziert):
`C:\Users\Administrator\Desktop\Ninjin_Leaderboard\Ninjins_LeaderBoard\scripts\3_Game\General Configs\Utils\TrackingModSort.c`
Ziel-Repo: `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework\Psyerns_Framework_V2`
Zieldatei des Umbaus: `scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c` (Methoden `SortByPvE`, `SortByPvP`, aufgerufen in `PushLeaderboard()`).

## Global Constraints (EnScript — Compile-/Crash-kritisch, gelten für jeden Task)

- Kein Ternary-Operator (`? :`) — nur `if/else`. Fehlermeldung wäre irreführend (`Broken expression`).
- Eine Variable pro Deklaration (`int a; int b;` — niemals `int a, b;`).
- Keine Variablen-Redeklaration in verschachtelten Scopes.
- Kein `delete` — Referenzen auf `null` setzen.
- Kein `auto`, `var`, `?.`, `??`, keine Lambdas, keine eigenen Generics.
- `ref` NUR auf Member-Variablen und Typedefs — niemals auf Parametern, Rückgabetypen oder Lokalen. (`array<ref PF_WP_PlayerData>` als Lokale ist OK — das `ref` gehört dort zum Template-Argument.)
- Funktionsaufrufe auf EINER Zeile.
- Tabs für Einrückung, öffnende Klammer auf derselben Zeile (Stil der bestehenden Framework-Dateien beibehalten).
- Klassen-Präfix im Framework: `PF_`. Konstanten `UPPER_CASE` mit `PF_SORT_`-Präfix.
- Script-Layer: alles in diesem Plan bleibt in `3_Game` — keine Referenzen auf `4_World`/`5_Mission`.
- KEINE Referenz auf Klassen des Ninjin-Mods (`TrackingMod*`, `NJN_*`) — Port ist eine Kopie.
- `requiredVersion` und `CfgMods`-Namen in config.cpp NICHT anändern (Datenverlust-Gefahr); dieser Plan braucht keine config.cpp-Änderung, da `files[]` bereits den ganzen Ordner `Psyerns_Framework/scripts/3_Game` lädt.
- Es gibt keinen lokalen EnScript-Testrunner. „Test" heißt hier: mechanische Diff-Verifikation gegen die Referenzdatei, Klammer-Bilanz-Script, Verbotene-Syntax-Greps. Der finale Laufzeittest passiert auf dem Testserver (Log-Zeilen von `PF_Logger`).
- Git: das maßgebliche Repo ist das INNERE `C:\Users\Administrator\Desktop\Psyerns_Framework\Psyerns_Framework` (verschachtelte Repos — `git rev-parse --show-toplevel` von der V2-Struktur aus liefert dieses Verzeichnis). Alle Git-Kommandos dort ausführen. Vor Task 1 einen Branch anlegen: `git checkout -b pf-sort-timsort`.

---

### Task 1: `PF_Sort.c` anlegen (mechanischer Port von `TrackingModSort`)

**Files:**
- Create: `Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c`
- Read (Quelle): `C:\Users\Administrator\Desktop\Ninjin_Leaderboard\Ninjins_LeaderBoard\scripts\3_Game\General Configs\Utils\TrackingModSort.c`

**Interfaces:**
- Produces (von Task 2 benutzt):
  - `static void PF_Sort.BuildOrderByInt(array<int> keys, bool ascending, array<int> outOrder)` — füllt `outOrder` mit den Original-Indizes 0..n-1 in sortierter Reihenfolge (stabil).
  - `static void PF_Sort.ApplyOrderPlayers(array<ref PF_WP_PlayerData> items, array<int> order)` — ordnet `items` in-place gemäß `order` um.
  - (ebenfalls vorhanden, aktuell ohne Nutzer: `BuildOrderByFloat`, `BuildOrderByString`, `CompareStrings` — bewusst behalten, siehe Step 1.)

- [ ] **Step 1: Quelldatei kopieren und Identifier umbenennen**

Die Quelldatei ist ~1050 Zeilen Timsort (Run-Erkennung, Binary-Insertion, Merge-Stack mit korrigierter Invariante, Galloping, Rückwärtskopien bei überlappenden Same-Array-Kopien). Sie wird NICHT manuell abgetippt, sondern mechanisch transformiert — jede Freihand-Änderung am Merge-Code ist ein Risiko. Im Bash-Tool:

```bash
sed -e 's/TrackingModSort/PF_Sort/g' -e 's/TRACKINGMOD_SORT_/PF_SORT_/g' \
  "/c/Users/Administrator/Desktop/Ninjin_Leaderboard/Ninjins_LeaderBoard/scripts/3_Game/General Configs/Utils/TrackingModSort.c" \
  > "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework/Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c"
```

Entscheidung (bewusst gegen YAGNI): `BuildOrderByFloat`/`BuildOrderByString` und die drei Key-Typ-Zweige in `Compare()` bleiben drin. Sie zu entfernen hieße, im Inneren des Algorithmus zu editieren — das Risiko ist den eingesparten Code nicht wert, und das Framework (AuctionHouse, weitere Exporte) kann sie später nutzen.

- [ ] **Step 2: Ninjin-spezifische Applier ersetzen**

Die kopierte Datei enthält drei Applier für Ninjin-Typen, die es im Framework NICHT gibt — sie MÜSSEN raus, sonst Compile-Fehler (`Unknown type`):
`ApplyOrderPlayerDeathData` (Typ `PlayerDeathData`), `ApplyOrderLeaderboardPlayers` (Typ `TrackingModLeaderboardPlayerData`), `ApplyOrderRewardEntries` (Typ `RewardConfigEntry`).

Alle drei Methoden komplett löschen und an ihrer Stelle (Abschnitt `// ------- order appliers`) exakt diesen einen Applier einfügen:

```c
	static void ApplyOrderPlayers(array<ref PF_WP_PlayerData> items, array<int> order)
	{
		array<ref PF_WP_PlayerData> snapshot;
		int i;
		int count;

		if (!items || !order)
			return;

		count = items.Count();
		if (count != order.Count())
			return;

		snapshot = new array<ref PF_WP_PlayerData>();
		for (i = 0; i < count; i++)
		{
			snapshot.Insert(items.Get(i));
		}

		for (i = 0; i < count; i++)
		{
			items.Set(i, snapshot.Get(order.Get(i)));
		}
	}
```

(`PF_WP_PlayerData` ist definiert in `scripts/3_Game/Psyerns_Framework/Web/Payload/PF_WordPressPayload.c` — gleicher Layer, keine neue Abhängigkeit.)

- [ ] **Step 3: Datei-Header-Kommentar ersetzen**

Der Kommentarblock am Dateianfang (Zeilen 1–30 der Kopie) referenziert das Ninjin-Leaderboard (`RebuildSortedLists()` etc.). Den gesamten führenden Kommentarblock (alles vor der ersten `const int`-Zeile) durch diesen ersetzen:

```c
// Stable Timsort utility for the framework (index-permutation based).
//
// Port of the verified Ninjin-Leaderboard sorter. Algorithm: standard
// Timsort (Peters/CPython, as adopted by java.util.TimSort) - natural run
// detection, binary insertion sort up to minrun, merge stack with the
// CORRECTED collapse invariant (includes the runLen[n-4] term the original
// java.util.TimSort omitted), galloping merges.
//
// Usage: extract an int/float/string key per element into a keys array,
// call BuildOrderBy*(keys, ascending, order), then ApplyOrder*(items, order).
// Only the int permutation array is moved during sorting; ref-counted object
// arrays are touched exactly once at the end.
//
// EnScript port notes:
//  - Same-array copies where dest > src MUST run backwards (Java relies on
//    System.arraycopy overlap handling; a forward loop clobbers unread data).
//  - STABLE: equal keys keep their input order.
```

- [ ] **Step 4: Verifikation — mechanischer Diff gegen die Quelle**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework/Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/Utils" \
&& sed -e 's/TrackingModSort/PF_Sort/g' -e 's/TRACKINGMOD_SORT_/PF_SORT_/g' \
  "/c/Users/Administrator/Desktop/Ninjin_Leaderboard/Ninjins_LeaderBoard/scripts/3_Game/General Configs/Utils/TrackingModSort.c" \
  | diff - PF_Sort.c
```

Expected: Abweichungen NUR in genau zwei Regionen — (a) der Datei-Header-Kommentar, (b) der Applier-Block (drei Methoden entfernt, `ApplyOrderPlayers` hinzugefügt). Jede Abweichung in `Compare`, `SortOrder`, `CountRunAndMakeOrdered`, `BinarySort`, `MergeCollapse`, `MergeAt`, `GallopLeft`, `GallopRight`, `MergeLo`, `MergeHi`, `MinRunLength`, `ReverseRange`, `CopyToTmp`, `PushRun`, `SeedOrder` ist ein FEHLER und muss rückgängig gemacht werden.

- [ ] **Step 5: Verifikation — Klammer-Bilanz und verbotene Syntax**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework/Psyerns_Framework_V2" \
&& python -c "
import io
src = io.open('scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c', encoding='utf-8').read()
depth = 0
for ln, line in enumerate(src.splitlines(), 1):
    for ch in line:
        if ch == '{': depth += 1
        elif ch == '}': depth -= 1
        if depth < 0: print('NEGATIVE at line', ln)
print('final depth:', depth)
" \
&& grep -nE '\? .+ :|^\s*(int|float|bool|string) \w+ *,|\bdelete \w|\b(ref) [A-Za-z_]+ [a-z]\w* *=' scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c; echo "greps done (keine Treffer = OK)" \
&& grep -n "TrackingMod\|NJN_" scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c; echo "ninjin-refs done (keine Treffer = OK)"
```

Expected: `final depth: 0`, keine Grep-Treffer.

- [ ] **Step 6: Commit**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework" \
&& git add "Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/Utils/PF_Sort.c" \
&& git commit -m "feat: add PF_Sort - stable Timsort utility (port of verified Ninjin sorter)"
```

---

### Task 2: `PF_LeaderboardExport` auf `PF_Sort` umstellen

**Files:**
- Modify: `Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c` (Methoden `SortByPvE` und `SortByPvP`, direkt hintereinander im hinteren Drittel der Datei; Kommentarblock „Teilsortierung: …" gehört zu `SortByPvE` und wird mit ersetzt)

**Interfaces:**
- Consumes (aus Task 1): `PF_Sort.BuildOrderByInt(array<int> keys, bool ascending, array<int> outOrder)`, `PF_Sort.ApplyOrderPlayers(array<ref PF_WP_PlayerData> items, array<int> order)`
- Produces: `SortByPvE(array<ref PF_WP_PlayerData> players)` und `SortByPvP(array<ref PF_WP_PlayerData> players)` behalten exakt ihre Signaturen — die Aufrufstellen in `PushLeaderboard()` bleiben unverändert.

**Verhaltens-Hinweis (gewollt):** Der alte Selection-Sort garantierte nur die ersten `m_MaxPlayers` Plätze und war instabil. Timsort sortiert vollständig und stabil — oberhalb des Caps identische Reihenfolge (bis auf Gleichstände, die jetzt deterministisch die Eingabereihenfolge behalten). Der Cap wird unverändert vom Aufrufer (`PushLeaderboard`, `pveLimit`/`pvpLimit`) beim Befüllen des Payloads angewendet. Bei typischen Größen gewinnt Timsort trotzdem deutlich: n=3000, k=100 → ~300.000 Vergleiche vorher, ~34.000 nachher; läuft im Hauptthread.

- [ ] **Step 1: Beide Sortiermethoden ersetzen**

Die beiden bestehenden Methoden `SortByPvE` und `SortByPvP` (je ~20 Zeilen mit den geschachtelten `for`-Schleifen und `int best = i;`) INKLUSIVE des davorstehenden Kommentarblocks („// Teilsortierung: …") vollständig löschen und durch exakt diesen Block ersetzen:

```c
	// Stable Timsort via PF_Sort (see Utils/PF_Sort.c). Keys are extracted
	// once per player instead of being read on every comparison; only an
	// int permutation array moves during the sort. Full descending order,
	// ties keep their input order; the caller still caps at m_MaxPlayers.
	protected void SortByPoints(array<ref PF_WP_PlayerData> players, bool usePvE)
	{
		array<int> keys;
		array<int> order;
		PF_WP_PlayerData entry;
		int points;
		int i;
		int count;

		if (!players)
			return;

		count = players.Count();
		if (count < 2)
			return;

		keys = new array<int>();
		order = new array<int>();

		for (i = 0; i < count; i++)
		{
			entry = players.Get(i);
			points = 0;
			if (entry)
			{
				if (usePvE)
					points = entry.pvePoints;
				else
					points = entry.pvpPoints;
			}
			keys.Insert(points);
		}

		// false == descending
		PF_Sort.BuildOrderByInt(keys, false, order);
		PF_Sort.ApplyOrderPlayers(players, order);
	}

	protected void SortByPvE(array<ref PF_WP_PlayerData> players)
	{
		SortByPoints(players, true);
	}

	protected void SortByPvP(array<ref PF_WP_PlayerData> players)
	{
		SortByPoints(players, false);
	}
```

Nichts anderes in der Datei anfassen — `PushLeaderboard()`, `OnUpdate`, Konstruktor, `GetTimestamp` bleiben unverändert.

- [ ] **Step 2: Verifikation — alter Sort restlos weg, neue Aufrufe da**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework/Psyerns_Framework_V2" \
&& grep -n "int best" "scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c"; echo "old-sort check done (keine Treffer = OK)" \
&& grep -n "PF_Sort\.\|SortByPoints\|SortByPvE\|SortByPvP" "scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c"
```

Expected: kein `int best` mehr; genau diese Treffer: Definition `SortByPoints` + 2 Wrapper-Definitionen + je 1 Aufruf `SortByPvE(pveSorted)` / `SortByPvP(pvpSorted)` in `PushLeaderboard` + die 2 `PF_Sort.`-Aufrufe in `SortByPoints`.

- [ ] **Step 3: Verifikation — Klammer-Bilanz und verbotene Syntax der Datei**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework/Psyerns_Framework_V2" \
&& python -c "
import io
src = io.open('scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c', encoding='utf-8').read()
depth = 0
for ch in src:
    if ch == '{': depth += 1
    elif ch == '}': depth -= 1
print('final depth:', depth)
" \
&& grep -nE '\? .+ :|^\s*(int|float|bool|string) \w+ *,|\bdelete \w' "scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c"; echo "greps done (keine Treffer = OK)"
```

Expected: `final depth: 0`, keine Grep-Treffer.

- [ ] **Step 4: Commit**

```bash
cd "/c/Users/Administrator/Desktop/Psyerns_Framework/Psyerns_Framework" \
&& git add "Psyerns_Framework_V2/scripts/3_Game/Psyerns_Framework/REST/Leaderboard/PF_LeaderboardExport.c" \
&& git commit -m "perf: PF_LeaderboardExport sorts via PF_Sort (stable Timsort) instead of O(n*k) selection sort"
```

---

### Task 3: Laufzeit-Smoke-Test (Testserver, manuell oder per Log-Prüfung)

**Files:** keine Änderungen — reine Verifikation.

- [ ] **Step 1: PBO bauen und auf dem Testserver mit aktivem Leaderboard-Export starten**

Voraussetzung in der Framework-WebConfig (`PsyernsFrameworkConfig`-JSON im Serverprofil): `EnableLeaderboardExport: true`, gültiger `Leaderboard`-Endpoint (BaseUrl/ApiKey), `NinjinPlayersPath` zeigt auf `$profile:Ninjins_Tracking_Mod/Data/Players/` und dort liegen Spieler-JSONs.

- [ ] **Step 2: Server-Log prüfen**

Erwartete `PF_Logger`-Zeilen nach Ablauf des Export-Intervalls:
- `LeaderboardExport: Uploaded <n> players (<m> online), PvE top: ..., PvP top: ...` — n/m plausibel.
- `LeaderboardExport: Top PvE player: <name> (<punkte> pts)` — muss der Spieler mit den meisten PvE-Punkten in den JSONs sein (Stichprobe: höchsten `pvePoints`-Wert in den Player-JSONs nachschlagen und vergleichen).
- KEINE Zeilen mit `[ERROR]`, keine `Unknown type`-Compile-Fehler beim Serverstart (crash-Log und Script-Log prüfen, nicht nur RPT).

- [ ] **Step 3: Payload-Stichprobe**

Im WordPress-Backend (oder per Endpoint-Log) prüfen: `topPVEPlayers` absteigend nach `pvePoints`, `topPVPPlayers` absteigend nach `pvpPoints`, Anzahl ≤ `LeaderboardMaxPlayers`.

---

## Ausdrücklich NICHT in diesem Plan (nicht anfassen)

- `PF_LeaderboardReader`, `PF_WordPressPayload`, `PF_RestInit`, `PF_RestBase`, `PF_WebApiBase` — bleiben unverändert.
- Keine config.cpp-Änderungen (Script-Module laden bereits den kompletten `3_Game`-Ordner).
- Keine Umbenennung bestehender Klassen, kein Aufräumen fremder Baustellen (z. B. `GetGame()`-Nutzung anderswo).
- Der Ninjin-Leaderboard-Mod wird NICHT verändert; er dient nur als Lese-Quelle für den Port.

# Coding Rules & Style

## Core Language & Engine

- Language: Enforce Script
- Engine: DayZ Enfusion
- Scripts structure: 3_Game, 4_World, 5_Mission
- Assume strict DayZ server/client separation.
- Never introduce logic that breaks replication or authority rules.

## General Rules

- Do NOT introduce new gameplay mechanics unless explicitly requested.
- Do NOT change variable, class, or method names unless required to fix errors.
- Prefer minimal, targeted fixes over refactors.
- Avoid speculative or "nice-to-have" changes.
- Maintain backward compatibility with existing save data where possible.
- Assume code may interact with other mods — avoid hard assumptions.

## Enforce Script Pitfalls

- **Kein Zeilenumbruch in String-Concatenation.** Der Compiler erkennt `+` am Zeilenende nicht als Fortsetzung. Immer auf einer Zeile halten.
  - Falsch: `Print("A" + var\n + "B");`
  - Richtig: `Print("A" + var + "B");`
- **`ref` nicht als Methodenparameter verwenden** — nur als Member-Deklaration in Klassen.
- **`PlayerBase`, `EntityAI`, `ItemBase`** existieren erst in `4_World` — niemals in `3_Game` verwenden.
- **Keine mehrzeiligen Method-Chains.** Enforce Script erkennt `.Method()` auf der nächsten Zeile nicht als Fortsetzung. Fluent-Chains immer in einzelne Statements aufteilen:
  - Falsch: `PF_JsonBuilder.Begin()\n.Add("k","v")\n.Build();`
  - Richtig: `PF_JsonBuilder b = PF_JsonBuilder.Begin(); b.Add("k","v"); return b.Build();`
- **`JsonSerializer.ReadFromString` hat 3 Parameter**, nicht 4: `ReadFromString(Class instance, string json, out string error)`. Keinen `bool` Parameter einfügen.
- **Void-Methoden nicht in `if`-Bedingungen verwenden.** Vanilla-APIs wie `GetGame().GetWorldName(out string)` geben `void` zurück, nicht `bool`. Immer die API-Signatur prüfen bevor ein Rückgabewert angenommen wird.
- **String-Indexer `str[i]` nicht mit `.ToString()` verwenden.** Gibt keinen Char-Typ zurück. Stattdessen `str.Substring(idx, 1)` für einzelne Zeichen nutzen.
- **`#ifdef` ist case-sensitive** und muss exakt dem CfgPatches-Klassennamen entsprechen. `#ifdef Psyerns_Framework` (nicht `PSYERNS_FRAMEWORK`). Immer gegen `config.cpp` prüfen.
- **Duplizierte Dateien mit `#ifndef` Guards:** Wenn eine Klasse in mehreren Dateien mit `#ifndef` definiert ist, gewinnt die zuerst geladene. Änderungen müssen in ALLEN Kopien gemacht werden. Im Ninjin Leaderboard: `TrackingModLeaderboardData.c` existiert in `TrackingModUI/` UND `General Configs/Data/` — beide synchron halten.

## Code Style & Quality

- Follow existing file and project style.
- Keep changes readable and minimal.
- Avoid unnecessary abstraction.
- Avoid adding comments unless they clarify non-obvious logic.

## Config.cpp Path Notes (Important)

- In `config.cpp` (e.g. `CfgSoundShaders.samples[]`, `CfgVehicles.model`), **use single backslashes** in paths.
- **Do not** use double backslashes (`\\`) in `config.cpp` paths in this repo (it does not work reliably for this project).
- Example (correct): `DME_Nuke\sounds\bomberplane`
- Example (wrong): `DME_Nuke\\sounds\\bomberplane`

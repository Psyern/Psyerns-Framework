# Copilot Instructions

You are assisting with a DayZ modding project written in Enforce Script.
This project targets server-authoritative gameplay and integrates multiple established DayZ frameworks.

## Core Language & Engine
- Language: Enforce Script
- Engine: DayZ Enfusion
- Scripts structure: 3_Game, 4_World, 5_Mission
- Assume strict DayZ server/client separation.
- Never introduce logic that breaks replication or authority rules.

## Referenced Codebases (Authoritative Sources)

When searching for existing implementations, patterns, or APIs, prioritize the following sources **in this order**:

1. **DayZ Expansion Scripts**
   Repository:
   https://github.com/salutesh/DayZ-Expansion-Scripts/tree/87424ce80c73cf54533f711be0d5a20bc2b8e40c/DayZExpansion

   Rules:
   - Treat Expansion code as production-grade reference.
   - Follow Expansion patterns for RPC, networking, permissions, UI, and gameplay systems.
   - Do NOT reimplement systems that already exist in Expansion unless explicitly instructed.

2. **DayZ Community Framework (CF)**
   Repository:
   https://github.com/Arkensor/DayZ-CommunityFramework/tree/production

   Rules:
   - Use CF conventions for RPC, logging, permissions, helpers, and lifecycle hooks.
   - Assume CF is present and loaded before dependent systems.
   - Do not bypass CF systems with vanilla alternatives.

3. **Dabs Framework**
   Repository:
   https://github.com/InclementDab/DayZ-Dabs-Framework/tree/production

   Rules:
   - Respect Dabs framework architecture and naming conventions.
   - Reuse Dabs utilities, base classes, and helpers where applicable.
   - Avoid duplicating Dabs functionality.

4. **Vanilla DayZ Code**
   Reference:
   https://dayzexplorer.zeroy.com/index.html

   Rules:
   - Use DayZ Explorer exclusively for searching vanilla implementations.
   - Match vanilla method signatures, naming, and expected behavior exactly.
   - Never guess vanilla APIs—verify before use.

5. **Enforce Script Style Guide**
   - Reference:
   https://github.com/TrueDolphin/references/wiki/EnScript-(Enforce-Script)-Style-Guide

   Introduction:
   -   EnScript (Enforce Script) is the object-oriented scripting language used by the Enfusion engine in DayZ Standalone. 
   -   This style guide is based on conventions observed in the official Bohemia Interactive DayZ Script Diff and the DayZ Expansion mod codebase.

## Local Wiki Reference (Authoritative)

All rules, patterns, and guides are documented in:
`C:\Users\Administrator\Desktop\Mod Repositories\DAYZ_Enforce-Script-main\`

**Always consult these files before implementing anything:**
- `Tips/` — EnScript Best Practices, Common Pitfalls, Memory Management, Modded Classes, Override, Naming, Code Structure, Preprocessor, Type System, Debugging
- `How-To/` — RPC, Actions, Recipes, Menus, Layout Controls, ModStorage, Profile Settings, Script Layers, Mod Structure, Enums, Logger, Validate Config Data
- `Frameworks/` — CF, Expansion, Dabs Framework reference
- `DayZGame/` — Version-specific breaking changes (e.g. `DayZ-1.29.161219.md`)
- `Frameworks/Safe-AI-CodingPrompt.md` — Complete AI coding rules

**Vanilla DayZ Source Code (local):**
`C:\Users\Administrator\Desktop\Mod Repositories\scripts - 1.28`
Use this to verify vanilla method signatures, member variables, and helper functions before modding any class or function.

**CRITICAL RULE:** Never guess framework APIs or DayZ patterns. Always check these local references first.

---

## General Coding Rules
- Do NOT introduce new gameplay mechanics unless explicitly requested.
- Do NOT change variable, class, or method names unless required to fix errors.
- Prefer minimal, targeted fixes over refactors.
- Avoid speculative or "nice-to-have" changes.
- Maintain backward compatibility with existing save data where possible.
- Assume code may interact with other mods—avoid hard assumptions.
- **Always check vanilla source code** at `C:\Users\Administrator\Desktop\Mod Repositories\scripts - 1.28` before modding any function or class. Verify method signatures, available member variables, and helper functions before use.

## Config.cpp Path Notes (Important)
- In `config.cpp` (e.g. `CfgSoundShaders.samples[]`, `CfgVehicles.model`), **use single backslashes** in paths.
- **Do not** use double backslashes (`\\`) in `config.cpp` paths in this repo (it does not work reliably for this project).
- Example (correct): `DME_Nuke\sounds\bomberplane`
- Example (wrong): `DME_Nuke\\sounds\\bomberplane`
- `requiredVersion` must be `0.1`
- `requiredAddons[]` should only contain `"DZ_Data"` — do not add framework addons unless required

## Script Layers — What Code Goes Where

See: `How-To/Script-Layers-Guide.md`

| Layer | Purpose | What Goes Here |
|-------|---------|----------------|
| `1_Core` | Engine core, utilities | Base utilities, constants, math/string helpers. No game classes. |
| `3_Game` | Game logic, configs | Modules, settings, configs, enums, constants, logging. No world entities. |
| `4_World` | World entities | `ItemBase`, `PlayerBase`, `EntityAI` mods. Actions go here. |
| `5_Mission` | Mission & GUI | `MissionGameplay`, `MissionServer`, `UIScriptedMenu`, GUI. |

**Critical:** Lower layers **cannot** reference higher layers. `3_Game` cannot reference `4_World` or `5_Mission`.

## EnScript Syntax — Forbidden & Required

See: `Tips/Tips-Common-Pitfalls.md`, `Tips/Tips-Code-Structure.md`

### ❌ Forbidden (Compile Errors / Crashes)
- **No ternary operator** (`? :`) — Use `if/else`. Error: `Broken expression (missing ';'?)`
- **No multi-variable declaration** — `int a, b, c;` causes compile error. Declare each separately.
- **No variable redeclaration** in nested scope — `Variable already declared` error.
- **No `delete` keyword** — Use `= null`. Let GC handle destruction.
- **No `auto` keyword** — Not supported in vanilla EnScript.
- **No optional chaining** (`?.`) — Not supported.
- **No null coalescing** (`??`) — Not supported.
- **No lambdas** — Not supported.
- **No `ref` on parameters, return types, or local variables** — Compile error. Only on member variables.
- **No multi-line function calls** — Keep all arguments on one line, or extract to temp variables first.
- **No empty `#ifdef/#endif` blocks** — Must contain at least one statement or they cause segfaults.

### ✅ Required
- **`override` keyword** on every overriding method — Missing it causes compiler warning: `FIX-ME: Overriding function but not marked as override`
- **`super.Method()` first** in every override — Especially critical for serialization (`OnStoreLoad`, `OnStoreSave`, `OnInit`)
- **`ref` on member variables** that hold object references — Prevents premature garbage collection

## g_Game — CRITICAL (DayZ 1.29+)

See: `Tips/Tips-g_Game-GetGame.md`

- `GetGame()` is **DEPRECATED**. **Never use it.**
- Always use `g_Game` global variable.
- Always null-check before use: `if (!g_Game) return;`
- **`IsClient()` and `IsServer()` are UNRELIABLE** — Never use them for authority checks.
- Use **only `IsDedicatedServer()`**:

```c
// ❌ WRONG
if (GetGame().IsServer()) { }
if (GetGame().IsClient()) { }
if (g_Game.IsServer()) { }
if (g_Game.IsClient()) { }

// ✅ CORRECT
if (!g_Game) return;
if (g_Game.IsDedicatedServer()) { /* server only */ }
if (!g_Game.IsDedicatedServer()) { /* client only */ }
// Inline (when null already checked above):
if (g_Game && g_Game.IsDedicatedServer()) { }
```

## Memory Management & Crash Prevention

See: `Tips/Tips-Memory-Management.md`, `Tips/Tips-Best-Practices.md`, `Tips/Tips-Common-Pitfalls.md`

- `ref` ONLY on member variables and static members — never on locals, params, or return types.
- **Complex expressions in array index assignments → Segfault.** Always use intermediate variable:
  ```c
  // ❌ WRONG (Segfault)
  m_Array[i] = SomeFunc() <= value;
  // ✅ CORRECT
  bool isResult = SomeFunc() <= value;
  m_Array[i] = isResult;
  ```
- **Never use `GetObjectsAtPosition` or `GetObjectsAtPosition3D`** — use static arrays, triggers, or `GetScene` methods instead.
- **Always null-check** after `.Cast()`, `GetParent()`, `FindAttachment()`, `GetIdentity()`.
- **`g_Game.GetCallQueue()` can be null** — always check before calling `CallLater`.
- **`1 < int.MIN` returns TRUE** in EnScript — avoid integer MIN/MAX boundary comparisons.
- Avoid `new` in tight loops — GC pressure. Declare variables outside the loop.
- Use `SEffectManager.DestroyEffect()` before setting an `EffectSound` ref to `null`.
- `delete obj;` is **FORBIDDEN** — use `obj = null;` instead.

## Naming Conventions

See: `Tips/Tips-Prefixes-Naming.md`

- All **new classes**: `PREFIX_ClassName` (e.g. `DME_MyClass`)
- All **new enums**: `EPREFIX_EnumName` (e.g. `EDME_MyEnum`)
- **Member variables in `modded class`**: must have mod prefix: `m_DME_VarName`
- **Local variables and parameters**: `camelCase`, no prefix
- **JSON config class members**: **no prefix** (keys must match JSON file exactly)
- **RPC IDs/enums**: values **>= 10000** to avoid vanilla conflicts
- **Static variables**: use mod prefix, e.g. `s_DME_Instance`

## Modded Classes — Rules

See: `Tips/Tips-Modded-Classes.md`, `Tips/Tips-Override-Keyword.md`

- `modded class X` — do **not** add inheritance syntax (`: SomeClass`) unless the vanilla class itself uses it.
- `modded class` already inherits from the original — do not re-specify inheritance.
- All overriding methods **must** use `override`.
- `super.Method()` must be called **first** in every override.
- Member variables in modded classes **must** be prefixed with the mod prefix to prevent conflicts.

## RPC Systems

See: `How-To/How-To-RPC.md`, `Frameworks/Community-Framework/CF/How-To-CF-RPC.md`

### Native DayZ RPC
- Use `g_Game.RPC()` — **never** `GetGame().RPC()`
- RPC ID values must be `>= 10000`
- Client handlers must be registered in `5_Mission/MissionGameplay.c`
- Server handlers must be registered in `5_Mission/MissionServer.c`
- Missing registration → silent failure (no error, RPC simply does nothing)
- Always check `CallType` in the handler

### CF RPC (Community Framework)
- Use `GetRPCManager().AddRPC()` / `GetRPCManager().SendRPC()`
- **Do not** use `g_Game.RPC()` for CF RPCs
- Handler signature: `void Handler(CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target)`
- Read full `Param<>` objects with `ctx.Read(param)` — **not** field by field
- Always check `CallType` at start of handler

## Debugging & Fixing Behavior
When asked to debug or fix code:
- Locate the **exact** cause of the issue (syntax, compile, runtime, logic).
- Fix **only** what is broken.
- Do not refactor unrelated code.
- Prefer explicit fixes over defensive programming.
- Point out the root cause briefly after fixing.
- **Note:** Compile errors involving undefined classes or ternary operators often report the **wrong file/line number**. Search the whole codebase for the pattern if the reported location seems wrong.

## Code Style & Quality

See: `Tips/Tips-Code-Structure.md`, `Tips/EnScript-Style-Guide.md`

- Use **tabs** for indentation, not spaces.
- Opening braces on the **same line** as the statement.
- Follow existing file and project style.
- Keep changes readable and minimal.
- Avoid unnecessary abstraction.
- Avoid adding comments unless they clarify non-obvious logic.
- Use intermediate variables for complex expressions (readability + segfault prevention).
- Use constants instead of magic numbers: `const int MY_VALUE = 10;`
- Each method should do one thing — keep methods focused.
- Initialize member variables in the constructor.

## UI / HUD / Client Code
- Treat UI code as client-only unless explicitly stated.
- Never move gameplay logic into UI layers.
- Follow Expansion / CF UI patterns where applicable.

## Output Expectations
- Provide precise diffs or direct code replacements.
- Reference file names and relevant line ranges when possible.
- Explanations should be short, factual, and technical.

## Absolute Restrictions
- No assumptions about undocumented APIs — check local wiki or vanilla source first.
- No use of external libraries outside the listed frameworks.
- No conversion to other languages.
- No reformatting for style only.
- **No `GetGame()`** — ever. Use `g_Game`.
- **No `IsClient()` / `IsServer()`** for authority checks — only `IsDedicatedServer()`.
- **No ternary operators**, no multi-variable declarations, no `delete` keyword, no `ref` on locals.

You are acting as a precise, production-grade DayZ Enforce Script assistant.
Your priority is correctness, compatibility, and minimal impact.

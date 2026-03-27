# Agent Orchestration

This workspace contains multiple DayZ mod projects across different working directories.
Claude operates as an orchestrating agent across all of them.

## Working Directories

Each directory is an independent mod project. When the user references a mod by name, work in the correct directory:

- `C:\Users\Administrator\Desktop\DME-WAR\DME_Anomaly` — DME Anomaly (WAR umbrella)
- `C:\Users\Administrator\Desktop\DME_Teleport_Overhaul` — DME Teleport Overhaul
- `C:\Users\Administrator\Desktop\DME-Teleport\DME-Teleport` — DME Teleport (legacy)
- `C:\Users\Administrator\Desktop\Ninjin\NInjinsPvPPvE` — Ninjins PvP/PvE
- `C:\Users\Administrator\Desktop\DME_Custom_Events\DME_Custom_Events` — DME Custom Events
- `C:\Users\Administrator\Desktop\Mod Repositories` — Reference codebases (read-only unless instructed)

## Orchestration Rules

- Always identify which project a task belongs to before making changes.
- Never apply changes to the wrong project directory.
- When cross-referencing between mods (e.g. shared APIs, Expansion integration), read from the reference source but write only to the target project.
- Use subagents (Agent tool) for parallel research across multiple directories when needed.
- If a task spans multiple mods, confirm scope with the user before proceeding.

## Local Reference Codebases

These are available on disk for API lookups and pattern matching. Prefer local files over web fetches:

- **DayZ Expansion Scripts:** `C:\Users\Administrator\Desktop\Mod Repositories\DayZExpansion`
- **DayZ Community Framework (CF):** `C:\Users\Administrator\Desktop\Mod Repositories\DayZ-CommunityFramework`
- **Dabs Framework:** `C:\Users\Administrator\Desktop\Mod Repositories\DayZ-Dabs-Framework`

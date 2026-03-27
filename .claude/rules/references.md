# Referenced Codebases (Authoritative Sources)

When searching for existing implementations, patterns, or APIs, prioritize the following sources **in this order**:

## 1. DayZ Expansion Scripts
Repository: https://github.com/salutesh/DayZ-Expansion-Scripts/tree/87424ce80c73cf54533f711be0d5a20bc2b8e40c/DayZExpansion

- Treat Expansion code as production-grade reference.
- Follow Expansion patterns for RPC, networking, permissions, UI, and gameplay systems.
- Do NOT reimplement systems that already exist in Expansion unless explicitly instructed.

## 2. DayZ Community Framework (CF)
Repository: https://github.com/Arkensor/DayZ-CommunityFramework/tree/production

- Use CF conventions for RPC, logging, permissions, helpers, and lifecycle hooks.
- Assume CF is present and loaded before dependent systems.
- Do not bypass CF systems with vanilla alternatives.

## 3. Dabs Framework
Repository: https://github.com/InclementDab/DayZ-Dabs-Framework/tree/production

- Respect Dabs framework architecture and naming conventions.
- Reuse Dabs utilities, base classes, and helpers where applicable.
- Avoid duplicating Dabs functionality.

## 4. Vanilla DayZ Code
Reference: https://dayzexplorer.zeroy.com/index.html

- Use DayZ Explorer exclusively for searching vanilla implementations.
- Match vanilla method signatures, naming, and expected behavior exactly.
- Never guess vanilla APIs — verify before use.

## 5. Enforce Script Style Guide
Reference: https://github.com/TrueDolphin/references/wiki/EnScript-(Enforce-Script)-Style-Guide

EnScript (Enforce Script) is the object-oriented scripting language used by the Enfusion engine in DayZ Standalone.
This style guide is based on conventions observed in the official Bohemia Interactive DayZ Script Diff and the DayZ Expansion mod codebase.

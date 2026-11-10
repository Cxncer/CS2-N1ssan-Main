# n1ssan

A Counter-Strike 2 customization tool for managing item collections and equipment loadouts.

## Features

- **Armory** — Browse and manage your item collection.
- **Loadout** — Configure the items assigned to your equipment slots.
- **DOMINATE** — Launcher and entry point for the application.
- **Native runtime** — Connects the application to CS2.
- **Web Bridge** — Handles communication between the interfaces and native runtime.

## Components

| Component | Purpose |
| --- | --- |
| `n1ssan-launcher` | Windows launcher and application lifecycle |
| `n1ssan-injectorv2` | Native DLL injection |
| `CS2-n1ssan` | Native runtime for Armory and Loadout |
| `n1ssan-web-bridge` | Web interfaces and runtime communication |

**Status:** Beta

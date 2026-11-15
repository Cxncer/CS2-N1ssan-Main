<div align="center">

  <p>
    <a href="https://github.com/Cxncer/CS2-N1ssan-Main/releases"><img src="https://img.shields.io/github/v/release/Cxncer/CS2-N1ssan-Main?include_prereleases&style=for-the-badge&label=Latest%20Release" alt="Latest Release"></a>
    <a href="https://github.com/Cxncer/CS2-N1ssan-Main/releases/latest"><img src="https://img.shields.io/github/v/release/Cxncer/CS2-N1ssan-Main?display_name=release&style=for-the-badge&label=Current%20Version&color=8A2BE2" alt="Current Version"></a>
    <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows">
    <img src="https://img.shields.io/badge/Status-Beta-orange?style=for-the-badge" alt="Beta">
  </p>

  <p>
    <img src="https://img.shields.io/badge/React-19-61DAFB?style=for-the-badge&logo=react&logoColor=black" alt="React">
    <img src="https://img.shields.io/badge/TypeScript-5-3178C6?style=for-the-badge&logo=typescript&logoColor=white" alt="TypeScript">
    <img src="https://img.shields.io/badge/Vite-8-646CFF?style=for-the-badge&logo=vite&logoColor=white" alt="Vite">
    <img src="https://img.shields.io/badge/Node.js-20+-339933?style=for-the-badge&logo=nodedotjs&logoColor=white" alt="Node.js">
    <img src="https://img.shields.io/badge/C%2B%2B-Native-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++">
    <img src="https://img.shields.io/badge/C%23-.NET%208-512BD4?style=for-the-badge&logo=dotnet&logoColor=white" alt="C# / .NET 8">
    <img src="https://img.shields.io/badge/Rust-Stable-000000?style=for-the-badge&logo=rust&logoColor=white" alt="Rust">
    <img src="https://img.shields.io/badge/Tauri-Desktop-FFC131?style=for-the-badge&logo=tauri&logoColor=black" alt="Tauri">
  </p>

  <img src="https://raw.githubusercontent.com/Cxncer/n1ssan-launcher/95b4050a7204772d3af12f8595303f6e2fe493d2/assets/icons/n1ssan-black.png" alt="N1ssan logo" width="160">

  # N1ssan

  **A modular Counter-Strike 2 inventory ecosystem.**

  Inventory ownership, loadout management, native runtime integration, and desktop orchestration — built as one connected system.

  [Overview](#-overview) · [Features](#-the-ecosystem) · [Instructions](#instructions) · [Documentation](#-documentation)

</div>


<div align="center">
  <img src="https://raw.githubusercontent.com/Cxncer/CS2-N1ssan-Main/solid-safe-house/assets/Risksaw%20in%20Focus.jpg" alt="N1ssan — Risksaw in Focus" width="100%">
</div>

## <img src="https://api.iconify.design/lucide/sparkles.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Overview

**N1ssan** is a Windows-focused Counter-Strike 2 project built from independent components with clearly defined responsibilities.

The main workspace brings together a desktop launcher, native runtime, local web bridge, inventory and loadout systems, diagnostics, and maintenance tooling. Each component has its own role; shared contracts keep them working together.

## <img src="https://api.iconify.design/lucide/workflow.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> The ecosystem

<table>
  <tr>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/backpack.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Armory</h3>
      <p>Persistent inventory ownership built around individual item instances. Supports weapon skins, stickers, charms, inventory profiles, and equipment assignments.</p>
    </td>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/puzzle.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Loadout</h3>
      <p>Catalogue-based equipment selection and saved profiles, kept separate from owned inventory.</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/monitor.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> DOMINATE</h3>
      <p>Windows desktop launcher coordinating game detection, Bridge startup, injection, and runtime readiness.</p>
    </td>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/globe.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Web Bridge &amp; Obsidian</h3>
      <p>Local API and React interfaces for inventory workflows, browser-based controls, and the Obsidian design system.</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/radio-tower.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Stormwatch</h3>
      <p>Unified diagnostics for launcher, loader, Bridge, and native runtime health.</p>
    </td>
    <td width="50%" valign="top">
      <h3><img src="https://api.iconify.design/lucide/satellite.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Apex / N1ssan Sync</h3>
      <p>Tauri desktop utility for live signature scanning and versioned offset dataset maintenance.</p>
    </td>
  </tr>
</table>

## <img src="https://api.iconify.design/lucide/blocks.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Repositories

| Component | Repository | Purpose |
| --- | --- | --- |
| Main workspace | [CS2-N1ssan-Main](https://github.com/Cxncer/CS2-N1ssan-Main) | Submodules, shared contracts, build and release orchestration |
| DOMINATE | [n1ssan-launcher](https://github.com/Cxncer/n1ssan-launcher) | Windows desktop launcher |
| Injector v2 | [n1ssan-injectorv2](https://github.com/Cxncer/n1ssan-injectorv2) | Native DLL loading |
| Native runtime | [CS2-n1ssan](https://github.com/Cxncer/CS2-n1ssan) | CS2 integration and inventory application |
| Web Bridge | [n1ssan-web-bridge](https://github.com/Cxncer/n1ssan-web-bridge) | Local API and web interfaces |
| Apex / N1ssan Sync | `development/n1ssan-apex` | Signature and offset maintenance utility |

## <img src="https://api.iconify.design/lucide/workflow.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> System Flow

```text
Counter-Strike 2
       ↓
DOMINATE Launcher
(game detection and orchestration)
       ↓
Injector
       ↓
Native N1ssan Runtime
       ↕
Local Web Bridge
       ↓
Obsidian Interface
       ↓
Armory Inventory  ↔  Loadout Profiles
```

DOMINATE coordinates game detection, Bridge startup, and injection. The injector loads the native runtime, which works alongside the local Web Bridge. Obsidian provides the interface for managing owned items through Armory and configuring equipment through Loadout. Stormwatch monitors component health.

## <img src="https://api.iconify.design/lucide/list-ordered.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Instructions

1. Launch **Counter-Strike 2** and wait for the main menu to appear.
2. Launch `n1ssan-launcher.exe` and wait for injection to finish.
3. Click **DOMINATE** once it lights up.
4. Choose your desired mode.
5. Configure your **Inventory / Loadout** system.



- **Ownership stays distinct from equipment.** Armory owns item instances; equipment assignments reference those instances. Loadout remains a separate catalogue-selection workflow.
- **Contracts are versioned.** Persisted documents and runtime status records use explicit schemas and protocol versions.
- **User data is protected.** Revision checks, backup and recovery paths, and data outside release packages help protect local inventory and configuration.
- **Runtime health is explicit.** Stale or unknown states are never reported as ready.
- **Builds are not acceptance tests.** Integration checks validate document compatibility; live in-game behavior requires separate verification.
- **Offset updates are controlled.** Apex scans the running game read-only; applying updates versions the dataset and preserves a backup.

## <img src="https://api.iconify.design/lucide/book-open.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Documentation

| Guide | What it covers |
| --- | --- |
| [DEVELOPMENT.md](DEVELOPMENT.md) | Workspace setup and development workflow |
| [RELEASE.md](RELEASE.md) | Packaging, manifests, and release verification |
| [contracts.md](contracts.md) | Shared component contracts and data formats |
| [failure-taxonomy.md](failure-taxonomy.md) | Standardized failure codes |
| [operation-roadmap.md](operation-roadmap.md) | Project milestones and current work |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Branch and contribution workflow |
| [LICENSE](LICENSE) | Proprietary use and distribution terms |

## <img src="https://api.iconify.design/lucide/package.svg?color=%2300ff88" width="18" height="18" alt="" align="absmiddle"> Releases

The workspace version is tracked in [`VERSION`](VERSION). The current recorded version is **Beta.Build.v1.0.0**.

Release archives use the format:

```text
n1ssan-<version>-win64.zip
```

Each release includes a staged runtime layout and a manifest recording component commits, contract versions, and artifact SHA-256 hashes. See [RELEASE.md](RELEASE.md) for the full process.

---

<div align="center">

  **N1ssan — Solid Safe House**

  *Build. Validate. Ship.*

  © 2026 Cxncer. All rights reserved.

</div>

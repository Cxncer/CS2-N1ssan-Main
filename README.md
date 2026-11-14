<div align="center">

  <img src="https://raw.githubusercontent.com/Cxncer/n1ssan-web-bridge/main/assets/images/Rickshaw-clay.jpg" alt="n1ssan" width="100%">

  # n1ssan

  **A modular Counter-Strike 2 ecosystem.**

  Inventory ownership, loadout management, native runtime integration, and desktop orchestration — built as one connected system.

  <p>
    <a href="https://github.com/Cxncer/CS2-N1ssan-Main/releases"><img src="https://img.shields.io/github/v/release/Cxncer/CS2-N1ssan-Main?include_prereleases&style=for-the-badge&label=Latest%20Release" alt="Latest Release"></a>
    <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white" alt="Windows">
    <img src="https://img.shields.io/badge/Status-Beta-orange?style=for-the-badge" alt="Beta">
  </p>

  [Overview](#-overview) · [Features](#-the-ecosystem) · [Architecture](#-architecture) · [Getting Started](#-getting-started) · [Documentation](#-documentation)

</div>


## ✨ Overview

**n1ssan** is a Windows-focused Counter-Strike 2 project built from independent components with clearly defined responsibilities.

The main workspace brings together a desktop launcher, native runtime, local web bridge, inventory and loadout systems, diagnostics, and maintenance tooling. Each component has its own role; shared contracts keep them working together.

## ⚡ The ecosystem

<table>
  <tr>
    <td width="50%" valign="top">
      <h3>🎒 Armory</h3>
      <p>Persistent inventory ownership built around individual item instances. Supports weapon skins, stickers, charms, inventory profiles, and equipment assignments.</p>
    </td>
    <td width="50%" valign="top">
      <h3>🧩 Loadout</h3>
      <p>Catalogue-based equipment selection and saved profiles, kept separate from owned inventory.</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top">
      <h3>🖥️ DOMINATE</h3>
      <p>Windows desktop launcher coordinating game detection, Bridge startup, injection, and runtime readiness.</p>
    </td>
    <td width="50%" valign="top">
      <h3>🌐 Web Bridge &amp; Obsidian</h3>
      <p>Local API and React interfaces for inventory workflows, browser-based controls, and the Obsidian design system.</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top">
      <h3>📡 Stormwatch</h3>
      <p>Unified diagnostics for launcher, loader, Bridge, and native runtime health.</p>
    </td>
    <td width="50%" valign="top">
      <h3>🛰️ Apex / N1ssan Sync</h3>
      <p>Tauri desktop utility for live signature scanning and versioned offset dataset maintenance.</p>
    </td>
  </tr>
</table>

## 🏗️ Architecture

The workspace is organized around **DOMINATE**, which coordinates the application lifecycle, and the **Web Bridge**, which exposes local APIs and browser-based workflows. **Injector v2** loads the native CS2 runtime. The supporting systems provide inventory, equipment, diagnostics, and maintenance capabilities.

```mermaid
flowchart TB
    User["User"] --> Dominate["DOMINATE<br/>Windows launcher"]

    subgraph Workspace["n1ssan workspace"]
        Dominate --> Bridge["Web Bridge<br/>Local API"]
        Dominate --> Injector["Injector v2<br/>Native loader"]

        Bridge --> Obsidian["Obsidian<br/>React interface"]
        Bridge --> Armory["Armory<br/>Owned inventory"]
        Bridge --> Loadout["Loadout<br/>Equipment profiles"]

        Injector --> Runtime["Native CS2 DLL<br/>In-game integration"]
        Runtime --> CS2["Counter-Strike 2"]

        Stormwatch["Stormwatch<br/>Diagnostics"] -.-> Dominate
        Stormwatch -.-> Bridge
        Stormwatch -.-> Injector
        Stormwatch -.-> Runtime

        Apex["Apex / N1ssan Sync<br/>Signature & offset maintenance"]
    end

    Contracts["Shared contracts & versioned data"]
    Contracts --- Dominate
    Contracts --- Bridge
    Contracts --- Runtime
    Contracts --- Armory
    Contracts --- Loadout

    classDef core fill:#17212b,stroke:#64748b,color:#f8fafc
    classDef support fill:#202b36,stroke:#475569,color:#e2e8f0
    class Dominate,Bridge,Injector,Runtime,CS2 core
    class Obsidian,Armory,Loadout,Stormwatch,Apex,Contracts support
```

**Runtime flow:** DOMINATE starts and coordinates the local components. The Bridge serves the web application and inventory workflows; Injector v2 loads the native DLL into CS2. Stormwatch observes component health, while Apex remains a separate maintenance utility for signature and offset data.
 
## 🧰 Technology

<p align="center">
  <img src="https://skillicons.dev/icons?i=cpp,cs,ts,react,rust,tauri,vite,windows&perline=8" alt="C++, C#, TypeScript, React, Rust, Tauri, Vite, and Windows">
</p>

| Area | Stack |
| --- | --- |
| Native runtime | C++ |
| Desktop launcher | C# · WPF · .NET |
| Web application | React · TypeScript · Vite |
| Desktop maintenance utility | Rust · Tauri |
| Platform | Windows |

## 🧱 Repositories

| Component | Repository | Purpose |
| --- | --- | --- |
| Main workspace | [CS2-N1ssan-Main](https://github.com/Cxncer/CS2-N1ssan-Main) | Submodules, shared contracts, build and release orchestration |
| DOMINATE | [n1ssan-launcher](https://github.com/Cxncer/n1ssan-launcher) | Windows desktop launcher |
| Injector v2 | [n1ssan-injectorv2](https://github.com/Cxncer/n1ssan-injectorv2) | Native DLL loading |
| Native runtime | [CS2-n1ssan](https://github.com/Cxncer/CS2-n1ssan) | CS2 integration and inventory application |
| Web Bridge | [n1ssan-web-bridge](https://github.com/Cxncer/n1ssan-web-bridge) | Local API and web interfaces |
| Apex / N1ssan Sync | `development/n1ssan-apex` | Signature and offset maintenance utility |

## 🚀 Getting started

### Requirements

- Windows
- Git
- The toolchains required by the individual components — see [DEVELOPMENT.md](DEVELOPMENT.md)

### Clone the workspace

```powershell
git clone --recurse-submodules https://github.com/Cxncer/CS2-N1ssan-Main.git
cd CS2-N1ssan-Main
```

### Verify the workspace

```powershell
n1ssan-build.bat verify
```

This runs the workspace build-and-check workflow without creating a release package. Follow the development guide for component-specific prerequisites and commands.

## 🔒 Engineering principles

- **Ownership stays distinct from equipment.** Armory owns item instances; equipment assignments reference those instances. Loadout remains a separate catalogue-selection workflow.
- **Contracts are versioned.** Persisted documents and runtime status records use explicit schemas and protocol versions.
- **User data is protected.** Revision checks, backup and recovery paths, and data outside release packages help protect local inventory and configuration.
- **Runtime health is explicit.** Stale or unknown states are never reported as ready.
- **Builds are not acceptance tests.** Integration checks validate document compatibility; live in-game behavior requires separate verification.
- **Offset updates are controlled.** Apex scans the running game read-only; applying updates versions the dataset and preserves a backup.

## 📚 Documentation

| Guide | What it covers |
| --- | --- |
| [DEVELOPMENT.md](DEVELOPMENT.md) | Workspace setup and development workflow |
| [RELEASE.md](RELEASE.md) | Packaging, manifests, and release verification |
| [contracts.md](contracts.md) | Shared component contracts and data formats |
| [failure-taxonomy.md](failure-taxonomy.md) | Standardized failure codes |
| [operation-roadmap.md](operation-roadmap.md) | Project milestones and current work |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Branch and contribution workflow |

## 📦 Releases

The workspace version is tracked in [`VERSION`](VERSION). The current recorded version is **Beta.Build.v1.0.0**.

Release archives use the format:

```text
n1ssan-<version>-win64.zip
```

Each release includes a staged runtime layout and a manifest recording component commits, contract versions, and artifact SHA-256 hashes. See [RELEASE.md](RELEASE.md) for the full process.

---

<div align="center">

  **n1ssan — Solid Safe House**

  *Build. Validate. Ship.*

</div>

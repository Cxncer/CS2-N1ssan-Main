# n1ssan

**A modular Counter-Strike 2 desktop ecosystem.**

n1ssan is a Windows-focused project built around a native CS2 integration layer, a desktop launcher, a web-based bridge, and supporting tools for inventory management, loadouts, diagnostics, and runtime maintenance.

The project is developed as a set of cooperating components rather than one monolithic application. Its guiding principles are clear boundaries, explicit contracts, persistent user data, reproducible builds, and validation against the real runtime.

> **Development status:** Active development. The project is preparing for its beta milestone. Features and workflows may change before the first beta release.

---

## Project at a glance

| Component | Responsibility |
| --- | --- |
| **DOMINATE** | Desktop launcher and application orchestration |
| **Native CS2 integration** | Runtime integration and in-game application |
| **n1ssan Web Bridge** | Local web interface and API bridge |
| **Armory** | Persistent inventory ownership, item instances, and attachments |
| **Loadout** | Equipment and loadout management |
| **Stormwatch** | Runtime diagnostics, health reporting, and failure visibility |
| **Apex / N1ssan Sync** | Signature and offset discovery, maintenance, and validation |
| **Obsidian** | Shared interface styling and reusable UI components |

The components communicate through defined interfaces and versioned contracts. The goal is to keep each subsystem independently understandable, testable, and maintainable.

## Architecture

At a high level, the application is organized like this:

```text
n1ssan
├── DOMINATE — Desktop launcher
│   ├── Web Bridge — Local API and web UI
│   ├── Injector v2 — Native module loading
│   └── Native CS2 integration
│       ├── Armory — Inventory ownership and attachments
│       ├── Loadout — Equipment selection
│       └── Stormwatch — Runtime diagnostics
│
└── Apex — N1ssan Sync
    └── Signature and offset maintenance
```

This is a conceptual view of the system, not a literal repository directory listing.

## Core systems

### Armory

Armory provides the persistent inventory foundation. It distinguishes owned item instances from the items selected for a loadout, and supports item metadata and attachments.

### Loadout

Loadout handles equipment selection separately from inventory ownership. Keeping these responsibilities distinct helps prevent selection state from being confused with the underlying inventory.

### Obsidian

Obsidian is the shared visual language for the n1ssan interface. It provides reusable components, design tokens, and a consistent dark industrial presentation.

### Stormwatch

Stormwatch focuses on runtime visibility: health reporting, diagnostics, failure classification, schema snapshots, and signature-related health information.

### Apex — N1ssan Sync

Apex is the supporting Windows utility for maintaining CS2 signatures and offsets. Its workflow includes scanning, resolving, comparing results, and preparing dataset updates. Runtime validation and confidence reporting remain important safeguards: a discovered value should not be treated as verified solely because a scan found it.

## Engineering principles

- **Preserve user data.** Migrations and updates must protect existing inventory and configuration.
- **Keep ownership and equipment separate.** An owned item is not automatically an equipped item.
- **Use explicit contracts.** Cross-component data shapes and versions should be deliberate.
- **Validate runtime behavior.** A successful build or simulation is not proof of in-game correctness.
- **Prefer reproducible releases.** Builds, versions, and release artifacts should be traceable.
- **Make failures visible.** Diagnostics should explain what failed and where possible why.
- **Keep the baseline safe.** Changes should preserve the known-good Solid Safe House baseline unless a deliberate migration is planned.

## Development status

The project has completed major foundation work across its launcher, inventory, interface, diagnostics, integration, and release infrastructure. Current development is focused on Apex and the remaining validation needed for a reliable beta.

A feature should only be considered complete when its expected behavior has been verified at the appropriate level. In particular, simulated tests and live CS2 acceptance are different forms of evidence.

## Repository ecosystem

The n1ssan project is maintained across multiple repositories. The main repository is the coordination point for the project; implementation details and component-specific instructions belong with their respective repositories.

- [Main repository](https://github.com/Cxncer/CS2-N1ssan-Main)
- [Native CS2 project](https://github.com/Cxncer/CS2-n1ssan)
- [Web Bridge](https://github.com/Cxncer/n1ssan-web-bridge)
- [Desktop Launcher](https://github.com/Cxncer/n1ssan-launcher)

Repository names and component boundaries may evolve as the project is prepared for beta.

## Building and running

Build and run instructions are maintained with the individual components because each has its own toolchain and prerequisites. Before building, check the component repository's current instructions and use the matching configuration for the branch you are working on.

Do not assume that a development build is equivalent to the packaged application. Release candidates should be tested from their actual packaged artifacts.

## Beta readiness

The beta preparation checklist includes:

- [ ] Complete the current cleanup and review changes for unintended behavior changes.
- [ ] Run the relevant automated tests and integration checks.
- [ ] Verify clean Release builds for the supported components.
- [ ] Test the packaged launcher and complete startup/shutdown lifecycle.
- [ ] Perform live CS2 acceptance for the supported Armory and Loadout workflows.
- [ ] Verify persistence across application and CS2 restarts.
- [ ] Confirm version and manifest consistency across release artifacts.
- [ ] Preserve backups and validate upgrade behavior before distributing updates.
- [ ] Document known limitations and unresolved issues.

## Project status and roadmap

The project roadmap is maintained separately from this overview so that operation-level plans, implementation notes, and completion evidence can evolve without making this README the technical source of truth.

---

**n1ssan — Solid Safe House**

Build. Validate. Ship.

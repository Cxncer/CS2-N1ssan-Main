# n1ssan — Operation Roadmap

Project: n1ssan
Baseline: Solid Safe House
Last audited: 2026-11-11
Purpose: Record approved operations, verified implementation, outstanding work, and future direction.

## Status Definitions

- **Complete:** The operation's defined objectives have been implemented.
- **Implemented:** Core functionality exists, with operational acceptance or remaining integration work still possible.
- **Active:** Currently under development.
- **In Progress:** Foundational implementation exists; objectives remain unfinished.
- **Planned:** Approved direction without verified implementation.
- **Unverified:** Insufficient evidence to claim completion.

---

## Operation 00 — Workspace Migration

**Status: COMPLETE**

### Objective

Establish a reliable development workspace, repository organization, recovery structure, and preserved project baseline.

### Verified implementation

- [x] Separated the main project and component repositories.
- [x] Established five component repositories as Git submodules: `CS2-n1ssan`, `n1ssan-web-bridge`, `n1ssan-launcher`, `n1ssan-injectorv2`, and `n1ssan-gloves`.
- [x] Created the project structure inventory.
- [x] Established independent Solid Safe House branches for the main repository and component repositories.
- [x] Published the local working state as the preserved baseline.
- [x] Removed the obsolete legacy injector submodule reference.
- [x] Restored the main project README.

### Outcome

The Solid Safe House branches are the designated project baseline. Previous GitHub branches remain historical references. The main repository's current `.gitmodules` records the five component submodules.

---

## Operation 01 — Overdrive

**Status: COMPLETE**

### Objective

Establish DOMINATE as the desktop execution and communication layer connecting the launcher, web bridge, injector, and native DLL.

### Verified implementation

- [x] WPF desktop launcher and standalone injector separation.
- [x] Dedicated injector v2 project; the obsolete injector is explicitly marked historical.
- [x] Packaged runtime discovery and development-layout discovery.
- [x] Bridge process startup, health polling, output/error capture, and startup timeout handling.
- [x] Packaged Bridge runtime discovery using the bundled Node runtime, Bridge server, and built web assets.
- [x] Development Bridge and Vite server startup paths.
- [x] Injector process management and machine-readable `RESULT=` outcomes.
- [x] DLL dependency and architecture inspection before injection.
- [x] Injector waits for CS2, `client.dll`, and the game window/settle period before injection.
- [x] Injector confirms the module is loaded and waits for a fresh DLL READY heartbeat.
- [x] DLL readiness status integration and process-specific status reporting.
- [x] Runtime bridge and browser-session management.
- [x] Launcher startup state machine covering game detection, Bridge, Web, loader, DLL initialization, READY, and failure states.

### Remaining validation

- [ ] Re-test the complete packaged launch workflow from a clean installation.
- [ ] Verify startup, failure, recovery, restart, and shutdown behavior together.
- [ ] Confirm all packaged component paths against the intended release layout.
- [ ] Verify behavior when CS2 exits or changes process during each startup stage.
- [ ] Confirm the launcher and injector recover cleanly from stale or missing status records.

---

## Operation 02 — Armory

**Status: IMPLEMENTED**

### Objective

Build a persistent inventory and equipment system that separates owned items from equipped selections.

### Verified implementation

- [x] Canonical Armory inventory data contract, schema version 1.
- [x] Inventory profiles and individual item instances.
- [x] Structural validation and revision conflict protection.
- [x] Persistent storage with backup recovery and preservation of corrupt primary data for inspection.
- [x] Skin, sticker, and charm inventory projection.
- [x] Inventory-to-native-runtime document generation, version 2 with attachment projection version 1.
- [x] Separate equipment assignment documents tied to Armory instance IDs, inventory revision, and active profile.
- [x] Loadout profile persistence and legacy Loadout migration.
- [x] Runtime mode separation between Loadout and Armory.
- [x] Live schema and paint-kit compatibility validation.
- [x] Native inventory watcher and SOCache item creation/update/removal path.
- [x] Stable native item identity across unchanged inventory re-reads.
- [x] Native attachment attribute handling for stickers and charms.
- [x] Native apply integration for CS2's own sticker/charm application flow.
- [x] Sticker placement and charm customization data structures.
- [x] Add to Inventory rarity/grade text coloring for weapon skins, stickers, and charms.
- [x] User-reported Web Bridge build completed after the latest rarity-coloring changes; Vite reported the existing large-chunk advisory.

### Remaining work

- [ ] Complete in-game inventory application validation.
- [ ] Verify item persistence and identity across repeated updates and CS2 restarts.
- [ ] Validate attachment application, readback, and rendering in-game.
- [ ] Resolve the remaining Charm entity-discovery/readback investigation.
- [ ] Test equipment assignments across T and CT, including duplicate owned instances.
- [ ] Verify item identity and metadata across catalogue, Armory, Bridge projection, and native runtime.
- [ ] Test edge cases involving unsupported, unresolved, or stale items.
- [ ] Establish reliable end-to-end acceptance tests.

### Important distinction

Loadout represents catalogue selections and equipment intent. Armory represents owned item instances. Equipment assignments reference owned Armory instances. These are separate data sources and must remain separate.

The integration simulator validates local document hand-off only; it does not attach to CS2 or prove successful in-game application.

---

## Operation 03 — Obsidian

**Status: COMPLETE**

### Objective

Establish a unified, minimal, dark industrial design system across the n1ssan interface.

### Verified foundation

- [x] Dedicated Obsidian Armory application and visual identity.
- [x] Dark industrial visual treatment, catalogue and inventory layouts.
- [x] Armory navigation, search, filtering, sorting, inventory cards, and dialogs.
- [x] Shared visual patterns within the Armory interface, including rarity/grade presentation.
- [x] Separate Loadout interface and redesign stylesheets exist.

### Remaining work

- [ ] Establish shared design tokens and reusable component standards across Armory, Loadout, and launcher-facing web surfaces.
- [ ] Unify navigation and interaction patterns.
- [ ] Align catalogue, inventory, and Loadout presentation without conflating their separate workflows.
- [ ] Standardize responsive layouts.
- [ ] Standardize loading, empty, error, disabled, and success states.
- [ ] Reduce visual inconsistency between application sections.

### Completion criteria

A cohesive interface with reusable components and consistent behavior across the application, rather than a polished Armory surface alongside independently styled sections.

---

## Operation 04 — Stormwatch

**Status: FOUNDATION IMPLEMENTED**

### Objective

Provide reliable diagnostics, runtime health reporting, schema inspection, and signature/offset visibility.

### Verified foundation

- [x] Structured DLL diagnostic logging.
- [x] Per-process runtime status snapshots under `%LOCALAPPDATA%\\n1ssan\\status`.
- [x] Protocol-versioned status records with process ID, timestamp, build ID, state, and reason.
- [x] INITIALIZING, CHECKS_PASSED, READY, DEGRADED, and FAILED states.
- [x] Atomic status-file replacement.
- [x] Runtime heartbeat every 10 seconds.
- [x] Stale status guidance: a missing or older-than-30-seconds heartbeat is unknown/stale, not READY.
- [x] Live CS2 schema snapshot export, version 2.
- [x] Paint-kit and item compatibility reporting using live definitions and generated-material availability.
- [x] Startup schema preflight before SOCache creation and hook installation.
- [x] Existing scanner and offset-related modules.
- [x] Session logging reset at the start of each DLL injection session.

### Remaining work

- [ ] Consolidate diagnostics into a unified reporting workflow.
- [ ] Improve offset and signature health reporting.
- [ ] Establish consistent failure classification across launcher, injector, Bridge, and DLL.
- [ ] Define recovery and stale-data handling across all consumers.
- [ ] Validate diagnostic information across the complete application lifecycle.
- [ ] Confirm that status-harness expectations match the current protocol and state transitions.

---

## Operation Clean State

**Status: COMPLETE**

### Objective

Complete the focused cleanup and stabilization pass across runtime logging, configuration layout, Bridge runtime discovery, Loadout migration, and inventory rarity presentation.

### Completed work

- [x] Reset session logging at the start of each DLL injection session.
- [x] Build the native DLL in x64 Release configuration; completion was reported by the user.
- [x] Migrate runtime configuration files into the designated configs directory.
- [x] Verify the Web Bridge runtime path against the updated configuration layout.
- [x] Migrate the legacy Loadout data into a separate profile while preserving the existing active profile and assignments.
- [x] Verify the migrated profile and its 20 assignments; the existing `main-loadout` profile remained active and unchanged.
- [x] Apply rarity/grade-based text coloring in the Add to Inventory dialog for weapon skins, stickers, and charms.

### Outcome

The cleanup objectives were completed and the existing user data and active Loadout profile were preserved. Deferred investigations and optional follow-up checks were not treated as blockers for this operation.

---

## Operation 05 — Reforge

**Status: ACTIVE**

### Objective

Standardize and validate integration contracts across all n1ssan components.

### Verified foundation

- [x] Canonical Armory inventory contract.
- [x] Separate equipment assignment contract.
- [x] Loadout runtime projection.
- [x] Armory inventory projection.
- [x] Local integration snapshot, schema version 3.
- [x] Integration simulator with ready, empty, stale, and unsupported states.
- [x] Revision and active-profile consistency validation.
- [x] Assignment-to-owned-instance validation.
- [x] Explicit separation between catalogue selections, owned instances, and equipment assignments.
- [x] Bridge API clients validate response documents before accepting them.
- [x] Test files cover Armory contracts, stores, Bridge clients, equipment, Loadout adapters, runtime configuration, and integration simulation.

### Remaining work

- [ ] Document all component communication contracts in one maintained reference.
- [ ] Validate contract compatibility across repositories and packaged component revisions.
- [ ] Run and record the complete automated integration test suite.
- [ ] Complete end-to-end API/UI verification; the previous API/UI verification pass was explicitly skipped.
- [ ] Verify packaged runtime paths and file ownership.
- [ ] Define versioning and migration rules for every persisted document.
- [ ] Test failure and recovery behavior across component boundaries.

### Important distinction

The schema-v3 integration snapshot and simulator establish a local contract-validation layer. They do not replace live launcher-to-Bridge-to-DLL-to-CS2 acceptance testing.

---

## Operation 06 — Atlas

**Status: IN PROGRESS**

### Objective

Standardize development, repository management, build processes, and release workflows.

### Verified foundation

- [x] Six-repository project topology: the main repository plus five component repositories.
- [x] Independent Solid Safe House branches for the project repositories.
- [x] Main repository submodule references aligned with the five component repositories.
- [x] Project structure documentation.
- [x] Dedicated build directories for Bridge, DLL, and launcher.
- [x] Existing launcher packaging and development/production runtime discovery logic.
- [x] Packaged production Bridge build script and runtime dependency installation workflow.
- [x] Existing component-specific build projects and scripts.

### Remaining work

- [ ] Standardize build and release procedures across all components.
- [ ] Define artifact naming, versioning, and release compatibility rules.
- [ ] Establish reproducible release packaging.
- [ ] Document development environment requirements.
- [ ] Establish repository contribution and maintenance standards.
- [ ] Verify clean-machine builds and deployment workflows.
- [ ] Define a repeatable process for updating submodule pointers and validating the combined baseline.

---

## Operation 07 — Apex

**Status: PLANNED**

### Objective

Develop a controlled process for detecting outdated definitions, discovering updated signatures and offsets, and validating findings.

### Existing groundwork

- [x] Memory scanning modules.
- [x] Offset configuration modules.
- [x] Automatic-offset-related source modules.
- [x] Live schema diagnostics and schema preflight.

### Planned direction

- [ ] Detect missing or outdated definitions.
- [ ] Discover candidate signatures and offsets.
- [ ] Validate candidates against live runtime information.
- [ ] Report confidence and validation results.
- [ ] Maintain versioned offset records.
- [ ] Prevent unverified findings from reaching production configuration.

### Completion criteria

A repeatable discovery and validation workflow with explicit failure reporting and human-verifiable results.

---

## Project-wide Principles

1. The local Solid Safe House baseline is the preserved reference state.
2. Component responsibilities must remain clearly separated.
3. Inventory ownership and equipment selection must not be conflated.
4. Runtime contracts must be explicit and versioned.
5. A source file or module existing does not prove its feature works.
6. Runtime claims require appropriate testing evidence.
7. Failed or incomplete operations must not be represented as complete.
8. Planned features must remain distinguishable from implemented features.
9. Local integration simulation is not equivalent to successful in-game validation.
10. Preserve existing user data and active Loadout profiles during migrations and maintenance.

---

## Current Priority

**Operation 05 — Reforge**

Armory has reached the Implemented milestone. Its remaining in-game acceptance work is recorded as follow-up validation rather than a blocker to moving the operation sequence forward.

The next priorities are:

1. **Reforge** — complete API/UI and cross-component integration verification.
2. **Stormwatch** — consolidate diagnostics and align the status harness with the current contract.
3. **Atlas** — establish reproducible builds and release packaging.
4. **Apex** — develop the controlled discovery-and-validation workflow.

Overdrive, Obsidian, and Clean State are accepted as complete. Any future hardening or polish can be tracked as follow-up work without reopening those operations unless scope materially changes.

Solid Safe House remains the preserved development baseline.

---

## Audit Notes — 2026-11-11

- Audited the main repository and the Solid Safe House component branches for the launcher, injector v2, native DLL, Web Bridge, and gloves component.
- Confirmed the five submodule entries in the main repository's `.gitmodules`.
- Inspected the launcher startup state machine, Bridge process manager, injector workflow documentation, DLL status contract, live schema snapshot documentation, Armory contracts/stores/projections, native inventory and attachment modules, and integration simulator.
- Recent Armory rarity-coloring build success is based on the user's reported local build output; it is not represented as a fresh remote CI or clean-machine build.
- No end-to-end CS2 acceptance test or full cross-component test-suite result was established by this repository audit.

- Operation status updates (user-approved): Overdrive and Obsidian accepted as COMPLETE; Armory advanced to IMPLEMENTED; Clean State recorded as COMPLETE; Reforge designated ACTIVE as the next operation.

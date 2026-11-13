# n1ssan â€” Operation Roadmap

Project: n1ssan
Baseline: Solid Safe House
Last audited: 2026-11-13
Purpose: Record approved operations, verified implementation, outstanding work, and future direction.

## Status Definitions

- **Complete:** The operation's defined objectives have been implemented.
- **Implemented:** Core functionality exists, with operational acceptance or remaining integration work still possible.
- **Active:** Currently under development.
- **In Progress:** Foundational implementation exists; objectives remain unfinished.
- **Planned:** Approved direction without verified implementation.
- **Unverified:** Insufficient evidence to claim completion.

---

## Operation 00 â€” Workspace Migration

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

## Operation 01 â€” Overdrive

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
- [x] Confirm all packaged component paths against the intended release layout: `n1ssan-build.bat verify-package` (`tools/verify-package.ps1`) checks every required file, x64 binaries, the single-runtime rule in `dll/`, the manifest version, and artifact hashes.
- [ ] Verify behavior when CS2 exits or changes process during each startup stage.
- [ ] Confirm the launcher and injector recover cleanly from stale or missing status records.

---

## Operation 02 â€” Armory

**Status: COMPLETE**

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

### In-game acceptance

User-verified in game on 2026-11-13: inventory application, attachments (stickers and charms), and the AWP/M4A4 double-charm fix all behaved correctly, and the Armory/Loadout web surfaces behaved correctly in the browser. Accepted as COMPLETE on that evidence.

Two native fixes made this acceptance pass hold:

- The apply pass refuses to write into a CS2-owned item view (no fallback attribute writes, no material rebuild), which removed the `CallSetAttribute` fault chain that crashed the game.
- A charm pass that cannot reach the HUD weapon is deferred and retried instead of being recorded as repaired, which removed the static second charm.

### Follow-up validation (tracked, not blockers)

- [ ] Repeat the acceptance pass across T and CT, including two owned instances of the same weapon.
- [ ] Verify item persistence and identity across repeated updates and CS2 restarts.
- [ ] Test edge cases involving unsupported, unresolved, or stale items.
- [ ] Establish automated end-to-end acceptance coverage; the local simulator validates documents only.
- [ ] Charm entity discovery/readback: the HUD weapon resolution now falls back to the last resolved viewmodel weapon and retries every 250 ms while deferred; a fuller native entity-discovery pass is still open.

### Important distinction

Loadout represents catalogue selections and equipment intent. Armory represents owned item instances. Equipment assignments reference owned Armory instances. These are separate data sources and must remain separate.

The integration simulator validates local document hand-off only; it does not attach to CS2 or prove successful in-game application.

---

## Operation 03 â€” Obsidian

**Status: COMPLETE**

### Objective

Establish a unified, minimal, dark industrial design system across the n1ssan interface.

### Verified foundation

- [x] Dedicated Obsidian Armory application and visual identity.
- [x] Dark industrial visual treatment, catalogue and inventory layouts.
- [x] Armory navigation, search, filtering, sorting, inventory cards, and dialogs.
- [x] Shared visual patterns within the Armory interface, including rarity/grade presentation.
- [x] Separate Loadout interface and redesign stylesheets exist.

### Design tokens and shared presentation

- [x] Shared design tokens: `src/shared/tokens.css` is the single source for surfaces, borders, text, accents, rarity, typography, radius, spacing, and runtime condition colors. The catalogue (`App.css`), Armory (`obsidian.css`), and Loadout (`loadout.css`) alias their local variables to those tokens, and the launcher mirrors them in `Theme/DesignTokens.xaml` merged into `App.xaml`.
- [x] Reusable components: `SearchSuggestions`, `BackToTop`, `LineSidebar`, `ShimmerButton`, and `ObsidianDropdown` now live in `src/shared/components` and are used by both Armory and Loadout.
- [x] Runtime condition colors are shared between the diagnostics panel, status records, and the launcher's failure text.

### Follow-up polish (does not reopen the operation)

- [ ] Unify remaining navigation and interaction patterns.
- [ ] Align catalogue, inventory, and Loadout presentation without conflating their separate workflows.
- [ ] Standardize responsive layouts.
- [ ] Standardize loading, empty, error, disabled, and success states.
- [ ] Reduce visual inconsistency between application sections.

### Completion criteria

A cohesive interface with reusable components and consistent behavior across the application, rather than a polished Armory surface alongside independently styled sections.

---

## Operation 04 â€” Stormwatch

**Status: IMPLEMENTED**

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

### Delivered in this pass

- [x] Unified reporting workflow: `GET /diagnostics` on the Bridge returns one document with the native runtime, Bridge, and loader sources together, and the Armory Diagnostics panel renders it (the static placeholder is now only a fallback when the Bridge is unreachable).
- [x] Signature health reporting: the DLL counts every resolved engine signature, logs each missing one by name, and publishes `signatures=<resolved>/<total>` inside the READY/DEGRADED reason.
- [x] Consistent failure classification: `failure-taxonomy.md` defines the codes; the DLL writes `code=` into a protocol 2 status record, the injector adds `name=` to `RESULT=FAIL`, the launcher (`FailureCatalog`) and the Bridge (`bridge/diagnostics.ts`) translate the same code into the same next action.
- [x] Status-harness alignment: `supporting/n1ssan-status-tests` now parses the real protocol 1/2 record, validates the production reason set, state/reason semantics, and code severity; 38/38 checks pass (`build.cmd`).
- [x] Stale-data rule is one rule everywhere: a heartbeat older than 30 seconds is stale, never READY; FAILED stays terminal for the session; a missing record is unknown, not a failure.

### Remaining work

- [ ] Validate diagnostic information across a complete live lifecycle (start, degrade, fail, recovery) in game.
- [x] Feed the injector's outcome into the Bridge diagnostics document: the injector writes `%LOCALAPPDATA%\n1ssan\status\injector.json` and `/diagnostics` classifies it as the loader source.
- [x] Extend the taxonomy to Armory HTTP validation errors: `armoryFailureCode` maps 409/400/422/404/5xx to `E_ARMORY_CONFLICT` / `E_ARMORY_VALIDATION` / `E_ARMORY_REJECTED` / `E_ARMORY_UNAVAILABLE`, and `ArmoryBridgeError` carries the code.
- [x] Diagnostics panel refresh action so a session can be re-read without reopening the dialog.
- [ ] Add re-inject / restart-Bridge actions to the diagnostics panel rather than instructions only.

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

## Operation 05 â€” Reforge

**Status: IMPLEMENTED**

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

### Delivered in this pass

- [x] All component contracts documented in one maintained reference: `contracts.md` (armory inventory, equipment, loadout runtime projection, loadout profiles, integration snapshot, runtime mode, DLL status record, injector outcome, Bridge HTTP API).
- [x] Versioning and migration rules defined for every persisted document: additive-only changes, readers accept known versions and reject newer ones, migrations must preserve user data (see `contracts.md` Â§10).
- [x] Failure-taxonomy contract established across the DLL, injector, launcher, Bridge, and UI (`failure-taxonomy.md`), including the protocol 2 status record and the `/diagnostics` document.
- [x] Complete automated suite run and recorded (2026-11-13): Web Bridge `tsc -b` clean, `npm run build` clean, `npm test` 100/100, status harness 38/38, native DLL x64 Release build clean, launcher and injector `dotnet build` clean.
- [x] API verification performed live: `/health` and `/diagnostics` served on a loopback test port, with a real stale status record correctly classified as `STALE` / `W_STATUS_STALE`.

### Remaining work

- [ ] Validate contract compatibility across repositories and packaged component revisions.
- [x] Verify packaged runtime paths and file ownership: `tools/verify-package.ps1` validates the staged layout, x64 binaries, single-runtime rule, manifest version, and artifact hashes.
- [x] Test failure classification across component boundaries: Bridge tests cover the DLL record (protocol 1/2, stale, failed, degraded), the injector outcome file, and Armory HTTP codes.
- [ ] Complete the UI verification pass for every Armory and Loadout flow in a live session (dialogs, suggestions, profile creation).

### Important distinction

The schema-v3 integration snapshot and simulator establish a local contract-validation layer. They do not replace live launcher-to-Bridge-to-DLL-to-CS2 acceptance testing.

---

## Operation 06 â€” Atlas

**Status: COMPLETE**

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

### Delivered in Operation Atlas

- [x] Standardized build and release procedure: `n1ssan-build.bat` builds all four runtime components in a fixed order (native DLL â†’ injector â†’ launcher â†’ Bridge + web bundle), stages the packaged layout, and gains a `verify` target that runs every build and test without packaging.
- [x] Versioning and artifact naming: `VERSION` holds `MAJOR.MINOR.PATCH` with bump rules in `RELEASE.md`; the archive is `n1ssan-<version>-win64.zip` next to the staged `release\n1ssan` folder.
- [x] Reproducible release packaging: `tools\release.ps1` writes `manifest.json` with the main and component commit IDs, contract versions in force, build time, and the size and SHA-256 of every runtime artifact, then creates the versioned archive.
- [x] Release compatibility rules: `contracts.md` and `RELEASE.md` define the contract versions a release must agree on (status protocol 2, armory schema 1, runtime document 2, integration snapshot 3, loadout profiles 1) and the same-commit-set rule.
- [x] Development environment documented: `DEVELOPMENT.md` covers prerequisites, per-component build/test commands, the development loop, runtime data locations, game requirements, and the observed gotchas (IPv6 Vite, MSBuild `-p:`, single-DLL rule).
- [x] Repository and contribution standards: `CONTRIBUTING.md` defines the Solid Safe House branch per repository, commit conventions, data-safety and contract rules, the submodule pointer procedure, combined-baseline validation, and the review checklist.
- [x] Artifact hygiene: `.gitignore` keeps `build/`, `release/`, and local scratch out of version control.
- [x] Verified in this workspace: full release build with manifest and archive, and `n1ssan-build.bat verify` passing end to end.

### Remaining work

- [ ] Verify a clean-machine build (fresh clone, prerequisites only) on a second machine.
- [ ] Add code signing for the launcher and DLL before any public distribution.
- [x] Automate the release note entry: `tools/release.ps1` writes one `## Release History` line per version from the manifest, replacing a previous line for the same version.
- [x] One-click release with a manual version: `n1ssan-release.bat` prompts for `<channel>.Build.v<MAJOR>.<MINOR>.<PATCH>`, validates it, writes `VERSION`, builds, and archives.
- [x] Release version visible in the product: the Armory and Loadout workspaces show the build version at the top right.

---

## Operation 07 â€” Apex

**Status: IN PROGRESS**

### Objective

Develop a controlled process for detecting outdated definitions, discovering updated signatures and offsets, and validating findings, delivered as a first-class desktop application.

### Reimagined scope (2026-11-13, user-approved)

Apex is a small Windows application with two jobs today, and room to grow:

1. After a CS2 update, check whether the offsets and signatures the build relies on are still valid, discover the moved ones, and fetch updated definitions.
2. Apply the newly found offsets/signatures to a newly built release, writing them into the files the build consumes.

For that to work the release now ships its offset data too: the packaged layout includes `offsets.json`, the definitions both the build and the runtime are checked against, so Apex has an authoritative record to compare against and a concrete file to rewrite.

Implementation stack: **Tauri** (Rust backend plus React/Tailwind frontend), so the app reuses the project's design language and React components. The window component is `N1ssanOffsetChecker` (working title **N1ssan Sync**).

### Existing groundwork

- [x] Memory scanning modules.
- [x] Offset configuration modules.
- [x] Automatic-offset-related source modules.
- [x] Live schema diagnostics and schema preflight.

### Planned direction

- [x] Detect missing or outdated definitions: Apex compares every dataset entry's stored RVA with the RVA the signature resolves to in the live `client.dll`.
- [x] Discover candidate signatures and offsets: the entry patterns are scanned against the running module and the referenced address is resolved.
- [ ] Validate candidates against live runtime information (beyond a signature hit: confirm the resolved global is the object it claims to be).
- [ ] Report confidence and validation results.
- [x] Maintain versioned offset records: `offsets.json` carries a dataset version that Apex bumps on every applied change, with a `.bak` of the previous file.
- [ ] Prevent unverified findings from reaching production configuration: `rva: null` is reported as `new`, never as current, but nothing yet blocks a release build on an unverified dataset.

### Apex application work

- [x] Create the Apex workspace (`development/n1ssan-apex`): Tauri 2 shell, React/TypeScript frontend, Tailwind, the compact 250×250 N1ssan Sync window with the liquid progress layer, offset result list, and the Find Offset / Update Offset actions.
- [x] Offset dataset contract: `offsets.json` schema version 1 (dataset name, version, update timestamp, and one entry per signature with module, pattern, displacement offset, and last resolved RVA), documented in `contracts.md` §9.
- [x] Ship the dataset with the release: `n1ssan-build.bat` stages `apex\n1ssan-apex.exe` and `offsets.json` into `release\n1ssan`, `tools/release.ps1` records both artifact hashes, and `tools/verify-package.ps1` checks presence, schema version, dataset name, unique entry names, and hexadecimal patterns.
- [x] Live scanning in Rust: open `cs2.exe`, locate `client.dll` through the module snapshot, read the module, scan each pattern, resolve the `rel32`, and report per-entry `match` / `moved` / `missing` / `new` with the found and stored RVA. Read-only: Apex never writes to the game process. Verified against a live CS2 session on 2026-11-13.
- [x] Apply workflow: rewrite the dataset with the discovered RVAs, bump the dataset version and timestamp, keep a backup of the previous file, and report exactly which entries changed. Applying requires a completed scan, so nothing is written speculatively. Verified against a live CS2 session on 2026-11-13.
- [x] Progress reporting: the Rust side emits `apex-progress` stages (dataset, process, reading, scanning per entry, resolving, writing) so the liquid fill reflects real work instead of a fixed animation.
- [x] Failure taxonomy: `E_APEX_NO_GAME`, `E_APEX_NO_MODULE`, `E_APEX_DATASET`, `E_APEX_SCAN_EMPTY`, `E_APEX_WRITE`, `E_APEX_INTERNAL` added to `failure-taxonomy.md` with the same code-to-next-action rule as the rest of the project.
- [ ] Version badge: the Apex window shows the release version from the same `VERSION` file, like Armory and Loadout.
- [ ] Fetch workflow: pull updated definitions from a configured source and validate them before they can be applied (needs a real endpoint before it can be implemented).
- [ ] Validate discovered offsets against the live runtime and report confidence, so unverified findings cannot silently reach a release.

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

**Operation 07 â€” Apex**

Operations 00 through 03 are accepted as complete. Armory's in-game acceptance was verified by the user on 2026-11-13. Stormwatch and Reforge are implemented: diagnostics are consolidated behind one document and one failure taxonomy, every contract is documented, and the full automated suite was run and recorded. Their remaining items are live-session validation, not unresolved design.

Atlas is complete: one build script, one version file, a reproducible manifest with component commits and artifact hashes, and documented environment, release, and contribution rules.

Apex is in progress and has been run against the live game. On 2026-11-13, with CS2 at the main menu, `Find Offset` resolved all three signatures from the running `client.dll` and `Update Offset` wrote them into `offsets.json` (dataset `2026.11.13.2`, previous file kept as `offsets.json.bak`). Apex is in progress. The first application exists: `development/n1ssan-apex` is a Tauri 2 shell (Rust backend, React + Tailwind frontend) whose 250x250 N1ssan Sync window detects `cs2.exe`, scans the shipped `offsets.json` patterns against the live `client.dll`, reports each entry as match / moved / missing / new, and writes verified offsets back into the dataset with a version bump and a `.bak` copy. The dataset is staged into every release and verified by `verify-package`. What remains: fetching definitions from a real source, and validating a resolved address against the live runtime instead of trusting a signature hit.

The next priorities are:

1. **Apex** â€” controlled signature/offset discovery and validation, consuming the signature-health reporting Stormwatch now publishes.
2. **Follow-up validation** â€” repeat the in-game acceptance pass across T/CT and restarts; verify packaged runtime paths.
3. **UI unification** â€” align Armory and Loadout visual language (deferred by the user on 2026-11-13).

Any future hardening or polish can be tracked as follow-up work without reopening completed operations unless scope materially changes.

Solid Safe House remains the preserved development baseline.

---

## Audit Notes â€” 2026-11-11

- Audited the main repository and the Solid Safe House component branches for the launcher, injector v2, native DLL, Web Bridge, and gloves component.
- Confirmed the five submodule entries in the main repository's `.gitmodules`.
- Inspected the launcher startup state machine, Bridge process manager, injector workflow documentation, DLL status contract, live schema snapshot documentation, Armory contracts/stores/projections, native inventory and attachment modules, and integration simulator.
- Recent Armory rarity-coloring build success is based on the user's reported local build output; it is not represented as a fresh remote CI or clean-machine build.
- No end-to-end CS2 acceptance test or full cross-component test-suite result was established by this repository audit.

- Operation status updates (user-approved): Overdrive and Obsidian accepted as COMPLETE; Armory advanced to IMPLEMENTED; Clean State recorded as COMPLETE; Reforge designated ACTIVE as the next operation.

---

## Audit Notes â€” 2026-11-13

- User verified in game that the apply pass no longer crashes CS2 and that AWP/M4A4 no longer render a second charm; the same session confirmed the web surfaces behave correctly. Armory is accepted as COMPLETE on that evidence.
- Status record contract advanced to protocol 2 (trailing `code=`), accepted by the launcher, injector watcher, Bridge, and the status harness; protocol 1 records remain readable.
- `failure-taxonomy.md` and `contracts.md` added at the project root as the maintained references for codes and documents.
- Verification actually performed: native DLL x64 Release build; launcher and injector `dotnet build`; Web Bridge `tsc -b`, `npm run build`, `npm test` (100/100); status harness (38/38); live `/health` and `/diagnostics` on a loopback test port.
- Not performed: an in-game run of this exact binary (the acceptance above predates the diagnostics/taxonomy changes) and a click-through of every Armory/Loadout dialog.
- User verification (2026-11-13, launcher run from the Debug folder): new profiles are empty in both workspaces, search suggestions and per-bar search history behave correctly. Armory/Loadout visual unification was explicitly deferred.
- Operation Atlas completed: `VERSION` 3.0.0, `n1ssan-build.bat` with `verify`, `tools\release.ps1`, and the environment/release/contribution references. Release 3.0.0 was built and archived as `release\n1ssan-3.0.0-win64.zip`, with `release\n1ssan\manifest.json` recording the component commits, contract versions, and artifact hashes.
- Baseline pushed on 2026-11-13: main `solid-safe-house` plus every component's Solid Safe House branch (`solid-safe-house-dll`, `-injectorv2`, `-launcher`, `-bridge`). Submodule pointers match the pushed commits.

## Release History

- **Beta.Build.v1.0.0** - 2026-11-13 | archive `n1ssan-Beta.Build.v1.0.0-win64.zip` | main `0689d2fd4f6d` dll `2e70165f8de1` injector `eb884d1d9271` launcher `95b4050a7204` bridge `ce65fd9993b7`

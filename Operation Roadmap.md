# n1ssan — Operation Roadmap

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
- [x] Confirm all packaged component paths against the intended release layout: `release\n1ssan-release.bat verify-package` (`tools/verify-package.ps1`) checks every required file, x64 binaries, the single-runtime rule in `dll/`, the manifest version, and artifact hashes.
- [ ] Verify behavior when CS2 exits or changes process during each startup stage.
- [ ] Confirm the launcher and injector recover cleanly from stale or missing status records.

---

## Operation 02 — Armory

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

### Design tokens and shared presentation

- [x] Shared design tokens: `src/shared/tokens.css` is the single source for surfaces, borders, text, accents, rarity, typography, radius, spacing, and runtime condition colors. The catalogue (`App.css`), Armory (`obsidian.css`), and Loadout (`loadout.css`) alias their local variables to those tokens, and the launcher mirrors them in `Theme/DesignTokens.xaml` merged into `App.xaml`.
- [x] Reusable components: `SearchSuggestions`, `BackToTop`, `LineSidebar`, `ShimmerButton`, and `ObsidianDropdown` now live in `src/shared/components` and are used by both Armory and Loadout.
- [x] Runtime condition colors are shared between the diagnostics panel, status records, and the launcher's failure text.

### Follow-up polish (does not reopen the operation)

- [x] Unify remaining navigation and interaction patterns: Loadout uses the same `LineSidebar`, `SearchSuggestions`, `ObsidianDropdown`, `ShimmerButton`, `BackToTop`, and `AppVersion` components as Armory, with the same props.
- [x] Align catalogue, inventory, and Loadout presentation without conflating their separate workflows: Loadout's separate navy palette and its legacy builder stylesheet were replaced by `shared/tokens.css` plus `obsidian.css` surfaces, item cards, dialogs and rarity presentation.
- [x] Standardize responsive layouts: both workspaces share one breakpoint set in their layout sheets.
- [ ] Standardize loading, empty, error, disabled, and success states.
- [x] Reduce visual inconsistency between application sections: Loadout no longer defines a second accent, surface, or typography scale.

### Completion criteria

A cohesive interface with reusable components and consistent behavior across the application, rather than a polished Armory surface alongside independently styled sections.

---

## Operation 04 — Stormwatch

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

## Operation 05 — Reforge

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

## Operation 06 — Atlas

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

- [x] Standardized build and release procedure: `release\n1ssan-release.bat build` builds all four runtime components in a fixed order (native DLL → injector → launcher → Bridge + web bundle), stages the packaged layout, and gains a `verify` target that runs every build and test without packaging.
- [x] Versioning and artifact naming: `VERSION` holds `MAJOR.MINOR.PATCH` with bump rules in `RELEASE.md`; the archive is `n1ssan-<version>-win64.zip` next to the staged `release\n1ssan` folder.
- [x] Reproducible release packaging: `tools\release.ps1` writes `manifest.json` with the main and component commit IDs, contract versions in force, build time, and the size and SHA-256 of every runtime artifact, then creates the versioned archive.
- [x] Release compatibility rules: `contracts.md` and `RELEASE.md` define the contract versions a release must agree on (status protocol 2, armory schema 1, runtime document 2, integration snapshot 3, loadout profiles 1) and the same-commit-set rule.
- [x] Development environment documented: `DEVELOPMENT.md` covers prerequisites, per-component build/test commands, the development loop, runtime data locations, game requirements, and the observed gotchas (IPv6 Vite, MSBuild `-p:`, single-DLL rule).
- [x] Repository and contribution standards: `CONTRIBUTING.md` defines the Solid Safe House branch per repository, commit conventions, data-safety and contract rules, the submodule pointer procedure, combined-baseline validation, and the review checklist.
- [x] Artifact hygiene: `.gitignore` keeps `build/`, `release/`, and local scratch out of version control.
- [x] Verified in this workspace: full release build with manifest and archive, and `release\n1ssan-release.bat verify` passing end to end.

### Remaining work

- [ ] Verify a clean-machine build (fresh clone, prerequisites only) on a second machine.
- [ ] Add code signing for the launcher and DLL before any public distribution.
- [x] Automate the release note entry: `tools/release.ps1` writes one `## Release History` line per version from the manifest, replacing a previous line for the same version.
- [x] One-click release with a manual version: `release\n1ssan-release.bat` prompts for `<channel>.Build.v<MAJOR>.<MINOR>.<PATCH>`, validates it, writes `VERSION`, builds, and archives. The former root-level build script was folded into it, so the repository root no longer carries build scripts and the same file exposes `build`, `verify`, and per-component targets.
- [x] Release version visible in the product: the Armory and Loadout workspaces show the build version at the top right.

---

## Operation 07 — Apex

**Status: IMPLEMENTED**

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
- [x] Validate candidates against live runtime information: the module is mapped, so Apex reads the pointer stored at the resolved RVA and checks it against the canonical x64 user-mode range and alignment instead of trusting the signature hit alone.
- [x] Report confidence and validation results: every entry carries `high` / `medium` / `low` / `none` plus a one-line validation message, shown in the result list and its tooltip.
- [x] Maintain versioned offset records: `offsets.json` carries a dataset version that Apex bumps on every applied change, with a `.bak` of the previous file.
- [x] Prevent unverified findings from reaching production configuration: `Apply` writes only `high`/`medium` results and reports `low`/unresolved entries as blocked, and `verify-package` fails a release whose dataset has an unresolved RVA or a low-confidence entry.

### Apex application work

- [x] Create the Apex workspace (`development/n1ssan-apex`): Tauri 2 shell, React/TypeScript frontend, Tailwind, the compact 250×250 N1ssan Sync window with the liquid progress layer, offset result list, and the Find Offset / Update Offset actions.
- [x] Offset dataset contract: `offsets.json` schema version 1 (dataset name, version, update timestamp, and one entry per signature with module, pattern, displacement offset, and last resolved RVA), documented in `contracts.md` §9.
- [x] Ship the dataset with the release: `release\n1ssan-release.bat` stages `apex\n1ssan-apex.exe` and `offsets.json` into `release\n1ssan`, `tools/release.ps1` records both artifact hashes, and `tools/verify-package.ps1` checks presence, schema version, dataset name, unique entry names, and hexadecimal patterns.
- [x] Live scanning in Rust: open `cs2.exe`, locate `client.dll` through the module snapshot, read the module, scan each pattern, resolve the `rel32`, and report per-entry `match` / `moved` / `missing` / `new` with the found and stored RVA. Read-only: Apex never writes to the game process. Verified against a live CS2 session on 2026-11-13.
- [x] Apply workflow: rewrite the dataset with the discovered RVAs, bump the dataset version and timestamp, keep a backup of the previous file, and report exactly which entries changed. Applying requires a completed scan, so nothing is written speculatively. Verified against a live CS2 session on 2026-11-13.
- [x] Progress reporting: the Rust side emits `apex-progress` stages (dataset, process, reading, scanning per entry, resolving, writing) so the liquid fill reflects real work instead of a fixed animation.
- [x] Failure taxonomy: `E_APEX_NO_GAME`, `E_APEX_NO_MODULE`, `E_APEX_DATASET`, `E_APEX_SCAN_EMPTY`, `E_APEX_WRITE`, `E_APEX_INTERNAL` added to `failure-taxonomy.md` with the same code-to-next-action rule as the rest of the project.
- [x] Version badge: the Apex window shows the release version from the same `VERSION` file, like Armory and Loadout.
- [x] Fetch workflow: `apex_fetch` reads a configured source (`N1SSAN_APEX_SOURCE`, or `%LOCALAPPDATA%\n1ssan\configs\apex-source.json` with `url`/`file`), validates the document as this dataset schema before replacing the current file, keeps a `.bak`, and clears the in-memory scan so findings from one file can never be applied to another. No endpoint is shipped with the project; the operator supplies one. HTTP uses the Windows `curl.exe`, so Apex gains no TLS dependency.
- [x] Validate discovered offsets against the live runtime and report confidence, so unverified findings cannot silently reach a release.
- [x] Failure classification: `E_APEX_FETCH` added to `failure-taxonomy.md` alongside the existing Apex codes, and `contracts.md` §9 records `confidence`, `verifiedAt`, and the fetch source rules.

### Completion criteria

A repeatable discovery and validation workflow with explicit failure reporting and human-verifiable results.

---

## Operation 08 — Casket & Detach

**Status: WITHDRAWN in v6.10** — the Armory-side container model and the detach
button were deleted at the user's request; the Tools tab now adds Lootline's own
tool items instead (see `### v6.10`). The history below is kept as the record of
what existed and what replaced it.

### Objective

Give the Armory its own control over two in-game capabilities: removing a charm from
an owned item without consuming a tool, and holding items inside a Storage Unit.

### Delivered

- [x] Charm detachment from the item detail dialog: the Armory clears
  `attachments.charm`; the apply pass already writes with `clearMissing`, so the
  attribute is cleared and the keychain entity removed. No tool is consumed and the
  charm stays in the inventory.
- [x] Storage Unit kind: `storage_unit` in the Armory document, validated
  (`containerId` must name a storage unit, no nesting, no customization), carried
  through the Obsidian adapter.
- [x] Runtime projection version 3 with `containersVersion: 1`: the unit projects as
  definition 1201 with its `itemsCount`; stored items carry `containerInstanceId`.
- [x] Native casket support: `NewItem.casketId`, `itemsCount`, `modificationDate`,
  written as "casket item id low/high" (272/273), "items count" (270) and
  "modification date" (271); `LootSOCache::SetCasketInfo` keeps a retained unit's
  count in step; units are created before the items stored in them.
- [x] Armory interface: Storage Unit card first in the Stickers catalogue tab, a
  Storage Unit category and filter, stored items hidden from All items but still
  counted, and Store / Retrieve in the detail dialog.
- [x] DLL version `Debug.v6.1`, Release x64; Web Bridge `tsc -b`, `npm test`
  117/117, `npm run build`, `npm run build:bridge`.
- [x] Charm removal now clears `keychain slot 0 id` **in place** on both attribute
  lists of the live view (`Attachments::ClearCharmInPlace`). Every schema setter
  faults on a live view on this client, so the old removal already existed in
  intent but never reached the attribute: the entities came off and the next
  model build hung the charm again. A charm-only change also skips the setter
  path entirely now.
- [x] Storage Unit label: the projection's `customName` reaches the created unit
  (`m_szCustomName`), and a retained unit takes an Armory rename in place. Before
  this the unit was projected unnamed, and CS2's own "Change Label" writes that
  field.
- [x] Inventory hitch: a sticker or charm tool whose card already carries its
  projected kit is no longer removed and re-created on every adoption. One
  re-projection was ~60 item creations on the game thread (measured 256-378 ms
  frames in `n1ssanlogs.md`, straight after `staged 190 items`).
- [x] Charm hitch: `Charm::Upkeep` no longer walks every entity slot in the game
  for a gun that wants no charm and carried none; that walk is ~32k reads per
  weapon and ran for every gun on the roster each check.
- [x] Death crash, panorama detour: `HkRunFrame` now catches its own work. It is a
  virtual function of the game's UI engine, so a fault inside it was handled by
  nobody and took the client down with it. The 2026-11-23 21:17 dump has exactly
  that shape - `0xc0000005` with the fault address and `RIP` both 0, i.e. a call
  through a null function pointer - on the thread that runs
  `LocalSpray::MainThreadTick()` and the queued apply jobs.
- [x] Death crash, queued applies: a sticker or charm job was built from an item
  view read before the death and then ran on panorama's frame while CS2 tore the
  round down. `NativeApply::SetLoadoutQuiet` holds them from the death until the
  pawn is back; the queue is kept, not cleared.
- [x] Death crash, knife swap: the Armory path dispatches `KnifeApply` before
  `ApplyWeaponSkin`, and only the latter checked the pawn's health, so
  `SetModel` / `UpdateSubclass` kept running against a knife entity CS2 was
  destroying on every pass spent in the death camera. The swap is skipped for a
  dead pawn and written by the respawn sweep.
- [x] Crash evidence: the unhandled-exception record now appends the faulting
  registers and the first twelve stack return addresses as `module+offset`, so a
  null-call fault is readable from the log alone. The 21:17 dump could not be
  symbolised at all: its fault address is 0, and the build that produced it was
  overwritten by the next one, PDB included.
- [x] Build symbols are archived with the DLL (`x64/Release/pdb/n1ssan-dll-v6.0.pdb`) so a future dump resolves against the build that wrote it.
- [x] Death crash, dead entity: `ApplyWeaponSkin` now refuses a weapon handle that
  no longer names a live entity (its vtable is outside client.dll). The item view
  is embedded in the weapon entity, so a handle left in a list from before the
  death read the attribute list of freed memory - the "unmapped-address fault"
  both 2026-11-24 dumps recorded one line after `[death] pawn died`, and the
  00:10 dump was written with the view write still in flight.
- [x] Charm removal, the half that never ran: a gun that wants no charm is
  cleaned once per viewmodel, not only when the gun's own keychain count is
  non-zero. The charm a player sees is the viewmodel's "keychain" addon, which
  is not a `keychain_module` entity and never appeared in that count, so the
  quiet path returned early and a charm detached in the Armory stayed on the gun
  (`[charm] ... wanted=0 ... hud=0000000000000000` on every pass). The removal
  now reports `[charm] ... wanted=0 keychain addon dropped`.
- [x] Charm repair loop: a gun whose keychain attribute CS2 never turns into an
  entity reads `hanging=0` forever, and `hanging != 1` repaired it on every apply
  - `AddKeyChainEntity`, the addon removal and the full entity walk, every pass.
  The 00:10 log repairs one gun repeatedly at `repair=1`, because a cache
  invalidation cleared its state each time. One create attempt per selection and
  model build now, and the addon is dropped once per viewmodel: this is the
  native entity churn the 23:20 session faulted on
  (`animationsystem.dll+0x44871D`, right after a knife `SetModel`).
- [x] Death guard, reviewed: the job pass no longer carries a pre-check for its
  own reentrancy flag (ProcessJobs takes it with an exchange), and the fault
  handler puts that flag back instead of leaving it set - with the old shape a
  single caught fault stopped every queued apply for the rest of the session,
  silently. The FrameStageNotify fallback path (`Tick`, used until panorama's
  frame hook installs) honours the same death gate as the hooked frame.
- [x] Knife swap, respawn check: skipping the swap while dead cannot leave the
  knife on the wrong model. `KnifeApply` reads the live view's definition index
  and the entity's subclass id and stores nothing between calls, so the first
  pass with a live pawn compares what CS2 rebuilt against the Armory's knife.
- [x] Knife HUD model churn: the first-person knife model is written once per
  viewmodel now, not on every build. The 23:20 session set the same butterfly
  model on the viewmodel 17 times; the first-chance fault that ended it
  (`animationsystem.dll+0x44871D`) was two lines after the last one, and a model
  rename is a skeleton rebuild the game does not need twice.
- [x] Keychain removal dedupe: the 00:10 log removes one entity at ten different
  indices in one pass. A pass now removes each entity once, so CS2's remover is
  never called twice on a module it already detached.
- [x] Storage Unit label, Armory side: the unit's `customization.customName` is now
  carried by the contract (label only - no float, seed or StatTrak), projected as
  the unit's `customName` (it was hard-coded to null, which is why every projected
  unit logged `name=""`) and editable in the item detail dialog. Renaming from the
  Armory is the path that reaches a client-injected unit.
- [x] Storage Unit label ordering: a retained unit's `m_szCustomName` is written
  before `SetCasketInfo` notifies CS2. Writing it after meant the card was
  rebuilt from the old label, so an Armory rename never reached the unit in game.
- [x] Knife view re-sync: CS2 puts its own item back on a knife's view every
  respawn (the same re-sync that resets `m_iItemIDHigh`), but the weapon entity
  keeps the Armory subclass. `KnifeApply` restored the view and then called
  `SetModel` again, tearing the model and its animation graph down for nothing -
  23:20 set the same butterfly model three times in one second this way. A view
  re-sync now writes the view's definition and stops; `SetModel` /
  `UpdateSubclass` run only when the entity's subclass really differs, which is
  what a respawn, drop or pickup that replaced the knife leaves behind. The
  `[knife] SetModel` line now prints the def and subclass it changed, and the
  skipped case is logged as `[knife] def=... view restored` (throttled).
- [x] Crash log, overwritten lines: `AppendCrashLine` writes through a second
  handle while the session log has a buffered `FILE*` whose position is behind
  it, so the next flush overwrote the `[crash]` / `[fault]` lines - the 22:27
  dump was written beside a log with no line for it. The stream is moved to the
  end of the file after every append (best effort under the log lock).
- [x] DLL version `Debug.v6.2` (Release x64), Web Bridge `npm run build` (fresh
  `dist`) and `npm run build:bridge`. The running client holds
  `x64/Release/n1ssan-dll.dll` open, so v6.2 was linked to `x64/Release-v6.2/`
  (stamp `Debug.v6.2` present, symbols archived as
  `x64/Release/pdb/n1ssan-dll-v6.2.pdb`); the archive is only replaced once CS2
  is closed and the build is run again.
- [x] Storage Unit label, the interface that was actually running: the built web
  bundle in `dist` predated the label field and the unit branch of the Obsidian
  adapter. It drew `CUSTOM NAME` for skins only, and mapped
  `customization.customName` to `null` for every non-skin, so a Storage Unit
  could not be named from the Armory at all and every projected unit carried
  `name=""`. `dist` is rebuilt from source; the field renders for a unit and its
  label is saved as `customization.customName`, which is what the projection
  reads.
- [x] Storage Unit evidence: a retained unit now logs
  `[socache] storage unit retained id=... items=N name="..."` on every staging
  pass instead of only on creation, so the next session shows whether an Armory
  label and item count reached the DLL.
- [x] Detach charm, the false success: `RebuildHudKeychainAddon` returned true
  even when its lookup found no addon, and `Charm::Upkeep` latched
  `hudCleaned = hudWeapon` on that answer - a removal that never happened was
  never tried again. The call now reports true only when CS2's remover was
  actually handed an addon (or a rebuild was requested), the latch is set only
  on that answer, and the failed viewmodel is logged once and re-examined on the
  next check.
- [x] Detach charm, removal by name: the addon entry is no longer taken only from
  the dictionary's hash lookup. The lookup index is tried first, then every slot
  of the entry table is resolved, and only an entity whose designer name contains
  `keychain` is ever removed - so a lookup that stops naming the addon leaves the
  charm removable instead of unlatchable. Every call logs
  `[charm-pass] hud=... index=... entries=... capacity=... vector=...`; a hit
  logs `[charm-pass] removed hud keychain addon ... name="..."`, and a dictionary
  whose header no longer looks like the addon vector reports `vector=0` instead
  of quietly returning success.

- [x] DLL version `Debug.v6.3` (Release x64, in place at `x64/Release/n1ssan-dll.dll`,
  symbols archived as `pdb/n1ssan-dll-v6.3.pdb`). This build also carries the v6.2
  charm and storage-unit changes: CS2 held the v6.1 DLL open during that build,
  so the client kept running the old one.
- [x] Crash lead, model state offsets: `CSkeletonInstance->m_modelState` (0x140)
  and `CModelState->m_hModel` (0xA0) were the last hardcoded members on the model
  path, and both are now resolved through the engine schema with the constants as
  the fallback (the same treatment `m_nSubclassID` got when it moved to 0x380).
  The evidence is in the mesh line the 2026-11-24 sessions wrote on a knife whose
  mask is 1: `[mesh-fit] ... mesh was 18446744073709551615/0`, i.e. all ones - a
  member that no longer lives where it is read, not a mask. The glove path reads
  these same two members by name, which is why it kept working while the mesh
  path read garbage. Both 23:33 and 23:42 died on the identical fault
  (`client.dll+0xAA9E3C`), which is a deterministic code path, and the mesh is
  the only state we write through a hardcoded member.
- [x] Mesh writes on an unreadable mask: `FitMeshToPaint` treats all ones like 0
  (unknown) and leaves the entity alone instead of writing through the same
  member that answered with garbage. Logged once per five seconds rather than
  per pass.
- [x] Crash evidence, game-module faults: a first-chance fault inside a module
  that is not our DLL now writes the first eight return addresses as
  `module+offset` and one minidump. Both crashes so far left a single line with
  no stack - the sessions died in client.dll on the death frame, with nothing to
  fix against.
- [x] Stutter, FPS pill recompose: the counter's bitmap was redrawn every 80 ms
  on the render thread, and its own log measured 4-6 ms per compose (947 ms for
  the first one). That is a lost frame every second or two while the counter is
  on screen, which is what reads as stutter. Five recomposes a second now
  (200 ms); the light still moves.
- [x] Stutter, measured: a 16-minute session logged 57 frame spikes over 25 ms
  (5.7 s total, the worst 981 ms), 33 full eager sweeps and 23 weapon-list
  changes. The sweep itself costs 12-98 ms per pass, so the counters above are
  the ceiling for what the pass can still cost once the model-state reads are
  correct.

- [x] DLL version `Debug.v6.4` (Release x64, in place at `x64/Release/n1ssan-dll.dll`,
  symbols archived as `pdb/n1ssan-dll-v6.4.pdb`).
- [x] v6.3 answer, model-state offsets: they did not move. The start-up log prints
  `[resolve] CSkeletonInstance->m_modelState = 0x140` with no `schema override`
  line, i.e. the engine schema returns 0x140 and 0xA0 exactly as hardcoded. The
  `client.dll+0xAA9E3C` fault that ended the 23:33 and 23:42 sessions does not
  appear in either v6.3 session, but the all-ones mesh read is still there, so
  the member was not the cause of that read.
- [x] Crash lead, keychain removal is a use-after-free: the 2026-11-25 logs show
  the same three keychain entities removed over and over on consecutive passes
  (`[charm-clean] removed world keychain index=16446 entity=0x...392E000`, then
  the same entity at index 17338, then 16890; two more entities the same way)
  while the gun's HUD half kept failing to resolve. CS2's remover was therefore
  handed modules it had already detached. The dedupe array was per call, so it
  only stopped one pass from removing an entity at several slots. A session-wide
  bounded history now does, and a repeat is skipped with
  `[charm-clean] skip repeat removal ... (already removed, N)`.
- [x] Crash lead, charm work at a round start: `Charm::SetPaused` now also holds
  through the settle window a death and a respawn both arm. A respawn is when CS2
  rebuilds the weapons, the viewmodel and their keychain entities, and the 00:44
  crash (`animationsystem.dll+0x44871D`, read of null+0x18, stack through
  panorama and video64) landed 7 s after one. Charm upkeep is the only part of an
  apply that calls CS2's entity creator and remover; it resumes by itself on its
  own timer.
- [x] Crash evidence, unmapped faults: the registers and the stack return
  addresses are now written for every access violation, not only for a fault
  inside a game module. The 00:55 session died on `execute of 0x0` - a null
  function pointer or vtable - which is in no module, so it left a minidump and
  not one frame. New lines: `[crash] fault access: <read|write|execute> of <addr>`,
  `[crash] fault registers: rip=... rsp=... rbp=... rax=... rcx=... rdx=...` and
  `[crash] fault stack: module+0x... ...` (up to fourteen frames).
- [x] Pass cost, charm upkeep measured: the 00:44 log spends 8566 ms in one apply
  pass whose material rebuild was under 20 ms (that line would have been printed),
  so the time is in the one remaining unmeasured part - the charm upkeep. It logs
  `[perf] charm upkeep took N ms` from 100 ms up, which names the cost next time.
- [x] Tools, dump reader: `tools/minidump-peek.py` prints the exception record
  (code, access, faulting module and offset), the faulting thread's registers and
  the stack frames as `module+0xoffset`, because no debugger is installed here and
  the dump was otherwise a 450 KB file nobody could read.
- [x] DLL version `Debug.v6.5` (Release x64, in place, symbols archived as
  `pdb/n1ssan-dll-v6.5.pdb`).
- [x] FPS counter, the look: the travelling band around the capsule is gone. The
  counter is one static image now - a purple gradient body (violet at the top,
  deep purple at the bottom), a soft purple halo, a glassy highlight on the left
  and a lavender hairline rim - with white text. The loading capsule keeps its
  light; it passes a phase, the counter passes -1 and the shell returns before
  the light is drawn. The bitmap is composed only when the number, the frame
  time, the scale or the spike tint changes, instead of five times a second.
- [x] FPS counter, the size: content height 38 -> 30 px at 1080p, number 18 -> 14,
  frame time 10 -> 8.5, mark 19 -> 15, paddings and gaps reduced, margin 13 -> 10
  for the smaller halo. The composed image is 125x50 px where it was about
  170x64, so the capsule is roughly a quarter smaller in area.
- [x] FPS counter, checked without the game: a throwaway GDI+ harness rendered
  `Compose(144, 7.2, 1.0, static)` to a bitmap, composited over a dark
  background and looked at (`x64/Release/fps-pill-preview.png`): purple gradient,
  readable white number and frame time, rim and halo intact, no band. The harness
  is deleted; the preview stays beside the build.
- [x] Ping: there is no networking code in this runtime at all - no sockets, no
  HTTP, no hook on any net path (`grep` over the DLL, the Bridge and the Launcher
  finds none; the hooks are FrameStageNotify, Present, the knife inspect/reflect
  pair, the lineup tool and the spray action). The ping the scoreboard shows is
  CS2's own server RTT, so nothing here computes, throttles or proxies it. What
  this runtime can do is hold the game thread long enough that the net channel is
  serviced late, which reads as a ping spike, so the pass was made cheaper at the
  worst moment: `APPLY_PASS_BUDGET_EAGER_MS` 40 -> 15 and
  `SKIN_APPLY_EAGER_INTERVAL_MS` 150 -> 200 (the cursor carries the remaining
  weapons into the next pass, so the same writes happen, spread over more
  frames). The 2026-11-25 00:58 log's worst frame was the overlay's own first
  compose (`[fps-overlay] pill compose took 624 ms`, 639 ms frame); with the
  static pill only value changes recompose.
### v6.6 — the Lootline comparison (charm path, stalls)

Source read: `E:\CS2\Lootline\Lootline_v1.5.5_source_[unknowncheats.me]_\Lootline`
(`skinchanger.cpp`, `skin_application.cpp`, `knife_animation.cpp`, `entity_scan.h`,
`hooks.cpp`). Session read: `C:\Users\mirac\.lootline\lootline_log.txt` -
806 lines, 22:00:42 to 02:23:04, no crash, no stall line, charms applied and
removed, `skin materials: N request(s) waited 2.0 s ...` as its only slow path.

What Lootline does differently, and what changed here to match it:

- [x] Charm removal, the walk. Lootline collects the charm entities into a
  fixed array (`CollectWorldKeychains`, `MAX_KEYCHAINS 16`) and removes each one
  **once, after the walk**. Its entity walk also requires the slot's own handle to
  name the slot (`(handle & 0x7FFF) == index`, `entity_scan.h`), so a cleared slot
  cannot hand back an entity that was already removed. This runtime removed inside
  the loop it was walking, with no handle/index check, and the 2026-11-25 logs are
  the result: entity `0x…392E000` removed at index 16446, then 17338, then 16890.
  `entities.cpp` now has Lootline's shape - `CollectKeychainsOn` (snapshot,
  handle/index validated, 16 entries) - and `RemoveKeychainsOn` works down the
  snapshot. The session-wide removal history that stood in for this is gone.
- [x] Charm timing. Lootline never creates or destroys a keychain entity from the
  skin pass: `QueueKeychainState` only records the desired charm, and
  `ProcessPendingKeychains()` runs at `FRAME_NET_UPDATE_END` **after** the
  original call, once the forced-update frames are over, for the gun in your
  hands only (`IsActiveWeapon`); an operation on a holstered gun waits, and one
  whose weapon pointer no longer matches is dropped. This runtime walked every
  equipped weapon - holstered ones, the world gun and the viewmodel - from inside
  the apply pass, twice per gun per pass, ~32k entity reads each. That is both the
  game-thread stall (the ping spike) and the use-after-free window.
  `inventory/charm.cpp` is now Lootline's split: `Charm::Request` from the apply
  pass (records, no entity work), `Charm::ProcessPending` from
  `frame_stage.cpp` stage 7 after `g_origFsn`, active weapon only, with
  Lootline's once-a-second count check (`count != 1 || (hanging != 0 && hanging
  != wanted)`) and its 20-repair cap.
- [x] Charm attribute. Lootline's `SetKeychainId` / zero-id write is the same
  intent; this client's schema setters fault, so the in-place write stays
  (`SetCharmIdInPlace` / `ClearCharmInPlace`), and the order around it is
  Lootline's: every charm off, `AddKeyChainEntity`, then the HUD addon.
- [x] The five-second post-death gate on charm work is gone. It was added while
  the removal itself was unsafe; Lootline does its keychain work 0.6 s after a
  spawn, and the gate only kept charms off the gun for the first five seconds of
  every round. Death and "out of a match" still pause it.
- [x] World keychain count read the charm id as well: `CountWorldKeychainEntities`
  takes an optional `firstKit` (`C_KeychainModule->m_nKeychainDefID`), which is
  what Lootline's repair test compares against the picked charm.
- [x] DLL version `Debug.v6.6` (Release x64, in place, 597,504 bytes, symbols
  archived as `pdb/n1ssan-dll-v6.6.pdb`). MSBuild Release x64 exit 0.

### v6.6, second pass — whole-project recheck against Lootline

- [x] Bug found by that recheck: `offsets.cpp` asked the schema for
  `C_EconItemView::m_bRestoreCustomMaterial`, a field that does not exist. The
  engine's name is `m_bRestoreCustomMaterialAfterPrecache` (`engine_schema_diagnostic`
  already used it, and resolved 0x1B8 with it). Every session logged
  `offsetsUnresolved=1 offsets=[EIV_RestoreCustom]` because of it, and the write
  only landed correctly by falling back to the hardcoded `FB_EIV_RestoreCustom`.
  Fixed: the lookup now succeeds, so a build that moves the field still reads it
  correctly instead of silently writing 0x1B8 forever.
- [x] Feature gap found: this runtime had no dropped-weapon pass at all. Lootline
  runs `ProcessWorldGuns` every 0.5 s and your session log shows why it matters -
  16 lines of "skin: your gun N out of your hands got your skin back". Ported:
  `entities.cpp` has Lootline's table walk (`ForEachEntity`, 1..0x3FFF, handle
  checked per slot, SEH around the visitor), and `dispatch.cpp` has
  `ProcessDroppedWeapons` - every 500 ms, `weapon_*` entities only, the guns you
  hold skipped, knives skipped, and only when the item view's account is provably
  yours (Lootline's `owner == mine`; a gun on the floor belongs to whoever dropped
  it). Called from the stage-7 post-original block, alive only, at most 8 guns and
  40 log lines per scan.

Remaining differences after the recheck (deliberately not copied):

- The first-person knife animation-graph rebuild (`AnimGraphRebuild`, mode 2,
  queued 250 ms after a knife model change, then only once the knife has been
  idle 2 s in your hands). Lootline has it and it works there. It needs
  `IsPlayingDemo` (IEngineClient vfunc 42) first: Lootline's own comment records
  that rebuilding a knife's graph under a playing demo crashed CS2's animation job
  on 2026-10-17, and this runtime has no engine-client interface, so it cannot
  tell a demo from a match. The last two crashes here were inside
  `animationsystem.dll`, which is that same graph. Adding it without the demo gate
  would be the risk, not the fix.
- `Lineup::UnstickYourWeapons` (a skin request shared with an identical one that is
  never drawn, made on its own every pass). This runtime has the same unshare code
  for lineup models on the menus, but not for in-match weapons, and copying it
  means walking the composite manager's requests - the code Lootline wraps in its
  own "crashed (LineupCompositeManager) - skipped this time" guard. No stuck-skin
  report here has needed it yet.
- Music kit changer, agent voice lines, the randomizers, the StatTrak-swap tool and
  the 3D model preview. Those are Lootline features this runtime does not have at
  all; none of them is a defect in a feature it does have.

Not copied, with the reason:

- Lineups and the knife inspect / reflect hooks. Lootline reports both as **not
  installed** on this build - `lineups: NOT installed (signature)`, `knife inspect
  fix: NOT installed (signature)`, `after a CS2 update? signature KnifeReflect
  (client.dll) wasn't found` - so there is no working Lootline behaviour to copy:
  its lineup and knife-animation code never ran in the session that had no
  problems. This runtime installs its own versions of both (`[lineup] builder
  hooked`, `[knife-anim] KnifeInspect/KnifeReflect HOOKED` in the 00:57 log).
  They are the only two features here with no Lootline counterpart to compare
  against, and they are the first thing to disable if a render-path crash recurs.

### v6.7 — the 2026-11-25 crashes, and the whole apply path against Lootline

Evidence read before any change: `x64/Release/n1ssan-crash-20261125-114342-13400.dmp`
and its log. The fault is `animationsystem.dll+0x44871D`, a read of `null+0x18`,
and its call chain is `animationsystem <- panorama.dll+0x5880F0 <- video64 <- client`
- a panorama model panel, i.e. the death screen / round-start UI, drawing through
the animation system. It landed six lines after the round-start sweep had renamed
the world knife (`[knife] SetModel (schema) ... butterfly`) and painted it. The
same log holds a **2242 ms** apply pass at that respawn (`[perf] apply pass took
2242 ms (weapons=3 deferred=3 changed=1)`), and the two first-chance faults
`n1ssan-dll+0x4FD90` / `+0x4FDA0` ("read of 0xFFFF...", `during apply pass`) right
after `[apply-cache] weapon model cache invalidated`. The connection drop reads
"Stop receiving communication from host" because the game thread sat inside that
work while the net channel needed it.

Side by side (only the rows that differed):

| Area | Lootline | n1ssan before v6.7 | v6.7 |
|---|---|---|---|
| Skin apply stage | `FRAME_NET_UPDATE_END` (7), before the original (`hooks.cpp:1011`) | render end (6), after the original | net-update end (7), after the original |
| Team switch | no hold: gloves cleared 2 frames, then applied | 3 s hold on gloves, agents and weapon models | hold removed |
| Knife model | world entity only: `SetModel` + subclass, then material on world **and** HUD copy | also renamed the HUD/viewmodel entity directly | HUD rename removed |
| Knife inspect/reflect | **not installed**: `knife inspect fix: NOT installed (signature)`, `signature KnifeReflect (client.dll) wasn't found` | both hooked, animation-graph parameters written at unverified offsets | not installed, log says so |
| Gloves beyond the apply | apply 4 frames, game-raised flag, lost id, spawn follow-ups 400/1200/3000 ms | same, **plus** the game's glove routine on an idle count every 1.5 s and the paint request freed/rebuild up to 4x per change, plus an agent-model refresh on every team change | extras removed; the Lootline set stays |
| Lineup UI | lineups not installed at all | builder hooked, ticked on every stage 7 - inside matches too | ticks in menus only |
| Charms | queue, `ProcessPendingKeychains` after the original, active weapon | same since v6.6 | unchanged |
| Guns out of your hands | `ProcessWorldGuns` every 0.5 s, owner must be you | same since v6.6 | unchanged |

Changes made:

- [x] `frame_stage.cpp`: the apply pass is a stage-7 pass now (the guard that
  read `stage != 6` reads `stage != 7`). Lootline does all of its skin work at
  `FRAME_NET_UPDATE_END`; this runtime did it in the middle of the render-end
  processing, which is the stage the frame being drawn comes from. v6.8 then put
  it on the correct side of the original call as well - see below.
- [x] `frame_stage.cpp`: `TeamModelWorkHeld` returns false and `TeamSwitchSettling`
  returns false. The 3 s hold kept the new side's gloves, agent and weapon
  materials unwritten through the intro - the exact window where the CT half's
  gloves were reported missing. Lootline has no equivalent; the switch is still
  logged.
- [x] `frame_stage.cpp`: `Lineup::FrameTick` only outside a match. The lineup
  builder hangs panorama model panels and asks for their skeletons, which is the
  call chain the 11:43 fault was in.
- [x] `apply.cpp`: the HUD/viewmodel knife `SetModel` is gone; the world knife is
  renamed as Lootline's `KnifeModelChanger::Apply` does it and the first-person
  copy follows the weapon. This runtime's own note on that path records the
  butterfly model set 17 times in one session and "the fault that ended it was
  inside animationsystem.dll two lines after the last one".
- [x] `knife_animation.cpp`: the inspect / reflect hooks are not installed and
  `g_ready` stays false, so `FrameStage` returns immediately. They wrote
  animation-graph parameter nodes at offsets (weapon +0x19A8, controller +0xC0 /
  +0xD8 / +0x270) that were never verified on this client.
- [x] `gloves.cpp`: the glove-model upkeep (`EnsureGloveModel`, `RepairGloveMaterials`,
  the `[glove-model]` line) and the agent-model refresh on team change are gone.
  They are not in Lootline; the last session shows the upkeep calling the game's
  own glove routine mid-round (`[glove-model] no glove model, routine run (state 1)
  team=CT`) and nothing in Lootline does that. What remains is Lootline's set: 4
  apply frames, the game-raised reapply flag, the lost item id, the 400 / 1200 /
  3000 ms spawn follow-ups, the 2-frame team clear, and the once-a-second paint
  attribute heartbeat.
- [x] DLL version `Debug.v6.7` (Release x64, in place, 585,728 bytes, symbols
  archived as `pdb/n1ssan-dll-v6.7.pdb`). MSBuild Release x64 exit 0, no warnings.
  Strings in the binary: `inspect/reflect fix NOT installed`, `no hold, applying
  at once`, `Debug.v6.7` present; `HUD SetModel`, `glove paint request freed`,
  `EnsureGloveModel` gone.

### v6.8 — the stage-7 order, exactly as Lootline has it

v6.7 moved the pass to stage 7 but left it after `g_origFsn`. Lootline's order in
`hooks.cpp` is: `FRAME_NET_UPDATE_END` -> `SkinChanger::Run()` -> `original(...)` ->
`ProcessPendingKeychains()`. So the pass now runs before the original and only the
keychain work stays after it. No hybrid.

Why it is safe, and what was needed to get there:

- The body that used to sit at the end of the detour computes everything it uses
  itself (`pawn`, `hp`, `network`, `connectedKnown`, `pawnAliveKnown`,
  `inMatch`, the state machine, the outside-a-match inventory work, the cadence).
  The only values it took from the detour were `now` and the glove-invalidation
  flag, so it is now `static void RunSkinStage(uint64_t now, bool gloveUpdate)`,
  called with `NowMs()` and the flag. Nothing inside it changed.
- The glove-invalidation consume (`g_gloveUpdatePending` -> `InvalidateWeaponModelCache`)
  moved with it, to before the call: the sweep after a glove identity change is
  now the same tick's pass instead of the next one.
- `Charm::ProcessPending` and `ProcessDroppedWeapons` stay where v6.6 put them -
  after the original. That is the one thing Lootline deliberately does after the
  game's net-update call (the keychain entities are created and destroyed on top
  of what CS2 has already done this tick), and it is unchanged.
- What moved before the original besides the weapons: the match-state bookkeeping
  and the outside-a-match inventory work (`ArmoryInventory::AdoptPending`,
  `NativeEquipment::Tick`, `Attachments::ObserveAndPersist`, `NativeApply::Tick`,
  `PlanA` reconciliation). All of it is client-side item work on a pawn that is
  not in a match, and Lootline likewise does its whole `Run()` on this side of the
  call. The in-match apply pass is the only part that reaches entities.
- [x] DLL version `Debug.v6.8` (Release x64). MSBuild Release x64 exit 0, no
  warnings, symbols archived as `pdb/n1ssan-dll-v6.8.pdb`. The first link failed
  with `LNK1104` because `cs2.exe` (PID 14772) held `x64/Release/n1ssan-dll.dll`
  open; once the game was closed the solution was rebuilt, and the stamped binary
  is installed in place - 586,752 bytes, `Debug.v6.8`, `inspect/reflect fix NOT
  installed`, `no hold, applying at once` present; `HUD SetModel` and
  `glove paint request freed` gone.

### v6.9 — the 2026-11-25 22:01 crash, and the attribute vector that pointed at -1

Session: `Beta.Build.v1.0.1.Debug.v6.8`, PID 9724, crash at 22:01:09. Its log was
kept as `n1ssanlogs.crash-20261125-220112.md`, its dump is
`n1ssan-crash-20261125-220109-9724.dmp`. The game had been in a match (three
deaths at log lines 2408 / 2525 / 2646, respawns after each) and crashed ~20 s
after the last respawn.

**The fatal fault is not in our code, and the crash handler says so:**

- faulting thread 22840, `tier0.dll+0x1FFF93`, `read of 0xFFFFFFFFFFFFFFFF`;
- 24-frame stack: `scenesystem.dll` x7 -> `engine2.dll` x5 -> `tier0.dll` x3 ->
  `d3d11.dll` -> `nvwgf2umx.dll` (the driver) -> `nvapi64.dll`; no n1ssan frame
  anywhere in it;
- `[crash] game-module fault`, i.e. the same classification as any other
  engine-side fault. Reading address -1 on the render thread is an
  index/sentinel inside the engine's own scene data.
- So v6.8 did **not** reproduce the 11:43 signature (`animationsystem+0x44871D`
  called from `panorama.dll`); this is a different fault, in the renderer, with
  no frame of ours between it and the driver. Nothing in this dump implicates
  n1ssan in it, and nothing exonerates it either: a bad write of ours would show
  up here as a fault somewhere else later, not as one of our frames.

**But the same session has a real fault inside our DLL, and that one is fixed:**

- log line 1050: `[fault] first-chance 0xC0000005 at n1ssan-dll+0x18484`, read of
  `0x2F`, during idle. CS2 survived it (the read is inside `__try`), and the very
  next line is `[attach] item 9DB0FB01A719B781 did not read back its saved
  attachments: stickers=0 charm=none; retrying`.
- Resolving `+0x18484` without a debugger: `dumpbin /disasm` of the shipped DLL,
  then the nearest symbol above that RVA — `?Read@Attachments@@YA_NPEAXAEAUSet@1@@Z`,
  i.e. `Attachments::Read`. The instruction is
  `movzx eax, word ptr [r10+rdx*8+30h]` (stride 0x48, `defIndex` at +0x30). The
  logged access `0x2F` is `-1 + 0x30`: **the vector's data pointer was
  `(void*)-1` while its size was greater than zero.**
- Cause: the only pointer check was `if (vector->size > 0 && vector->data < 0x10000u)`,
  and `-1` passes it. Every "did not read back its attachments" line in that
  session may be this case rather than an item without stickers.

Changes:

- [x] `attachments.cpp`: `AttributeArrayUsable()` — size bounded by
  `MAX_ATTRIBUTES`, and for a non-empty vector the data pointer must be at least
  `0x10000`, at most `0x00007FFFFFFEFFFF` and 8-byte aligned. Used by `Read` and
  by the three in-place writers (`SetCharmIdInPlace`, `ClearCharmOnVector`,
  `StripNetworkedCharmAndStatTrak`). A rejected non-empty vector now logs
  `[attach] attribute vector rejected: size=… data=…` (debug log).
- [x] `log.cpp`: the fault-register line now includes r8-r15, rbx, rsi and rdi.
  Recovering r10 cost a full disassembly round-trip today; it is now in the log.
- [x] DLL version `Debug.v6.9`, MSBuild Release x64 exit 0, no warnings, symbols
  archived as `pdb/n1ssan-dll-v6.9.pdb`. The link could not write the in-place
  DLL while `cs2.exe` (PID 17668) held it, so it was built to `x64/Release/v69/`;
  once the game closed that binary was copied over `x64/Release/n1ssan-dll.dll`
  (in place now: 586,752 bytes, `Debug.v6.9` present, `Debug.v6.8` gone) and the
  temporary folder was removed.

Also worth recording from that session: `[charm-pass] ... addon slots: ...
2=0x1A19019A:end_of_match_lineup_end` resolves to a real CS2 HUD-model addon
entity, so the dictionary walk is not reading pure garbage — but neighbouring
slots hold `0x3F800000` (1.0f) and `0x0000001D` where a handle belongs. The
entry stride / handle offset (0x10 / +0x0C) is therefore not uniformly right, and
the "no keychain addon removed" result cannot be trusted until it is.

Charm detachment and the Storage Unit (the "Tools tab"):

- Lootline's Tools tab exists because its items live in its own id namespace
  (`Unboxer::FauxId`) that Valve does not know: a Detachment or a Storage Unit has
  to be answered locally, so it hooks the panorama charm popup
  (`RemoveKeychain`), the casket transaction (`PerformItemCasketTransaction`) and
  the tool charges field (`KeychainRemoveToolCharges.charges`).
- This runtime does not use that namespace: Armory items are created in the real
  local inventory (`LootSOCache`), so a charm is detached by unassigning it in the
  Armory - the next apply clears the charm attribute and `Charm::ProcessPending`
  takes the entity off - and a Storage Unit's count comes from
  `LootSOCache::SetCasketInfo`. Copying the three hooks would mean copying the
  faux-id item store underneath them, not three hooks. Not done, and no longer
  hidden behind a menu tab this runtime does not have.

### v6.10 — Operation 08 withdrawn, and Lootline's Tools tab copied over

The user: *"Delete both Storage Unit and Detachment logics we have, port over
Lootline's tool directly and add another tab in Add to Inventory. for Tools: add
both to Tools tab. No guess, no hybrid version, completely Lootline's."*

**What Lootline actually does** (`source/chadware2/menu/unboxer/unboxer_menu.cpp`
4179-4203, `source/chadware2/features/unboxer/unboxer.*`):

- The Tools tab is four cards, in this order: `DEF_CASKET 1201` "Storage Unit",
  `DEF_NAME_TAG_TOOL 1200` "Name Tag", `DEF_STATTRAK_SWAP 1324` "StatTrak Swap
  Tool", `DEF_CHARM_DETACH_PACK 4950` "Charm Detachment Pack" (`unboxer.h`
  42-47, `CHARM_DETACHMENTS_PER_PACK = 3`).
- Each one is added as an ordinary drop item of that definition (`AddItem`,
  `unboxer.cpp` 2140-2195). Nothing is a container: a Storage Unit holds nothing
  on Lootline's side, because CS2 keeps what the player puts inside it
  (`out.casketId = item.casket` only mirrors what the game reports).
- Two fields are set on add, and only those two: a named Storage Unit gets
  `modified = now` (`unboxer.cpp` 2160-2163), so CS2 offers "put in" / "look
  inside" straight away instead of asking for a name first, and a Charm
  Detachment Pack gets `itemsCount = 3` so its "use" button says how many it
  gives (`unboxer.cpp` 1638-1639).
- There is no Store / Retrieve / Detach button anywhere in Lootline's Armory:
  storing and retrieving happen in CS2's own inventory, and a charm comes off
  through CS2's charm popup using detachment charges.

**Deleted (Operation 08):**

- `storage_unit` as an Armory kind, `InventoryItemInstance.containerId` and its
  validation (no nesting, only a unit may hold items, links must resolve),
  `containersVersion`, `containerInstanceId` in the projection and in the native
  reader, and the native container resolution (units created first, a child's
  `casketId` looked up by instance id).
- The armory-side "casket item id low/high" (272/273) writer. CS2 writes and
  reads that pair itself for anything put in a unit in game.
- The Armory UI's Storage Unit catalogue card (it was injected first into the
  Stickers tab), the Storage Unit category and filter, the "N stored" card, the
  Store / Retrieve rows and the **Detach charm** button with its
  `attachments.charm = null` patch.
- `containersVersion: 1` and the `containerId` field are gone from the Bridge
  document. A profile written before v6.10 is converted as it is read
  (`migrateContainerItems` in `bridge/armory-store.ts`, also applied to the undo
  history): the unit becomes the Storage Unit tool and its former contents become
  loose items, so the change can never quarantine - let alone empty - an
  inventory that was valid under the old rules.

**Added, to Lootline's spec:**

- `toolsCatalogue` in `src/armory/ui/catalogue.ts`: the same four cards, same
  definitions, same order, `type: 'Tool'`, Consumer Grade.
- A **Tools** tab in the Add to Inventory dialog (`AddToInventoryDialog.tsx`),
  listing them, with its own search and no facet panel (they carry no facets);
  the Stickers tab no longer injects the Storage Unit card and its count is one
  lower.
- A Storage Unit is named where it is added, in the quantity step, as Lootline's
  panel does; a tool never merges into a stack, so two units keep their two
  names.
- `kind: 'tool'` projects as one plain item of its own `definitionIndex`: no
  attributes beyond the two Lootline sets — a named Storage Unit's modification
  date (the instance's `createdAt`, in seconds) and a pack's `itemsCount: 3`.
- The native reader takes `modificationDate` and passes it to
  `LootSOCache::SetCasketInfo` for a retained unit, so a rename reaches the date
  that enables "put in".
- Inventory: the Storage Unit filter became **Tools**, and a tool card reads its
  description instead of a stored count.

**First-run incident (23:00, the user's live Armory):** the page reported
`The n1ssan Bridge returned an invalid Armory document:
profiles[2].items[164].kind: unsupported item kind; profiles[3].items[14].kind:
unsupported item kind`. The two items were the user's real Storage Units. The
Bridge process (PID 24984, `node --import tsx bridge/server.ts`, parent already
gone) had been started before this change and was serving its in-memory document,
so the read-time conversion had never run for it. Restarted that Bridge; it read
`armory-data.json`, converted both units to `cs2:tool:external:tool-storage-unit`
and kept every other item (184 skins, 15 stickers, 29 charms, 2 music kits, 3
graffiti). The conversion now also rewrites the primary file once
(`migrateContainerItems(...).changed` in `readDocument`), so the disk copy no
longer carries `storage_unit` or `containerId` at all. Checked afterwards: the
live `/armory` document passes `validateArmoryDocument` with 0 errors, one
`tool` item in each of the two profiles, and the file on disk holds no
`storage_unit` / `containerId` string.

**Follow-up (23:20, "not working both detachment and Storage Unit and missing
previews in Armory"):** two separate defects, both mine.

1. *The tools never reached the game.* The projection resolved a tool's item
   definition from the Armory catalogue (`entry?.definition.gameDefinitionId`),
   but the Bridge is never handed the browser's tool catalogue: its records are
   the skin, sticker and charm feeds. Every tool therefore hit the
   "no CS2 definition index could be resolved" skip and the projection carried
   187 items with zero of definitions 1200/1201/1324/4950, so nothing could exist
   in CS2 to use or detach with. Fixed with `TOOL_DEFINITIONS` in
   `armory-inventory.ts`: the four canonical ids map to Lootline's numbers
   (1201/1200/1324/4950), used when the catalogue has no entry. A test now
   projects a tool against an empty catalogue and asserts 1201/4950, the name and
   the pack's three detachments. After a Bridge restart the live projection reads
   189 items including `1201` named "666" with `modificationDate` and `4950` with
   `itemsCount: 3`.
2. *No previews for the tools.* `toolsCatalogue` shipped no `image`, and both
   cards render `<img src="">`, so the two tool items sat in the Armory as broken
   tiles (verified in the running page: 168 cards, 248 images, exactly 2 broken,
   both tools). Four icons were drawn as SVG in `public/tools/` — a stack of
   crates, a tag, a StatTrak swap ring and a charm with a broken link — and each
   tool card now names its own file. Re-checked in the page: 0 broken, 0 empty
   `src`, and the Add-to-Inventory Tools tab draws all four cards with their
   icons (Storage Unit, Name Tag, StatTrak Swap Tool, Charm Detachment Pack).

**Verified:** Web Bridge `tsc -b` + `vite build` (71 modules, exit 0);
`npm test` 118/118 (the storage-unit projection test replaced by a tools test
asserting definitions 1201/4950, the name, the modification date and the pack's
three detachments; a store test loading a pre-v6.10 document with a unit and a
linked item); `npm run build:bridge` (bundled `runtime/bridge/server.js`);
native Release x64 exit 0, no warnings, in place as `x64/Release/n1ssan-dll.dll`
(584,192 bytes, `Debug.v6.10` present, `containerInstanceId` and `casket id=`
absent), symbols archived as `x64/Release/pdb/n1ssan-dll-v6.10.pdb`.

**Not verified:** the dialog itself was not opened in a browser, so the new Tools
tab, its count and its four cards have not been looked at. And nothing in game:
the four tool definitions have to exist in
this client's item schema for CS2 to draw them; the projection no longer checks
the schema for tools (Lootline does not either), so a missing definition would
fail at creation and log it instead of being skipped up front. That CS2 accepts
"put in" on a unit dated from `createdAt`, and that the pack shows three charges,
have not been seen.

### Not yet verified

- [ ] In game: detaching a charm removes it from an applied weapon on the next pass
  and leaves the charm item in the inventory. v6.6 changes when and how that
  happens; it has not been seen in game. What the next log says: `[charm] gun=…
  charm off: attribute cleared=N, M module(s) removed`, at most one `[charm]`
  line per second per gun, and only while that gun is in your hands. A charm that
  stays on screen reads `[charm] gun=… wanted=0 hanging=1 first=…: repairing`
  once a second until the cap - that would mean CS2's `AddKeyChainEntity` no
  longer takes a charm off when the attribute is zero.
- [ ] In game: the stall and the ping. The per-pass entity walks are gone
  (`Charm::ProcessPending` is the only caller, once a second, active weapon), so
  the `[perf-spike]` frames of 60-333 ms should not recur. Not measured in game.
- [ ] In game (v6.7): the round-start / death-screen crash is gone. Expected log:
  no `[knife] HUD SetModel`, `[knife-anim] inspect/reflect fix NOT installed` at
  start-up, no `[glove-model]`, `[team-switch] ... no hold, applying at once`.
  What must not come back: `[apply-cache] weapon model cache invalidated` followed
  by a `[fault] ... n1ssan-dll.dll+...` line, and an `animationsystem.dll` fault
  with `panorama.dll` in its stack.
- [ ] In game (v6.7): the apply pass no longer takes seconds. Snapshot log line:
  `[perf] apply pass took N ms`. The 2242 ms pass of 11:43 was one knife write
  (SetModel + material) on a round-start frame; the next session should show tens
  of milliseconds, and no "Stop receiving communication from host".
- [ ] In game (v6.7): CT gloves arrive with the first frame of the half (no 3 s
  hold) and survive the round. Expected: one `[glove-runtime] respawn detected`
  line per respawn, `[glove-runtime] paint kit gone; re-applying` only if CS2
  really dropped the kit, and no bare hands in the first seconds.
- [ ] In game (v6.7): charm detachment still works without the glove upkeep (the
  two were independent, but the upkeep used to kick the material manager).
- [ ] In game: the T-side crash is gone. The `client.dll+0xAA9E3C` fault of the
  23:33 and 23:42 sessions did not recur in v6.3; both v6.3 sessions instead
  ended in the render path (`animationsystem.dll` reading null+0x18 at 00:44,
  an `execute of 0x0` at 00:55), with repeated removal of already-removed
  keychain entities in the log before them. v6.4 stops that repeat and holds
  charm entity work through a round start. What to look for: no
  `[charm-clean] removed world keychain` twice for the same entity pointer
  (`[charm-clean] skip repeat removal` instead), no `[charm]` line inside the
  five seconds after a respawn, and - if it still dies - the new
  `[crash] fault access/registers/stack` lines, which name the faulting frame
  instead of a bare address.
- [ ] In game: the counter is the new capsule - purple gradient, no moving
  outline, smaller - and it does not flicker when the value changes. The artwork
  was rendered and looked at outside the game; where it sits, and how it reads
  over a bright sky, was not.
- [ ] In game: the ping the scoreboard shows. Nothing in this runtime sends or
  receives network traffic, so if the number is high in every match regardless of
  what the runtime is doing, the cause is the connection or the server. The
  round-start pass is lighter now; whether that removed any ping spikes has to be
  watched in a session (`[perf-spike]` lines next to the scoreboard).
- [ ] In game: deaths no longer end the session. Three unguarded paths were closed
  on the code shape the dump shows; the null pointer itself is not yet proven to
  be gone until the client survives a session of deaths, and if it survives only
  partly, the next `[crash] return N: ...` lines name the frame in the log.
- [ ] In game: a Storage Unit shows the name the Armory gave it. Renaming a unit
  from CS2's own "Change Label" still has to be intercepted at the request the
  client sends to the Game Coordinator: the popup waits for that reply and a
  client-injected item never receives one, so the label is set from the Armory
  until that hook exists.
- [ ] CS2's own Deposit / Retrieve are not observed yet; the document is the only
  authority, so an in-game move is not mirrored back to the Armory.
- [ ] The unit's count after a deposit made in game (the attribute is only rewritten
  when the Armory projection changes).

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

**Operation 07 — Apex**

Operations 00 through 03 are accepted as complete. Armory's in-game acceptance was verified by the user on 2026-11-13. Stormwatch and Reforge are implemented: diagnostics are consolidated behind one document and one failure taxonomy, every contract is documented, and the full automated suite was run and recorded. Their remaining items are live-session validation, not unresolved design.

Atlas is complete: one build script, one version file, a reproducible manifest with component commits and artifact hashes, and documented environment, release, and contribution rules.

Apex is code-complete and has been run against the live game. On 2026-11-13, with CS2 at the main menu, `Find Offset` resolved all three signatures from the running `client.dll` and `Update Offset` wrote them into `offsets.json` (dataset `2026.11.13.2`, previous file kept as `offsets.json.bak`). `development/n1ssan-apex` is a Tauri 2 shell (Rust backend, React + Tailwind frontend) whose 250x250 N1ssan Sync window detects `cs2.exe`, scans the shipped `offsets.json` patterns against the live `client.dll`, reports each entry as match / moved / missing / new with a runtime confidence and validation message, writes only trusted offsets back into the dataset with a version bump and a `.bak` copy, and fetches definitions from an operator-configured source before applying anything. The dataset is staged into every release and `verify-package` refuses one that carries unresolved or low-confidence entries. What still needs a human: confirming the confidence classification against a live CS2 session after the next game update, and supplying the real definition source.

The next priorities are:

1. **Apex live confirmation** — re-scan after the next CS2 update to confirm the confidence classes against the real module, and point Apex at the real definition source.
2. **Follow-up validation** — repeat the in-game acceptance pass across T/CT and restarts; verify packaged runtime paths.
3. **Remaining state polish** — align loading, empty, error, and success states across both workspaces; the shared visual language itself is now in place.

Any future hardening or polish can be tracked as follow-up work without reopening completed operations unless scope materially changes.

Solid Safe House remains the preserved development baseline.

---

## Audit Notes — 2026-11-11

- Audited the main repository and the Solid Safe House component branches for the launcher, injector v2, native DLL, Web Bridge, and gloves component.
- Confirmed the five submodule entries in the main repository's `.gitmodules`.
- Inspected the launcher startup state machine, Bridge process manager, injector workflow documentation, DLL status contract, live schema snapshot documentation, Armory contracts/stores/projections, native inventory and attachment modules, and integration simulator.
- Recent Armory rarity-coloring build success is based on the user's reported local build output; it is not represented as a fresh remote CI or clean-machine build.
- No end-to-end CS2 acceptance test or full cross-component test-suite result was established by this repository audit.

- Operation status updates (user-approved): Overdrive and Obsidian accepted as COMPLETE; Armory advanced to IMPLEMENTED; Clean State recorded as COMPLETE; Reforge designated ACTIVE as the next operation.

---

## Audit Notes — 2026-11-13

- User verified in game that the apply pass no longer crashes CS2 and that AWP/M4A4 no longer render a second charm; the same session confirmed the web surfaces behave correctly. Armory is accepted as COMPLETE on that evidence.
- Status record contract advanced to protocol 2 (trailing `code=`), accepted by the launcher, injector watcher, Bridge, and the status harness; protocol 1 records remain readable.
- `failure-taxonomy.md` and `contracts.md` added at the project root as the maintained references for codes and documents.
- Verification actually performed: native DLL x64 Release build; launcher and injector `dotnet build`; Web Bridge `tsc -b`, `npm run build`, `npm test` (100/100); status harness (38/38); live `/health` and `/diagnostics` on a loopback test port.
- Not performed: an in-game run of this exact binary (the acceptance above predates the diagnostics/taxonomy changes) and a click-through of every Armory/Loadout dialog.
- User verification (2026-11-13, launcher run from the Debug folder): new profiles are empty in both workspaces, search suggestions and per-bar search history behave correctly. Armory/Loadout visual unification was explicitly deferred.
- Operation Atlas completed: `VERSION` 3.0.0, a single release script with `verify`, `tools\release.ps1`, and the environment/release/contribution references. Release 3.0.0 was built and archived as `release\n1ssan-3.0.0-win64.zip`, with `release\n1ssan\manifest.json` recording the component commits, contract versions, and artifact hashes.
- Baseline pushed on 2026-11-13: main `solid-safe-house` plus every component's Solid Safe House branch (`solid-safe-house-dll`, `-injectorv2`, `-launcher`, `-bridge`). Submodule pointers match the pushed commits.

## Audit Notes — 2026-11-13 (workspace pass)

- Apex icon set regenerated from the project brand glyph: `src-tauri/icons` now carries consistent 32/64/128/256/512 PNGs plus a multi-frame `icon.ico` (the previous `.ico` was still the Tauri default while the PNGs were already rebranded).
- Build scripts consolidated: `n1ssan-build.bat` was removed and its build driver folded into `release\n1ssan-release.bat`, which resolves the workspace root from its own folder and keeps the `build` / `verify` / `verify-package` / per-component targets. `.gitignore` keeps the script tracked while release output stays ignored.
- Apex implemented the last of Operation 07: runtime pointer validation with confidence, an apply gate that refuses low-confidence findings, `E_APEX_FETCH` with a configurable definition source, and a `verify-package` rule that fails a release with unresolved or low-confidence dataset entries.
- Loadout adopted Armory's visual system: the navy palette and the legacy Loadout builder stylesheet were replaced by `shared/tokens.css` and `obsidian.css` surfaces, item cards, dialogs, and the same shared components and navigation props.
- Verification run in this pass: `npx tsc -b` and `npm run build` (Web Bridge), `npm test` (103/103), `npm run build` (Apex), `cargo check`, and `cargo test` (Apex). No in-game session was performed, so the new confidence classification is not yet confirmed against a live CS2 module.

## Release History

- **Beta.Build.v1.0.1** - 2026-11-26 | archive `n1ssan-Beta.Build.v1.0.1-win64.zip` | main `0383bdb170f3` dll `8880e58a2d06` injector `4d8b0015263e` launcher `568ede3c0902` bridge `a47358693d79`
- **Beta.Build.v1.0.2** - 2026-11-15 | archive `n1ssan-Beta.Build.v1.0.2-win64.zip` | main `54f1779989f7` dll `2e70165f8de1` injector `eb884d1d9271` launcher `95b4050a7204` bridge `ce65fd9993b7`
- **Beta.Build.v1.0.3** - 2026-11-14 | archive `n1ssan-Beta.Build.v1.0.3-win64.zip` | main `54f1779989f7` dll `2e70165f8de1` injector `eb884d1d9271` launcher `95b4050a7204` bridge `ce65fd9993b7`
- **Beta.Build.v1.0.0** - 2026-11-13 | archive `n1ssan-Beta.Build.v1.0.0-win64.zip` | main `0689d2fd4f6d` dll `2e70165f8de1` injector `eb884d1d9271` launcher `95b4050a7204` bridge `ce65fd9993b7`


## v6.11 - Lootline's tool hooks, ported

*v6.10 shipped the tools as catalogue entries and projection fields. That was
half of Lootline: the items existed in CS2, and nothing answered them. This
version ports the other half - the three InventoryAPI functions Lootline hooks
in `chadware2/hooks/hooks.cpp`, which is why its Storage Unit and Charm
Detachment Pack work and ours did not.*

**What Lootline does (evidence, Lootline 1.5.5 source):**
- `hooks.cpp:586 hkPerformItemCasketTransaction` -> `unboxer.cpp:5063
  OnCasketTransaction`: operation `1` put in, `-1` take out, `0` look inside;
  refused when the unit is unnamed, full (`CASKET_CAPACITY`, `unboxer.h:46`), a
  unit inside a unit, or the item is not in that unit. Installed at
  `hooks.cpp:1895` from `InventoryAPI.PerformItemCasketTransaction`.
- `hooks.cpp:745 hkCacheTypeField`: for type `KeychainRemoveToolCharges`, field
  `charges`, returns Valve's count plus `Unboxer::CharmDetachments()`
  (`unboxer.cpp:7600`). Installed at `hooks.cpp:1928` from
  `InventoryAPI.GetCacheTypeElementFieldByIndex`. Without that number CS2's
  charm popup refuses every detach.
- `hooks.cpp:284 hkRemoveKeychain` -> `unboxer.cpp:4908 OnRemoveKeychain`: takes
  the charm off locally and spends one detachment (`--g_charmDetachments`).
- After each transaction the popup is told what happened with the ids Lootline
  uses: `casket_added` 1013, `casket_removed` 1014, `casket_too_full` 1011,
  `casket_contents` 1012 (`unboxer.cpp:6932-6934`), `keychain_remove` 1092
  (`unboxer.cpp:79`).

**What n1ssan now has:**
- `src/inventory/tools.h` / `tools.cpp` (new): the same three detours, resolved
  **by registered name** through the resolver `native_apply.cpp` already used
  for `UseTool` and now exposes as `NativeApply::ResolveInventoryFunction` -
  no new byte signatures, so a game update that keeps the registrations keeps
  the hooks. `NativeApply::NotifyCustomization` exposes the existing
  Coordinator-style notification dispatch for the five ids above.
- Storage membership is kept per real item id (`item -> unit`) with the unit's
  count, written into `n1ssan-tools.json` beside the Bridge documents, the same
  way Lootline keeps it in its own save.
- The unit's `items count` is written with `LootSOCache::SetCasketInfo`; the
  unspent detachment count is `3 x packs in the projection - spent`, so a
  re-Apply never inflates it.
- `armory_inventory.{h,cpp}` gained `FindManaged`, `CountManagedByDefinition`
  and `AdoptionGeneration` (bumped when a projection is published) so the tools
  module can re-apply counts after an Apply has rewritten the items, and drop
  membership for items the profile no longer carries (Lootline's
  `ReleaseCasketContents`).
- A charm taken off through CS2's own popup is written with
  `Attachments::RecordApplied`, so the Bridge merges it into the profile and it
  stays off across an Apply and a restart.
- Hooked from `frame_stage.cpp`, in the same once-a-second Armory pass as
  `NativeApply::Tick`.
- Both writes that notify CS2 (`SetCasketInfo`) happen outside the module's
  lock: the notification makes CS2 read the detachment charges, which is the
  same lock again.

**Known divergence, deliberate:** a stored item stays visible on CS2's
inventory grid. Lootline's items have faux ids and live only in its own store,
so they are invisible to the grid by construction; n1ssan's are real objects in
the local shared-object cache and the grid lists that cache. Hiding one would
mean removing the live object (`LootSOCache::Remove`) and rebuilding it from
the projection on take-out - a destructive path that cannot be tested without
the game, and one that loses the item outright if the rebuild fails. The unit's
count and the membership map are authoritative until that is proven.

**Version:** `Debug.v6.11` (`overlay_version.h`), PDB archived as
`x64/Release/pdb/n1ssan-dll-v6.11.pdb`, DLL in place at
`x64/Release/n1ssan-dll.dll` (602,624 bytes).

**Verified:** MSBuild Release x64 exit 0 with no warnings; the built DLL
contains `PerformItemCasketTransaction`, `GetCacheTypeElementFieldByIndex`,
`Debug.v6.11`, `[tools] ready` and `n1ssan-tools.json`.

**Not verified (needs the game):** whether CS2's casket popup accepts the local
answer, whether the charm popup enables itself from the added charges, and
whether `RemoveKeychain` fires before the game sends anything to Valve. Log
lines to check: `[tools] ready: storage unit ..., charm detachment charges ...,
charm take-off ...`, `[tools] storage: ... answered locally`, `[tools] charm:
taking the charm off ...`, and `[tools] storage: unit N now holds`.
## v6.12 - why the tools still spun, and the fix

*User report after v6.11: "still not working, stuck spinning in game." The
session log (`x64/Release/n1ssanlogs.md`, 2026-11-26 00:16-00:20) says why.*

**What the log showed:**
- `[tools] InventoryAPI.PerformItemCasketTransaction hooked at 0x...5AD0`
- `[tools] InventoryAPI.GetCacheTypeElementFieldByIndex hooked at 0x...7110`
- `[tools] InventoryAPI.RemoveKeychain not found in this build`
- `[tools] ready: storage unit ok, charm detachment charges ok, charm take-off
  NOT installed`
- and **not one** `[tools]` transaction line after that, while four
  `[use-tool] hook-entry #1..#4` lines prove clicks were happening with
  `mode=2` (Armory).

Two independent faults, one of them mine and structural:

1. **The answer was sent from the wrong thread.** v6.11 drained the tool jobs
   and sent `casket_added` / `casket_removed` / `casket_contents` /
   `keychain_remove` from `FrameStageNotify` (the game thread). Panorama
   accepts UI events only from its own frame callback: Lootline does its tool
   work in `CUIEngine::RunFrame` and says so at `unboxer.cpp:6961`
   (`NotifyItemCustomization` out of the casket job loop) and
   `native_apply.cpp` in this runtime already had to learn the same lesson for
   sticker and charm popups. An event from anywhere else is dropped, so the
   popup spins forever even though the state changed.
2. **`RemoveKeychain` never resolved.** The strict registration scan wants
   `lea rdx, [rip+function]` exactly seven bytes before `lea rcx, [rip+name]`.
   This build registers at least that one differently, so the charm popup was
   never intercepted at all.

**What changed:**
- `Tools::MainThreadTick()` (new) drains the queue and sends every notification;
  it is called from the `CUIEngine::RunFrame` vfunc hook in
  `native_apply.cpp`, guarded like the other frame work. `Tools::Tick()` keeps
  only installation and unit-count syncs. The job list now has its own mutex
  because it is filled on the UI thread and drained on panorama's thread.
- `ResolveScriptFunctionLoose` in `native_apply.cpp`: when the strict scan
  misses, every rip-relative load of the name in `client.dll` is paired with
  the nearest rip-relative code load in the 0x40 bytes before it, which covers
  the registration shapes the strict form does not. `ResolveInventoryFunction`
  tries strict first, then loose.
- Instrumentation, so the next session is not guesswork: `[tools] casket entry
  #N op=.. unit=.. item=.. mode=..`, `[tools] casket: unit X is not an Armory
  Storage Unit (found=.. definition=..) - forwarded to CS2`, `[tools] casket:
  mode is N, not Armory`, `[tools] charm entry #N item=.. slot=..`,
  `[tools] charges read #N: CS2=.. ours=.. answered=..`.

**Version:** `Debug.v6.12`. CS2 was still running (PID 28492) and had v6.11
mapped, so the build could not replace the loaded file; it is staged at
`x64/Release-v612/n1ssan-dll.dll` with its PDB at
`x64/Release-v612/pdb/n1ssan-dll-v6.12.pdb`. Install with
`cp x64/Release-v612/n1ssan-dll.dll x64/Release/n1ssan-dll.dll` once the game
is closed.

**Verified:** MSBuild Release x64 exit 0, no warnings; the staged DLL contains
`Debug.v6.12`, `casket entry`, `charges read`.

**Not verified:** the in-game result. The new log will answer it directly:
- `casket entry` lines appear -> the detour is reached; the following
  `[tools] storage: ... answered locally` plus `popup told` is the fix
  working.
- no `casket entry` line at all -> the addressed function is not the one the
  popup calls, and the next step is the vtable entry rather than the
  registration.
- `charges read` with `ours=3` -> the charm popup should now offer a detach.
## v6.13 - parity check against Lootline's source

*A line-by-line read of Lootline 1.5.5 (`hooks.cpp`, `unboxer.cpp`) against
`tools.cpp`, asked for as "double check if the code matches Lootline". Two real
mismatches were found and fixed; the rest is listed with what remains.*

**Fixed in this version:**
- **The pack is used, not owned.** Lootline banks the detachments when the
  player USES the Charm Detachment Pack: `FinishToolJobs` (`unboxer.cpp:7201`)
  does `g_charmDetachments += CHARM_DETACHMENTS_PER_PACK`, erases and removes
  the pack item, then
  `NotifyItemCustomization(0, "keychain_tool_charges", toolId)`. v6.11/v6.12
  instead derived charges from packs merely being in the projection, and
  `UseTool` on a pack was forwarded to Valve - which is a spinner of its own.
  Now: `Tools::HandleUseTool` is called from `QueueUseTool` before the
  sticker/charm checks (the same place Lootline's `OnUseTool` calls
  `QueueToolUse`), banks three charges, removes the pack, sends
  `keychain_tool_charges`, and remembers the id so `DropSpentPacks()` removes it
  again after every Apply (Lootline's save simply forgets the item).
- **Charges are banked, not derived.** `detachmentCharges` in
  `n1ssan-tools.json` replaces the `3 x packs - spent` formula, matching
  Lootline's persisted `charm_detachments` (`unboxer.cpp:1401`). One take-off
  spends one, floored at zero, like `OnRemoveKeychain`.

**Matches Lootline (verified against its source):**
- `PerformItemCasketTransaction` answered locally: op 1 put in, -1 take out,
  0 look inside; unnamed unit refused, capacity 1000 (`unboxer.h:46`), no unit
  inside a unit, item must belong to that unit.
- Unit count and date rewritten with `SetCasketInfo`; notification ids
  `casket_added` 1013 / `casket_removed` 1014 / `casket_too_full` 1011 /
  `casket_contents` 1012, the 0x30-byte event through the UI engine's dispatch.
- `KeychainRemoveToolCharges` / `charges` carries Valve's count plus ours.
- `RemoveKeychain` only answers for our items that actually carry a charm.
- Detachment spend on take-off, and the popup told with `keychain_remove` 1092.

**Still different, deliberately or not yet done:**
- A stored item stays on CS2's grid (v6.11 note: real cache objects, and hiding
  them means destroying and rebuilding them).
- Lootline removes and remakes the item after a casket move and unequips it
  (`unboxer.cpp:6995`, `7023`); we do neither.
- Lootline's `ReleaseCasketContents` puts a deleted unit's items back; ours
  drops the membership when the profile no longer carries the unit.
- **StatTrak Swap Tool (1324) is not ported at all.** Lootline hooks
  `InventoryAPI.SetStatTrakSwapToolItems` (`hooks.cpp:1925`) to learn the two
  items and swaps their kill counts in `FinishToolJobs`
  (`unboxer.cpp:7245-7258`), then notifies `stattrack_swap`. Ours has no such
  hook; using the tool is forwarded to Valve. This is the remaining hole in the
  Tools tab.
- Slab "crack and extract" through `RemoveKeychain` (a charm item carrying a
  sealed sticker) is not handled.

**Diagnostics added:** `[tools] casket field: type=.. field=..` (first eight
calls whose type contains "Casket"), so a session in which nothing happens
still shows whether CS2 ever asked the Storage Unit screen for casket data -
which distinguishes "the popup never got that far" from "our detour was never
reached".

**Version:** `Debug.v6.13`, installed at `x64/Release/n1ssan-dll.dll`
(609,280 bytes) once CS2 was closed, with `pdb/n1ssan-dll-v6.13.pdb` archived;
the previous build is kept as `x64/Release/n1ssan-dll-v6.11.bak.dll`. The
installed file was checked for `Debug.v6.13` (and the absence of `Debug.v6.11`),
`charm detachments: pack`, `keychain_tool_charges`, `detachmentCharges` and
`casket field`.

**Verified:** MSBuild Release x64 exit 0, no warnings; the staged DLL contains
`Debug.v6.13`, `charm detachments: pack`, `keychain_tool_charges`,
`detachmentCharges`.

**Not verified:** everything in game.

## v6.14 - why the two screens still spun (from the v6.13 log)

The v6.13 log named both causes; neither is in the tool hooks' own logic.

- **The pack click never reached the tools module.** `[use-tool] hook-entry` ×3
  with `mode=2` and no `[tools]` lines: `QueueUseTool` refused every request
  with an empty item id *before* `Tools::HandleUseTool`. Lootline takes the
  pack as `UseTool(tool, "")` (`unboxer.cpp:8956` "our StatTrak Swap Tool /
  Charm Detachment Pack (UseTool(tool, ""))" -> `QueueToolUse`,
  `unboxer.cpp:7065`: `if (subject != 0 && subject != toolId) return false;` -
  an empty subject is accepted). Fixed: the tool check runs first, and the
  empty id now only means "no apply target", not "malformed request". The
  hook-entry line also logs the two ids as strings, so the next log shows
  exactly what CS2 asks for.
- **The Storage Unit had no "modification date".** `[loot-schema]
  SetAttributeInt(271): def null` / `271` and `270` "not written (attribute not
  in this schema)": `LootSchema::GetCachedAttrDef` only knew ids 6, 7, 8, 80,
  81, so "items count" (270) and "modification date" (271) could never be
  written - which is exactly the attribute CS2 checks before it lets a unit be
  used, so it opened "Personalize Your Storage Unit" and no casket request was
  ever made. Lootline resolves any attribute by index through the econ item
  schema's vtable slot 27 (`game_inventory.cpp:406`, `600`,
  `VFUNC_ATTRIBUTE_DEFINITION = 27`). Fixed: `AttrDefByIndex` does the same and
  is the fallback when the by-name lookup has nothing; 270/271 are cached.
- **A named unit gets its date.** New `WriteNamedUnitDates()` (called from
  `Tools::Tick` after an Apply publishes a new projection) writes
  `SetCasketInfo(id, storedCount, date)` for every Storage Unit the projection
  names, so CS2 stops asking for a name. Lootline names a unit by writing
  `modificationDate` (`unboxer.cpp:3572`, `1634`), which is the same thing.
- **Pack preview**: `[socache] items count=3 not written` is now written too -
  Lootline sets the pack's `itemsCount` to `CHARM_DETACHMENTS_PER_PACK`
  (`unboxer.cpp:1639`) for the "how many it gives" line.

**Expected new lines in the next log:** `[tools] storage unit <id> "<name>":
0 item(s), modification date <n>`, then `[tools] casket entry #1 op=...`,
`[tools] storage: ...`; on the pack `[use-tool] tool <id> handled by the tools
module`, `[tools] charm detachments: pack <id> used - 3 now`, and
`[tools] charges read #N: CS2=0, ours=3, answered=3`.

**Version:** `Debug.v6.14`, installed at `x64/Release/n1ssan-dll.dll`
(611,840 bytes, md5 337ef053228603e07f7f1d1230458613, identical to the staged
build) once CS2 was closed. The previous build is kept as
`x64/Release/n1ssan-dll-v6.13.bak.dll` (609,280 bytes) and its symbols as
`pdb/n1ssan-dll-v6.13.pdb`; this build's symbols are `pdb/n1ssan-dll-v6.14.pdb`
and `x64/Release/n1ssan-dll.pdb`. The installed file was checked for
`Debug.v6.14` (and the absence of `Debug.v6.13`),
`modification date could not be written`, `handled by the tools module`,
`charm detachments: pack`, `casket entry`, `casket field`.

**Verified:** MSBuild Release x64 exit 0, no warnings; installed bytes as above.

**Not verified:** everything in game. No pack use, no casket transaction and no
charm take-off has been observed running v6.14.

## v7.5 - the 2026-11-26 session: what the log actually says

**Version:** `Debug.v7.5`. The counter had run past 0.9 (v6.10 - v6.14);
it rolls over now, as `overlay_version.h` says it should: after .9 the major
goes up and the minor restarts (v6.9 -> v7.0), so 6.14 should have been 7.4.

### The session was clean, and three things happened

`x64/Release/n1ssanlogs.md`, PID 25444, 18:22:29 - 18:24:22, ended with
`[session] clean exit`. No crash, no fault line. What the log shows:

1. `[use-tool] hook-entry #2 ... toolArg=("11106208501144225413") targetArg=("")`
   -> `[use-tool] tool ... handled by the tools module` ->
   `[tools] charm detachments: pack ... used - 3 now (item used up, popup NOT told)`
   -> `[tools] charges read #3: CS2=0, ours=3, answered=3`.
   The pack is unpacked, the charges are banked, and CS2 asks for them and is
   answered 3. The v6.14 empty-id fix works.
2. Three charm applies (`hook-entry #3/#4/#5`) completed in full: attributes
   written, charm item consumed, `[attach] immediate native observation`,
   `file=written`. The charm path works.
3. **No `[tools] casket` line of any kind**, and
   `[socache] storage unit retained id=99137C159E61486B items=0 name=""` -
   the unit the game has carries no name.

### Storage Unit: the unit in the active profile has no name

Read from the profile the mod projects (`armory-inventory.json`, generated
18:24:10): the active profile `Horus` (`profile-1793694856473-vrmr`) holds
`item-1795630866168-myzyd`, definition 1201, `customName: null`. The named unit
("333") belongs to another profile (`Main Acc`) and is not the one in the game.

That is exactly Lootline's own rule, in its own words
(`unboxer.cpp:2160`): *"a Storage Unit named in Add items: named today, so CS2
offers 'put in' / 'look inside' right away (without a 'modification date' it
asks for a name first)"*, and its casket handler refuses with
*"name the Storage Unit first"* when `modified == 0` (`unboxer.cpp:5091`).

So CS2's "Personalize Your Storage Unit" screen is not a bug in the hook: an
unnamed unit has no modification date, CS2 asks for the name, and never sends a
casket transaction for us to answer. `WriteNamedUnitDates()` correctly skips
it. Lootline has no code for CS2's rename dialog either - its units are always
named before they reach the game.

**Fix (v7.5):** the skip is now logged, once per unit:
`[tools] storage unit <id> has no name in the profile - CS2 asks to personalize
it and sends no casket request (Lootline: "name the Storage Unit first")`.

**Action:** name the unit in the Armory (Tools -> Storage Unit -> name), so the
projection carries a `customName`; v6.14's `WriteNamedUnitDates()` then writes
the name's date and CS2 offers put in / look inside.

### Charm detach: the answer Lootline gives is `RemoveKeychain`, and we never install it

The detachment is the one thing our module cannot answer at all:

```
[tools] RemoveKeychain: loose registration scan found no code pointer (name loads=0, code loads=128)
[tools] InventoryAPI.RemoveKeychain not found in this build
[tools] ready: storage unit ok, charm detachment charges ok, charm take-off NOT installed
```

Lootline hooks it by name (`hooks.cpp:1855`,
`GameInv::InventoryApiFunction("RemoveKeychain")`) and logs "charms: ok (on and
off Lootline guns)". Applying a charm goes through `UseTool` (that path works,
see above); taking one off goes through `RemoveKeychain` only. With the hook
missing, CS2's detach request is never answered, and its popup spins until the
player backs out - which is what was seen.

**Why the name was never found:** our `FindModuleString` accepted a match at any
offset as long as the bytes *ended* with `name\0`. A copy that starts inside a
longer string (so a different address from the one the client registers) was
therefore taken for the name, and nothing in the code loads that interior
address: `name loads=0`. Lootline's `FindStringLocked` also requires a NUL
*before* the match (`hit[-1] == 0`), i.e. a standalone string.

**Fix (v7.5):** new `FindModuleStrings()` returns every *standalone* copy
(the byte before is NUL), and both resolvers now try each of them:
`ResolveScriptFunction` (strict `lea rdx,[function] / lea rcx,[name]`) and
`ResolveScriptFunctionLoose` (the fallback pairing). The failure line now prints
`copies=%d` beside the load counts.

### The notification: "sent" and "handled" are different things

Every apply logged `popup NOT told`, including the ones that plainly worked.
That was our logging, not the event: Lootline's `NotifyItemCustomization`
returns true once the engine has taken the event and keeps the dispatch's own
answer in `handled` - *"an event nobody 'handles' still reaches
RegisterForUnhandledEvent"*. Ours returned the dispatch's answer as if it were
delivery.

**Fix (v7.5):** the notification now returns "sent", and logs a line of its own
when CS2 reports nobody handled it:
`[use-tool] popup notification sent (type <t> "<name>", item <id>): CS2 handled it = no`.

**Verified:** MSBuild Release x64 exit 0, no warnings; installed at
`x64/Release/n1ssan-dll.dll` (613,376 bytes, md5
2cb9d0fa1e6f58be8b14363866533705, identical to the staged build), symbols at
`pdb/n1ssan-dll-v7.5.pdb`; the previous build kept as
`x64/Release/n1ssan-dll-v6.14.bak.dll`. The installed file was checked for
`Debug.v7.5` (and the absence of `Debug.v6.14`), `no name in the profile`,
`popup notification sent`, `copies=%d`.

**Not verified in game:** whether the widened name lookup now finds
`RemoveKeychain` and the detach popup closes; whether a named unit opens its
casket. Neither can be run from here.

**Next log lines to look for:** `[tools] InventoryAPI.RemoveKeychain hooked at
0x...` and `[tools] ready: ... charm take-off ok`; on a detach attempt
`[tools] charm take-off ...` / `[use-tool] popup notification sent`; on a named
unit `[tools] storage unit <id> "<name>": N item(s), modification date <n>` and
then `[tools] casket entry #1 op=...`.

## Lootline 2.0 - measured against our tree (2026-11-26)

Source: `E:\CS2\Lootline\Lootline_v2_source_[unknowncheats.me]_\Lootline`.
Measured, not estimated: `features/*.cpp` is **45,521 lines** across 30+ files
(`unboxer/unboxer.cpp` alone is 10,600), plus ~20,000 lines of ImGui menu.
`development/CS2-n1ssan/src` is 30,181 lines in total. The new feature set is
roughly a second Lootline on top of the first, so it is a phased programme, not
a patch. The full item-by-item map is the "n1ssan x Lootline 2.0 - Parity Map"
doc; this is the short form.

What we already have (verified by reading our own source): gun skins, knives,
gloves, agents in the loadout, stickers, charms, StatTrak kill counting, local
graffiti spray, start/end lineup dressing (`hooks/lineup.cpp`), Storage Unit and
Charm Detachment tool hooks.

What v2 has and we have nothing for (grep over `src/` returns zero hits):
Souvenir, "Tradable After" (Lootline `game_inventory.cpp:110` ATTR 75, `:778`;
`unboxer.cpp:1589` TradeHoldEnd), the new-item window and its acknowledgement,
scoreboard pin / flair, kill feed, replays, chat lines, loadout shuffle, music
kit application (the item exists in our contract, `skin/apply.cpp` never
applies it), custom agent models from a model file, the 3D preview
(`model_preview_panorama.cpp` 2,522), inspect-link decoding, case opening
entirely, and Lootline Online (`online/online.cpp` 1,384, Steam flat API P2P).

Architecture note: Lootline's UI lives in the DLL (ImGui). Ours does not - the
Armory app and bridge are the UI - so behaviour is replicated into our
architecture (DLL = what the game must answer, app = inventory, add, preview),
not by porting their menu.

**Order:** 1) finish the tools already in use (remove-keychain v7.5, StatTrak
Swap, Delete, name tags, new-item window, attr 75); 2) the missing item families
(patches, graffiti colours, pins, coins, medals, pets, music kits, Souvenir);
3) inspect links then the 3D preview; 4) case opening + the chat line;
5) shuffle, pin, kill feed; 6) custom models; 7) Lootline Online.

**Not started:** every block in that list. Nothing here has been implemented or
verified in game yet; the map exists so the work can be taken in an order that
can be tested.

## v7.6 - Phase 1: CS2's item actions (Lootline 2.0)

First slice of the Phase 1 list: the three InventoryAPI actions Lootline hooks
in `hooks.cpp` and we had none of.

- **AcknowledgeNewItembyItemID(itemId) -> bool** (`hooks.cpp:329`
  `hkAcknowledgeNewItem`). Lootline answers it locally for its own items and
  never lets the id reach Valve. Ours does the same through new
  `LootSOCache::Acknowledge(id, wasNew)`, which is `GameInv::Acknowledge` copied
  over: an item's position carries `NEW_ITEM_FLAG 0x40000000`
  (`game_inventory.cpp:70`) until it is acknowledged, so the acknowledged item
  gets `NextPosition()` and an `SO_UPDATED` listener call, and `wasNew` is what
  the native returns.
- **DeleteItem(itemId)** (`hooks.cpp:314`): our items are removed locally
  (`LootSOCache::Remove(id, false)`, object left alone because the UI may still
  point at it - the same reason Lootline does not delete its object), anything
  else goes to the game.
- **ClearCustomName(itemId)** (`hooks.cpp:563`): the name comes off a managed
  item and the inventory is told. The empty-name write sits in its own function
  because a `std::string` and `__try` may not share one (C2712 - it failed to
  compile the first time).

Registered beside the existing three in `Tools::Initialize`; the ready line now
reads `[tools] ready: storage unit ..., charm detachment charges ..., charm
take-off ..., item actions ...`.

**Verified:** MSBuild Release x64 exit 0, no warnings (one C2712 fixed);
installed at `x64/Release/n1ssan-dll.dll` (614,912 bytes, md5
d4c216d536cd4efc9608c1b723dbd8ed), symbols `pdb/n1ssan-dll-v7.6.pdb`, previous
build kept as `n1ssan-dll-v7.5.bak.dll`; file checked for `Debug.v7.6`,
`inventory-ops`, `AcknowledgeNewItembyItemID`, `item actions`.

**Not verified in game.** Whether the three natives resolve by name in this
build, whether the new-item popup now closes, whether Delete works on an Armory
item, and whether a name tag's Remove comes off. Nothing here can be run from
this machine.

**Still open in Phase 1:** the StatTrak Swap Tool (Lootline `QueueToolUse`,
`FinishToolJobs` swap branch), and `SetNameToolString` (Lootline only remembers
the typed text; our names come from the Armory, so it may not be needed).

**Next:** Phase 2 - the item families we have no data model for (patches,
graffiti colours, pins, coins, medals, pets, music kits) and the Souvenir flag.
That work starts in the Armory contract/projection, not the DLL, so it lands in
`development/n1ssan-web-bridge` first.

## v7.7 - the 2026-11-26 09:47 session, and the two things it broke

Session log (PID 5000, `Debug.v7.6`, clean exit). All six InventoryAPI hooks
installed (`PerformItemCasketTransaction`, `GetCacheTypeElementFieldByIndex`,
`RemoveKeychain`, `AcknowledgeNewItembyItemID`, `DeleteItem`,
`ClearCustomName`). Results from the user: detach works
(`[tools] charm: taking the charm off 11037818938482124452 - answered locally`),
Delete works (`[inventory-ops] delete: ... removed locally`), clean exit. Two
failures, both with a cause in the log:

### 1. Storage Unit - the name was ours to lose

`[tools] storage unit 11030296344653809771 has no name in the profile`, and
`[socache] storage unit retained id=99137C159E61486B items=0 name=""` on every
Apply. The Armory profile still has `customName: null` for that unit, but
Lootline's own hint (`unboxer_menu.cpp:4280`) is *"give it a name on the right
(or name it in CS2: right-click -> Name)"* - CS2's rename is a legitimate way to
name the unit.

It could not survive here: the retained path wrote the Armory's (empty) label
over whatever the item carried, on every Apply. So a unit named in CS2 was
wiped seconds later, which is also why `WriteNamedUnitDates()` never saw a name.

- `armory_inventory.cpp`: the retained path now writes a label only when the
  Armory actually has one.
- new `LootSOCache::ReadCustomName(id, out)` reads the name off the item.
- `WriteNamedUnitDates()` uses the item's own name first, the Armory's second,
  so a unit named in CS2 gets its modification date and CS2 offers put in /
  look inside.

### 2. New items never arrived as new

`[inventory-ops] new item ... acknowledged locally (was new = no)`: the bit was
never set, because `Create` always wrote a plain `NextPosition()`. Lootline sets
`NEW_ITEM_FLAG 0x40000000` on the position of an item that is added
(`game_inventory.cpp:1206`), and only for a normal addition - a bulk add is
"not NEW (too many at once)".

- `NewItem::asNew` + `Create` writes `NEW_ITEM_FLAG | NextPosition()`.
- `armory_inventory.cpp` counts how many projected items are missing from the
  cache and marks them new when that is 1..16, logging
  `[armory-inventory] N item(s) missing from the cache, arrives as new: yes|no`.

**Verified:** MSBuild Release x64 exit 0, no warnings. Installed
`x64/Release/n1ssan-dll.dll` (md5 c5212ebd0205f7b3658ccf74c014dd52, checked for
`Debug.v7.7`, `arrives as new`), symbols `pdb/n1ssan-dll-v7.7.pdb`, previous
build kept as `n1ssan-dll-v7.6.bak.dll`.

**Not verified in game:** whether a unit named in CS2 now keeps its name and
opens; whether an added item shows CS2's new-item window.

### Phase 2 - what the tree says about the missing families

The Armory supports six item kinds (`contracts.ts:11`): skin, sticker, charm,
graffiti, music_kit, tool. The projection already writes graffiti (unsealed
1349, 50 charges, `spray tint id` for its 19 colours) and music kits (1314).

Patch, pin, coin, medal and pet exist nowhere: no `cs2:patch` / `bymykel-patches`
catalogue, no definition indices, no contract kind. Adding them is data work
first - each family needs a catalogue (like `skins.json`) before the DLL can
project or write it. That is the blocker to raise, not a code gap.

Souvenir needs no catalogue (it is a flag on a skin), so it is the Phase 2 piece
that can be built next without new data.

## v7.8 - "Steam Connection Error" when putting an item into the Storage Unit

Session log (PID 16444, `Debug.v7.7`). The new-item flag works (items show NEW on
the grid; `[armory-inventory] 1 item(s) missing ... arrives as new: yes`) and the
casket hook fires: `casket entry #1 op=1 unit=11030296344653809771
item=11237217115195574031`. But every transaction was refused -

    [tools] storage: unit ... has no name yet - name it in the Armory first

- and the hook still reported it as handled, so CS2 closed the storage dialog and
waited for an answer that never came: the "We were unable to connect to the
Steam network" popup. Two bugs made that state possible:

1. **A junk name got the unit its modification date.** v7.7's `ReadCustomName`
   took whatever was at `item + EIV_CustomName` as a label; the real value was
   binary (`"BA..."` in the log). Because it was non-empty, the date
   was stamped - and a date is exactly what makes CS2 offer "put in". So CS2
   offered a transaction the tools loop then refused.
2. **Two different truths for the label.** The date stamp used the item's name,
   the transaction used the Armory's (empty) one, so the two disagreed forever.

- `ReadCustomName` now copies the field into a buffer (SEH kept in its own
  function, C2712) and accepts it only when it is 1-64 printable ASCII, trimmed;
  anything else is no name.
- `Tools::UnitLabel()` is the single resolver (Armory name first, then a name CS2
  gave the unit) and is used by both the date stamp and `QueueCasket`.
- The Armory's label is written on the tick (`LootSOCache::SetCustomNameById` +
  `NotifyUpdated`), not at creation: a write at creation is reset when CS2 adds
  the object to its cache, which is why a unit could log a name and show none.
  Log: `[tools] storage unit <id> labelled "<name>"`.
- An unnamed unit is still refused, but now nothing offers the transaction in
  the first place, so there is no dialog to hang: name the unit in the Armory
  (item details -> CUSTOM NAME) or in CS2.

**Verified:** MSBuild Release x64 exit 0. Installed
`x64/Release/n1ssan-dll.dll` (md5 64cc0d6cb0c09e5a127b431e8c67cf3d, checked for
`Debug.v7.8`, `labelled "%s"`), symbols `pdb/n1ssan-dll-v7.8.pdb`, previous build
kept as `n1ssan-dll-v7.7.bak.dll`.

**Not verified in game:** that a named unit now takes items without the Steam
popup.

## v7.9 - Armory names in game, items that stay inside a unit, and NEW only once

Reported after the v7.8 session: CUSTOM NAME in the Armory does nothing any more,
the Storage Unit still does not work, and every skin comes back NEW on each
profile load and each "Apply to CS2".

1. **The name went into the wrong field of the view.** CS2's
   `C_EconItemView::GetCustomName` reads `m_szCustomNameOverride` first and only
   falls back to `m_szCustomName` when the view has no inventory item behind it
   (Lootline's own 2026-11-06 note, `game_inventory.cpp` SetViewName). Every
   Armory item has an item behind it, so the name was written where nothing read
   it. The offset table now resolves `m_szCustomNameOverride` by schema name and
   `SetCustomName` writes both fields (and clears both when a name is taken off).
   `ReadCustomName` reads the override first, so the tick no longer rewrites a
   label it just wrote.
2. **A stored item never left the inventory.** Lootline lists a stored item
   *inside* its unit by re-creating it with the unit's id in "casket item id
   low/high" (attributes 272/273) - `FinishCasketJobs`: remove, create again,
   then tell the unit. This runtime only kept a map (`n1ssan-tools.json`), so
   after "put in" the item stayed on the grid, the unit's contents stayed empty
   and a second put-in answered "already in a unit". Now:
   - `NewItem.casketId` writes the two attributes at creation;
   - put-in and take-out remove the item and make it again (inside the unit /
     back in the inventory) on the main thread;
   - after every Apply (and every start) `Tools::RestoreStoredItems()` puts each
     remembered item back inside its unit, because the Armory projection knows
     nothing about a unit's contents.
3. **NEW was decided for the whole batch, not per item.** `request.asNew` was one
   flag for every item created in that Apply, so one genuinely new item marked
   the ~60 sticker/charm/graffiti items that are rebuilt each time NEW as well;
   and a rebuild of the projection at start marked the whole profile NEW again.
   Lootline marks an item NEW only when it is a normal addition and not too many
   arrive at once (`AsAdded`, `MAX_NEW_ITEMS`). Now:
   - the ids CS2 has already been given live in
     `<config>/armory-new-items.json` (written next to the other Bridge files);
   - an item is NEW only when it is absent from that record and at most 16 are
     new at once (a first start therefore records everything without badges);
   - a NEW position carries the pickup method in its low bits
     (`0x40000000 | 4`, Lootline `PICKUP_PURCHASED`) instead of a slot number.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors). The built
`x64/Release/n1ssan-dll.dll` (md5 c077263a0d4182bf2b9643c939d14ba6) contains
`Debug.v7.9`, `armory-new-items.json`, `never delivered before`,
`made again inside the unit`, `casket id=`, `m_szCustomNameOverride`; symbols
`pdb/n1ssan-dll-v7.9.pdb`.

**Not verified in game:** that a named weapon/store unit now shows its name, that
"put in" moves the item out of the grid and into the unit's contents, and that a
restart shows no NEW badges. The next session log should show
`[armory-inventory] N item(s) have already reached CS2`,
`[armory-inventory] ... never delivered before, arrives as new: no`, and
`[tools] storage: <item> made again inside the unit (unit <id>) - ok`.

**Caveat:** the v7.8 binary was overwritten by this build; the newest kept backup
is `x64/Release/n1ssan-dll-v7.7.bak.dll`.

## v7.10 - the unit's contents reach CS2, and the name goes into the view CS2 reads

Reported after the v7.9 session: the "Add to Storage" dialog appears and the
process runs, but nothing lands in the unit (an empty unit), and CUSTOM NAME is
still not shown.

1. **The casket id was never attempted.** `Tools::RecreateInUnit` re-creates a
   stored item with the unit's id, and v7.9 wrote it as attributes 272/273 - but
   `LootSchema::GetCachedAttrDef` only had slots for 6, 7, 8, 80, 81, 270 and
   271, and returned `nullptr` for every other index. `SetAttributeInt` then
   failed before calling anything, which is exactly what the session log says:
   `[socache] casket id=10561306935738156449 not written (attribute not in this
   schema)`. CS2 therefore never learned the item was inside the unit, the item
   stayed on the grid and the unit stayed empty. The cache now resolves
   "casket item id low" / "casket item id high" by schema name with the index
   (schema vtable slot 27) as fallback, like the count and the date.
2. **The name was written into the object we created, not the view CS2 hands
   out.** Lootline writes a name through `g_itemViewById(id)` and reads it back
   with `C_EconItemView::GetCustomName` (vfunc 26) - `game_inventory.cpp`
   `SetViewName` / `CallSetViewName`. v7.9 wrote the fields at the resolved
   schema offsets on the cache object only, which is why the log could read the
   name back while the card showed none. Now:
   - `ItemViewById` (the same Lootline signature already used by
     `attachments.cpp`) resolves the view by id; the name is written there, and
     to the cache object as well when the two differ;
   - `ReadCustomName` asks CS2 first (vfunc 26), so "does the name show" is
     answered by the game, not by a field only this runtime can see; the tick
     rewrites the label while CS2 still answers something else;
   - a name is written for any named item at creation, not only for a Storage
     Unit;
   - after `AddObject` the fields CS2 reads (both name fields and the casket id)
     are written once more, because the object entering the cache is what
     previously swallowed a name written at creation.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors). The built
`x64/Release/n1ssan-dll.dll` (md5 8371c9daa93240935cb3e1a15b7afc59) contains
`Debug.v7.10`, `casket item id low`, `CS2 reads`, `Lootline ItemViewById`,
`no C_EconItemView name field resolved`, `armory-new-items.json`; symbols
`pdb/n1ssan-dll-v7.10.pdb`.

**Not verified:** the in-game result (this host cannot run CS2). The next
`development/CS2-n1ssan/x64/Release/n1ssanlogs.md` should contain
`[socache] name "<name>" written to <id> at <view> ... CS2 reads "<name>"`
with the name in the read-back, `[socache] casket id=<unit> in the cache: low
written, high written`, and `[tools] storage unit <id> "<name>": <n> item(s)`.
If the read-back is still empty while the write reports success, the view the
inventory hands out is not the one the card is drawn from and the next step is
the "custom name" attribute (Lootline `SetName` -> `SetAttributeText`), which
needs the string attribute setter this runtime does not have yet.
## v7.11 — a detached charm comes back, tool artwork, Souvenirs

**Reported:** "after detaching, the charm disappears. It should be available to
use for another weapon" · "use those [four PNGs] as preview" for the Tools cards.

1. **The charm was spent by the apply, not by the detach.** `ApplyCharm` removed
   the charm item from the cache and told the Bridge to consume the profile
   instance (`consumedToolInstanceId`), so the detach had no charm item left to
   give back when it came off. That is not what Lootline does:
   `FinishCharmJobs` (unboxer.cpp:3130-3170) creates a *returned* charm item -
   definition 1355, the same keychain kit, seed and slab sticker - and only then
   clears the gun. Now:
   - an apply no longer consumes the profile instance (the record carries no
     `consumedToolInstanceId`), so the charm stays owned while it hangs on a gun,
     and its projected id cannot move;
   - `Tools::ReturnCharmItem` makes the charm's object again on detach (the same
     remove-and-create the Storage Unit uses), so it is back on the grid and can
     be hung on another weapon at once;
   - a charm applied before this build is gone from the profile and cannot be
     reconstructed: the log says so in one line instead of failing silently.
2. **Tools cards use the game's own pictures.** `assets/cs2_storage_unit.png`,
   `cs2_nametag.png`, `cs2_stattrak_swap.png` and `cs2_detachment.png` are copied
   unchanged into `n1ssan-web-bridge/public/tools/` and `catalogue.ts` points at
   them; the four hand-drawn SVGs stay in place, unused.
3. **Phase 2: Souvenirs.** A skin can now be a Souvenir, the way Lootline's
   `QUALITY_SOUVENIR` (12) is: `SkinCustomization.souvenir`, carried by the
   adapter in both directions, offered as an SV toggle next to ST in the
   Configure and Item-details dialogs (mutually exclusive with StatTrak, never on
   knives or gloves), and projected as quality 12 with StatTrak forced off.
   The Bridge validates the flag and rejects "Souvenir + StatTrak".
   Still missing for full parity: `RollSouvenirStickers` (the four gold stickers
   of the package's Major) - it needs the tournament/team sticker catalogue
   Lootline reads from `items_game`, which this tree does not have yet.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors), built
`x64/Release/n1ssan-dll.dll` md5 `4416fff586879b2f0d74dde537abc1d8` contains
`Debug.v7.11`, `charm item %s`, `the Armory profile no longer owns the charm
item`; symbols `pdb/n1ssan-dll-v7.11.pdb`. Web: `npm run build` exit 0,
`npm run build:bridge` exit 0, `node --test bridge/armory-inventory.test.ts
bridge/armory.test.ts` 28/28 pass, `dist/tools/` carries the four PNGs.

**Not verified:** everything in game (this host cannot run CS2). Expected log on
a detach: `[tools] charm <kit> taken off <id> locally (charm item back in the
inventory): inventory told, popup told, <n> detachment(s) left`.

## v7.12 — Armory items keep the top of CS2's inventory, newest first

**Reported:** "Armory items should always be on top of CS2's inventory page and
new items should be added accordingly."

CS2's inventory is ordered by item id, newest first - which is why Valve's ids
grow with time and why Lootline hands its items `ID_BASE 0x7000000000000000`
plus a timestamp (`unboxer.cpp:56,:1251`, and its own note "every id from
ID_BASE up is Lootline's (Valve's are far below)"). n1ssan's ids were already
above Valve's (`0x9...`) but were a hash of the instance, so the group sat at the
top with a new item landing wherever its hash fell.

`StableIdFor` (armory_inventory.cpp) now keeps the FNV hash only in the low 12
bits and puts the instance's creation millisecond in bits 12-53:
`0x9000000000000000 | (createdAtMs << 12) | (hash & 0xFFF)`. Both halves are
stable for an unchanged profile, so CS2's equipment references do not move.
The projection (`armory-inventory.ts`) now emits `createdAtMs` per item, taken
from the instance's `createdAt`.

Carrying the old ids across the change (the ids did change once):
- `LegacyIdFor` keeps the pre-v7.11 hash, so anything that remembers an old id
  can find its item again;
- the delivered record (`armory-new-items.json`) maps a legacy id onto the new
  one before deciding what is NEW, so an update does not cover the inventory in
  badges;
- `Tools::MigrateStoredIds` resolves both the stored item and its unit through
  `FindManaged` / the new `ArmoryInventory::FindManagedByLegacyId` and rewrites
  `n1ssan-tools.json`, so a filled Storage Unit stays filled.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors), built
`x64/Release/n1ssan-dll.dll` md5 `cfa4f3e7f911169375af0a9609d84922` contains
`Debug.v7.12`, `creation-ordered ids`, `charm item %s`; symbols
`pdb/n1ssan-dll-v7.12.pdb`. Web: `npm run build` exit 0, `npm run build:bridge`
exit 0, `node --test` over armory-inventory / armory / attachments-sync 41/41
pass, including the two new Souvenir assertions (quality 12 with StatTrak off,
never on a knife).

**Not verified:** the grid order itself (this host cannot run CS2). What to look
for: Armory items grouped at the top of the inventory and the newest addition
first. The log should read `[tools] v7.11 ids: <n> stored item(s) and their
unit(s) carried over to the creation-ordered ids` once, if a unit had contents
before the update.


## v7.13 - a renamed item's name reaches CS2, and the tools cards lose the blurb

**Reported:** "Armory Custom name still doesn't work" | "remove the orange text on
the Tools card, music kits and graffiti".

1. **A name was only ever reconciled on a Storage Unit.** v7.10 recorded "a name
   is written for any named item at creation" and "the tick rewrites the label
   while CS2 still answers something else", but the only reconcile that shipped
   was `Tools::WriteNamedUnitDates` (definition 1201), and the retained branch of
   the projection (`armory_inventory.cpp`) wrote a name only inside that same
   `definition == 1201` block. Two consequences:
   - a skin, knife or glove was named at `LootSOCache::Create` and never again -
     and a write at creation is exactly what CS2 resets when the object enters
     the cache, which is why a freshly added item drew no name;
   - renaming an existing item in the Armory did change the projection, but the
     item is *retained*, and a retained item only had its attachments synced.
     Nothing wrote the new name, so it never reached the card.

   Now the name is reconciled for **every** retained managed item, before
   anything tells CS2 the item changed, through `LootSOCache::SetCustomNameById`
   - the write that resolves the view CS2 reads a name from (`ItemViewById` +
   vfunc 26) and notifies the card afterwards. A slow pass,
   `Tools::SyncCustomNames` (2 s), does the same for items already in the cache,
   so a name wiped by `AddObject` is put back in the same session instead of
   waiting for the next Apply. An empty Armory label still never clears a name
   the player gave the item in CS2.
2. **The orange line on the Tools / Graffiti / Music Kits cards was the
   catalogue blurb.** `CS2AttachmentCard`'s footer printed `qualityOrFinish`, and
   the inventory grid passed each tool's `description` - and a graffiti's or
   music kit's ByMykel blurb - in the spot where a skin prints float/seed. Those
   kinds carry neither, so a whole sentence in the accent colour sat there.
   - `ObsidianApp` now passes no detail for tool / graffiti / music_kit (a
     sticker keeps its finish, a charm its pattern seed);
   - the card draws that badge only when it has something to say, instead of
     falling back to "Armory".

**Verified:** MSBuild Release x64 exit 0, `x64/Release/n1ssan-dll.dll` md5
`4dfb3fbe98ad51f76469f87046fcf666` contains `Debug.v7.13`; web `tsc -b` exit 0.

**Not verified:** the in-game result (this host cannot run CS2) and the cards
themselves (not opened in a browser). What to look for: `[armory-inventory] name
"<name>" written on retained id=<id>` or `[tools] name "<name>" written on <id>`
in `x64/Release/n1ssanlogs.md` after an Apply, then the name on the item in CS2.


## v7.14 - no name rewrite while the game is loading (the load crash)

**Reported:** "Game crashed while Loading."

Evidence read before any change: the v7.13 session (`x64/Release/n1ssanlogs.md`)
ends with `[fault] first-chance 0xC0000005 at ... networksystem.dll+0x4C5B4`, a
read of `0x388` (null + one field) on CS2's network thread, seconds after the
128-item apply and the switch into the Armory - which is the moment the game is
loading that map. The minidump `n1ssan-crash-20261126-134524-15708.dmp` names the
same frame (`tools/dump-peek.py`: thread 13584, `networksystem.dll+0x4C5B4`,
params `0x0 0x388`). The v7.12 session (`...130142.md`) died on the same
signature, so this is not a fault introduced by v7.13 - but v7.13 is what added
work into that window.

1. **The two-second name pass is gone.** `Tools::SyncCustomNames` wrote a name
   into CS2's live item view and fired `SO_UPDATED` every two seconds for every
   named item that did not read back. Each notify makes CS2 re-serialize the
   item, and the pass ran while the Armory world was loading (five names,
   seconds before the fault in the v7.13 log). A name is now written only where
   the projection is adopted (`armory_inventory.cpp`, retained branch) and when
   an item is created - never on a timer.
2. **`SetCustomNameById` no longer writes the cache object on top of the view.**
   v7.10 wrote both whenever the two pointers differed. The view write is the one
   that succeeds and reads back; the second put 161 bytes at view-relative
   offsets inside an object whose real layout is not proven, which does not fault
   where it is written and can surface later as a string the game owns being
   overwritten. The cache object is now only the fallback for a failed view
   write.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors). The built
`x64/Release/n1ssan-dll.dll` md5 `8dbcd54db722e72aa5b39b4ccf5325fe` contains `Debug.v7.14` and no
`SyncCustomNames` symbol; symbols `pdb/n1ssan-dll-v7.14.pdb`.

**Not verified:** everything in game (this host cannot run CS2). What to look
for: no `[tools] name ... written on ...` lines (that pass no longer exists),
`[armory-inventory] name "<name>" written on retained id=<id>` after an Apply,
and a clean Armory map load. If the load still faults in `networksystem.dll`,
the name path is ruled out and the next step is the apply itself: hold item
creation/delivery until the map reports loaded instead of doing it on the mode
switch.


## v7.15 - the Souvenir flag reaches the gun, and Phase 7 starts

**Reported:** "Souvenir toggle shows up in Armory, however it does nothing to the
skin in-game. Fix that then start Phase 7 now, we keep Phase3-6 for later."

1. **Souvenir now writes quality 12 onto the live item view.** The Bridge already
   projected a Souvenir as `quality: 12` (`armory-inventory.ts`, Lootline's
   `ITEM_QUALITY_TOURNAMENT`, `skin_application.cpp:39`), and the SOCache item was
   created with it. Every in-match and lineup write then recomputed the quality
   from StatTrak alone:
   `IsKnifeDefinition(t.defIndex) ? 3 : (t.statTrak ? 9 : 0)` - three copies in
   `skin/apply.cpp` (the already-applied gate, `ApplyWeaponSkin`, and
   `DressItemView`). A Souvenir therefore read back as a plain quality-0 weapon in
   game, which is exactly "the toggle does nothing".
   - `TargetSkin` carries `souvenir`, filled from `item.quality == 12` in
     `armory_inventory.cpp` (both `FillTarget` and `GetTargetForItemId`);
   - one `WantQuality()` (knife 3, Souvenir 12, StatTrak 9, plain 0) replaces the
     three copies, so the gate and the write agree and an already-painted
     Souvenir is rewritten once instead of being left as quality 0.
   - Decoration is still missing: the four gold Major stickers a real souvenir
     package puts on the gun (`RollSouvenirStickers`) need the tournament / team
     sticker catalogue, which is Phase 4's data. The Souvenir *item* is now real.

2. **Phase 7 - Lootline Online, first slice.** New module
   `src/online/online.{h,cpp}`, ported from Lootline 2.0
   `source/chadware2/features/online/online.cpp`:
   - Steam P2P through the game's own `steam_api64.dll`
     (`ISteamNetworkingMessages` flat API), preferring CS2's in-process
     `steamnetworkingsockets.dll` copy and reading the Steam client's copy too -
     Lootline's friend test is the reason: through the client's copy both hellos
     "sent", both sessions "connected", nothing was ever read;
   - relay only: `P2P_Transport_ICE_Enable = 0`, so no IP address is shared;
   - one 20-byte entry per weapon skin, magic `N1SN` on our own channel 7418
     (a Lootline client's `LOOT` packets are ignored, and ours by them);
   - the peer list comes from `cs_player_controller` + `m_steamID`
     (`g_off.CTRL_SteamID`); hello every 30 s, own packet on change / every 15 s,
     sessions closed when Online turns off;
   - settings: `<bridge>/n1ssan-online.json` `{"enabled": true}`; a missing file
     means on (presence only);
   - `Online::RemoteSkin(account, def, TargetSkin&)` exposes a peer's skin for the
     next slice. Nothing writes another player's gun yet - that, plus knife,
     gloves, agent and the dropped-gun owner, is the next increment.
   - Wired into `hooks/frame_stage.cpp` at stage 7 behind `Online::Enabled()`,
     guarded like the other optional passes.

**Verified:** MSBuild Release x64 exit 0 (no warnings/errors). `x64/Release/n1ssan-dll.dll`
md5 `6ad114fb1ca30a64d4f2aee849cef0a9` contains `Debug.v7.15`; symbols
`pdb/n1ssan-dll-v7.15.pdb`.

**Not verified:** everything in game (this host cannot run CS2). What to look
for: a Souvenir skin in hand now has quality 12 on its view (the item's inspect
line names it a Souvenir) instead of a plain weapon; and in a match with another
n1ssan user, `[online] Steam P2P ready ...`, `[online] hello to SteamID ...` then
`[online] SteamID ... runs n1ssan Online - N gun skin(s)` in
`x64/Release/n1ssanlogs.md`. Phase 7 has no visible effect until the apply slice
lands.


## v7.16 - Phase 4 for souvenir stickers, then Phase 7 paints peer guns

**Reported:** "If you need Phase 4 for souvenirs stickers to work, then do Phase 4
for stickers only then continue Phase 7."

1. **Phase 4, stickers only: a Souvenir now carries its four gold stickers.**
   Lootline's `Cases::RollSouvenirStickers` (cases.cpp:1503) draws the event's
   organizer logo, two of its teams and one player's autograph; the game's own
   `items_game` sticker kits end in `_gold` and belong to a `tournament_event`.
   This tree had no tournament catalogue, so one was generated:
   - `scripts/build-souvenir-stickers.mjs` reads ByMykel's stickers.json (which
     has no event id fields) and rebuilds Lootline's grouping from the data it
     does have: `original.name` ending `_gold`, the event as the last `|` part of
     the display name, `type` = Event / Team / Autograph. It writes
     `src/armory/souvenir-stickers.ts` (21 Majors, organizer + teams + autographs).
   - `src/armory/souvenir-roll.ts` picks event, the two teams and the autograph
     deterministically from the instance id (FNV-1a + xorshift32), so an Apply
     never reshuffles the stickers. The Armory has no event picker, so the event
     is drawn the way a package draws it.
   - `armory-inventory.ts` adds `sticker{n}Kit/Wear/Scale/Rotation/OffsetX/Y` for
     the first free slots of a Souvenir weapon skin. A sticker the player already
     applied in CS2 owns its slot and is never replaced. The existing attachment
     writer (`Attachments::Write`) puts them on the SOCache item, so they show on
     the gun in game - no DLL change was needed for the stickers themselves.
   - The Bridge bundle was rebuilt (`npm run build:bridge`): the runtime serves
     `runtime/bridge/server.js`, so the projection change would not ship otherwise.

2. **Phase 7, second slice: a peer's skins are painted on their guns.**
   `online.cpp` now walks the level's `weapon_*` entities every 250 ms while alive
   in a match (Lootline's `Online::ApplyAll`, online.cpp:1018, uses the same
   approach) and paints a gun when the account CS2 stamped on its item view
   belongs to an answering peer with an entry for that definition. Ours are
   skipped (the Armory pass owns them), knives/gloves/agents are skipped until
   they have packet entries, and two material builds per pass keep it off the
   frame budget. `__try` lives in its own helper (C2712).

**Verified:** MSBuild Release x64 exit 0, no warnings/errors; `x64/Release/n1ssan-dll.dll`
md5 `f190f4b015c08363d1cf31d787df783c` contains `Debug.v7.16`; symbols
`pdb/n1ssan-dll-v7.16.pdb`. Web `tsc -b` exit 0; `bridge/armory-inventory.test.ts`
10/10 pass; the roll returns 4 valid kit ids and is stable for the same instance;
`runtime/bridge/server.js` contains the souvenir roll (built 2026-11-26 15:09).

**Not verified:** everything in game (this host cannot run CS2), and no two-client
test was possible. What to look for: a Souvenir skin in CS2 now carries the gold
organizer / two teams / autograph stickers on the gun; and in a match with another
n1ssan user, `[online] peer <account> def=<def> painted (entity=<ptr>)` plus
`[online] N peer gun(s) painted this pass` in `x64/Release/n1ssanlogs.md`.
Knives, gloves, agent, spectate, dropped-gun revert and the Online menu are the
next slice.

## v7.17 - Phase 7: a peer's knife, gloves and agent, and spectating them

**Reported:** "Continue Phase 7: send knife, gloves and agent in the online packet
and apply them to a peer's pawn and knife, then handle spectate. Follow
Lootline's online.cpp for the order and keep the per-pass budget small."

1. **The packet is v2 and carries everything a peer can wear.** `online.cpp` now
   has `KIND_KNIFE`, `KIND_GLOVES` and `KIND_AGENT` beside `KIND_GUN`, and a
   `Loadout` holds one entry of each next to the gun map (Lootline's own shape,
   online.cpp:132). A v1 packet fails the version check, so the two builds never
   paint each other. Armory mode sends the first knife the projection carries
   (with its paint), and resolves the gloves and the agent through
   `ArmoryInventory::ResolveTargetForSlot` - the same equipment assignment the
   glove and agent passes apply, so the packet names exactly what the player
   sees. Loadout mode classifies by definition (`IsKnifeDefinition`, the glove
   ids) and by slot. Receiving validates each entry against what it claims to be:
   a paint on a gun, a knife on a knife definition, a glove on a glove id, an
   agent with no paint. Entries are sorted by kind then definition, like Lootline.

2. **Applying follows Lootline's order.** `ApplyAll` there walks the weapon
   entities first and knives are a branch of that loop (`KnifeOn`,
   online.cpp:1018-1160), then `ApplyPawns` deals with the gloves and the agent
   (online.cpp:1251). Ours does the same: `VisitPeerWeapon` handles a knife by
   changing the model with `KnifeApply` and painting it with the existing
   `ApplyWeaponSkin`, remembering the definition the knife really had so a peer
   who takes their knife off gets it back (`RestorePeerKnife`, paint then model);
   `ApplyPeerPawns` scans the controllers' pawns, writes the peer's gloves into
   their own glove item view through `DressGloveItemView` (their account id, so
   CS2 renders it) with Lootline's two-repeat guard after a rebuild, and swaps the
   agent through `GloveModelRefresh::ApplyAgentModel`. The agent is only applied
   when its model path says it fits the peer's side (`/ctm_` on a CT, `/tm_` on a
   T), three tries per spawn, and their own model is recorded first so it can be
   put back. Gloves are not restored: we never know which ones they had, and CS2
   rebuilds them on the next spawn - Lootline has the same gap.

3. **Spectating.** The 250 ms pass used to return the moment the local pawn died.
   It now keeps running at 1 s while dead (the same interval the Armory's own pass
   uses on the death camera), so the spectated player's gun, knife and body are
   painted while you watch - the death camera builds from the item view we write.
   The write budget stays small: 2 gun/knife material builds and 2 glove/model
   rebuilds per pass (Lootline uses 6 and 3).

**Verified:** MSBuild Release x64 compiled with no warnings and no errors. The
link into `x64/Release/n1ssan-dll.dll` was refused because `cs2.exe` is running
with that DLL loaded (LNK1104), so the same build was linked into
`x64/Release/verify/n1ssan-dll.dll` instead - exit 0, md5
`ae68310bfdd789c49365692eb1d40aaf`, contains `Debug.v7.17` and the new
`[online] peer %u knife back to def=%u` line. **The DLL the injector loads is
still v7.16 until CS2 is closed and the build runs again.**

**Not verified:** anything in game - this host cannot run CS2 and there is no
second client here. What to look for in a match with another n1ssan user:
`[online] SteamID <id> runs n1ssan Online - N gun skin(s), knife X, gloves Y,
agent Z` in `x64/Release/n1ssanlogs.md`, then `[online] peer <account> knife
def=<def> painted`, `... gloves def=<def> painted` and `... agent def=<def>
painted`, and `[online] N peer item(s) painted this pass`. Spectate: die and
watch a peer - their gun and knife should be theirs, not the defaults.

### v7.18 - online stickers, SV badge, Graffiti/Music Kit labels

- Online packet **v3**: every `Entry` now carries four sticker kits (`26` bytes; `ENTRY_SOUVENIR` unchanged). Souvenir gold stickers and player stickers reach a peer the same way, and `Take` drops kit ids above 1,000,000 so a wild id can never fault a material build.
- `AttachmentsOf(entry)` feeds those kits into `ApplyWeaponSkin` for guns and knives (`ApplyPeerWeapon` / `ApplyPeerKnife` take the set; empty stays null).
- Every packet rebuild logs `[online] sending N item(s): knife X, gloves Y, agent Z, N sticker kit(s)` - the 2026-11-26 session showed `knife 0` in a peer hello and no `knife ... painted` line, and this line says whether we send one at all.
- Armory: **SV** badge on the skin card next to ST (`CS2SkinCard isSouvenir`, gold, wired from `item.souvenir`), and Graffiti / Music Kit cards are labelled as themselves - `cardType` typed everything that was not a sticker or charm as "Tool" (`CS2AttachmentCard` union widened). The four Tools cards are untouched.
- Build: Release x64 clean; `x64/Release/n1ssan-dll.dll` md5 `996b50468b0c1d18cfc58f4cb986c182`, build stamp `Debug.v7.18`, symbols at `x64/Release/pdb/n1ssan-dll-v7.18.pdb`. Web bundle rebuilt (`npm run build`, `npm run build:bridge`).
- Unverified in game: stickers on a peer, the peer knife, glove overlap, and the first-injection crash (no dump carries a stack).


### v7.19 - patches, collectibles, containers, Tradable After, StatTrak Swap, agent model path, peer gloves

- **Phase 2 - the data-blocked item families.** `scripts`-free runtime fetches (same pattern as stickers): ByMykel `patches.json` (112) and `collectibles.json` (677 - pins, coins, service medals, trophies). New kinds `patch` and `collectible` through `contracts.ts`, `catalogue-identity.ts` (sources `bymykel-patches` / `bymykel-collectibles`), `catalogue.ts` (`getCategory` -> Patches / Collectibles), the Armory filters, counts, card labels and the catalogue tabs. Each is ONE CS2 definition (`def_index` is the item, unlike a sticker's shared definition), so `projectArmoryInventory` writes definition + quality 4 and nothing else; a definition the live schema does not list is skipped with that reason. Pets: CS2 has no pet item family and ByMykel has no catalogue for one, so it is not the same problem - documented instead of faked.
- **Phase 4 - container opening.** New `src/armory/container-open.ts`: `buildCrateCatalog` reads `crates.json`, keeps the crates with a resolvable pool, and splits `contains` / `contains_rare`; `openCrate` rolls Valve's tiers (Mil-Spec 79.92%, Restricted 15.98%, Classified 3.20%, Covert 0.64%, rare special 0.26%), then a float inside the finish's own range, a seed 1-1000, StatTrak 1 in 10 (never on a Souvenir), Souvenir quality from a souvenir package, and `tradableAfter` +7 days. Armory: "Open container" dialog with search and a result line (rarity, float, pattern, ST/SV, trade date).
- **Phase 1 - attribute 75 "Tradable After".** `tradableAfter` travels projection -> `ArmoryItem` -> `LootSOCache::NewItem` -> written by `LootSOCache::Create` as `ATTR_TRADABLE_AFTER = 75` (logged, non-fatal when the schema does not name it) -> carried by `ManagedItem` and re-applied by `Tools::RecreateInUnit`.
- **Phase 1 - StatTrak Swap Tool.** Armory item details -> **Swap StatTrak**: two StatTrak instances exchange kill counts and one tool is used up. No CS2 popup can name a second item, so the operation belongs to the profile the runtime projects.
- **Phase 6 - a model path of your own for an agent.** `SkinCustomization.modelPath` -> projection `modelPath` -> `ArmoryItem::modelPath` -> `TargetSkin::modelPath` -> `Agents::Run` uses it instead of the definition's model, with the same guard the online path uses: a model CS2 cannot load leaves the pawn on its own. Armory: **Agent model path** on an agent's details. A raw `.vmdl` still needs a mount step, and its picture still needs the Phase 3 preview.
- **Phase 7 - peer glove overlap.** `GlovesOn` now calls `GloveModelRefresh::Request()` after `DressGloveItemView`: writing the glove item view does not release the glove mesh CS2 already built from their own gloves, which is why both models were drawn in the same place. Same one-frame swap our own glove routine uses.
- **Phase 5 and the chat line are NOT shipped.** `IGameEventManager2` (kill feed, chat) and the `InventoryAPI` vtable slots behind `GetShuffleItems` / `IsShuffleEnabled` / `GetSlotEquippedWithDefIndex` and `GetFlairItemId` / `GetFlairItemImage` have no pattern or offset in the tree; guessing either is what faults the client. They get anchored from a dumper run, then ported.
- Build: Release x64 clean, linked to `x64/Release/verify/n1ssan-dll.dll` (md5 `5c685a89bb239e6a6a8b30abbe1c6915`, build stamp `Debug.v7.19`) because `cs2.exe` holds the deploy copy. Web: `tsc` clean, `npm run build` and `npm run build:bridge` exit 0.
- Unverified: every Armory surface below (nothing was opened in a browser) and everything in game (no CS2 and no second client on this host).
### v7.20 - peer-glove crash fix (the v7.19 `Request()` call)

- **The death / round-start crash was ours, and v7.19 is what put it back.** `GlovesOn` called `GloveModelRefresh::Request()` after dressing a peer's glove item view. That function is not a glove refresh at all: it queues the *alternate-agent model discovery swap on the local pawn* - econ item map walk, model precache, `SetModel` on your own skeleton, restore one frame later. Every peer whose gloves we painted therefore rebuilt the local player's model mid-frame: at every round start, and at every death while spectating a peer. That is the death / round-start crash, and it reached CT too because the peer pass paints both teams' gloves.
- `GlovesOn` now sets the peer pawn's own reapply byte (`PAWN_NeedReapplyGloves`, schema `C_CSPlayerPawn::m_bNeedToReApplyGloves`) - the same single byte our glove routine writes after the same item-view write, so CS2 rebuilds that pawn's glove mesh from the view it now holds. A guarded byte write, no call into the model system, and peers are only touched while alive.
- Correction to the v7.19 entry: the peer-glove change it describes is the crash, not a mesh refresh. The overlap can be back; a per-pawn mesh rebuild that is not a skeleton rename is what would replace it.
- Build: Release x64 clean. **Installed** into `x64/Release/n1ssan-dll.dll` (md5 `76501ca7eed3bb30d71df557fabe728b`, build stamp `Debug.v7.20`) once CS2 was closed; the previous deploy copy is kept as `x64/Release/n1ssan-dll-v7.18.bak.dll` and symbols as `x64/Release/pdb/n1ssan-dll-v7.20.pdb`.
- Not verified: in game - this host cannot run CS2. What to look for: a full round start and a death with peers online, no crash, and `[online] ... gloves def=<def> painted` still in `x64/Release/n1ssanlogs.md`.

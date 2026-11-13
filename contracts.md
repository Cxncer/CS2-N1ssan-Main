# n1ssan — Component Contracts

Maintained reference for every document, file, and endpoint one component consumes
from another. Failure codes live in [failure-taxonomy.md](failure-taxonomy.md);
diagnostics transport is documented here and in that file.

Versioning rule for all persisted documents: **additive only**. A reader must accept
a document whose `version`/`schemaVersion` it knows, and must reject (not silently
repair) a document from a newer version. Renaming or re-typing an existing field
requires a new version and a migration that preserves user data.

## 1. Armory inventory document

| | |
| --- | --- |
| Writer | Bridge (`bridge/armory-store.ts`) |
| Readers | Bridge API, Armory UI, native `ArmoryInventory` |
| File | `%LOCALAPPDATA%\n1ssan\configs\armory-inventory.json` |
| Version | `schemaVersion: 1` |
| Validation | `bridge/armory-contract.ts` (structural, revision, profile identity) |

Owned item instances. `revision` increases on every accepted write; a write carrying
a stale revision is rejected with a conflict error rather than overwriting.
Every attachment and slot references an instance ID, never a catalogue key.

## 2. Armory equipment document

| | |
| --- | --- |
| Writer | Bridge (`bridge/armory-equipment-store.ts`) |
| Readers | Bridge API, native equipment projection |
| File | `%LOCALAPPDATA%\n1ssan\configs\armory-equipment.json` |
| Version | equipment document version 1 |
| Validation | `src/armory/equipment.ts` |

Assignments reference owned instance IDs plus the inventory revision and active
profile they were resolved against. Equipment is not a loadout: it never contains
catalogue selections.

## 3. Loadout runtime projection

| | |
| --- | --- |
| Writer | Bridge (`projectLoadoutRuntime` in `src/armory/runtime-config.ts`) |
| Reader | Native DLL (`PlanA`, `loadout.cpp`) |
| File | `%LOCALAPPDATA%\n1ssan\configs\n1ssan-data.json` |
| Version | document version 2, attachment projection version 1 |

Loadout mode's items. The legacy `/skins` snapshot lives in a separate file
(`n1ssan-legacy-loadout.json`) and must never be mixed into this document: the DLL
would read items CS2 cannot create.

## 4. Loadout profiles

| | |
| --- | --- |
| Writer | UI through `bridge/loadout-profiles-store.ts` |
| Reader | Bridge API, Loadout UI |
| File | `%LOCALAPPDATA%\n1ssan\configs\loadout-profiles.json` |
| Validation | `LoadoutValidationError` on structural mismatch |

Catalogue selections and equipment intent per profile. A newly created profile is
empty; duplicating is the explicit copy operation. Legacy browser-stored loadouts
migrate into a separate profile during `config-migration`.

## 5. Integration snapshot and simulator

| | |
| --- | --- |
| Writer | Bridge (`src/integration/local-snapshot.ts`) |
| Reader | `GET /integration/snapshot`, `GET /integration/simulator` |
| Version | snapshot schema version 3 |

Local hand-off validation only. It confirms the documents are internally consistent;
it does not prove in-game application.

## 6. Runtime mode

| | |
| --- | --- |
| Writer | Bridge (`runtime_mode.cpp` watcher on the native side) |
| Reader | DLL activation thread, Loadout preflight |
| File | `%LOCALAPPDATA%\n1ssan\configs\runtime-mode.json` |

`loadout` or `armory`. The DLL starts mode-neutral and only adopts a source after
this file names one and the compatibility preflight passes.

## 7. DLL status record

| | |
| --- | --- |
| Writer | DLL (`src/diagnostics/log.cpp`) |
| Readers | Launcher (`DllStatusMonitor`), Bridge (`bridge/diagnostics.ts`), status harness |
| File | `%LOCALAPPDATA%\n1ssan\status\cs2-<pid>.status` (TEMP fallback) |
| Version | protocol 2 (protocol 1 still accepted) |

`N1SSAN_STATUS|2|<utc>|<pid>|<build>|<state>|reason=<text>|code=<CODE>`, replaced
atomically. States: `INITIALIZING`, `CHECKS_PASSED`, `READY`, `DEGRADED`, `FAILED`.
The heartbeat interval is 10s; anything older than 30s is stale, never READY.
The `reason` of `READY`/`DEGRADED` carries the signature health summary
(`signatures=<resolved>/<total>`), so an outdated build is visible before an apply
fails.

## 8. Injector outcome

| | |
| --- | --- |
| Writer | `development/n1ssan-injectorv2` (stdout, captured by the launcher) |
| Reader | Launcher `InjectorRunner` |

`RESULT=OK pid=<pid> status=injected dll="<path>"` or
`RESULT=FAIL code=<n> name=<CODE>`. Exit codes stay numeric for process-level
consumers; `name=` is the taxonomy code.

## 9. Bridge HTTP API

Bound to `127.0.0.1:3210`. All responses are `no-store` where they describe live
state.

| Endpoint | Method | Contract |
| --- | --- | --- |
| `/health` | GET | Bridge liveness, runtime mode, armory revision, item count |
| `/diagnostics` | GET | Consolidated diagnostics document (protocol 2) |
| `/armory` | GET/PUT | Armory inventory document (schema 1), revision-checked |
| `/armory/equipment` | GET/PUT | Equipment document |
| `/armory/apply` | POST | Push the active inventory into the native runtime |
| `/armory/events` | GET | SSE notifications: `armory`, `equipment` |
| `/loadout/profiles` | GET | Stored loadout profiles |
| `/loadout/apply` | POST | Push a Loadout selection into the runtime document |
| `/runtime-mode` | POST | Publish `loadout`/`armory`; reset clears the choice |
| `/integration/snapshot`, `/integration/simulator` | GET | Local contracts |
| `/session/heartbeat`, `/session/active` | POST/GET | Browser session tracking |

The UI validates every document it accepts (`isDiagnosticsDocument`,
`parseArmoryDocument`); an unknown shape is a rejection, not a best-effort read.

## 10. Change checklist

1. Bump the document/record version and keep the previous one readable.
2. Add a migration that preserves user data; never rewrite in place without backup.
3. Update the writer, every reader, this file, and the tests that assert the shape
   (`bridge/*.test.ts`, `supporting/n1ssan-status-tests` for status records).
4. Record the change in the operation roadmap.

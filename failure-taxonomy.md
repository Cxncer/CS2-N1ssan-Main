# n1ssan — Failure Taxonomy

One classification vocabulary for every component. A fault is named the same way in
the DLL status record, the injector output, the launcher UI, the Bridge API, and the
Armory diagnostics panel.

Owners:

- **DLL** — `development/CS2-n1ssan` (`LogStatus(state, reason, code)`)
- **Injector** — `development/n1ssan-injectorv2` (`RESULT=FAIL code=<n> name=<CODE>`)
- **Launcher** — `development/n1ssan-launcher` (`FailureCatalog.cs`, `DllStatusMonitor.cs`)
- **Bridge** — `development/n1ssan-web-bridge/bridge/diagnostics.ts`
- **UI** — `development/n1ssan-web-bridge/src/shared/diagnostics.ts`

## Wire formats

DLL status record (protocol 2, `%LOCALAPPDATA%\n1ssan\status\cs2-<pid>.status`):

```
N1SSAN_STATUS|2|2026-11-13T09:00:00Z|4242|build-3|READY|reason=runtime_worker_heartbeat signatures=11/11|code=E_NONE
```

Protocol 1 records (seven fields, no `code=`) are still accepted; the code is then
absent and consumers fall back to their own default for that state.

Injector outcome line (stdout):

```
RESULT=OK pid=4242 status=injected dll="...\n1ssan-dll.dll"
RESULT=FAIL code=6 name=E_MODULE_REJECTED
```

The same outcome is written to `%LOCALAPPDATA%\n1ssan\status\injector.json` for the
Bridge's diagnostics document.

Bridge diagnostics document (`GET /diagnostics`):

```json
{ "protocol": 2, "generatedAt": "...", "condition": "DEGRADED",
  "sources": [ { "id": "dll", "label": "Native runtime", "condition": "DEGRADED",
                 "code": "W_OPTIONAL_FEATURE_UNAVAILABLE", "message": "...",
                 "action": "...", "updatedAt": "..." } ] }
```

## Codes

`E_` = fault (operation did not complete). `W_` = usable degradation. `E_NONE` = healthy.

| Code | Owner | Meaning | Consumer action |
| --- | --- | --- | --- |
| `E_NONE` | DLL | Healthy state or heartbeat | None |
| `W_OPTIONAL_FEATURE_UNAVAILABLE` | DLL | READY with an optional capability missing (glove schema/paint kit) | Skins and attachments still apply; glove schema needs a signature refresh |
| `W_STATUS_MISSING` | Bridge/Launcher | No status record for this process yet | Inject the DLL |
| `W_STATUS_STALE` | Bridge/Launcher | Heartbeat older than 30s | Confirm CS2 is running, then inject again |
| `W_STATUS_PROTOCOL` | Bridge | Status record from a newer protocol | Update the Bridge |
| `E_STATUS_INVALID` | Bridge/Launcher | Record exists but cannot be trusted (bad timestamp, unknown state) | Re-inject to rewrite it |
| `E_MINHOOK_LOAD` | DLL | MinHook could not be loaded | Place `MinHook.dll` next to `n1ssan-dll.dll` |
| `E_CLIENT_MODULE` | DLL | `client.dll` never appeared | Wait for the game to finish loading |
| `E_BRIDGE_DIRECTORY` | DLL | Bridge runtime directory not found | Start the Bridge from the launcher |
| `E_FUNCTION_RESOLUTION` | DLL | Required signatures did not resolve | Build predates this CS2 update; refresh signatures |
| `E_INVENTORY_RUNTIME` | DLL | SOCache/inventory runtime failed to initialise | Restart CS2 with `-allow_third_party_software` |
| `E_FSN_HOOK` | DLL | FrameStageNotify hook installation failed | Close overlay software and re-inject |
| `E_MODE_WORKER` | DLL | Runtime-mode worker thread could not be created | Restart CS2 |
| `E_GAME_NOT_FOUND` | Injector | No `cs2.exe` to target | Start Counter-Strike 2 first |
| `E_DLL_PROBLEM` | Injector | DLL missing, or wrong architecture | Rebuild x64 Release |
| `E_INJECTION_FAILED` | Injector | `LoadLibraryW` failed inside the game | Run the launcher elevated |
| `E_MODULE_REJECTED` | Injector | CS2 unloaded the module (Trusted Mode) | Launch CS2 with `-allow_third_party_software` |
| `E_DLL_FAILED` | Injector | DLL loaded but never reached READY | Read the session log |
| `E_STATUS_TIMEOUT` | Injector | No fresh status heartbeat within the timeout | Confirm the DLL matches the installed build |
| `E_BRIDGE_UNAVAILABLE` | Bridge | Bridge API unreachable from the UI | Start the launcher/Bridge |
| `E_ARMORY_UNAVAILABLE` | Bridge/UI | Armory API unreachable or 5xx | Start the launcher/Bridge and retry the request |
| `E_ARMORY_CONFLICT` | Bridge/UI | Write rejected: stale inventory/equipment revision | Reload the Armory before editing again |
| `E_ARMORY_VALIDATION` | Bridge/UI | Document failed structural validation | Fix the reported field; nothing was saved |
| `E_ARMORY_REJECTED` | Bridge/UI | Request refused (unknown route or malformed payload) | Reopen the Armory and retry |
| `E_UNEXPECTED` | Injector | Unhandled exception in the injector | Read the injector stderr output |
| `E_USAGE` | Injector | Bad command-line arguments | Fix the invocation |
| `E_CANCELLED` | Injector | Operator cancelled | None |

## Rules

1. A component may only emit codes listed here; adding a code means updating this
   table, `failure-taxonomy.md` consumers, and the tests that assert it.
2. `FAILED` is terminal for a DLL session and remains actionable after the worker
   exits. `READY` requires a live heartbeat — a stale record is never READY.
3. The launcher and Bridge translate a code into a next action; they never invent
   their own diagnosis text for a code they do not know.
4. Absence of evidence is `W_STATUS_MISSING` / `UNKNOWN`, never `FAILED`.

## Not yet classified

Armory HTTP validation errors (`ArmoryBridgeError`) keep their own HTTP status and
document-level codes. Folding them into this table is follow-up work.

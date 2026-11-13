# n1ssan — Release Procedure

How a version is built, named, verified, and published. Environment setup is in
[DEVELOPMENT.md](DEVELOPMENT.md); cross-component contracts are in
[contracts.md](contracts.md) and failure codes in [failure-taxonomy.md](failure-taxonomy.md).

## Versioning

`VERSION` at the repository root holds the release version, `MAJOR.MINOR.PATCH`.

| Change | Bump |
| --- | --- |
| New operation or user-facing capability | MINOR |
| Bug fix, diagnostics, packaging, docs | PATCH |
| Contract break (a component can no longer read another's document) | MAJOR |

Component repositories carry no version of their own; a release is defined by the
commit set recorded in its manifest.

## Artifacts

A release build produces one staged folder and one archive:

```
release/
  n1ssan/                      staged runtime layout
    n1ssan-launcher.exe
    injector/n1ssan-injector.exe
    dll/n1ssan-dll.dll
    dll/MinHook.x64.dll
    runtime/node.exe
    runtime/bridge/server.js
    runtime/package.json
    runtime/node_modules/
    web/dist/index.html
    web/dist/obsidian.html
    web/dist/assets/
    manifest.json
  n1ssan-<version>-win64.zip   the same folder, compressed
```

Naming: `n1ssan-<version>-win64.zip`. Never republish a version that has shipped; bump
instead. Keep every published archive: it is the rollback target.

The staged DLL folder intentionally contains exactly one runtime. Never add
`n1ssan-gloves`, `n1ssan-matchdetect`, or `n1ssan-attribdetect` binaries from
`supporting/` — those are development references, and a second copy hooking
FrameStageNotify crashes the client during a match.

## Build

```powershell
n1ssan-build.bat            # build, stage, write manifest, create the archive
n1ssan-build.bat verify     # builds + every automated check, nothing staged
n1ssan-build.bat dll        # one component, no staging/archiving
```

The script builds in this order: native DLL (MSBuild x64 Release) → injector
(`dotnet publish` win-x64 self-contained) → launcher (`dotnet publish` win-x64
self-contained) → Bridge + web bundle (`npm run build:production`, including the packaged
Bridge output and production `node_modules`) → staging → manifest → archive.

`manifest.json` records the product version, platform, build time, the main repository and
every component commit, the contract versions in force, and the size and SHA-256 of every
runtime artifact. A build is only reproducible when the manifest's commit set is used:
check out those commits in the main repository and each submodule before rebuilding.

## Compatibility rules

All components in a release must come from the same commit set. The contracts they must
agree on (also recorded in the manifest):

| Contract | Version | Writer → Reader |
| --- | --- | --- |
| DLL status record | protocol 2 (accepts 1) | DLL → Launcher, Injector watcher, Bridge, status harness |
| Injector outcome | `code=<n> name=<CODE>` | Injector → Launcher |
| Armory inventory | schema 1 | Bridge → UI, native inventory |
| Armory equipment | document 1 | Bridge → native projection |
| Loadout runtime document | 2 (attachments 1) | Bridge → DLL |
| Integration snapshot | 3 | Bridge → UI |
| Loadout profiles | 1 | Bridge → UI |

A newer component must keep reading the previous document version; a reader must reject a
document from a newer version instead of guessing. When a contract changes, update
`contracts.md`, the writer, every reader, and the tests in the same change.

## Verification checklist

Before publishing:

1. `n1ssan-build.bat verify` — native DLL builds, status harness 38/38, launcher and
   injector build, Bridge `tsc -b` clean, `npm test` 100/100, `npm run build` clean.
2. `n1ssan-build.bat` — packaging succeeds; inspect `release/n1ssan/manifest.json` and
   confirm the component commits are the intended ones.
3. Launch `release/n1ssan/n1ssan-launcher.exe` on a clean data directory
   (`%LOCALAPPDATA%\n1ssan` moved aside) and confirm the launcher reaches READY.
4. In game: apply a Loadout profile and an Armory inventory; confirm skins, stickers, and
   one charm per gun; confirm the Diagnostics panel shows READY or DEGRADED with a code.
5. Confirm no `FAILED` status record or `RESULT=FAIL` line remains from the test session.

Steps 3–5 require a machine with CS2 and cannot be performed by the build script.

## Publishing

1. Commit and push every component, then the main repository's submodule pointers.
2. Run the release build and keep `release/n1ssan-<version>-win64.zip` plus
   `manifest.json`.
3. Record the release (version, manifest commit set, verification notes) in
   `operation-roadmap.md`.

## Rollback

Restore the previous archive over the installed folder. User data in
`%LOCALAPPDATA%\n1ssan` is never touched by a release build or rollback, and no migration
is allowed to delete it; keep a copy before any data-layout change.

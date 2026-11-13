# n1ssan — Contribution and Repository Standards

Branch model, commit conventions, and the rules for keeping six repositories consistent.
Build and test commands are in [DEVELOPMENT.md](DEVELOPMENT.md); release steps are in
[RELEASE.md](RELEASE.md).

## Branch model

The Solid Safe House branches are the working baseline. Never develop on `main` or
`master`.

| Repository | Working branch |
| --- | --- |
| `CS2-N1ssan-Main` | `solid-safe-house` |
| `CS2-n1ssan` (DLL) | `solid-safe-house-dll` |
| `n1ssan-injectorv2` | `solid-safe-house-injectorv2` |
| `n1ssan-launcher` | `solid-safe-house-launcher` |
| `n1ssan-web-bridge` | `solid-safe-house-bridge` |
| `n1ssan-gloves` | `solid-safe-house-gloves` |

Short-lived work branches are allowed (`fix/...`, `operation-<n>-...`) but land back on
the Solid Safe House branch before the submodule pointer is updated. `main`/`master` are
historical and must not receive new commits from this workspace.

## Commit messages

```
type(scope): imperative summary

Why the change was made, if it is not obvious from the summary.
```

`type` is `feat`, `fix`, `refactor`, `docs`, `build`, `test`, or `chore`; `scope` is the
component or module (`armory`, `dll`, `launcher`, `bridge`, `injector`, `release`).
One logical change per commit. Keep subjects under ~72 characters.

## Rules

1. **User data is sacred.** Never delete or rewrite `%LOCALAPPDATA%\n1ssan` content
   without a backup and a migration that preserves it.
2. **No secrets in the repository.** No API keys, tokens, passwords, or machine paths.
3. **No build output.** `build/`, `release/`, `dist/`, `node_modules/`, binaries, logs,
   and PDBs stay untracked.
4. **Contracts are explicit.** A change to a document, status record, or endpoint updates
   `contracts.md`, every reader, and the tests in the same commit.
5. **Failure codes are shared.** New codes go in `failure-taxonomy.md` and in the
   component that emits them, with the consumer action.
6. **One owner per responsibility.** The launcher orchestrates; the injector injects; the
   DLL owns hooks and inventory writes; the Bridge owns documents; the UI owns presentation.
   Do not duplicate another component's job.
7. **Every change is verified before it is pushed.** `n1ssan-build.bat verify` at minimum,
   plus the in-game check when the change touches native behaviour.

## Submodule pointers

The main repository pins one commit per component. Update pointers only after the
component commit is pushed.

```powershell
# 1. in each changed component
cd development/n1ssan-web-bridge
git add -A
git commit -m "fix(bridge): ..."
git push origin solid-safe-house-bridge

# 2. in the main repository
cd ../..
git add development/n1ssan-web-bridge
git commit -m "build: update web bridge pointer to <summary>"
```

Do not stage a submodule pointer to an unpushed commit: the workspace cannot be cloned by
anyone else, including on a clean machine.

## Combined baseline validation

After updating pointers:

```powershell
git submodule status                     # every pinned commit present, no '+' prefix
n1ssan-build.bat verify                  # every build and test passes from the merged tree
```

A `+` prefix means a submodule working tree is ahead of its pointer — commit and push, or
reset it, before declaring the baseline valid.

## Review checklist

- Does the change belong to the component that owns the responsibility?
- Are contracts, taxonomy, and tests updated together?
- Is user data preserved, including the active Loadout/Armory profile?
- Are failure paths classified with a taxonomy code and an actionable message?
- Was `n1ssan-build.bat verify` run, and was the result recorded?
- For native changes: was the in-game check performed (or explicitly reported as not
  performed)?

## Documentation to keep current

| File | When it changes |
| --- | --- |
| `operation-roadmap.md` | Operations start, advance, or are accepted |
| `contracts.md` | A document, record, or endpoint changes |
| `failure-taxonomy.md` | A code is added, renamed, or retired |
| `DEVELOPMENT.md` / `RELEASE.md` / `CONTRIBUTING.md` | Tooling, packaging, or process changes |
| `cs2-n1ssan-project-structure.md` | Directories are added, moved, or removed |

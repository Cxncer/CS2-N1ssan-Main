# n1ssan — Development Environment

Everything needed to build, run, and check the project from a clean Windows machine.
Release packaging is in [RELEASE.md](RELEASE.md); branch and submodule rules are in
[CONTRIBUTING.md](CONTRIBUTING.md).

## Repositories

| Path | Repository | Role |
| --- | --- | --- |
| `.` | `CS2-N1ssan-Main` | Workspace: docs, build script, submodule pointers |
| `development/CS2-n1ssan` | `CS2-n1ssan` | Native DLL (hooks, inventory, skin application) |
| `development/n1ssan-injectorv2` | `n1ssan-injectorv2` | Standalone loader |
| `development/n1ssan-launcher` | `n1ssan-launcher` | WPF launcher (DOMINATE) |
| `development/n1ssan-web-bridge` | `n1ssan-web-bridge` | Bridge API + Vite React UIs |
| `supporting/n1ssan-gloves` | `n1ssan-gloves` | Gloves reference/archive (not built) |

## Prerequisites

| Tool | Version | Notes |
| --- | --- | --- |
| Windows | 10/11 x64 | WPF launcher and the DLL are Windows-only |
| Visual Studio 2022 | Community or higher | "Desktop development with C++" workload; MSBuild + v143 toolset |
| .NET SDK | 8.0 (8.0.410 tested) | `net8.0-windows`, WPF |
| Node.js | 20+ (current LTS tested) | Vite 8 / React 19; the same `node.exe` is bundled into the release |
| Git | any recent | Submodules are required |

`n1ssan-build.bat` locates MSBuild through `vswhere.exe`; no PATH setup is required for
Visual Studio. Node and .NET must be on PATH.

## Bootstrap

```powershell
git clone --recurse-submodules https://github.com/Cxncer/CS2-N1ssan-Main.git
cd CS2-N1ssan-Main
cd development/n1ssan-web-bridge; npm ci; cd ../..
```

If the workspace was cloned without submodules:

```powershell
git submodule update --init --recursive
```

## Build and test by component

| Component | Build | Test / check |
| --- | --- | --- |
| Native DLL | `"<VS>\MSBuild\Current\Bin\MSBuild.exe" development\CS2-n1ssan\n1ssan-dll.sln -p:Configuration=Release -p:Platform=x64` | Compiles to `development\CS2-n1ssan\x64\Release\n1ssan-dll.dll` |
| Status harness | `supporting\n1ssan-status-tests\build.cmd` | 38 protocol checks; prints `TOTAL: n/n` |
| Injector | `dotnet build development\n1ssan-injectorv2\n1ssan-injector.csproj -c Release` | — |
| Launcher | `dotnet build development\n1ssan-launcher\n1ssan-launcher.csproj -c Release` | Run from `bin\Debug\net8.0-windows\` while developing |
| Bridge + web | `cd development\n1ssan-web-bridge; npx tsc -b; npm run build` | `npm test` (100 checks) |

All of the above in one command, with no packaging:

```powershell
n1ssan-build.bat verify
```

> In Git Bash use `-p:` for MSBuild properties; `/p:` is mangled into a path.

## Development loop

1. Bridge API: `cd development/n1ssan-web-bridge && npm run bridge` (127.0.0.1:3210).
2. Web UI: `npm run dev` (Vite). **Use `http://localhost:<port>`, not `127.0.0.1`** —
   the dev server binds the IPv6 loopback only.
   - `/` skins catalogue, `/loadout` Loadout mode, `/obsidian.html` Armory mode.
   - Armory requires the Bridge; without it the page reports connection failures.
3. Native DLL: build, launch `n1ssan-launcher.exe` from `bin\Debug\net8.0-windows\`,
   then use it to inject. The DLL writes a fresh session log every injection.

## Runtime locations

| Path | Contents |
| --- | --- |
| `%LOCALAPPDATA%\n1ssan\configs\` | Armory inventory, equipment, runtime document, loadout profiles, runtime mode |
| `%LOCALAPPDATA%\n1ssan\status\cs2-<pid>.status` | DLL status record (TEMP fallback) |
| `%LOCALAPPDATA%\n1ssan\logs\` | Launcher startup log (DOMINATE) |
| `development\CS2-n1ssan\x64\Release\n1ssanlogs.md` | Native session log (overwritten each injection) |

## Game requirements

- Counter-Strike 2 started with `-allow_third_party_software`; otherwise Trusted Mode
  unloads the DLL and the injector reports `E_MODULE_REJECTED`.
- Only one `n1ssan-dll.dll` may be loaded per process. A second copy installs a second
  FrameStageNotify hook and duplicates inventory writes.

## Known gotchas

- Vite binds `::1`; `curl http://127.0.0.1:<port>` fails while `localhost` works.
- The Bridge port is fixed at 3210 unless `N1SSAN_PORT` is set; UI clients assume 3210.
- A DLL status heartbeat older than 30 seconds is stale, never READY.
- `npm run build` prints the existing large-chunk advisory from Vite; it is not an error.
- `release/` and `build/` are build output and are not committed.

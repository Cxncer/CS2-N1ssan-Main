@echo off
rem n1ssan-build.bat - one-step release build for the whole project.
rem
rem Builds, in order:
rem   1. n1ssan-dll.dll          (x64 Release, MSBuild)
rem   2. n1ssan-injector.exe     (dotnet publish, win-x64)
rem   3. n1ssan-launcher.exe     (dotnet publish, win-x64)
rem   4. Bridge + web bundle     (npm run build:production)
rem   5. Apex / N1ssan Sync      (Tauri: npm build + cargo release build)
rem and stages the packaged release layout the launcher expects:
rem
rem   release\n1ssan\
rem     n1ssan-launcher.exe
rem     injector\n1ssan-injector.exe
rem     dll\n1ssan-dll.dll (+ MinHook.x64.dll)
rem     runtime\node.exe, runtime\bridge\server.js, runtime\node_modules
rem     web\dist\index.html, web\dist\obsidian.html
rem     apex\n1ssan-apex.exe, offsets.json
rem
rem Every build finishes with release\n1ssan\manifest.json (version, component
rem commits, artifact SHA-256) and release\n1ssan-<version>-win64.zip.
rem
rem Usage: n1ssan-build.bat            build everything, verify nothing, stage the release
rem        n1ssan-build.bat verify     run every check (builds + tests + harness), no staging
rem        n1ssan-build.bat dll        build only the native DLL
rem        n1ssan-build.bat injector   build only the injector
rem        n1ssan-build.bat launcher   build only the launcher
rem        n1ssan-build.bat bridge     build only the Bridge + web bundle
rem        n1ssan-build.bat apex       build only Apex (N1ssan Sync)
rem        n1ssan-build.bat verify-package   check the staged release layout
rem
rem For a new version use n1ssan-release.bat, which prompts for the build version
rem and then calls this script.
rem
rem See DEVELOPMENT.md (environment), RELEASE.md (packaging and compatibility), and
rem CONTRIBUTING.md (branch and submodule rules).
setlocal enabledelayedexpansion
cd /d "%~dp0"
set "ROOT=%CD%"
set "RELEASE=%ROOT%\release\n1ssan"
set "TARGET=%~1"

rem ---------------------------------------------------------------- tools
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -prerelease -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"

rem Apex is Rust: use the per-user rustup toolchain when it is not on PATH.
set "CARGO=cargo"
if exist "%USERPROFILE%\.cargo\bin\cargo.exe" set "CARGO=%USERPROFILE%\.cargo\bin\cargo.exe"

echo === n1ssan release build ===
echo Root: %ROOT%

if /i "%TARGET%"=="verify" goto verify
if /i "%TARGET%"=="verify-package" goto verify_package

rem ---------------------------------------------------------------- native DLL
if "%TARGET%"=="" goto build_dll
if /i "%TARGET%"=="dll" goto build_dll
goto after_dll

:build_dll
if not defined MSBUILD (
    echo Could not find Visual Studio / MSBuild ^(vswhere.exe^).
    goto failed
)
echo.
echo === 1/5 n1ssan-dll.dll ^(x64 Release^) ===
"%MSBUILD%" "%ROOT%\development\CS2-n1ssan\n1ssan-dll.sln" /m /nologo /v:minimal /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto failed
if /i "%TARGET%"=="dll" goto done

:after_dll
rem ---------------------------------------------------------------- injector
if "%TARGET%"=="" goto build_injector
if /i "%TARGET%"=="injector" goto build_injector
goto after_injector

:build_injector
echo.
echo === 2/5 n1ssan-injector.exe ^(win-x64^) ===
dotnet publish "%ROOT%\development\n1ssan-injectorv2\n1ssan-injector.csproj" -c Release -r win-x64 --self-contained true -o "%ROOT%\build\injector" -v:minimal
if errorlevel 1 goto failed
if /i "%TARGET%"=="injector" goto done

:after_injector
rem ---------------------------------------------------------------- launcher
if "%TARGET%"=="" goto build_launcher
if /i "%TARGET%"=="launcher" goto build_launcher
goto after_launcher

:build_launcher
echo.
echo === 3/5 n1ssan-launcher.exe ^(win-x64^) ===
dotnet publish "%ROOT%\development\n1ssan-launcher\n1ssan-launcher.csproj" -c Release -r win-x64 --self-contained true -o "%ROOT%\build\launcher" -v:minimal
if errorlevel 1 goto failed
if /i "%TARGET%"=="launcher" goto done

:after_launcher
rem ---------------------------------------------------------------- bridge
if "%TARGET%"=="" goto build_bridge
if /i "%TARGET%"=="bridge" goto build_bridge
goto after_bridge

:build_bridge
echo.
echo === 4/5 Bridge + web bundle ===
pushd "%ROOT%\development\n1ssan-web-bridge"
if not exist node_modules (
    call npm install
    if errorlevel 1 goto bridge_failed
)
call npm run build:production
if errorlevel 1 goto bridge_failed
popd
if /i "%TARGET%"=="bridge" goto done

:after_bridge
rem ---------------------------------------------------------------- apex
if "%TARGET%"=="" goto build_apex
if /i "%TARGET%"=="apex" goto build_apex
goto after_apex

:build_apex
echo.
echo === 5/5 Apex ^(N1ssan Sync, Tauri^) ===
pushd "%ROOT%\development\n1ssan-apex"
if not exist node_modules (
    call npm install
    if errorlevel 1 goto apex_failed
)
rem The Tauri CLI, not bare cargo: it marks the build as non-dev so the frontend
rem is embedded. A plain cargo release build keeps the dev URL and the window
rem opens on "localhost refused to connect".
call npm run tauri:build -- --no-bundle
if errorlevel 1 goto apex_failed
popd
if /i "%TARGET%"=="apex" goto done

:after_apex
rem ---------------------------------------------------------------- staging
echo.
echo === staging %RELEASE% ===
if exist "%RELEASE%" rmdir /s /q "%RELEASE%"
mkdir "%RELEASE%\dll"                       || goto failed
mkdir "%RELEASE%\injector"                  || goto failed
mkdir "%RELEASE%\runtime\bridge"            || goto failed
mkdir "%RELEASE%\web\dist"                  || goto failed
mkdir "%RELEASE%\apex"                       || goto failed

rem The DLL folder holds exactly one runtime. Stale companion DLLs
rem (n1ssan-gloves / n1ssan-matchdetect / n1ssan-attribdetect) are deliberately
rem not copied: a second copy hooking FrameStageNotify duplicates inventory
rem writes and crashes the client when a match starts.
rem Apex and the offset dataset it checks and rewrites travel with the release.
copy /y "%ROOT%\development\n1ssan-apex\src-tauri\target\release\n1ssan-apex.exe" "%RELEASE%\apex\" >nul || goto failed
copy /y "%ROOT%\development\n1ssan-apex\definitions\offsets.json" "%RELEASE%\offsets.json" >nul || goto failed

copy /y "%ROOT%\development\CS2-n1ssan\x64\Release\n1ssan-dll.dll"  "%RELEASE%\dll\" >nul || goto failed
if exist "%ROOT%\development\CS2-n1ssan\x64\Release\MinHook.x64.dll" copy /y "%ROOT%\development\CS2-n1ssan\x64\Release\MinHook.x64.dll" "%RELEASE%\dll\" >nul || goto failed

xcopy /y /q /e /i "%ROOT%\build\injector\*" "%RELEASE%\injector\" >nul || goto failed
xcopy /y /q /e /i "%ROOT%\build\launcher\*" "%RELEASE%\" >nul || goto failed

copy /y "%ROOT%\development\n1ssan-web-bridge\dist\index.html"    "%RELEASE%\web\dist\" >nul || goto failed
if exist "%ROOT%\development\n1ssan-web-bridge\dist\obsidian.html" copy /y "%ROOT%\development\n1ssan-web-bridge\dist\obsidian.html" "%RELEASE%\web\dist\" >nul
xcopy /y /q /e /i "%ROOT%\development\n1ssan-web-bridge\dist\assets" "%RELEASE%\web\dist\assets\" >nul

copy /y "%ROOT%\development\n1ssan-web-bridge\runtime\bridge\server.js" "%RELEASE%\runtime\bridge\" >nul || goto failed
copy /y "%ROOT%\development\n1ssan-web-bridge\runtime\package.json"    "%RELEASE%\runtime\" >nul || goto failed
xcopy /y /q /e /i "%ROOT%\development\n1ssan-web-bridge\runtime\node_modules" "%RELEASE%\runtime\node_modules\" >nul || goto failed

rem The packaged Bridge runs on the bundled Node runtime.
set "NODE_EXE="
for /f "delims=" %%i in ('where node 2^>nul') do (
    set "NODE_EXE=%%i"
    goto node_found
)
:node_found
if defined NODE_EXE (
    copy /y "!NODE_EXE!" "%RELEASE%\runtime\node.exe" >nul || goto failed
) else (
    echo WARNING: node.exe was not found on PATH; copy it to %RELEASE%\runtime\node.exe
)

rem ---------------------------------------------------------------- manifest + archive
set "N1SSAN_VERSION=0.0.0"
if exist "%ROOT%\VERSION" for /f "usebackq delims=" %%v in ("%ROOT%\VERSION") do set "N1SSAN_VERSION=%%v"

echo.
echo === manifest + archive ^(version %N1SSAN_VERSION%^) ===
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%\tools\release.ps1" -Root "%ROOT%" -Release "%RELEASE%" -Version "%N1SSAN_VERSION%"
if errorlevel 1 goto failed

echo.
echo BUILD OK
echo   Version  : %N1SSAN_VERSION%
echo   Launcher : %RELEASE%\n1ssan-launcher.exe
echo   DLL      : %RELEASE%\dll\n1ssan-dll.dll
echo   Injector : %RELEASE%\injector\n1ssan-injector.exe
echo   Bridge   : %RELEASE%\runtime\bridge\server.js
echo   Apex     : %RELEASE%\apex\n1ssan-apex.exe
echo   Offsets  : %RELEASE%\offsets.json
echo   Manifest : %RELEASE%\manifest.json
echo   Archive  : %ROOT%\release\n1ssan-%N1SSAN_VERSION%-win64.zip
exit /b 0

rem ---------------------------------------------------------------- verify-package
:verify_package
if not exist "%ROOT%\release\n1ssan\manifest.json" (
    echo No staged release found. Run n1ssan-release.bat first.
    exit /b 1
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%\tools\verify-package.ps1" -Root "%ROOT%" -Release "%ROOT%\release\n1ssan"
if errorlevel 1 goto failed
exit /b 0

rem ---------------------------------------------------------------- verify
:verify
echo.
echo === verify 1/5 native DLL ===
if not defined MSBUILD (
    echo Could not find Visual Studio / MSBuild ^(vswhere.exe^).
    goto failed
)
"%MSBUILD%" "%ROOT%\development\CS2-n1ssan\n1ssan-dll.sln" -p:Configuration=Release -p:Platform=x64 /m /nologo /v:minimal
if errorlevel 1 goto failed

echo.
echo === verify 2/5 status harness ===
call "%ROOT%\supporting\n1ssan-status-tests\build.cmd"
if errorlevel 1 goto failed

echo.
echo === verify 3/5 launcher + injector ===
dotnet build "%ROOT%\development\n1ssan-launcher\n1ssan-launcher.csproj" -c Release -v:minimal
if errorlevel 1 goto failed
dotnet build "%ROOT%\development\n1ssan-injectorv2\n1ssan-injector.csproj" -c Release -v:minimal
if errorlevel 1 goto failed

echo.
echo === verify 4/5 Bridge + web ===
pushd "%ROOT%\development\n1ssan-web-bridge"
if not exist node_modules (
    call npm ci
    if errorlevel 1 goto bridge_failed
)
call npx tsc -b
if errorlevel 1 goto bridge_failed
call npm test
if errorlevel 1 goto bridge_failed
call npm run build
if errorlevel 1 goto bridge_failed
popd

echo.
echo === verify 5/5 Apex (Tauri) ===
pushd "%ROOT%\development\n1ssan-apex"
if not exist node_modules (
    call npm install
    if errorlevel 1 goto apex_failed
)
call npm run build
if errorlevel 1 goto apex_failed
"%CARGO%" test --manifest-path "src-tauri\Cargo.toml"
if errorlevel 1 goto apex_failed
popd

echo.
echo VERIFY OK - every build and test passed; nothing was staged.
exit /b 0

:apex_failed
popd
goto failed

:bridge_failed
popd
goto failed

:done
echo.
echo BUILD OK
exit /b 0

:failed
echo.
echo BUILD FAILED - scroll up for the first error.
exit /b 1

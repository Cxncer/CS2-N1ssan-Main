@echo off
rem n1ssan-release.bat - one-click release and build driver.
rem
rem This is the single build entry point for the whole project. It lives in
rem release\ and resolves the workspace root from its own location, so the
rem repository root holds no build scripts.
rem
rem Release (no target):
rem   release\n1ssan-release.bat                     prompt (Enter = same version)
rem   release\n1ssan-release.bat next                bump the patch of VERSION
rem   release\n1ssan-release.bat Beta.Build.v1.0.3   unattended (CI)
rem
rem Build / check targets:
rem   release\n1ssan-release.bat build            build everything, stage, archive
rem   release\n1ssan-release.bat verify           build every component and run every check
rem   release\n1ssan-release.bat verify-package   check the staged release layout
rem   release\n1ssan-release.bat dll              native DLL only
rem   release\n1ssan-release.bat injector         injector only
rem   release\n1ssan-release.bat launcher         launcher only
rem   release\n1ssan-release.bat bridge           Bridge + web bundle only
rem   release\n1ssan-release.bat apex             Apex / N1ssan Sync only
rem
rem Version scheme: <channel>.Build.v<MAJOR>.<MINOR>.<PATCH>
rem   e.g. Beta.Build.v1.0.0   Beta.Build.v1.0.1   Release.Build.v2.0.0
rem
rem The build stages a temporary folder outside the repository, writes
rem release\n1ssan-<version>-win64.zip and deletes the staging folder: the archive
rem is the only release output.
rem
rem Releasing the version that is already in VERSION is allowed: it rebuilds and
rem replaces the archive without touching the version number.
rem
rem See DEVELOPMENT.md (environment), RELEASE.md (packaging and compatibility),
rem and CONTRIBUTING.md (branch and submodule rules).
setlocal EnableDelayedExpansion
for %%I in ("%~dp0..") do set "ROOT=%%~fI"
cd /d "%ROOT%"

rem Staging happens outside the repository and is deleted once the archive is
rem written: release\ holds the .zip and this script, never an n1ssan\ folder.
set "STAGE=%TEMP%\n1ssan-release"
set "RELEASE=%STAGE%\n1ssan"
set "TARGET=%~1"

rem ---------------------------------------------------------------- tools
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -prerelease -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"

rem Apex is Rust: use the per-user rustup toolchain when it is not on PATH. The
rem Tauri CLI looks up cargo itself, so the toolchain directory goes on PATH too.
set "CARGO=cargo"
if exist "%USERPROFILE%\.cargo\bin\cargo.exe" (
    set "CARGO=%USERPROFILE%\.cargo\bin\cargo.exe"
    set "PATH=%USERPROFILE%\.cargo\bin;%PATH%"
)

rem ---------------------------------------------------------------- dispatch
if /i "%TARGET%"=="verify" goto verify
if /i "%TARGET%"=="verify-package" goto verify_package
if /i "%TARGET%"=="dll" goto build_dll
if /i "%TARGET%"=="injector" goto build_injector_only
if /i "%TARGET%"=="launcher" goto build_launcher_only
if /i "%TARGET%"=="bridge" goto build_bridge_only
if /i "%TARGET%"=="apex" goto build_apex_only
if /i "%TARGET%"=="build" goto build_all
if not "%TARGET%"=="" goto release_with_version
goto release_prompt

rem ---------------------------------------------------------------- release flow
:release_prompt
set "CURRENT_VERSION=0.0.0"
if exist "%ROOT%\VERSION" for /f "usebackq delims=" %%v in ("%ROOT%\VERSION") do set "CURRENT_VERSION=%%v"
echo ============================================
echo  n1ssan release
echo ============================================
echo  Current version : %CURRENT_VERSION%
echo  Format          : Beta.Build.v1.0.3
echo.
echo  Enter        = rebuild %CURRENT_VERSION% as it is
set "NEXT_PATCH="
for /f "usebackq delims=" %%v in (`powershell -NoProfile -Command "if ('%CURRENT_VERSION%' -match '^(.*\.)(\d+)$') { $matches[1] + ([int]$matches[2] + 1) } else { '%CURRENT_VERSION%' }"`) do set "NEXT_PATCH=%%v"
if defined NEXT_PATCH echo  next         = %NEXT_PATCH%  (bump patch)
echo  Beta.Build.v1.0.3 = a new build version you type yourself
echo.
set "NEW_VERSION="
set /p "NEW_VERSION=New build version: "
if not defined NEW_VERSION set "NEW_VERSION=%CURRENT_VERSION%"
goto validate_version

:release_with_version
rem A bare argument that is not a build target is a version, or "next" for the
rem next patch of the current one.
set "NEW_VERSION=%TARGET%"
if /i "%NEW_VERSION%"=="next" (
    set "CURRENT_VERSION=0.0.0"
    if exist "%ROOT%\VERSION" for /f "usebackq delims=" %%v in ("%ROOT%\VERSION") do set "CURRENT_VERSION=%%v"
    set "NEW_VERSION="
    for /f "usebackq delims=" %%v in (`powershell -NoProfile -Command "if ('!CURRENT_VERSION!' -match '^(.*\.)(\d+)$') { $matches[1] + ([int]$matches[2] + 1) } else { '!CURRENT_VERSION!' }"`) do set "NEW_VERSION=%%v"
)

:validate_version
set "CURRENT_VERSION=0.0.0"
if exist "%ROOT%\VERSION" for /f "usebackq delims=" %%v in ("%ROOT%\VERSION") do set "CURRENT_VERSION=%%v"

powershell -NoProfile -Command "if ('%NEW_VERSION%' -notmatch '^(Alpha|Beta|Release)\.Build\.v\d+\.\d+\.\d+$') { exit 1 }"
if errorlevel 1 (
    echo.
    echo Invalid version "%NEW_VERSION%".
    echo Use: Beta.Build.v1.0.0  ^(channel Alpha/Beta/Release, then Build.vMAJOR.MINOR.PATCH^)
    exit /b 2
)

set "SAME_VERSION="
if /i "%NEW_VERSION%"=="%CURRENT_VERSION%" (
    set "SAME_VERSION=1"
    echo.
    echo Re-releasing %NEW_VERSION% as it is - VERSION stays unchanged.
)

> "%ROOT%\VERSION" echo %NEW_VERSION%
for /f "usebackq delims=" %%v in ("%ROOT%\VERSION") do set "WRITTEN_VERSION=%%v"
if /i not "%WRITTEN_VERSION%"=="%NEW_VERSION%" (
    echo Could not write VERSION.
    exit /b 1
)

echo.
if defined SAME_VERSION (
    echo Building %NEW_VERSION% again
) else (
    echo Recycling %CURRENT_VERSION% -^> %NEW_VERSION%
)
echo.
goto build_all

rem ---------------------------------------------------------------- build everything
:build_all
set "TARGET="
echo === n1ssan release build ===
echo Root: %ROOT%

rem ---------------------------------------------------------------- native DLL
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

:build_injector_only
echo.
echo === 2/5 n1ssan-injector.exe ^(win-x64^) ===
dotnet publish "%ROOT%\development\n1ssan-injectorv2\n1ssan-injector.csproj" -c Release -r win-x64 --self-contained true -o "%ROOT%\build\injector" -v:minimal
if errorlevel 1 goto failed
if /i "%TARGET%"=="injector" goto done

:build_launcher_only
echo.
echo === 3/5 n1ssan-launcher.exe ^(win-x64^) ===
rem Single-file publish: the packaged root must hold only the two executables.
rem AppContext.BaseDirectory is still the folder holding the exe, so the launcher's
rem dll\, injector\, runtime\ and web\ discovery is unchanged.
rem Publish into an empty folder: publishing over an existing output makes
rem MSBuild skip the content items, which leaves assets\icons\ empty and the
rem archive would ship without the launcher icons. Clearing costs a moment.
if exist "%ROOT%\build\launcher" rmdir /s /q "%ROOT%\build\launcher"
dotnet publish "%ROOT%\development\n1ssan-launcher\n1ssan-launcher.csproj" -c Release -r win-x64 --self-contained true -o "%ROOT%\build\launcher" -v:minimal -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true -p:DebugType=none
if errorlevel 1 goto failed
if /i "%TARGET%"=="launcher" goto done

:build_bridge_only
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

:build_apex_only
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

rem ---------------------------------------------------------------- staging
echo.
echo === staging %RELEASE% ===
if exist "%RELEASE%" rmdir /s /q "%RELEASE%"
mkdir "%RELEASE%\dll"                       || goto failed
mkdir "%RELEASE%\injector"                  || goto failed
mkdir "%RELEASE%\runtime\bridge"            || goto failed
mkdir "%RELEASE%\web\dist"                  || goto failed
mkdir "%RELEASE%\data"                      || goto failed

rem The DLL folder holds exactly one runtime. Stale companion DLLs
rem (n1ssan-gloves / n1ssan-matchdetect / n1ssan-attribdetect) are deliberately
rem not copied: a second copy hooking FrameStageNotify duplicates inventory
rem writes and crashes the client when a match starts.
rem Apex sits beside the launcher; the dataset it checks and rewrites lives in data\.
copy /y "%ROOT%\development\n1ssan-apex\src-tauri\target\release\n1ssan-apex.exe" "%RELEASE%\" >nul || goto failed
copy /y "%ROOT%\development\n1ssan-apex\definitions\offsets.json" "%RELEASE%\data\" >nul || goto failed

copy /y "%ROOT%\development\CS2-n1ssan\x64\Release\n1ssan-dll.dll"  "%RELEASE%\dll\" >nul || goto failed
if exist "%ROOT%\development\CS2-n1ssan\x64\Release\MinHook.x64.dll" copy /y "%ROOT%\development\CS2-n1ssan\x64\Release\MinHook.x64.dll" "%RELEASE%\dll\" >nul || goto failed

xcopy /y /q /e /i "%ROOT%\build\injector\*" "%RELEASE%\injector\" >nul || goto failed

rem Launcher: the single-file exe at the root, plus the icon art it loads at runtime.
copy /y "%ROOT%\build\launcher\n1ssan-launcher.exe" "%RELEASE%\" >nul || goto failed
if exist "%ROOT%\build\launcher\assets" xcopy /y /q /e /i "%ROOT%\build\launcher\assets" "%RELEASE%\assets\" >nul || goto failed

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

rem The archive is the release: drop the temporary staging tree completely.
if exist "%STAGE%" rmdir /s /q "%STAGE%"

echo.
echo BUILD OK
echo   Version  : %N1SSAN_VERSION%
echo   Archive  : %ROOT%\release\n1ssan-%N1SSAN_VERSION%-win64.zip
echo   Layout   : n1ssan\ [n1ssan-launcher.exe n1ssan-apex.exe assets\ data\ dll\ injector\ runtime\ web\]
echo   Staging  : removed - the archive is the whole release
exit /b 0

rem ---------------------------------------------------------------- verify-package
:verify_package
if not exist "%ROOT%\release\n1ssan-*-win64.zip" (
    echo No release archive found. Run release\n1ssan-release.bat first.
    exit /b 1
)
rem No folder is staged, so verify-package unpacks the newest archive itself.
powershell -NoProfile -ExecutionPolicy Bypass -File "%ROOT%\tools\verify-package.ps1" -Root "%ROOT%"
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

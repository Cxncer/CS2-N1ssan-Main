@echo off
rem n1ssan-build.bat - one-step release build for the whole project.
rem
rem Builds, in order:
rem   1. n1ssan-dll.dll          (x64 Release, MSBuild)
rem   2. n1ssan-injector.exe     (dotnet publish, win-x64)
rem   3. n1ssan-launcher.exe     (dotnet publish, win-x64)
rem   4. Bridge + web bundle     (npm run build:production)
rem and stages the packaged release layout the launcher expects:
rem
rem   release\n1ssan\
rem     n1ssan-launcher.exe
rem     injector\n1ssan-injector.exe
rem     dll\n1ssan-dll.dll (+ MinHook.x64.dll)
rem     runtime\node.exe, runtime\bridge\server.js, runtime\node_modules
rem     web\dist\index.html, web\dist\obsidian.html
rem
rem Usage: n1ssan-build.bat            build everything and stage the release
rem        n1ssan-build.bat dll        build only the native DLL
rem        n1ssan-build.bat bridge     build only the Bridge + web bundle
setlocal enabledelayedexpansion
cd /d "%~dp0"
set "ROOT=%CD%"
set "RELEASE=%ROOT%\release\n1ssan"
set "TARGET=%~1"

rem ---------------------------------------------------------------- tools
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "MSBUILD="
if exist "%VSWHERE%" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -prerelease -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%i"

echo === n1ssan release build ===
echo Root: %ROOT%

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
echo === 1/4 n1ssan-dll.dll ^(x64 Release^) ===
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
echo === 2/4 n1ssan-injector.exe ^(win-x64^) ===
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
echo === 3/4 n1ssan-launcher.exe ^(win-x64^) ===
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
echo === 4/4 Bridge + web bundle ===
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
rem ---------------------------------------------------------------- staging
echo.
echo === staging %RELEASE% ===
if exist "%RELEASE%" rmdir /s /q "%RELEASE%"
mkdir "%RELEASE%\dll"                       || goto failed
mkdir "%RELEASE%\injector"                  || goto failed
mkdir "%RELEASE%\runtime\bridge"            || goto failed
mkdir "%RELEASE%\web\dist"                  || goto failed

rem The DLL folder holds exactly one runtime. Stale companion DLLs
rem (n1ssan-gloves / n1ssan-matchdetect / n1ssan-attribdetect) are deliberately
rem not copied: a second copy hooking FrameStageNotify duplicates inventory
rem writes and crashes the client when a match starts.
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

echo.
echo BUILD OK
echo   Launcher : %RELEASE%\n1ssan-launcher.exe
echo   DLL      : %RELEASE%\dll\n1ssan-dll.dll
echo   Injector : %RELEASE%\injector\n1ssan-injector.exe
echo   Bridge   : %RELEASE%\runtime\bridge\server.js
exit /b 0

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

@echo off
rem n1ssan-release.bat - one-click release.
rem
rem  1. asks for the new build version (or takes it as the first argument)
rem  2. writes it into VERSION
rem  3. runs the full release build (components, staging, manifest, archive)
rem  4. records the release in operation-roadmap.md
rem
rem Version scheme: <channel>.Build.v<MAJOR>.<MINOR>.<PATCH>
rem   e.g. Beta.Build.v1.0.0   Beta.Build.v1.0.1   Release.Build.v2.0.0
rem
rem Usage:
rem   n1ssan-release.bat                 prompt for the version
rem   n1ssan-release.bat Beta.Build.v1.0.1   unattended (CI)
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "CURRENT_VERSION=0.0.0"
if exist "%CD%\VERSION" for /f "usebackq delims=" %%v in ("%CD%\VERSION") do set "CURRENT_VERSION=%%v"

set "NEW_VERSION=%~1"
if not defined NEW_VERSION (
    echo ============================================
    echo  n1ssan release
    echo ============================================
    echo  Current version : %CURRENT_VERSION%
    echo  Format          : Beta.Build.v1.0.1
    echo.
    set /p "NEW_VERSION=New build version: "
)

if not defined NEW_VERSION (
    echo No version entered - release cancelled.
    exit /b 2
)

powershell -NoProfile -Command "if ('%NEW_VERSION%' -notmatch '^(Alpha|Beta|Release)\.Build\.v\d+\.\d+\.\d+$') { exit 1 }"
if errorlevel 1 (
    echo.
    echo Invalid version "%NEW_VERSION%".
    echo Use: Beta.Build.v1.0.0  ^(channel Alpha/Beta/Release, then Build.vMAJOR.MINOR.PATCH^)
    exit /b 2
)

if /i "%NEW_VERSION%"=="%CURRENT_VERSION%" (
    echo.
    echo Version %NEW_VERSION% is already the current version. Bump it for a new release.
    exit /b 2
)

> "%CD%\VERSION" echo %NEW_VERSION%
for /f "usebackq delims=" %%v in ("%CD%\VERSION") do set "WRITTEN_VERSION=%%v"
if /i not "%WRITTEN_VERSION%"=="%NEW_VERSION%" (
    echo Could not write VERSION.
    exit /b 1
)

echo.
echo Recycling %CURRENT_VERSION% -^> %NEW_VERSION%
echo.

call "%CD%\n1ssan-build.bat"
if errorlevel 1 (
    echo.
    echo Release build FAILED. VERSION still holds %NEW_VERSION%.
    exit /b 1
)

echo.
echo ============================================
echo  RELEASE READY
echo  Version : %NEW_VERSION%
echo  Archive : %CD%\release\n1ssan-%NEW_VERSION%-win64.zip
echo ============================================
exit /b 0

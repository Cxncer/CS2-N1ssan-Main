# n1ssan — Packaged Release Verification
#
# Checks a staged release against what the launcher and injector expect: required
# files, x64 binaries, a manifest whose version matches VERSION, and artifact
# hashes that still match the files on disk.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify-package.ps1
#   powershell ... -Release <folder> -Root <repo root>

param(
    [string]$Root = (Split-Path -Parent $PSScriptRoot),
    [string]$Release = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($Release)) {
    $Release = Join-Path $Root 'release\n1ssan'
}

$failures = 0

function Check([string]$Name, [bool]$Passed, [string]$Detail = '') {
    if ($Passed) {
        Write-Host ("[PASS] " + $Name + $(if ($Detail) { " - $Detail" } else { '' }))
    }
    else {
        Write-Host ("[FAIL] " + $Name + $(if ($Detail) { " - $Detail" } else { '' }))
        $script:failures++
    }
}

function Get-PeMachine([string]$Path) {
    # Reads the COFF header machine field: 0x8664 = x64.
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 0x40) { return $null }
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    if ($peOffset -le 0 -or $peOffset + 6 -ge $bytes.Length) { return $null }
    if ($bytes[$peOffset] -ne 0x50 -or $bytes[$peOffset + 1] -ne 0x45) { return $null }
    return [BitConverter]::ToUInt16($bytes, $peOffset + 4)
}

Write-Host "=== n1ssan packaged release verification ==="
Write-Host "Release: $Release"

Check 'staged release folder exists' (Test-Path $Release) $Release
if ($failures -gt 0) { exit 1 }

$required = @(
    'n1ssan-launcher.exe',
    'apex\n1ssan-apex.exe',
    'offsets.json',
    'dll\n1ssan-dll.dll',
    'dll\MinHook.x64.dll',
    'injector\n1ssan-injector.exe',
    'runtime\node.exe',
    'runtime\bridge\server.js',
    'runtime\package.json',
    'web\dist\index.html',
    'web\dist\obsidian.html',
    'manifest.json'
)

foreach ($relative in $required) {
    Check "present $relative" (Test-Path (Join-Path $Release $relative))
}

# The offset dataset Apex reads and rewrites must travel with the release and
# stay parseable: the runtime, Apex, and the build all answer to this file.
$datasetPath = Join-Path $Release 'offsets.json'
if (Test-Path $datasetPath) {
    try {
        $dataset = Get-Content $datasetPath -Raw | ConvertFrom-Json
        Check 'offsets.json schema version' ($dataset.schemaVersion -eq 1) "schemaVersion=$($dataset.schemaVersion)"
        Check 'offsets.json dataset name' ($dataset.dataset -eq 'n1ssan-offsets') "dataset=$($dataset.dataset)"
        Check 'offsets.json has a version' (![string]::IsNullOrWhiteSpace($dataset.version)) "version=$($dataset.version)"
        $entryCount = @($dataset.entries).Count
        Check 'offsets.json has entries' ($entryCount -gt 0) "entries=$entryCount"
        $names = @($dataset.entries | ForEach-Object { $_.name })
        Check 'offsets.json entry names are unique' (($names | Select-Object -Unique).Count -eq $names.Count) ($names -join ', ')
        $badPatterns = @($dataset.entries | Where-Object { $_.pattern -notmatch '^[0-9A-F?\s]+$' } | ForEach-Object { $_.name })
        Check 'offsets.json patterns are hexadecimal' ($badPatterns.Count -eq 0) ($badPatterns -join ', ')
    }
    catch {
        Check 'offsets.json parses' $false $_.Exception.Message
    }
}

# The DLL folder holds exactly one runtime. A second copy hooking
# FrameStageNotify crashes the client, so any extra n1ssan*.dll is a failure.
$extraDlls = @(Get-ChildItem (Join-Path $Release 'dll') -Filter 'n1ssan*.dll' -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ne 'n1ssan-dll.dll' })
Check 'dll folder holds exactly one n1ssan runtime' ($extraDlls.Count -eq 0) (($extraDlls | ForEach-Object { $_.Name }) -join ', ')

foreach ($relative in @('n1ssan-launcher.exe', 'apex\n1ssan-apex.exe', 'injector\n1ssan-injector.exe', 'dll\n1ssan-dll.dll', 'runtime\node.exe')) {
    $path = Join-Path $Release $relative
    if (!(Test-Path $path)) { continue }
    $machine = Get-PeMachine $path
    Check "x64 binary $relative" ($machine -eq 0x8664) ("machine=0x{0:X4}" -f [int]$machine)
}

$versionFile = Join-Path $Root 'VERSION'
Check 'VERSION file exists' (Test-Path $versionFile)
$expectedVersion = if (Test-Path $versionFile) { (Get-Content $versionFile -Raw).Trim() } else { '' }

$manifestPath = Join-Path $Release 'manifest.json'
Check 'manifest exists' (Test-Path $manifestPath)
if (Test-Path $manifestPath) {
    $manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json
    Check 'manifest version matches VERSION' ($manifest.version -eq $expectedVersion) ("manifest=$($manifest.version) VERSION=$expectedVersion")
    Check 'manifest records every component commit' (
        $manifest.components -and $manifest.components.dll -and $manifest.components.injector -and
        $manifest.components.launcher -and $manifest.components.bridge) ($manifest.components | ConvertTo-Json -Compress)
    Check 'manifest records contract versions' (
        $manifest.contracts.statusProtocol -eq 2 -and
        $manifest.contracts.armorySchema -eq 1 -and
        $manifest.contracts.runtimeDocument -eq 2 -and
        $manifest.contracts.integrationSnapshot -eq 3)

    $mismatched = @()
    foreach ($artifact in $manifest.artifacts) {
        $path = Join-Path $Release ($artifact.path -replace '/', '\')
        if (!(Test-Path $path)) { $mismatched += "$($artifact.path) (missing)"; continue }
        $current = (Get-FileHash $path -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($current -ne $artifact.sha256) { $mismatched += "$($artifact.path) (hash changed)" }
    }
    Check 'every recorded artifact hash matches the staged file' ($mismatched.Count -eq 0) ($mismatched -join ', ')
}

Write-Host ''
if ($failures -gt 0) {
    Write-Host "PACKAGE VERIFY FAILED: $failures check(s) failed."
    exit 1
}

Write-Host "PACKAGE VERIFY OK - $expectedVersion staged layout matches the release contract."
exit 0

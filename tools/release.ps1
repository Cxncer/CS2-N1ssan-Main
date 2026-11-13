# n1ssan — Release Manifest Writer
#
# Called by n1ssan-build.bat after staging release\n1ssan. It records exactly what
# went into a build (main repository commit, every component commit, artifact
# hashes) and writes the versioned archive next to the staged folder.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\release.ps1 `
#       -Root <repo root> -Release <staged folder> -Version <x.y.z>

param(
    [Parameter(Mandatory = $true)][string]$Root,
    [Parameter(Mandatory = $true)][string]$Release,
    [Parameter(Mandatory = $true)][string]$Version,
    [switch]$SkipArchive
)

$ErrorActionPreference = 'Stop'

function Get-GitRevision([string]$Directory) {
    try {
        $value = & git -C $Directory rev-parse --short=12 HEAD 2>$null
        if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($value)) { return $null }
        return $value.Trim()
    }
    catch {
        return $null
    }
}

$components = [ordered]@{
    'main'         = $Root
    'dll'          = Join-Path $Root 'development\CS2-n1ssan'
    'injector'     = Join-Path $Root 'development\n1ssan-injectorv2'
    'launcher'     = Join-Path $Root 'development\n1ssan-launcher'
    'bridge'       = Join-Path $Root 'development\n1ssan-web-bridge'
    'gloves'       = Join-Path $Root 'supporting\n1ssan-gloves'
}

$revisions = [ordered]@{}
foreach ($name in $components.Keys) {
    $revisions[$name] = Get-GitRevision $components[$name]
}

$artifactNames = @(
    'n1ssan-launcher.exe',
    'n1ssan-apex.exe',
    'data\offsets.json',
    'dll\n1ssan-dll.dll',
    'dll\MinHook.x64.dll',
    'injector\n1ssan-injector.exe',
    'runtime\node.exe',
    'runtime\bridge\server.js',
    'web\dist\index.html',
    'web\dist\obsidian.html'
)

$artifacts = @()
foreach ($relative in $artifactNames) {
    $path = Join-Path $Release $relative
    if (!(Test-Path $path)) {
        Write-Warning "Manifest: missing artifact $relative"
        continue
    }

    $file = Get-Item $path
    $hash = (Get-FileHash $path -Algorithm SHA256).Hash.ToLowerInvariant()
    $artifacts += [ordered]@{
        path   = ($relative -replace '\\', '/')
        bytes  = $file.Length
        sha256 = $hash
    }
}

$manifest = [ordered]@{
    product     = 'n1ssan'
    version     = $Version
    platform    = 'win64'
    builtAtUtc  = (Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')
    builtBy     = $env:USERNAME
    components  = $revisions
    contracts   = [ordered]@{
        statusProtocol      = 2
        armorySchema        = 1
        runtimeDocument     = 2
        integrationSnapshot = 3
        loadoutProfiles     = 1
        offsetsDataset      = 1
    }
    artifacts   = $artifacts
}

$manifestPath = Join-Path $Release 'data\manifest.json'
if (!(Test-Path (Split-Path -Parent $manifestPath))) {
    New-Item -ItemType Directory -Path (Split-Path -Parent $manifestPath) | Out-Null
}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -Path $manifestPath -Encoding UTF8
Write-Host "Manifest: $manifestPath"

$archiveDirectory = Join-Path $Root 'release'
if (!(Test-Path $archiveDirectory)) { New-Item -ItemType Directory -Path $archiveDirectory | Out-Null }

$archive = Join-Path $archiveDirectory "n1ssan-$Version-win64.zip"

# ---------------------------------------------------------------- release note
# One line per version in operation-roadmap.md, so the built artifact and the
# roadmap can never disagree about what shipped. Re-releasing a version replaces
# its line instead of appending a duplicate.
$roadmap = Join-Path $Root 'operation-roadmap.md'
if (Test-Path $roadmap) {
    $entry = "- **$Version** - $($manifest.builtAtUtc.Substring(0, 10)) | archive ``$(Split-Path -Leaf $archive)`` | main ``$($revisions['main'])`` dll ``$($revisions['dll'])`` injector ``$($revisions['injector'])`` launcher ``$($revisions['launcher'])`` bridge ``$($revisions['bridge'])``"
    $lines = New-Object System.Collections.Generic.List[string]
    foreach ($line in (Get-Content $roadmap)) { $lines.Add($line) }

    $sectionIndex = $lines.IndexOf('## Release History')

    if ($sectionIndex -lt 0) {
        $lines.Add('')
        $lines.Add('## Release History')
        $lines.Add('')
        $lines.Add($entry)
    }
    else {
        # Replace a previous line for the same version instead of duplicating it.
        for ($i = $lines.Count - 1; $i -gt $sectionIndex; $i--) {
            if ($lines[$i].StartsWith("- **$Version**")) { $lines.RemoveAt($i) }
        }

        $insertAt = $sectionIndex + 1
        if ($insertAt -ge $lines.Count) { $lines.Add('') }
        if ($lines[$insertAt].Trim() -ne '') { $lines.Insert($insertAt, '') }
        $lines.Insert($insertAt + 1, $entry)
    }

    Set-Content -Path $roadmap -Value $lines -Encoding UTF8
    Write-Host "Roadmap:  $roadmap (release history updated)"
}

if ($SkipArchive) { return }

if (Test-Path $archive) { Remove-Item $archive -Force }

# Compress the folder itself so the archive unpacks into a single `n1ssan\`
# directory instead of scattering the layout into the extraction target.
Compress-Archive -Path $Release -DestinationPath $archive -CompressionLevel Optimal
$size = [math]::Round((Get-Item $archive).Length / 1MB, 1)
Write-Host "Archive:  $archive ($size MB)"

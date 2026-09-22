[CmdletBinding()]
param([string]$EngineRoot = $env:UE_ROOT)
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Manifest = Get-Content "$Root/ContentManifest.json" -Raw | ConvertFrom-Json
if ($Manifest.status -ne "integration-ready") { throw "Production content is not integrated. This source release intentionally cannot be packaged as a pretend game. Follow docs/CONTENT.md." }
$Required = @("world-map", "player-character", "player-animation", "vehicle-mesh", "vehicle-physics", "npc-character", "ambient-audio")
foreach ($Role in $Required) {
    $Entries = @($Manifest.assets | Where-Object { $_.role -eq $Role })
    if ($Entries.Count -lt 1) { throw "Missing asset role: $Role" }
}
foreach ($Asset in $Manifest.assets) {
    if (-not $Asset.path.StartsWith("Content/")) { throw "Asset must be under Content: $($Asset.path)" }
    $Path = [IO.Path]::GetFullPath((Join-Path $Root $Asset.path))
    $ContentRoot = [IO.Path]::GetFullPath((Join-Path $Root "Content")) + [IO.Path]::DirectorySeparatorChar
    if (-not $Path.StartsWith($ContentRoot, [StringComparison]::OrdinalIgnoreCase)) { throw "Invalid asset path." }
    if (-not (Test-Path $Path)) { throw "Missing asset: $Path" }
    if (-not $Asset.license -or -not $Asset.origin) { throw "Asset provenance/license missing: $Path" }
    $Header = [IO.File]::ReadAllBytes($Path)
    if ($Header.Length -lt 64 -or ([Text.Encoding]::ASCII.GetString($Header,0,[Math]::Min(80,$Header.Length))).StartsWith("version https://git-lfs")) { throw "Asset is empty or still an LFS pointer: $Path" }
}
if (-not (Test-Path "$Root/Content/World/Maps/OpenWorld.umap")) { throw "Required authored World Partition map is missing." }
& "$EngineRoot/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$Root/OpenWorld.uproject" -run=DataValidation -unattended -nop4 -NullRHI -stdout -FullStdOutLogOutput
if ($LASTEXITCODE -ne 0) { throw "UE DataValidation failed ($LASTEXITCODE)." }

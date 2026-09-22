[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$Reports = Join-Path $Root 'Artifacts/HostedProbe'
New-Item -ItemType Directory -Force -Path $Reports | Out-Null

$Candidates = [Collections.Generic.List[string]]::new()
if ($env:UE_ROOT) { $Candidates.Add($env:UE_ROOT) }
foreach ($Base in @("$env:ProgramFiles/Epic Games", 'C:/Unreal', 'D:/Unreal', 'D:/Epic Games')) {
    if (Test-Path $Base) {
        foreach ($Dir in (Get-ChildItem $Base -Directory -ErrorAction SilentlyContinue)) {
            $Candidates.Add($Dir.FullName)
        }
    }
}
foreach ($Hive in @('HKLM:/SOFTWARE/EpicGames/Unreal Engine', 'HKCU:/SOFTWARE/Epic Games/Unreal Engine/Builds')) {
    if (Test-Path $Hive) {
        if ($Hive.EndsWith('/Builds')) {
            $Props = Get-ItemProperty $Hive
            foreach ($P in $Props.PSObject.Properties) {
                if (-not $P.Name.StartsWith('PS') -and $P.Value -is [string]) { $Candidates.Add($P.Value) }
            }
        } else {
            foreach ($Key in (Get-ChildItem $Hive)) {
                $Props = Get-ItemProperty $Key.PSPath
                if ($Props.InstalledDirectory) { $Candidates.Add($Props.InstalledDirectory) }
            }
        }
    }
}
$EngineRoot = ''
foreach ($Candidate in ($Candidates | Select-Object -Unique)) {
    $VersionFile = Join-Path $Candidate 'Engine/Build/Build.version'
    $BuildFile = Join-Path $Candidate 'Engine/Build/BatchFiles/Build.bat'
    if ((Test-Path $VersionFile) -and (Test-Path $BuildFile)) {
        $Version = Get-Content $VersionFile -Raw | ConvertFrom-Json
        if ($Version.MajorVersion -eq 5 -and $Version.MinorVersion -eq 6) {
            $EngineRoot = $Candidate
            break
        }
    }
}
$VsWhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$VisualStudio = if (Test-Path $VsWhere) {
    & $VsWhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
} else { '' }
$Memory = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory
$Drives = @(Get-PSDrive -PSProvider FileSystem | Where-Object { $_.Name -in @('C', 'D') } | ForEach-Object {
    @{ Drive = $_.Name; FreeGB = [Math]::Round($_.Free / 1GB, 1); UsedGB = [Math]::Round($_.Used / 1GB, 1) }
})
$Report = [ordered]@{
    Scope = 'Hosted Windows environment probe; not a gameplay or packaging test'
    TimestampUTC = [DateTime]::UtcNow.ToString('o')
    RunnerOS = $env:RUNNER_OS
    RunnerArchitecture = $env:RUNNER_ARCH
    ImageVersion = $env:ImageVersion
    CPUCount = [Environment]::ProcessorCount
    MemoryGB = [Math]::Round($Memory / 1GB, 1)
    Drives = $Drives
    VisualStudioCppInstallation = "$VisualStudio"
    UE56Found = [bool]$EngineRoot
    UE56Root = $EngineRoot
    EngineLocationsChecked = @($Candidates | Select-Object -Unique)
}
$Json = $Report | ConvertTo-Json -Depth 5
$Json | Set-Content (Join-Path $Reports 'environment.json') -Encoding utf8
Write-Host $Json
if ($env:GITHUB_OUTPUT) { "engine_root=$EngineRoot" | Out-File $env:GITHUB_OUTPUT -Append -Encoding utf8 }
if ($env:GITHUB_STEP_SUMMARY) {
    @"
## Hosted Windows / Unreal build attempt

- UE 5.6 found in standard install locations/registry: **$([bool]$EngineRoot)**
- Visual Studio C++ installation: $VisualStudio
- RAM: $($Report.MemoryGB) GB; CPUs: $($Report.CPUCount)
- Next step invokes the real project Editor build script. If UE is absent, it fails before UHT/C++ compilation.
- No Epic credentials are used, and no engine binaries are fetched from unofficial sources.
- Even a successful Editor build would not mean the missing game content or Windows package exists.
"@ | Out-File $env:GITHUB_STEP_SUMMARY -Append -Encoding utf8
}

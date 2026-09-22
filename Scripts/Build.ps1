[CmdletBinding()]
param(
    [string]$EngineRoot = $env:UE_ROOT,
    [ValidateSet("Development", "Shipping")][string]$Configuration = "Development",
    [ValidateSet("Editor", "Package")][string]$Mode = "Editor"
)
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Project = Join-Path $Root "OpenWorld.uproject"
if (-not $EngineRoot -or -not (Test-Path "$EngineRoot/Engine/Build/BatchFiles/Build.bat")) {
    throw "Set UE_ROOT to the UE 5.6 installation directory (not its Engine subdirectory)."
}
$Version = Get-Content "$EngineRoot/Engine/Build/Build.version" -Raw | ConvertFrom-Json
if ($Version.MajorVersion -ne 5 -or $Version.MinorVersion -ne 6) { throw "This source targets UE 5.6. Other engine versions are not validated." }
& "$EngineRoot/Engine/Build/BatchFiles/Build.bat" OpenWorldEditor Win64 Development "-Project=$Project" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw "Unreal Editor C++/UHT build failed ($LASTEXITCODE)." }
if ($Mode -eq "Editor") { return }
& "$PSScriptRoot/ValidateContent.ps1" -EngineRoot $EngineRoot
if ($LASTEXITCODE -ne 0) { throw "Content validation failed." }
$Archive = Join-Path $Root "Artifacts/$Configuration"
& "$EngineRoot/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun "-project=$Project" -nop4 -unattended -utf8output -platform=Win64 "-clientconfig=$Configuration" -build -cook -stage -pak -iostore -prereqs -archive "-archivedirectory=$Archive" -map=/Game/World/Maps/OpenWorld
if ($LASTEXITCODE -ne 0) { throw "BuildCookRun failed ($LASTEXITCODE). No playable release should be published." }
$Package = Join-Path $Archive "Windows"
if (-not (Test-Path "$Package/OpenWorld.exe")) { throw "Expected native bootstrap executable not found in $Package." }
$Zip = Join-Path $Root "Artifacts/OpenWorld-Win64-$Configuration.zip"
if (Test-Path $Zip) { Remove-Item $Zip }
Compress-Archive -Path "$Package/*" -DestinationPath $Zip -CompressionLevel Optimal
$Hash = (Get-FileHash $Zip -Algorithm SHA256).Hash.ToLowerInvariant()
"$Hash  $(Split-Path $Zip -Leaf)" | Set-Content "$Zip.sha256" -Encoding ascii
Write-Host "Native package created: $Zip. Manual gameplay and clean-machine smoke tests are still required."

[CmdletBinding()]
param([string]$EngineRoot = $env:UE_ROOT)
$ErrorActionPreference = "Stop"
$Root = Split-Path $PSScriptRoot -Parent
$Editor = "$EngineRoot/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
if (-not (Test-Path $Editor)) { throw "UE_ROOT / UnrealEditor-Cmd.exe is missing." }
$Report = Join-Path $Root "Artifacts/Tests"
if (Test-Path $Report) { Remove-Item $Report -Recurse -Force }
& $Editor "$Root/OpenWorld.uproject" -unattended -nop4 -NullRHI -nosound '-ExecCmds=Automation RunTests OpenWorld' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$Report" -stdout -FullStdOutLogOutput
if ($LASTEXITCODE -ne 0) { throw "Unreal automation process failed ($LASTEXITCODE)." }
$ReportFile = Join-Path $Report "index.json"
if (-not (Test-Path $ReportFile)) { throw "No Unreal test report was produced; tests are not considered passed." }
$Data = Get-Content $ReportFile -Raw | ConvertFrom-Json
if ($Data.failed -gt 0 -or $Data.succeeded -lt 4 -or $Data.notRun -gt 0) { throw "Unreal tests failed, were skipped, or the test suite was incomplete." }
Write-Host "Unreal automation report verified: $($Data.succeeded) passed."

# Intentionally does not download executables or register a runner with a secret.
# Use GitHub Settings > Actions > Runners > New self-hosted runner for registration.
$ErrorActionPreference = "Stop"
if (-not $env:UE_ROOT) { throw "Set machine-level UE_ROOT to your UE 5.6 installation and restart the runner service." }
if (-not (Test-Path "$env:UE_ROOT/Engine/Binaries/Win64/UnrealEditor-Cmd.exe")) { throw "UE 5.6 is not installed at UE_ROOT." }
& git lfs version
if ($LASTEXITCODE -ne 0) { throw "Install Git LFS on the runner." }
Write-Host "Register a PRIVATE, trusted Windows x64 runner with labels: self-hosted, Windows, X64, ue-5.6."
Write-Host "Create and protect the windows-release GitHub environment. Never execute untrusted PR code on this runner."

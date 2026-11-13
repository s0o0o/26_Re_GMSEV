# Executes real server/client code and writes approved test accounts. No UI input injection.
param([switch]$ConfirmAccountWrites)
$ErrorActionPreference='Stop'
if(!$ConfirmAccountWrites){throw 'Requires explicit confirmation of approved test account writes.'}
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue){throw 'Existing listener; no process will be stopped.'}
& (Join-Path $PSScriptRoot 'check-load-range.ps1') -ExpectedRows 300
& (Join-Path $PSScriptRoot 'check-load-range.ps1') -ExpectedRows 300 -TestAccountStart 900001000
$server=$null; $scene=$null
try {
    $server=Start-Process -FilePath (Join-Path $root 'TP_2023192003/x64/Release/TP_2023192003.exe') -WorkingDirectory (Join-Path $root 'TP_2023192003/TP_2023192003') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $root 'build/tests/live-scene-server.log') -RedirectStandardError (Join-Path $root 'build/tests/live-scene-server-error.log')
    $until=[DateTime]::UtcNow.AddSeconds(30)
    do { if($server.HasExited){throw 'Server exited'}; $ready=Select-String -LiteralPath (Join-Path $root 'build/tests/live-scene-server.log') -SimpleMatch '[READY]' -Quiet; if(!$ready){Start-Sleep -Milliseconds 100} } until($ready -or [DateTime]::UtcNow -gt $until)
    if(!$ready){throw 'READY timeout'}
    $scene=Start-Process -FilePath (Join-Path $root 'build/tests/live_client_scene.exe') -WorkingDirectory $root -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $root 'build/tests/live-scene-client.log') -RedirectStandardError (Join-Path $root 'build/tests/live-scene-client-error.log')
    if(!$scene.WaitForExit(60000)){throw 'Capture scene deadline exceeded'}
    $scene.Refresh(); Get-Content -LiteralPath (Join-Path $root 'build/tests/live-scene-client.log') -Tail 9
    if($scene.ExitCode -ne 0){throw 'Scene failed; inspect logs'}
    Start-Sleep -Seconds 3
} finally {
    if($scene -and !$scene.HasExited){Stop-Process -Id $scene.Id}
    if($server -and !$server.HasExited){Stop-Process -Id $server.Id}
}

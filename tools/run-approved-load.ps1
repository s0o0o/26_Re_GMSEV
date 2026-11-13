param([switch]$ConfirmAccountWrites,[switch]$ReuseApprovedTestAccounts)
$ErrorActionPreference='Stop'
if(!$ConfirmAccountWrites) { throw 'Requires -ConfirmAccountWrites: creates/updates IDs 900000000..900000299. No automatic deletion.' }
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue) { throw 'Port 3000 already has a listener; no existing process will be stopped.' }
$serverExe=Join-Path $root 'TP_2023192003/x64/Release/TP_2023192003.exe'
$clientExe=Join-Path $root 'build/tests/bounded_load.exe'
if(!(Test-Path -LiteralPath $serverExe) -or !(Test-Path -LiteralPath $clientExe)) { throw 'Build the Release server and bounded_load first.' }
& (Join-Path $PSScriptRoot 'check-load-range.ps1') -AllowApprovedReuse:$ReuseApprovedTestAccounts
$runDirectory=Join-Path $root ('build/tests/load-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')+'-'+[guid]::NewGuid().ToString('N').Substring(0,8))
$null=New-Item -ItemType Directory -Path $runDirectory
$server=$null; $client=$null
$oldMonster=[Environment]::GetEnvironmentVariable('GMSEV_MONSTER_COUNT','Process')
try {
    $env:GMSEV_MONSTER_COUNT='3'
    $server=Start-Process -FilePath $serverExe -WorkingDirectory (Join-Path $root 'TP_2023192003/TP_2023192003') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runDirectory 'server.log') -RedirectStandardError (Join-Path $runDirectory 'server-error.log')
    $readyDeadline=[DateTime]::UtcNow.AddSeconds(30)
    do {
        if($server.HasExited) { throw 'Owned server exited before READY; inspect run logs.' }
        $ready=Select-String -LiteralPath (Join-Path $runDirectory 'server.log') -SimpleMatch '[READY] Server ready.' -Quiet
        if(!$ready) { Start-Sleep -Milliseconds 100 }
    } until($ready -or [DateTime]::UtcNow -gt $readyDeadline)
    if(!$ready) { throw 'Server READY deadline exceeded; load not started.' }
    $client=Start-Process -FilePath $clientExe -ArgumentList @('300','900000000','15','45','3000') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runDirectory 'client.log') -RedirectStandardError (Join-Path $runDirectory 'client-error.log')
    if(!$client.WaitForExit(70000)) { throw 'Load client exceeded the outer deadline.' }
    $client.Refresh()
    Get-Content -LiteralPath (Join-Path $runDirectory 'client.log')
    if($client.ExitCode -ne 0) { throw 'Load failed; inspect run logs. Test accounts may already exist.' }
    Start-Sleep -Seconds 3
    if($server.HasExited) { throw 'Server exited after load.' }
    & (Join-Path $PSScriptRoot 'check-load-range.ps1') -ExpectedRows 300
    Write-Output 'Server remained alive after disconnect wait. This is not a graceful server drain assertion.'
} finally {
    if($client -and !$client.HasExited) { Stop-Process -Id $client.Id }
    if($server -and !$server.HasExited) { Stop-Process -Id $server.Id }
    [Environment]::SetEnvironmentVariable('GMSEV_MONSTER_COUNT',$oldMonster,'Process')
    Write-Output "Run logs: $runDirectory"
    Write-Output 'Only owned processes were stopped. Test accounts were NOT deleted. Server has no graceful shutdown command.'
}

param([switch]$ConfirmAccountWrites,[switch]$ReuseApprovedTestAccounts,
    [ValidateSet('mixed','separate')][string]$LoadMode='separate',
    [ValidateSet('drain','immediate')][string]$CloseMode='drain',[switch]$Trace,
    [ValidateSet(900000000,900001000)][int]$TestAccountStart=900000000)
$ErrorActionPreference='Stop'
if(!$ConfirmAccountWrites) { throw 'Requires -ConfirmAccountWrites: creates/updates 300 accounts from TestAccountStart. No automatic deletion.' }
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue) { throw 'Port 3000 already has a listener; no existing process will be stopped.' }
$serverExe=Join-Path $root 'TP_2023192003/x64/Release/TP_2023192003.exe'
$clientExe=Join-Path $root 'build/tests/bounded_load.exe'
if(!(Test-Path -LiteralPath $serverExe) -or !(Test-Path -LiteralPath $clientExe)) { throw 'Build the Release server and bounded_load first.' }
& (Join-Path $PSScriptRoot 'check-load-range.ps1') -AllowApprovedReuse:$ReuseApprovedTestAccounts -TestAccountStart $TestAccountStart
$runDirectory=Join-Path $root ('build/tests/load-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss')+'-'+[guid]::NewGuid().ToString('N').Substring(0,8))
$null=New-Item -ItemType Directory -Path $runDirectory
$server=$null; $client=$null
$oldMonster=[Environment]::GetEnvironmentVariable('GMSEV_MONSTER_COUNT','Process')
$oldTrace=[Environment]::GetEnvironmentVariable('GMSEV_TRACE_FILE','Process')
try {
    $env:GMSEV_MONSTER_COUNT='3'
    [Environment]::SetEnvironmentVariable('GMSEV_TRACE_FILE',$(if($Trace){Join-Path $runDirectory 'server-trace.log'}else{$null}),'Process')
    $server=Start-Process -FilePath $serverExe -WorkingDirectory (Join-Path $root 'TP_2023192003/TP_2023192003') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runDirectory 'server.log') -RedirectStandardError (Join-Path $runDirectory 'server-error.log')
    $readyDeadline=[DateTime]::UtcNow.AddSeconds(30)
    do {
        if($server.HasExited) { throw 'Owned server exited before READY; inspect run logs.' }
        $ready=Select-String -LiteralPath (Join-Path $runDirectory 'server.log') -SimpleMatch '[READY] Server ready.' -Quiet
        if(!$ready) { Start-Sleep -Milliseconds 100 }
    } until($ready -or [DateTime]::UtcNow -gt $readyDeadline)
    if(!$ready) { throw 'Server READY deadline exceeded; load not started.' }
    $client=Start-Process -FilePath $clientExe -ArgumentList @('300',"$TestAccountStart",'15','45','3000',$LoadMode,$CloseMode) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $runDirectory 'client.log') -RedirectStandardError (Join-Path $runDirectory 'client-error.log')
    if(!$client.WaitForExit(70000)) { throw 'Load client exceeded the outer deadline.' }
    $client.Refresh()
    Get-Content -LiteralPath (Join-Path $runDirectory 'client.log')
    Start-Sleep -Seconds 3
    if($server.HasExited) { throw 'Server exited after load.' }
    & (Join-Path $PSScriptRoot 'check-load-range.ps1') -ExpectedRows 300 -TestAccountStart $TestAccountStart
    Write-Output 'Server remained alive after disconnect wait. This is not a graceful server drain assertion.'
    if($client.ExitCode -ne 0) { throw 'Load failed; inspect run logs. Test accounts may already exist.' }
} finally {
    if($client -and !$client.HasExited) { Stop-Process -Id $client.Id }
    if($server -and !$server.HasExited) { Stop-Process -Id $server.Id }
    [Environment]::SetEnvironmentVariable('GMSEV_MONSTER_COUNT',$oldMonster,'Process')
    [Environment]::SetEnvironmentVariable('GMSEV_TRACE_FILE',$oldTrace,'Process')
    Write-Output "Run logs: $runDirectory"
    Write-Output 'Only owned processes were stopped. Test accounts were NOT deleted. Server has no graceful shutdown command.'
}

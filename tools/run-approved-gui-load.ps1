param([switch]$ConfirmAccountWrites)
$ErrorActionPreference='Stop'
if(!$ConfirmAccountWrites) { throw 'Explicit approved existing test account writes required.' }
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue) { throw 'Existing listener; no unrelated process will be stopped.' }
& "$PSScriptRoot/check-load-range.ps1" -ExpectedRows 300 -TestAccountStart 900001000
& "$PSScriptRoot/check-load-range.ps1" -ExpectedRows 300 -TestAccountStart 900000000
$directory=Join-Path $root ('build/tests/gui-load-'+[DateTime]::Now.ToString('yyyyMMdd-HHmmss'))
$null=New-Item -ItemType Directory -Path $directory
$saved=@{}; foreach($name in @('GMSEV_GUI_CONFIRM','GMSEV_GUI_ACCOUNT_START','GMSEV_LOAD_ATTEMPTS','GMSEV_MONSTER_COUNT','GMSEV_TRACE_FILE')) { $saved[$name]=[Environment]::GetEnvironmentVariable($name,'Process') }
$results=@()
try {
    foreach($count in @(30,100,300)) {
        $phase=Join-Path $directory "$count"; $null=New-Item -ItemType Directory -Path $phase
        $server=$null;$gui=$null
        try {
            $env:GMSEV_GUI_CONFIRM='1';$env:GMSEV_GUI_ACCOUNT_START='900001000';$env:GMSEV_LOAD_ATTEMPTS="$count";$env:GMSEV_MONSTER_COUNT='3';$env:GMSEV_TRACE_FILE=Join-Path $phase 'server-trace.log'
            $server=Start-Process (Join-Path $root 'TP_2023192003/x64/Release/TP_2023192003.exe') -WorkingDirectory (Join-Path $root 'TP_2023192003/TP_2023192003') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $phase 'server.log') -RedirectStandardError (Join-Path $phase 'server-error.log')
            $until=[DateTime]::UtcNow.AddSeconds(30);$ready=$false
            do { if($server.HasExited) { throw 'Server exited before READY' }; $ready=Select-String -LiteralPath (Join-Path $phase 'server.log') -SimpleMatch '[READY]' -Quiet; if(!$ready) { Start-Sleep -Milliseconds 100 } } until($ready -or [DateTime]::UtcNow -gt $until)
            if(!$ready) { throw 'Server READY timeout' }
            $gui=Start-Process (Join-Path $root 'stress_test/x64/Release/STRESS_TEST.exe') -WorkingDirectory $root -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $phase 'gui.log') -RedirectStandardError (Join-Path $phase 'gui-error.log')
            if(!$gui.WaitForExit(90000)) { throw 'GUI deadline exceeded (failure)' };$gui.Refresh()
            $lines=Get-Content (Join-Path $phase 'gui.log');$result=$lines | Where-Object { $_ -like 'GUI_RESULT *' } | Select-Object -Last 1
            $sceneDrawn=$lines -contains '[GUI] SCENE_DRAWN'; $frameRendered=$lines -contains '[GUI] FRAME_RENDERED'
            $results+=@{clients=$count;exit_code=$gui.ExitCode;result=$result;window_created=($lines -contains '[GUI] WINDOW_CREATED OpenGL=1');frame_rendered=$frameRendered;scene_drawn=$sceneDrawn;server_alive=(!$server.HasExited)}
            $results | ConvertTo-Json -Depth 5 | ForEach-Object { [IO.File]::WriteAllText((Join-Path $directory 'results.json'),$_,[Text.UTF8Encoding]::new($false)) }
            Write-Output "GUI phase $count exit=$($gui.ExitCode) $result"
            if($gui.ExitCode -ne 0 -or !$result -or $result -notmatch 'passed=1 ' -or !$sceneDrawn -or !$frameRendered -or $server.HasExited) { throw 'GUI phase failed; logs preserved, no further phase started' }
            Start-Sleep -Seconds 3
        } finally {
            if($gui -and !$gui.HasExited) { Stop-Process -Id $gui.Id }
            if($server -and !$server.HasExited) { Stop-Process -Id $server.Id }
        }
    }
    & "$PSScriptRoot/check-load-range.ps1" -ExpectedRows 300 -TestAccountStart 900001000
    & "$PSScriptRoot/check-load-range.ps1" -ExpectedRows 300 -TestAccountStart 900000000
} finally { foreach($name in $saved.Keys) { [Environment]::SetEnvironmentVariable($name,$saved[$name],'Process') }; Write-Output "GUI logs: $directory" }

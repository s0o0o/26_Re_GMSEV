$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$serverExe=Join-Path $root 'TP_2023192003/x64/Release/TP_2023192003.exe'
$clientExe=Join-Path $root 'build/tests/native_wardrobe_db.exe'
if(Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue) { throw 'Port 3000 already in use; no existing process was stopped' }
$tag=Get-Date -Format 'yyyyMMdd-HHmmss'
$results=Join-Path $root "build/tests/live-wardrobe-$tag"
New-Item -ItemType Directory -Path $results -ErrorAction Stop | Out-Null
$oldTrace=$env:GMSEV_TRACE_FILE
try {
    foreach($mode in @('SaveA','RestoreA')) {
        $trace=Join-Path $results "$mode-trace.log"
        $env:GMSEV_TRACE_FILE=$trace
        $server=$null; $client=$null
        try {
            $server=Start-Process -FilePath $serverExe -WorkingDirectory (Join-Path $root 'TP_2023192003/TP_2023192003') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $results "$mode-server.log") -RedirectStandardError (Join-Path $results "$mode-server-error.log")
            $timer=[Diagnostics.Stopwatch]::StartNew(); $ready=$false
            while($timer.ElapsedMilliseconds -lt 10000) {
                if($server.HasExited) { throw 'Owned server exited during startup' }
                $listener=Get-NetTCPConnection -LocalPort 3000 -State Listen -ErrorAction SilentlyContinue
                if($listener) {
                    if($listener.OwningProcess -ne $server.Id) { throw 'Unexpected listener owner; refusing connection' }
                    $ready=$true; break
                }
                Start-Sleep -Milliseconds 100
            }
            if(!$ready) { throw 'Owned server startup deadline exceeded' }
            $client=Start-Process -FilePath $clientExe -ArgumentList $mode -WorkingDirectory $root -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $results "$mode-client.log") -RedirectStandardError (Join-Path $results "$mode-client-error.log")
            $null=$client.Handle # Retain the process handle so Windows PowerShell can read ExitCode reliably.
            if(!$client.WaitForExit(25000)) { throw 'Owned client deadline exceeded' }
            $client.Refresh()
            if($null -eq $client.ExitCode -or $client.ExitCode -ne 0) { throw "Native test failed ($mode); inspect scoped logs" }
            $timer.Restart(); $saved=$false
            while($timer.ElapsedMilliseconds -lt 10000) {
                if((Test-Path -LiteralPath $trace) -and ((Get-Content -LiteralPath $trace -Raw) -match 'event=disconnect_saved[^\r\n]*value=1')) { $saved=$true; break }
                Start-Sleep -Milliseconds 100
            }
            if(!$saved) { throw 'Disconnect DB-save evidence missing; not reporting success' }
            "$mode PASS; owned server/client and approved existing account only"
        } finally {
            if($client -and !$client.HasExited) { $client.Kill(); $client.WaitForExit(5000) | Out-Null }
            if($server -and !$server.HasExited) { $server.Kill(); $server.WaitForExit(5000) | Out-Null }
        }
    }
    "Actual DB save + fresh server restart/login restore PASS; logs=$results"
    'The scoped server has no graceful shutdown command; only the task-owned server was terminated after committed appearance and disconnect-save evidence. Not a server drain test.'
} finally {
    if($null -eq $oldTrace) { Remove-Item Env:GMSEV_TRACE_FILE -ErrorAction SilentlyContinue } else { $env:GMSEV_TRACE_FILE=$oldTrace }
}

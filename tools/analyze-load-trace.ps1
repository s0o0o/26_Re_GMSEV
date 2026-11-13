param([Parameter(Mandatory=$true)][string]$RunDirectory)
$ErrorActionPreference='Stop'
$events=@(foreach($line in Get-Content -LiteralPath (Join-Path $RunDirectory 'server-trace.log')) {
    if($line -notmatch '^ms=(\d+) event=([a-z_]+) session=(-?\d+) connection=(\d+) value=(\d+)$') { throw 'Malformed trace line' }
    [pscustomobject]@{ms=[long]$Matches[1];event=$Matches[2];session=[int]$Matches[3];connection=[long]$Matches[4];value=[long]$Matches[5]}
})
$client=Get-Content -LiteralPath (Join-Path $RunDirectory 'client.log')
$line=$client | Where-Object { $_.StartsWith('RESULT ') } | Select-Object -Last 1
if(!$line) { throw 'No client result' }
$phase=$client | Where-Object { $_ -match '^PHASE ms=(\d+) event=shutdown_begin ' } | Select-Object -Last 1
if(!$phase -or $phase -notmatch '^PHASE ms=(\d+)') { throw 'No shutdown phase timestamp' }
$shutdown=[long]$Matches[1]
$send=@($events | Where-Object event -eq 'send_error')
$stages=@{}
$loginTimes=@(foreach($connection in ($events | Group-Object connection)) {
    $received=$connection.Group | Where-Object event -eq 'login_received' | Select-Object -First 1
    $ready=$connection.Group | Where-Object event -eq 'login_ready' | Select-Object -First 1
    if($received -and $ready) { [long]($ready.ms-$received.ms) }
})
foreach($name in @('login_db_done','login_commit_wait','login_dequeued','login_session_wait','login_presence_wait','world_wait','world_hold')) {
    $rows=@($events | Where-Object event -eq $name)
    $maximum=0; if($rows.Count) { $maximum=($rows | Measure-Object value -Maximum).Maximum }
    $stages[$name]=[ordered]@{count=$rows.Count;max_ms=[long]$maximum}
}
$summary=[ordered]@{
    run=(Split-Path $RunDirectory -Leaf)
    client=($line.Substring(7) | ConvertFrom-Json)
    throughput=$( $row=$client | Where-Object { $_.StartsWith('THROUGHPUT ') } | Select-Object -Last 1; if($row){$row.Substring(11) | ConvertFrom-Json}else{$null} )
    login_trace_pairs=$loginTimes.Count
    login_received_to_ready_max_ms=$(if($loginTimes.Count){[long]($loginTimes | Measure-Object -Maximum).Maximum}else{0})
    stage_maxima=$stages
    send_error_count=$send.Count
    send_errors_before_shutdown=@($send | Where-Object ms -lt $shutdown).Count
    send_errors_at_or_after_shutdown=@($send | Where-Object ms -ge $shutdown).Count
    recv_eof=@($events | Where-Object event -eq 'recv_eof').Count
    disconnect_begin=@($events | Where-Object event -eq 'disconnect_begin').Count
    disconnect_saved_success=@($events | Where-Object { $_.event -eq 'disconnect_saved' -and $_.value -eq 1 }).Count
    disconnect_saved_failure=@($events | Where-Object { $_.event -eq 'disconnect_saved' -and $_.value -ne 1 }).Count
}
$summary | ConvertTo-Json -Depth 8

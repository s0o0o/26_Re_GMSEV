param([ValidateRange(0,300)][int]$ExpectedRows=0,[switch]$AllowApprovedReuse,
    [ValidateSet(900000000,900001000)][int]$TestAccountStart=900000000)
# Read-only aggregate for the explicitly approved test range; no row values are read.
$ErrorActionPreference='Stop'
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$source=[System.IO.File]::ReadAllText((Join-Path $root 'TP_2023192003/TP_2023192003/game_database.cpp'),[System.Text.Encoding]::GetEncoding(949))
$match=[regex]::Match($source,'SQLConnect\(hdbc,\s*\(SQLWCHAR\*\)L"([^"]+)"')
if(!$match.Success) { throw 'Project DSN not found; no database call made' }
$connection=New-Object System.Data.Odbc.OdbcConnection
$builder=New-Object System.Data.Odbc.OdbcConnectionStringBuilder
$builder['DSN']=$match.Groups[1].Value
$connection.ConnectionString=$builder.ConnectionString
try {
    $connection.Open()
    $command=$connection.CreateCommand()
    $command.CommandTimeout=5
    $command.CommandText='SELECT COUNT_BIG(*),COUNT_BIG(DISTINCT USER_ID) FROM SERVER_TABLE1 WHERE USER_ID BETWEEN ? AND ?'
    $null=$command.Parameters.Add('first',[System.Data.Odbc.OdbcType]::Int)
    $command.Parameters[0].Value=$TestAccountStart
    $null=$command.Parameters.Add('last',[System.Data.Odbc.OdbcType]::Int)
    $command.Parameters[1].Value=$TestAccountStart+299
    try {
        $reader=$command.ExecuteReader()
        try { if(!$reader.Read()) { throw 'No aggregate row' }; $count=$reader.GetInt64(0); $distinct=$reader.GetInt64(1) } finally { $reader.Dispose() }
    } finally { $command.Dispose() }
    Write-Output "Approved test range existing rows: $count"
    if($count -ne $distinct -or $count -gt 300) { throw 'Approved test range contains duplicate rows.' }
    if(!$AllowApprovedReuse -and $count -ne $ExpectedRows) { throw 'Approved test range row count differs from the expected value.' }
} catch {
    # Do not disclose DSN/driver error strings or connection settings.
    throw 'Load preflight failed or range occupied. No account data was changed; do not start load.'
} finally { $connection.Dispose() }

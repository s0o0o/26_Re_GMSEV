$ErrorActionPreference='Stop'
$root=[System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$source=[System.IO.File]::ReadAllText((Join-Path $root 'TP_2023192003/TP_2023192003/game_database.cpp'),[System.Text.Encoding]::GetEncoding(949))
# Reuse the DSN already configured in the project. Never print it or its settings.
$match=[regex]::Match($source,'SQLConnect\(hdbc,\s*\(SQLWCHAR\*\)L"([^"]+)"')
if(!$match.Success) { throw 'Project DSN not found; no database call made' }
& (Join-Path $root 'build/tests/inspect-db-contract.exe') $match.Groups[1].Value
if($LASTEXITCODE -ne 0) { throw 'Metadata inspection unavailable; database schema was not changed' }

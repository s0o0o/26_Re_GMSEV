param([ValidateSet('Inspect','Apply')][string]$Action='Inspect')
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$source=[IO.File]::ReadAllText((Join-Path $root 'TP_2023192003/TP_2023192003/game_database.cpp'),[Text.Encoding]::GetEncoding(949))
# Reuse the configured project DSN without logging its name or credentials.
$match=[regex]::Match($source,'SQLConnect\(hdbc,\s*\(SQLWCHAR\*\)L"([^"]+)"')
if(!$match.Success) { throw 'Project DSN not found; no database call made' }
& (Join-Path $root 'build/tests/appearance-db-admin.exe') $match.Groups[1].Value $Action
if($LASTEXITCODE -ne 0) { throw 'Appearance schema action failed; inspect state before retrying' }

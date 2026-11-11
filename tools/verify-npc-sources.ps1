$ErrorActionPreference='Stop'
$root=Join-Path $PSScriptRoot '../TP_2023192003_CLIENT/client_sample/assets/npcs/source'
$entries=Get-Content -LiteralPath (Join-Path $root 'SHA256SUMS')
if($entries.Count -ne 21 -or @(Get-ChildItem -LiteralPath $root -Filter '*.png').Count -ne 21) { throw 'Expected 21 NPC source PNGs' }
foreach($entry in $entries) {
    if($entry -notmatch '^([A-F0-9]{64})  ([a-z0-9-]+\.png)$') { throw "Invalid entry: $entry" }
    $expected=$Matches[1]; $file=$Matches[2]
    $stream=[System.IO.File]::OpenRead((Join-Path $root $file))
    $algorithm=[System.Security.Cryptography.SHA256]::Create()
    try { $actual=[System.BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','') }
    finally { $algorithm.Dispose(); $stream.Dispose() }
    if($actual -ne $expected) { throw "Hash mismatch: $file" }
}
Write-Output 'All 21 NPC source SHA256 hashes verified.'

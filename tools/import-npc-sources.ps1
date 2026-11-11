# Import only the user's 21 named files. Existing different files are never overwritten.
$ErrorActionPreference='Stop'
function Get-NpcSourceHash([string]$path) {
    $stream=[System.IO.File]::OpenRead($path)
    $algorithm=[System.Security.Cryptography.SHA256]::Create()
    try { return [System.BitConverter]::ToString($algorithm.ComputeHash($stream)).Replace('-','') }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}
$sources=@(
    @('arnies-brother',"Non-Playable Characters - Arnie's Brother"),
    @('blushie-ham','Non-Playable Characters - Blushie Ham'),
    @('harmony','Non-Playable Characters - Harmony'),
    @('shopkeeper','Non-Playable Characters - Shopkeeper'),
    @('shy-ham','Non-Playable Characters - Shy Ham'),
    @('snoozer','Non-Playable Characters - Snoozer'),
    @('mistress','Non-Playable Characters - Mistress'),
    @('penelope','Non-Playable Characters - Penelope'),
    @('pashmina','Non-Playable Characters - Pashmina'),
    @('panda','Non-Playable Characters - Panda'),
    @('oxnard','Non-Playable Characters - Oxnard'),
    @('cappy','Non-Playable Characters - Cappy'),
    @('bye-q-child','Non-Playable Characters - Bye-Q Child'),
    @('busy-mother','Non-Playable Characters - Busy Mother'),
    @('bulls-eye-kid',"Non-Playable Characters - Bull's-Eye Kid"),
    @('boss','Non-Playable Characters - Boss'),
    @('ham-chat-j-l','Ham-Chats - Ham-Chat J-L'),
    @('ham-chat-a-c','Ham-Chats - Ham-Chat A-C'),
    @('hungry-ham-2','Non-Playable Characters - Hungry Ham 2'),
    @('inventor','Non-Playable Characters - Inventor'),
    @('postie','Non-Playable Characters - Postie')
)
$destination=Join-Path $PSScriptRoot '../TP_2023192003_CLIENT/client_sample/assets/npcs/source'
New-Item -ItemType Directory -Path $destination -Force | Out-Null
foreach($entry in $sources) {
    $source=Join-Path 'C:/Users/yeons/Downloads' ('Game Boy Advance - Hamtaro_ Ham-Ham Heartbreak - '+$entry[1]+'.png')
    if(!(Test-Path -LiteralPath $source)) { throw "Missing attachment: $source" }
    $target=Join-Path $destination ($entry[0]+'.png')
    $hash=Get-NpcSourceHash $source
    if(Test-Path -LiteralPath $target) {
        if((Get-NpcSourceHash $target) -ne $hash) { throw "Different existing asset: $target" }
    } else { Copy-Item -LiteralPath $source -Destination $target }
    if((Get-NpcSourceHash $target) -ne $hash) { throw "Copy mismatch: $target" }
    Write-Output ($entry[0]+' '+$hash)
}

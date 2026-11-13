param([Parameter(Mandatory=$true)][string]$RunDirectory)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
$line=Get-Content -LiteralPath (Join-Path $RunDirectory 'client.log') | Where-Object { $_.StartsWith('RESULT ') } | Select-Object -Last 1
if(!$line) { throw 'No measured load result found.' }
$result=$line.Substring(7) | ConvertFrom-Json
$serverErrors=@(Get-Content -LiteralPath (Join-Path $RunDirectory 'server-error.log') | Select-String -SimpleMatch 'Send submission failed').Count
$bitmap=New-Object System.Drawing.Bitmap 1200,750
$g=[System.Drawing.Graphics]::FromImage($bitmap)
$g.SmoothingMode='AntiAlias'
$g.Clear([System.Drawing.Color]::FromArgb(28,31,38))
$title=[System.Drawing.Font]::new('Segoe UI',[single]30,[System.Drawing.FontStyle]::Bold)
$body=[System.Drawing.Font]::new('Segoe UI',[single]18)
$small=[System.Drawing.Font]::new('Segoe UI',[single]13)
$white=[System.Drawing.Brushes]::White
$muted=New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(185,195,209))
$green=New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(109,218,167))
$track=New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(55,65,78))
try {
    $g.DrawString('Hamster RPG | 300-client load test',$title,$white,45,35)
    $g.DrawString('Measured result chart - NOT a game screenshot',$body,$muted,45,96)
    $g.DrawString('Actual local server + actual DB | Dedicated test accounts',$small,$muted,45,137)
    $g.DrawString(('Client criteria passed: '+$result.passed+' | 15-second hold: '+$result.hold_completed),$body,$green,45,190)
    $labels=@('Login accepted','Peak concurrent','Still active at end','Clients with move replies')
    $values=@($result.accepted,$result.peak,$result.active_at_end,$result.moved_clients)
    for($i=0;$i -lt 4;$i++) {
        $y=255+$i*72
        $g.DrawString($labels[$i],$body,$white,45,$y)
        $g.FillRectangle($track,370,$y+4,650,30)
        $g.FillRectangle($green,370,$y+4,[int](650*$values[$i]/300),30)
        $g.DrawString(([string]$values[$i]+' / 300'),$body,$white,1040,$y)
    }
    $g.DrawString(('CLIENT counters | Rejected: '+$result.rejected+'    Transport: '+$result.transport_errors+'    Protocol: '+$result.protocol_errors+'    Timeout: '+$result.timeouts),$small,$muted,45,563)
    $g.DrawString(('Elapsed: '+$result.elapsed_ms+' ms | Short smoke load only; no long soak / field combat benchmark.'),$small,$muted,45,615)
    $g.DrawString(('Server log send failures: '+$serverErrors+' (10053) | Cleanup timing / mixed-load issue #41 remains OPEN.'),$small,$muted,45,657)
    $path=Join-Path ([System.IO.Path]::GetFullPath($RunDirectory)) 'load-result.png'
    $bitmap.Save($path,[System.Drawing.Imaging.ImageFormat]::Png)
    Write-Output $path
} finally { $g.Dispose(); $bitmap.Dispose(); $title.Dispose(); $body.Dispose(); $small.Dispose(); $muted.Dispose(); $green.Dispose(); $track.Dispose() }

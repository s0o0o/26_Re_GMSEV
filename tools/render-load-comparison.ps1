param([Parameter(Mandatory=$true)][string[]]$RunDirectories)
$ErrorActionPreference='Stop'
if($RunDirectories.Count -ne 4){throw 'Provide immediate separate, drain separate, drain mixed, and fresh-account mixed run folders.'}
$summaries=@(foreach($directory in $RunDirectories) { (& (Join-Path $PSScriptRoot 'analyze-load-trace.ps1') -RunDirectory $directory) | ConvertFrom-Json })
Add-Type -AssemblyName System.Drawing
$bitmap=[Drawing.Bitmap]::new(1280,905); $g=[Drawing.Graphics]::FromImage($bitmap)
$title=[Drawing.Font]::new('Segoe UI',[single]28,[Drawing.FontStyle]::Bold)
$font=[Drawing.Font]::new('Segoe UI',[single]19); $small=[Drawing.Font]::new('Segoe UI',[single]15)
$g.Clear([Drawing.Color]::FromArgb(28,31,38))
try {
    $g.DrawString('Hamster RPG | Measured shutdown comparison',$title,[Drawing.Brushes]::White,40,35)
    $g.DrawString('Actual DB, dedicated 300 accounts | Result chart, NOT a game screenshot',$small,[Drawing.Brushes]::LightGray,40,98)
    $names=@('Existing accounts: separate + immediate close','Existing accounts: separate + receive drain','Existing accounts: mixed + receive drain','NEW accounts: mixed + drain = FAIL (300 cleanup timeouts)')
    for($i=0;$i -lt 4;$i++) {
        $y=170+$i*145; $s=$summaries[$i]
        $g.DrawString($names[$i],$font,[Drawing.Brushes]::White,40,$y)
        $g.DrawString(('Login '+$s.client.accepted+'/300 | Active '+$s.client.active_at_end+' | EOF '+$s.client.drain_eof+' | Save OK '+$s.disconnect_saved_success),$small,[Drawing.Brushes]::LightGray,40,($y+40))
        $g.DrawString(('Send errors: '+$s.send_error_count+' | Before client shutdown: '+$s.send_errors_before_shutdown+' | At/after: '+$s.send_errors_at_or_after_shutdown),$small,[Drawing.Brushes]::Khaki,40,($y+78))
    }
    $g.DrawString('Same 45s login limit, 15s hold. No server lock removal or timeout increase.',$small,[Drawing.Brushes]::LightGray,40,780)
    $g.DrawString('Original login timeout not reproduced; fresh-account cleanup FAILED. Issue #41 stays OPEN.',$small,[Drawing.Brushes]::Khaki,40,827)
    $output=Join-Path ([IO.Path]::GetFullPath($RunDirectories[3])) 'load-comparison.png'
    $bitmap.Save($output,[Drawing.Imaging.ImageFormat]::Png); Write-Output $output
} finally { $g.Dispose(); $bitmap.Dispose(); $title.Dispose(); $font.Dispose(); $small.Dispose() }

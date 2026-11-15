param([Parameter(Mandatory=$true)][string[]]$RunDirectories)
$ErrorActionPreference='Stop'
if($RunDirectories.Count -ne 5){throw 'Provide baseline, first attempt and final trace-on/off/on runs.'}
Add-Type -AssemblyName System.Drawing
$bitmap=[Drawing.Bitmap]::new(1280,940); $g=[Drawing.Graphics]::FromImage($bitmap)
$title=[Drawing.Font]::new('Segoe UI',[single]28,[Drawing.FontStyle]::Bold)
$font=[Drawing.Font]::new('Segoe UI',[single]20); $small=[Drawing.Font]::new('Segoe UI',[single]15)
$g.Clear([Drawing.Color]::FromArgb(28,31,38))
try {
    $g.DrawString('Hamster RPG | Send completion contention fix',$title,[Drawing.Brushes]::White,40,30)
    $g.DrawString('Actual DB / 300-client mixed workload / Measured chart - NOT a game screenshot',$small,[Drawing.Brushes]::LightGray,40,95)
    $names=@('Before: normal SEND waits for session lock','First attempt: active SEND fast path only','Final #1: active + closing SEND fast path, trace ON','Final #2: same fix, trace OFF','Final #3: same fix, trace ON')
    for($i=0;$i -lt 5;$i++) {
        $directory=$RunDirectories[$i]; $lines=Get-Content (Join-Path $directory 'client.log')
        $r=(($lines | Where-Object {$_.StartsWith('RESULT ')} | Select-Object -Last 1).Substring(7)) | ConvertFrom-Json
        $t=(($lines | Where-Object {$_.StartsWith('THROUGHPUT ')} | Select-Object -Last 1).Substring(11)) | ConvertFrom-Json
        $y=155+$i*130; $brush=if($r.passed){[Drawing.Brushes]::LightGreen}else{[Drawing.Brushes]::Khaki}
        $g.DrawString($names[$i],$font,[Drawing.Brushes]::White,40,$y)
        $g.DrawString(('Passed: '+$r.passed+' | Login '+$r.accepted+'/300 | Active '+$r.active_at_end+' | EOF '+$r.drain_eof+' | Cleanup timeouts '+$r.drain_timeouts),$small,$brush,40,($y+39))
        $g.DrawString(('Queued moves '+$t.moves_queued+' | Own replies '+$t.own_move_replies+' | Unanswered at measurement end '+$t.unanswered_at_measurement_end),$small,[Drawing.Brushes]::LightGray,40,($y+75))
    }
    $g.DrawString('Unchanged limits: 45s login / 15s hold / 5s drain. Original accounts retained.',$small,[Drawing.Brushes]::LightGray,40,845)
    $g.DrawString('Short bounded regression only; no long soak, maximum capacity or historic crash proof.',$small,[Drawing.Brushes]::Khaki,40,887)
    $output=Join-Path ([IO.Path]::GetFullPath($RunDirectories[4])) 'send-fast-comparison.png'
    $bitmap.Save($output,[Drawing.Imaging.ImageFormat]::Png); Write-Output $output
} finally {$g.Dispose();$bitmap.Dispose();$title.Dispose();$font.Dispose();$small.Dispose()}

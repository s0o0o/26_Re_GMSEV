param([Parameter(Mandatory=$true)][string]$RunDirectory)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Drawing
$rows=@(Get-Content -LiteralPath (Join-Path $RunDirectory 'results.json') -Raw | ConvertFrom-Json)
$bitmap=[Drawing.Bitmap]::new(1200,700);$graphics=[Drawing.Graphics]::FromImage($bitmap)
$title=[Drawing.Font]::new('Segoe UI',[single]28,[Drawing.FontStyle]::Bold)
$body=[Drawing.Font]::new('Segoe UI',[single]18);$small=[Drawing.Font]::new('Segoe UI',[single]14)
try {
    $graphics.Clear([Drawing.Color]::FromArgb(28,31,38))
    $white=[Drawing.Brushes]::White;$green=[Drawing.Brushes]::LightGreen
    $graphics.DrawString('Original GUI IOCP tool | staged test',$title,$white,40,30)
    $graphics.DrawString('Measured report - NOT an OS or game screenshot',$body,$white,40,95)
    $graphics.DrawString('Real GUI executable / OpenGL frame logs / actual DB / 15-second hold each',$small,$white,40,140)
    $graphics.DrawString('Clients    Accepted / Moved    Errors / Pending    Exit / Drained',$body,$white,40,215)
    $y=280; $afterNotes=@();$beforeTotal=0; $completionNotes=@();$completionBefore=0
    foreach($row in $rows) {
        if($row.result -notmatch 'accepted=(\d+) moved=(\d+) closed=(\d+) errors=(\d+) pending=(\d+) drained=(\d+)') { throw 'Missing measured GUI result' }
        $label=('{0,-10} {1} / {2}                 {3} / {4}                 {5} / {6}' -f $row.clients,$Matches[1],$Matches[2],$Matches[4],$Matches[5],$row.exit_code,$Matches[6])
        $graphics.DrawString($label,$body,$green,40,$y);$y+=70
        $phase=Join-Path $RunDirectory ([string]$row.clients)
        $shutdown=Get-Content -LiteralPath (Join-Path $phase 'gui.log') | Where-Object { $_ -match '^GUI_SHUTDOWN ms=(\d+)' } | Select-Object -Last 1
        if($shutdown -notmatch 'ms=(\d+)') { throw 'Missing shutdown timestamp' };$stop=[long]$Matches[1]
        $errors=@(Get-Content -LiteralPath (Join-Path $phase 'server-trace.log') | Where-Object { $_ -match 'event=send_error ' })
        $before=@($errors | Where-Object { $_ -match '^ms=(\d+)' -and [long]$Matches[1] -lt $stop }).Count
        $beforeTotal+=$before;$afterNotes+=($errors.Count-$before)
        $completions=@(Get-Content -LiteralPath (Join-Path $phase 'server-trace.log') | Where-Object { $_ -match 'event=completion_disconnect .*value=[1-9]\d*$' })
        $prior=@($completions | Where-Object { $_ -match '^ms=(\d+)' -and [long]$Matches[1] -lt $stop }).Count
        $completionBefore+=$prior; $completionNotes+=($completions.Count-$prior)
    }
    $graphics.DrawString('One run per stage. Historical crash NOT reproduced here; initial cause remains unknown.',$small,$white,40,535)
    $graphics.DrawString('Dedicated existing accounts only. No new accounts, deletion or schema changes.',$small,$white,40,580)
    $graphics.DrawString(('GUI errors above. Server send errors AFTER stop: '+($afterNotes -join ' / ')+'; before: '+$beforeTotal),$small,$white,40,615)
    $graphics.DrawString(('Server failed completions AFTER stop: '+($completionNotes -join ' / ')+'; before: '+$completionBefore),$small,$white,40,655)
    $path=Join-Path ([IO.Path]::GetFullPath($RunDirectory)) 'gui-result.png'
    $bitmap.Save($path,[Drawing.Imaging.ImageFormat]::Png);Write-Output $path
} finally { $graphics.Dispose();$bitmap.Dispose();$title.Dispose();$body.Dispose();$small.Dispose() }

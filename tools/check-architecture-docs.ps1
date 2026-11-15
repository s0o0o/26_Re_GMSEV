# Link/encoding/entry-point checks only; not proof of semantic code/document agreement.
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$encoding=[Text.UTF8Encoding]::new($false,$true)
$files=@(Get-Item (Join-Path $root 'README.md'))+@(Get-ChildItem (Join-Path $root 'docs/architecture') -Filter '*.md')
$checked=0
foreach($file in $files) {
    $text=$encoding.GetString([IO.File]::ReadAllBytes($file.FullName))
    if(!$text.Trim()) { throw "Empty document: $($file.Name)" }
    foreach($match in [regex]::Matches($text,'\]\(([^)]+)\)')) {
        $target=$match.Groups[1].Value.Split('#')[0]
        if(!$target -or $target -match '^https?://') { continue }
        $resolved=[IO.Path]::GetFullPath((Join-Path $file.DirectoryName $target))
        if(!$resolved.StartsWith($root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Document link escaped workspace' }
        if(!(Test-Path -LiteralPath $resolved)) { throw "Missing link in $($file.Name): $target" }
        ++$checked
    }
}
$agents=[IO.File]::ReadAllText((Join-Path $root 'AGENTS.md'),$encoding)
if(!$agents.Contains('docs/architecture/README.md')) { throw 'Architecture maintenance rule missing' }
Write-Output "Architecture docs: $($files.Count) UTF-8 documents, $checked local links and AGENTS entry passed. Semantic review is separate."

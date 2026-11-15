# Non-mutating parameter validation: invalid input must fail before DB/process work.
$ErrorActionPreference='Stop'
$runner=Join-Path $PSScriptRoot '../tools/run-approved-load.ps1'
$command=Get-Command $runner
$range=$command.Parameters['HoldSeconds'].Attributes | Where-Object { $_ -is [System.Management.Automation.ValidateRangeAttribute] }
if($range.MinRange -ne 2 -or $range.MaxRange -ne 300) { throw 'HoldSeconds bounds changed' }
$tokens=$null;$errors=$null
$ast=[System.Management.Automation.Language.Parser]::ParseFile($runner,[ref]$tokens,[ref]$errors)
if($errors.Count) { throw 'Runner syntax errors' }
$parameter=$ast.ParamBlock.Parameters | Where-Object { $_.Name.VariablePath.UserPath -eq 'HoldSeconds' }
if($parameter.DefaultValue.SafeGetValue() -ne 15) { throw 'Default hold must stay 15 seconds' }
foreach($value in @(1,301)) {
    $rejected=$false
    try { & $runner -HoldSeconds $value }
    catch {
        if($_.FullyQualifiedErrorId -notlike 'ParameterArgumentValidationError*') { throw }
        $rejected=$true
    }
    if(!$rejected) { throw 'Invalid hold reached runner body' }
}
Write-Output 'Runner hold range/default/early rejection passed; no DB/process work executed.'

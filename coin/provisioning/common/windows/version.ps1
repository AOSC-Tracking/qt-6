Write-Host '******************** SW VERSIONS ********************'
type ~/versions.txt
Write-Host '*****************************************************'
Write-Host '******************** Get-PSDrive ********************'
Get-PSDrive
Write-Host '*****************************************************'
Write-Host '******************** Path Content *******************'
$env:Path -split ';'
Write-Host '*****************************************************'

$ErrorActionPreference = 'Stop'

param([string]$OfferId)
if (-not $OfferId) { throw "Passe -OfferId <id>" }

Write-Host "[OFFERS][UNIT-ENDPOINT] Accept" -ForegroundColor Cyan
$resp = Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/offers/{0}/aceitar" -f $OfferId)
if ($resp.status -ne 'confirmado') { throw "Aceite falhou" }
Write-Host "OK - Aceito" -ForegroundColor Green



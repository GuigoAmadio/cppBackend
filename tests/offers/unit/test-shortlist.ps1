$ErrorActionPreference = 'Stop'

param([string]$OfferId)
if (-not $OfferId) { throw "Passe -OfferId <id>" }

Write-Host "[OFFERS][UNIT-ENDPOINT] Shortlist" -ForegroundColor Cyan
Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/offers/{0}/shortlist" -f $OfferId) -ContentType 'application/json' -Body '{"shortlisted":true}'
Write-Host "OK - Shortlist aplicado" -ForegroundColor Green



$ErrorActionPreference = 'Stop'

param(
  [string]$OfferId = "offer-id"
)

Write-Host "[TEST] Accept Offer" -ForegroundColor Cyan

$resp = Invoke-RestMethod -Method Put -Uri ("http://localhost:8080/offers/{0}/aceitar" -f $OfferId)

if (-not $resp.status -or $resp.status -ne 'confirmado') { throw "Aceite falhou" }

if (-not $resp.valor_final) { throw "Faltando valor_final" }

Write-Host "OK - Offer aceita. Valor final: $($resp.valor_final)" -ForegroundColor Green



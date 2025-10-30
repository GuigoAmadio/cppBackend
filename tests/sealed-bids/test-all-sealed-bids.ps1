$ErrorActionPreference = 'Stop'

Write-Host "[TEST SUITE] Sealed Bids - Start" -ForegroundColor Cyan

& "$PSScriptRoot/test-1-create-leilao.ps1"

# Para continuidade ideal: capturar o ID retornado e repassar; aqui usamos IDs de placeholder
& "$PSScriptRoot/test-2-create-offers-and-shortlist.ps1" -LeilaoId "test-leilao-id"

# Aceitar uma offer (ID placeholder); em cenário real, capturar de saída do script anterior
& "$PSScriptRoot/test-3-accept-offer.ps1" -OfferId "offer-id"

& "$PSScriptRoot/test-4-scheduler-close.ps1"

Write-Host "[TEST SUITE] Sealed Bids - Done" -ForegroundColor Green



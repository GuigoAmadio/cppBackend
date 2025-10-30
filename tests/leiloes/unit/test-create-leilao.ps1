$ErrorActionPreference = 'Stop'

Write-Host "[LEILOES][UNIT-ENDPOINT] Create" -ForegroundColor Cyan

$body = @{
  restaurante_id   = "rest-0001"
  titulo           = "Garçom evento"
  categoria        = "garcom"
  valor_ideal      = 120
  valor_max_quero  = 180
  data_trabalho    = "2025-12-20T20:00:00Z"
  data_limite_offers = "2025-12-19T20:00:00Z"
} | ConvertTo-Json

$resp = Invoke-RestMethod -Method Post -Uri http://localhost:8080/leiloes -ContentType 'application/json' -Body $body
if (-not $resp.id) { throw "Create leilao failed" }
Write-Host "OK - id: $($resp.id)" -ForegroundColor Green



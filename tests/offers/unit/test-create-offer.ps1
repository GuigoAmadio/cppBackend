$ErrorActionPreference = 'Stop'

param([string]$LeilaoId)
if (-not $LeilaoId) { throw "Passe -LeilaoId <id>" }

Write-Host "[OFFERS][UNIT-ENDPOINT] Create" -ForegroundColor Cyan
$body = @{ user_id="freela-01"; restaurante_id="rest-0001"; leilao_id=$LeilaoId; valor_oferecido=130; valor_minimo_aceito=120; mensagem="posso" } | ConvertTo-Json
$resp = Invoke-RestMethod -Method Post -Uri http://localhost:8080/offers -ContentType 'application/json' -Body $body
if (-not $resp.message) { throw "Create offer falhou" }
Write-Host "OK - Offer criada" -ForegroundColor Green


